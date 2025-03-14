// TODO docstrings

// TODO iterate through peer list exchanging longest chains.
#ifndef INCLUDE_CONSENSUS_PEER_CLIENT_THREAD_H_
#define INCLUDE_CONSENSUS_PEER_CLIENT_THREAD_H_
#include <pthread.h>
#include <stdatomic.h>
#include "include/return_codes.h"
#include "include/linked_list.h"
#include "include/blockchain.h"

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

return_code_t *run_consensus_peer_client(
    run_consensus_peer_client_args_t *args);

void *run_consensus_peer_client_pthread_wrapper(void *args);

#endif  // INCLUDE_CONSENSUS_PEER_CLIENT_THREAD_H_
