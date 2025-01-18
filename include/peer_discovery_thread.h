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

/**
 * @brief Contains the arguments to the discover_peers function.
 * 
 * This struct packages arguments so that users can call discover_peers in
 * pthread_create.
 * 
 * @param peer_discovery_bootstrap_server_addr The address to which to initially
 * connect to retrieve the list of peers.
 * @param peer_addr The listen address of the peer making this function call.
 * The function will register this address with the bootstrap server and other
 * peers. Peers will attempt to connect to this address to share newly mined
 * blocks and other messages.
 * @param communication_interval_seconds The number of seconds between attempts
 * to connect to the server to keep alive peer_addr as an active entry in the
 * server's list of peers. Callers should choose this value based on how long
 * the server keeps peer addresses active. A sensible choice is one third of the
 * server's keep alive time.
 * @param peer_info_list The list of peers that this peer is aware of. Each
 * entry is a peer_info_t struct. This function communicates with the bootstrap
 * server and other peers to maintain the list of active peers. Other threads
 * use this information, for instance to determine who should receive broadcasts
 * of newly mined blocks. When calling this function, the list may be empty.
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
typedef struct discover_peers_args_t {
    struct sockaddr_in6 peer_discovery_bootstrap_server_addr;
    struct sockaddr_in6 peer_addr;
    uint64_t communication_interval_seconds;
    linked_list_t *peer_info_list;
    pthread_mutex_t peer_info_list_mutex;
    bool print_progress;
    atomic_bool *should_stop;
    bool *exit_ready;
    pthread_cond_t exit_ready_cond;
    pthread_mutex_t exit_ready_mutex;
} discover_peers_args_t;

/**
 * @brief Maintains the list of active peers until interrupted.
 * 
 * @param args Contains the function arguments. See discover_peers_args_t for
 * details
 * @return return_code_t A pointer to a return code indicating success or
 * failure. Callers must free.
 */
return_code_t *discover_peers(discover_peers_args_t *args);

/**
 * @brief Calls discover_peers on args; eliminates warnings in pthread_create.
 */
void *discover_peers_pthread_wrapper(void *args);

#endif  // INCLUDE_PEER_DISCOVERY_THREAD_H_
