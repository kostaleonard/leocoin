#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/blockchain.h"
#include "include/networking.h"
#include "include/sleep.h"
#include "include/peer_discovery.h"
#include "include/consensus_peer_client_thread.h"
#include "tests/test_consensus_peer_client_thread.h"

void test_run_consensus_peer_client_exits_when_should_stop_is_set() {
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes_required = 3;
    return_code_t return_code = blockchain_create(
        &blockchain, num_zero_bytes_required);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    synchronized_blockchain_t *sync = NULL;
    return_code = synchronized_blockchain_create(&sync, blockchain);
    assert_true(SUCCESS == return_code);
    linked_list_t *peer_info_list = NULL;
    return_code = linked_list_create(
        &peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    peer_info_t *peer1 = calloc(1, sizeof(peer_info_t));
    peer1->listen_addr.sin6_family = AF_INET6;
    peer1->listen_addr.sin6_port = htons(12345);
    peer1->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer1->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer1->listen_addr.sin6_scope_id = 0;
    peer1->last_connected = 100;
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = htons(23456);
    peer2->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer2->listen_addr.sin6_addr))[0] = 0xfe;
    ((unsigned char *)(&peer2->listen_addr.sin6_addr))[1] = 0x80;
    ((unsigned char *)(&peer2->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer2->listen_addr.sin6_scope_id = 0;
    peer2->last_connected = 200;
    return_code = linked_list_prepend(peer_info_list, peer2);
    assert_true(SUCCESS == return_code);
    return_code = linked_list_prepend(peer_info_list, peer1);
    assert_true(SUCCESS == return_code);
    run_consensus_peer_client_args_t args = {0};
    args.sync = sync;
    args.peer_info_list = peer_info_list;
    pthread_mutex_init(&args.peer_info_list_mutex, NULL);
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = true; // TODO try setting to false and using a circularly linked peer list
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    pthread_t thread;
    pthread_create(
        &thread, NULL, run_consensus_peer_client_pthread_wrapper, &args);
    // Pause for a short period to allow the thread to start.
    sleep_microseconds(100000);
    *args.should_stop = true;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    // One second timeout.
    ts.tv_sec += 1;
    pthread_mutex_lock(&args.exit_ready_mutex);
    while (!*args.exit_ready) {
        int result = pthread_cond_timedwait(
            &args.exit_ready_cond, &args.exit_ready_mutex, &ts);
        if (ETIMEDOUT == result) {
            assert_true(false);
        }
    }
    pthread_mutex_unlock(&args.exit_ready_mutex);
    void *retval = NULL;
    pthread_join(thread, &retval);
    return_code_t *return_code_ptr = (return_code_t *)retval;
    assert_true(NULL != return_code_ptr);
    free(return_code_ptr);
    pthread_mutex_destroy(&args.peer_info_list_mutex);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    synchronized_blockchain_destroy(args.sync);
}
