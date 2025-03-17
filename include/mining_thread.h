/**
 * @brief Contains functions for the mining thread.
 */

#ifndef INCLUDE_MINING_THREAD_H_
#define INCLUDE_MINING_THREAD_H_

#include "include/return_codes.h"
#include "include/blockchain.h"
#include "include/cryptography.h"

/**
 * @brief Contains the arguments to the mine_blocks function.
 * 
 * This struct packages arguments so that users can call mine_blocks in
 * pthread_create.
 * 
 * @param sync A synchronized blockchain on which to mine. This function will
 * generate new blocks on this blockchain. Periodically, this function will
 * check the version number to see if the networking thread has received a
 * longer blockchain. If so, this function will begin mining on the new
 * blockchain and free all memory associated with the old blockchain.
 * @param miner_public_key The public key with which to mine blocks. This
 * function uses it to create the minting transaction.
 * @param miner_private_key The private key with which to mine blocks. This
 * function uses it to digitally sign the minting transaction.
 * @param peer_info_list The list of peers that this client is aware of. Each
 * entry is a peer_info_t struct.
 * @param peer_info_list_mutex Protects peer_info_list.
 * @param print_progress If true, display progress on the screen.
 * @param outfile If not NULL, this function will save the blockchain to this
 * filename every time it mines a new block. If NULL, this function will only
 * keep the blockchain in memory. Unless you are just testing, you should
 * provide this argument. Otherwise there is no local record of your mining and
 * you may lose all the coin you have mined thus far.
 * @param should_stop This should initially be false. Setting this flag while
 * the function is running requests that the function terminate gracefully.
 * Users should expect the function to terminate in a timely manner (on the
 * order of seconds), but not necessarily immediately.
 * @param exit_ready This should initially be false. mine_blocks will set this
 * shared flag when it is about to exit. This flag allows users to wait for
 * mine_blocks with a timeout.
 * @param exit_ready_cond mine_blocks will monitor should_stop and attempt to
 * gracefully shutdown when it is set. Just before the function returns, it sets
 * exit_ready and signals this condition variable. Callers can use
 * pthread_cond_timedwait on the condition variable to attempt to join the miner
 * thread with a timeout. So, if the function does not signal the condition
 * variable in a timely manner, callers can assume that something has gone wrong
 * and the miner thread will not join.
 * @param exit_ready_mutex Protects exit_ready and exit_ready_cond.
 * @param sync_version_currently_mined Contains the version number of the
 * synchronized blockchain that mine_blocks is currently mining. When the user
 * updates the version number in the sync parameter, mine_blocks knows to update
 * the blockchain that it is currently mining. When it makes the switch, it
 * updates this number and signals the condition variable.
 * @param sync_version_currently_mined_cond mine_blocks signals this condition
 * variable when it switches the blockchain that it is mining. Callers can use
 * pthread_cond_timedwait on the condition variable to ensure that the switch
 * has occurred in a timely manner.
 * @param sync_version_currently_mined_mutex Protects
 * sync_version_currently_mined and sync_version_currently_mined_cond.
 */
typedef struct mine_blocks_args_t {
    synchronized_blockchain_t *sync;
    ssh_key_t *miner_public_key;
    ssh_key_t *miner_private_key;
    linked_list_t **peer_info_list;
    pthread_mutex_t *peer_info_list_mutex;
    bool print_progress;
    char *outfile;
    atomic_bool *should_stop;
    bool *exit_ready;
    pthread_cond_t exit_ready_cond;
    pthread_mutex_t exit_ready_mutex;
    atomic_size_t *sync_version_currently_mined;
    pthread_cond_t sync_version_currently_mined_cond;
    pthread_mutex_t sync_version_currently_mined_mutex;
} mine_blocks_args_t;

/**
 * @brief Mines blocks continuously until interrupted.
 * 
 * @param args Contains the function arguments. See mine_blocks_args_t for
 * details.
 * @return return_code_t A pointer to a return code indicating success or
 * failure. Callers must free.
 */
return_code_t *mine_blocks(mine_blocks_args_t *args);

/**
 * @brief Calls mine_blocks on args; eliminates type warnings in pthread_create.
 */
void *mine_blocks_pthread_wrapper(void *args);

#endif  // INCLUDE_MINING_THREAD_H_
