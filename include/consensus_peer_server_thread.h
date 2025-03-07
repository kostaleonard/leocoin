// TODO docstring

//TODO consensus peer has two threads:
//Server thread: waits for connections to (1) receive transactions, or (2) receive new blocks.
//Client thread: broadcasts new blocks to peers


#ifndef INCLUDE_CONSENSUS_PEER_SERVER_H_
#define INCLUDE_CONSENSUS_PEER_SERVER_H_
#include <pthread.h>
#include <stdatomic.h>
#include "include/return_codes.h"
#include "include/linked_list.h"
#include "include/blockchain.h"

typedef struct run_consensus_peer_server_args_t {
    synchronized_blockchain_t *sync;
    linked_list_t *peer_info_list;
    pthread_mutex_t peer_info_list_mutex;
    bool print_progress;
    atomic_bool *should_stop;
    bool *exit_ready;
    pthread_cond_t exit_ready_cond;
    pthread_mutex_t exit_ready_mutex;
} run_consensus_peer_server_args_t;

// TODO helper function for receiving transaction

// TODO helper function for receiving new blocks

return_code_t handle_one_consensus_request(run_consensus_peer_server_args_t *args, int conn_fd);
 
return_code_t *run_consensus_peer_server(
    run_consensus_peer_server_args_t *args);

void *run_consensus_peer_server_pthread_wrapper(void *args);

#endif  // INCLUDE_CONSENSUS_PEER_SERVER_H_
 