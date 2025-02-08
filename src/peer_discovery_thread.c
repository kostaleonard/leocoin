#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/networking.h"
#include "include/peer_discovery.h"
#include "include/peer_discovery_thread.h"
#include "include/sleep.h"

// TODO go through the logic in this function one more time--I think it's almost there
return_code_t discover_peers_once(discover_peers_args_t *args) {
    return_code_t return_code = SUCCESS;
    if (args->print_progress) {
        printf("Attempting to connect to peer discovery server.\n");
    }
    int client_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (client_fd < 0) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    if (SUCCESS != wrap_connect(
        client_fd,
        (struct sockaddr *)&args->peer_discovery_bootstrap_server_addr,
        sizeof(struct sockaddr_in6))) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    if (args->print_progress) {
        printf("Connected.\n");
    }
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_register_peer_t command_register_peer = {0};
    command_register_peer.header = command_header;
    command_register_peer.sin6_family = args->peer_addr.sin6_family;
    command_register_peer.sin6_port = args->peer_addr.sin6_port;
    command_register_peer.sin6_flowinfo = args->peer_addr.sin6_flowinfo;
    memcpy(
        &command_register_peer.addr,
        &args->peer_addr.sin6_addr,
        sizeof(IN6_ADDR));
    command_register_peer.sin6_scope_id = args->peer_addr.sin6_scope_id;
    unsigned char *send_buf = NULL;
    uint64_t send_buf_size = 0;
    return_code = command_register_peer_serialize(
        &command_register_peer, &send_buf, &send_buf_size);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = send_all(client_fd, (char *)send_buf, send_buf_size, 0);
    free(send_buf);
    if (SUCCESS != return_code) {
        goto end;
    }
    char recv_buf[BUFSIZ] = {0};
    return_code = recv_all(client_fd, recv_buf, sizeof(command_header_t), 0);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = command_header_deserialize(
        &command_header, (unsigned char *)recv_buf, sizeof(command_header_t));
    if (SUCCESS != return_code) {
        goto end;
    }
    if (COMMAND_SEND_PEER_LIST != command_header.command) {
        return_code = FAILURE_INVALID_COMMAND;
        goto end;
    }
    // TODO buffer overflow potential? need to malloc recv_buf, and maybe have a max size for peer list?
    return_code = recv_all(
        client_fd,
        recv_buf + sizeof(command_header_t),
        command_header.command_len,
        0);
    if (SUCCESS != return_code) {
        goto end;
    }
    command_send_peer_list_t command_send_peer_list = {0};
    return_code = command_send_peer_list_deserialize(
        &command_send_peer_list,
        (unsigned char *)recv_buf,
        sizeof(command_header_t) + command_header.command_len);
    if (SUCCESS != return_code) {
        goto end;
    }
    if (0 != pthread_mutex_lock(&args->peer_info_list_mutex)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    return_code = peer_info_list_deserialize(
        &args->peer_info_list,
        command_send_peer_list.peer_list_data,
        command_send_peer_list.peer_list_data_len);
    if (0 != pthread_mutex_unlock(&args->peer_info_list_mutex)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    if (SUCCESS != return_code) {
        goto end;
    }
    if (args->print_progress) {
        if (0 != pthread_mutex_lock(&args->peer_info_list_mutex)) {
            return_code = FAILURE_PTHREAD_FUNCTION;
            goto end;
        }
        for (node_t *node = args->peer_info_list->head;
            NULL != node;
            node = node->next) {
            peer_info_t *peer = (peer_info_t *)node->data;
            char ip_str[INET6_ADDRSTRLEN];
            if (NULL == inet_ntop(
                AF_INET6,
                &peer->listen_addr.sin6_addr,
                ip_str,
                sizeof(ip_str))) {
                perror("inet_ntop");
            }
            int port = ntohs(peer->listen_addr.sin6_port);
            printf("IPv6 Address: %s\n", ip_str);
            printf("Port: %d\n", port);
        }
        if (0 != pthread_mutex_unlock(&args->peer_info_list_mutex)) {
            return_code = FAILURE_PTHREAD_FUNCTION;
            goto end;
        }
    }
    #ifdef _WIN32
        closesocket(client_fd);
    # else
        close(client_fd);
    #endif
end:
    return return_code;
}

return_code_t *discover_peers(discover_peers_args_t *args) {
    return_code_t return_code = SUCCESS;
    bool should_stop = *args->should_stop;
    while (!should_stop) {
        return_code = discover_peers_once(args);
        if (SUCCESS != return_code && args->print_progress) {
            printf("Error in peer discovery; retrying\n");
        }
        sleep_microseconds(args->communication_interval_microseconds);
        should_stop = *args->should_stop;
    }
    if (should_stop) {
        if (args->print_progress) {
            printf("Stopping peer discovery.\n");
        }
    }
    pthread_mutex_lock(&args->exit_ready_mutex);
    *args->exit_ready = true;
    pthread_cond_signal(&args->exit_ready_cond);
    pthread_mutex_unlock(&args->exit_ready_mutex);
    return_code_t *return_code_ptr = malloc(sizeof(return_code_t));
    *return_code_ptr = return_code;
    return return_code_ptr;
}

void *discover_peers_pthread_wrapper(void *args) {
    discover_peers_args_t *discover_peers_args = (discover_peers_args_t *)args;
    return_code_t *return_code_ptr = discover_peers(discover_peers_args);
    return (void *)return_code_ptr;
}
