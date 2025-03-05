/**
 * @brief Contains functions for the peer discovery bootstrap server thread.
 */

#ifndef INCLUDE_PEER_DISCOVERY_BOOTSTRAP_SERVER_THREAD_H_
#define INCLUDE_PEER_DISCOVERY_BOOTSTRAP_SERVER_THREAD_H_
#define DEFAULT_PEER_KEEPALIVE_SECONDS 60
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
    #include <poll.h>
#endif
#include "include/return_codes.h"
#include "include/linked_list.h"

/**
 * @brief Contains the arguments to the handle_peer_discovery_requests function.
 * 
 * This struct packages arguments so that users can call
 * handle_peer_discovery_requests in pthread_create.
 * 
 * @param peer_discovery_bootstrap_server_addr The address on which to listen
 * for peer connections.
 * @param peer_keepalive_microseconds If a peer has not connected in this
 * interval, the server removes it from the list of active peers.
 * @param peer_info_list The list of peers that this server is aware of. Each
 * entry is a peer_info_t struct.
 * @param peer_info_list_mutex Protects peer_info_list.
 * @param print_progress If true, display progress on the screen.
 * @param should_stop This should initially be false. Setting this flag while
 * the function is running requests that the function terminate gracefully.
 * Users should expect the function to terminate in a timely manner (on the
 * order of seconds), but not necessarily immediately.
 * @param exit_ready This should initially be false. The function will set this
 * shared flag when it is about to exit. This flag allows users to wait for
 * the function with a timeout.
 * @param exit_ready_cond The function will monitor should_stop and attempt to
 * gracefully shutdown when it is set. Just before the function returns, it sets
 * exit_ready and signals this condition variable. Callers can use
 * pthread_cond_timedwait on the condition variable to attempt to join the
 * thread with a timeout. So, if the function does not signal the condition
 * variable in a timely manner, callers can assume that something has gone wrong
 * and the thread will not join.
 * @param exit_ready_mutex Protects exit_ready and exit_ready_cond.
 */
typedef struct handle_peer_discovery_requests_args_t {
    struct sockaddr_in6 peer_discovery_bootstrap_server_addr;
    uint64_t peer_keepalive_microseconds;
    linked_list_t *peer_info_list;
    pthread_mutex_t peer_info_list_mutex;
    bool print_progress;
    atomic_bool *should_stop;
    bool *exit_ready;
    pthread_cond_t exit_ready_cond;
    pthread_mutex_t exit_ready_mutex;
} handle_peer_discovery_requests_args_t;

/**
 * @brief Receives one register peer command and sends back the peer list.
 * 
 * @param conn_fd The open socket with the connected peer.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t handle_one_peer_discovery_request(
    handle_peer_discovery_requests_args_t *args, int conn_fd);

/**
 * @brief Maintains the list of active peers until interrupted.
 * 
 * @return return_code_t A pointer to a return code indicating success or
 * failure. Callers must free.
 */
return_code_t *handle_peer_discovery_requests(
    handle_peer_discovery_requests_args_t *args);

/**
 * @brief Calls handle_peer_discovery_requests on args.
 */
void *handle_peer_discovery_requests_pthread_wrapper(void *args);

#endif  // INCLUDE_PEER_DISCOVERY_BOOTSTRAP_SERVER_THREAD_H_
