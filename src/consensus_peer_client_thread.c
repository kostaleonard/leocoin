#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/networking.h"
#include "include/peer_discovery.h"
#include "include/consensus_peer_client_thread.h"

return_code_t run_consensus_peer_client_once(
    run_consensus_peer_client_args_t *args, peer_info_t *peer) {
    return_code_t return_code = SUCCESS;
    if (args->print_progress) {
        printf("Attempting to connect to peer consensus server.\n");
    }
    int client_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (client_fd < 0) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    if (SUCCESS != wrap_connect(
        client_fd,
        (struct sockaddr *)&peer->listen_addr,
        sizeof(struct sockaddr_in6))) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    if (args->print_progress) {
        printf("Connected.\n");
    }
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    return_code = pthread_mutex_lock(&args->sync->mutex);
    if (SUCCESS != return_code) {
        goto end;
    }
    blockchain_t *our_blockchain = args->sync->blockchain;
    return_code = blockchain_serialize(
        our_blockchain,
        &command_send_blockchain.blockchain_data,
        &command_send_blockchain.blockchain_data_len);
    if (SUCCESS != return_code) {
        pthread_mutex_unlock(&args->sync->mutex);
        goto end;
    }
    return_code = pthread_mutex_unlock(&args->sync->mutex);
    if (SUCCESS != return_code) {
        goto end;
    }
    unsigned char *send_buf = NULL;
    uint64_t send_buf_len = 0;
    return_code = command_send_blockchain_serialize(
        &command_send_blockchain, &send_buf, &send_buf_len);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = send_all(client_fd, send_buf, send_buf_len, 0);
    if (SUCCESS != return_code) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    free(command_send_blockchain.blockchain_data);
    char *recv_buf = calloc(sizeof(command_header_t), 1);
    if (NULL == recv_buf) {
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    return_code = recv_all(client_fd, recv_buf, sizeof(command_header_t), 0);
    if (SUCCESS != return_code) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    return_code = command_header_deserialize(
        &command_header, (unsigned char *)recv_buf, sizeof(command_header_t));
    if (SUCCESS != return_code) {
        goto end;
    }
    if (COMMAND_SEND_BLOCKCHAIN != command_header.command) {
        return_code = FAILURE_INVALID_COMMAND;
        goto end;
    }
    recv_buf = realloc(
        recv_buf, sizeof(command_header_t) + command_header.command_len);
    return_code = recv_all(
        client_fd,
        recv_buf + sizeof(command_header_t),
        command_header.command_len,
        0);
    if (SUCCESS != return_code) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    return_code = command_send_blockchain_deserialize(
        &command_send_blockchain,
        (unsigned char *)recv_buf,
        sizeof(command_header_t) + command_header.command_len);
    if (SUCCESS != return_code) {
        goto end;
    }
    blockchain_t *peer_blockchain = NULL;
    return_code = blockchain_deserialize(
        &peer_blockchain,
        command_send_blockchain.blockchain_data,
        command_send_blockchain.blockchain_data_len);
    if (SUCCESS != return_code) {
        goto end;
    }
    bool peer_blockchain_is_valid = false;
    return_code = blockchain_verify(
        peer_blockchain, &peer_blockchain_is_valid, NULL);
    if (SUCCESS != return_code) {
        goto end;
    }
    if (!peer_blockchain_is_valid && args->print_progress) {
        printf("Client received invalid blockchain\n");
    }
    uint64_t peer_blockchain_length = 0;
    return_code = linked_list_length(
        peer_blockchain->block_list, &peer_blockchain_length);
    if (SUCCESS != return_code) {
        goto end;
    }
    bool switched_to_peer_chain = false;
    if (peer_blockchain_is_valid) {
        return_code = pthread_mutex_lock(&args->sync->mutex);
        if (SUCCESS != return_code) {
            goto end;
        }
        // Read the pointer again just in case something changed.
        our_blockchain = args->sync->blockchain;
        uint64_t our_blockchain_length = 0;
        return_code = linked_list_length(
            our_blockchain->block_list, &our_blockchain_length);
        if (SUCCESS != return_code) {
            pthread_mutex_unlock(&args->sync->mutex);
            goto end;
        }
        bool same_number_leading_zeros =
            our_blockchain->num_leading_zero_bytes_required_in_block_hash ==
            peer_blockchain->num_leading_zero_bytes_required_in_block_hash;
        if (peer_blockchain_length > our_blockchain_length &&
            same_number_leading_zeros) {
            args->sync->blockchain = peer_blockchain;
            atomic_fetch_add(&args->sync->version, 1);
            switched_to_peer_chain = true;
            return_code = blockchain_destroy(our_blockchain);
            if (SUCCESS != return_code) {
                pthread_mutex_unlock(&args->sync->mutex);
                goto end;
            }
            if (args->print_progress) {
                printf(
                    "Client switched to longer blockchain: "
                    "%"PRIu64" -> %"PRIu64"\n",
                    our_blockchain_length,
                    peer_blockchain_length);
            }
        } else if (args->print_progress) {
            printf("Client did not switch blockchain\n");
        }
        return_code = pthread_mutex_unlock(&args->sync->mutex);
        if (SUCCESS != return_code) {
            goto end;
        }
    }
    if (!switched_to_peer_chain) {
        return_code = blockchain_destroy(peer_blockchain);
        if (SUCCESS != return_code) {
            goto end;
        }
    }
    free(command_send_blockchain.blockchain_data);
    free(send_buf);
    free(recv_buf);
    #ifdef _WIN32
        closesocket(client_fd);
    # else
        close(client_fd);
    #endif
end:
    return return_code;
}

return_code_t *run_consensus_peer_client(
    run_consensus_peer_client_args_t *args) {
    return_code_t return_code = SUCCESS;
    linked_list_t *peer_info_list_copy = NULL;
    return_code = linked_list_create(
        &peer_info_list_copy, free, compare_peer_info_t);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = pthread_mutex_lock(&args->peer_info_list_mutex);
    if (SUCCESS != return_code) {
        linked_list_destroy(peer_info_list_copy);
        goto end;
    }
    for (node_t *node = args->peer_info_list->head;
        NULL != node;
        node = node->next) {
        peer_info_t *peer = (peer_info_t *)node->data;
        peer_info_t *peer_copy = calloc(1, sizeof(peer_info_t));
        if (NULL == peer_copy) {
            linked_list_destroy(peer_info_list_copy);
            pthread_mutex_unlock(&args->peer_info_list_mutex);
            goto end;
        }
        memcpy(peer_copy, peer, sizeof(peer_info_t));
        // This will copy the peers in reverse, but order does not matter here.
        return_code = linked_list_prepend(peer_info_list_copy, peer_copy);
        if (SUCCESS != return_code) {
            free(peer_copy);
            linked_list_destroy(peer_info_list_copy);
            pthread_mutex_unlock(&args->peer_info_list_mutex);
            goto end;
        }
    }
    return_code = pthread_mutex_unlock(&args->peer_info_list_mutex);
    if (SUCCESS != return_code) {
        linked_list_destroy(peer_info_list_copy);
        goto end;
    }
    node_t *node = peer_info_list_copy->head;
    bool should_stop = *args->should_stop;
    while (!should_stop && NULL != node) {
        peer_info_t *peer = (peer_info_t *)node->data;
        return_code = run_consensus_peer_client_once(args, peer);
        if (SUCCESS != return_code && args->print_progress) {
            printf("Error exchanging blockchains with peer; continuing\n");
        }
        should_stop = *args->should_stop;
        node = node->next;
    }
    if (args->print_progress) {
        printf("Finished exchanging blockchains with peers.\n");
    }
    return_code = linked_list_destroy(peer_info_list_copy);
end:
    pthread_mutex_lock(&args->exit_ready_mutex);
    *args->exit_ready = true;
    pthread_cond_signal(&args->exit_ready_cond);
    pthread_mutex_unlock(&args->exit_ready_mutex);
    return_code_t *return_code_ptr = malloc(sizeof(return_code_t));
    *return_code_ptr = return_code;
    return return_code_ptr;
}

void *run_consensus_peer_client_pthread_wrapper(void *args) {
    run_consensus_peer_client_args_t *client_args =
        (run_consensus_peer_client_args_t *)args;
    return_code_t *return_code_ptr = run_consensus_peer_client(client_args);
    return (void *)return_code_ptr;
}
