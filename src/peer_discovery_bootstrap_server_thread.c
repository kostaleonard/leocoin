
#include "include/peer_discovery_bootstrap_server_thread.h"

return_code_t accept_peer_discovery_requests_once(accept_peer_discovery_requests_args_t *args) {
    return FAILURE_INVALID_INPUT;
}

return_code_t accept_peer_discovery_requests(accept_peer_discovery_requests_args_t *args) {
    return FAILURE_INVALID_INPUT;
}

void *accept_peer_discovery_requests_pthread_wrapper(void *args) {
    accept_peer_discovery_requests_args_t *server_args = (accept_peer_discovery_requests_args_t *)args;
    return_code_t *return_code_ptr = accept_peer_discovery_requests(server_args);
    return (void *)return_code_ptr;
}
