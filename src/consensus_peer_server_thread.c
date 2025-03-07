#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/networking.h"
#include "include/consensus_peer_server_thread.h"

return_code_t handle_one_consensus_request(run_consensus_peer_server_args_t *args, int conn_fd) {
    // TODO
    return FAILURE_INVALID_INPUT;
}
 
return_code_t *run_consensus_peer_server(
    run_consensus_peer_server_args_t *args) {
    // TOOD
    return NULL;
}

void *run_consensus_peer_server_pthread_wrapper(void *args) {
    run_consensus_peer_server_args_t *server_args =
        (run_consensus_peer_server_args_t *)args;
    return_code_t *return_code_ptr = run_consensus_peer_server(server_args);
    return (void *)return_code_ptr;
}
