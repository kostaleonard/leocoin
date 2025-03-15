#include "include/consensus_peer_client_thread.h"

return_code_t *run_consensus_peer_client(
    run_consensus_peer_client_args_t *args) {
    // TODO
    return NULL;
}

void *run_consensus_peer_client_pthread_wrapper(void *args) {
    run_consensus_peer_client_args_t *client_args =
        (run_consensus_peer_client_args_t *)args;
    return_code_t *return_code_ptr = run_consensus_peer_client(client_args);
    return (void *)return_code_ptr;
}
