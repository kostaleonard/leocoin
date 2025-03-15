#include <stdio.h>
#include "include/peer_discovery.h"
#include "include/consensus_peer_client_thread.h"

return_code_t run_consensus_peer_client_once(
    run_consensus_peer_client_args_t *args, peer_info_t *peer) {
    return_code_t return_code = SUCCESS;
    // TODO
    return return_code;
}

return_code_t *run_consensus_peer_client(
    run_consensus_peer_client_args_t *args) {
    return_code_t return_code = SUCCESS;
    // TODO lock, make copy of peer list, unlock
    // TODO iterate over peer list
    // TODO connect to peer
    // TODO send and receive blockchain
    // TODO update longest chain
    linked_list_t *peer_info_list_copy = args->peer_info_list;
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
// TODO end:
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
