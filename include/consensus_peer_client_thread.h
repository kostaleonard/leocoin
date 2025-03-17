/**
 * @brief Contains functions for the consensus peer client.
 * 
 * The consensus peer client exchanges blockchains with peers.
 */

#ifndef INCLUDE_CONSENSUS_PEER_CLIENT_THREAD_H_
#define INCLUDE_CONSENSUS_PEER_CLIENT_THREAD_H_
#include <pthread.h>
#include <stdatomic.h>
#include "include/peer_discovery.h"
#include "include/return_codes.h"
#include "include/linked_list.h"
#include "include/blockchain.h"

/**
 * @brief Contains the arguments to the run_consensus_peer_client function.
 * 
 * This struct packages arguments so that users can call
 * run_consensus_peer_client in pthread_create.
 * 
 * @param sync A reference to the synchronized blockchain. The client will
 * update the synchronized blockchain whenever it receives a longer chain that
 * is valid and has the same number of leading zeros required.
 * @param peer_info_list The list of peers that this client is aware of. Each
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
typedef struct run_consensus_peer_client_args_t {
    synchronized_blockchain_t *sync;
    linked_list_t *peer_info_list;
    pthread_mutex_t peer_info_list_mutex;
    bool print_progress;
    atomic_bool *should_stop;
    bool *exit_ready;
    pthread_cond_t exit_ready_cond;
    pthread_mutex_t exit_ready_mutex;
} run_consensus_peer_client_args_t;

/**
 * @brief Sends one send blockchain command to a peer.
 * 
 * @param peer The peer to which to connect.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t run_consensus_peer_client_once(
    run_consensus_peer_client_args_t *args, peer_info_t *peer);

/**
 * @brief Exchanges blockchains with all peers in the peer list, then exits.
 * 
 * @return return_code_t A pointer to a return code indicating success or
 * failure. Callers must free.
 */
return_code_t *run_consensus_peer_client(
    run_consensus_peer_client_args_t *args);

/**
 * @brief Calls run_consensus_peer_client on args.
 */
void *run_consensus_peer_client_pthread_wrapper(void *args);

#endif  // INCLUDE_CONSENSUS_PEER_CLIENT_THREAD_H_
