/**
 * @brief Contains functions for the peer discovery thread.
 */

#ifndef INCLUDE_PEER_DISCOVERY_THREAD_H_
#define INCLUDE_PEER_DISCOVERY_THREAD_H_
#define DEFAULT_COMMUNICATION_INTERVAL_SECONDS 20

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

// TODO docstring. Look at mining_thread.h, because the synchronization stuff is the same
typedef struct discover_peers_args_t {
    struct sockaddr_in6 peer_discovery_bootstrap_server_addr;
    uint64_t communication_interval_seconds;
    linked_list_t *peer_info_list; // TODO shared between threads. Initially empty.
    pthread_mutex_t peer_info_list_mutex;
    bool print_progress;
    atomic_bool *should_stop;
    bool *exit_ready;
    pthread_cond_t exit_ready_cond;
    pthread_mutex_t exit_ready_mutex;
} discover_peers_args_t;

// TODO docstrings
return_code_t *discover_peers(discover_peers_args_t *args);

void *discover_peers_pthread_wrapper(void *args);

#endif  // INCLUDE_PEER_DISCOVERY_THREAD_H_
