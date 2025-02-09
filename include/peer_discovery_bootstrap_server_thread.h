/**
 * @brief Contains functions for the peer discovery bootstrap server thread.
 */

// TODO docstrings

#ifndef INCLUDE_PEER_DISCOVERY_BOOTSTRAP_SERVER_THREAD_H_
#define INCLUDE_PEER_DISCOVERY_BOOTSTRAP_SERVER_THREAD_H_

#include <pthread.h>
#include <stdatomic.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif
#include "include/return_codes.h"
#include "include/linked_list.h"

typedef struct accept_peer_discovery_requests_args_t {
    struct sockaddr_in6 peer_discovery_bootstrap_server_addr;
    uint64_t peer_keepalive_microseconds;
    linked_list_t *peer_info_list;
    pthread_mutex_t peer_info_list_mutex;
    bool print_progress;
    atomic_bool *should_stop;
    bool *exit_ready;
    pthread_cond_t exit_ready_cond;
    pthread_mutex_t exit_ready_mutex;
} accept_peer_discovery_requests_args_t;

return_code_t accept_peer_discovery_requests_once(accept_peer_discovery_requests_args_t *args);

return_code_t accept_peer_discovery_requests(accept_peer_discovery_requests_args_t *args);

void *accept_peer_discovery_requests_pthread_wrapper(void *args);

#endif  // INCLUDE_PEER_DISCOVERY_BOOTSTRAP_SERVER_THREAD_H_
