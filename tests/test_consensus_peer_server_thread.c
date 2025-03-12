#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/blockchain.h"
#include "include/linked_list.h"
#include "include/peer_discovery.h"
#include "include/networking.h"
#include "include/sleep.h"
#include "include/consensus_peer_server_thread.h"
#include "tests/test_consensus_peer_server_thread.h"
#include "tests/mocks.h"

void test_handle_one_consensus_request_receives_peer_blockchain() {
    wrap_connect = mock_connect;
    wrap_recv = mock_recv;
    wrap_send = mock_send;
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    blockchain_t *peer_blockchain = NULL;
    size_t num_zero_bytes = 4;
    return_code_t return_code = blockchain_create(
        &peer_blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
    return_code = block_create_genesis_block(&genesis_block);
    genesis_block->created_at = 100;
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(peer_blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_serialize(
        peer_blockchain,
        &command_send_blockchain.blockchain_data,
        &command_send_blockchain.blockchain_data_len);
    assert_true(SUCCESS == return_code);
    unsigned char *send_blockchain_buffer = NULL;
    uint64_t send_blockchain_buffer_len = 0;
    return_code = command_send_blockchain_serialize(
        &command_send_blockchain,
        &send_blockchain_buffer,
        &send_blockchain_buffer_len);
    assert_true(SUCCESS == return_code);
    will_return(mock_recv, send_blockchain_buffer);
    will_return(mock_recv, sizeof(command_header_t));
    will_return(
        mock_recv, send_blockchain_buffer + sizeof(command_header_t));
    will_return(
        mock_recv,
        send_blockchain_buffer_len - sizeof(command_header_t));
    will_return_always(mock_send, 1);
    int conn_fd = 99;
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes_required = 2;
    return_code = blockchain_create(&blockchain, num_zero_bytes_required);
    assert_true(SUCCESS == return_code);
    return_code = block_create_genesis_block(&genesis_block);
    genesis_block->created_at = 200;
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
    peer1->last_connected = time(NULL);
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = htons(23456);
    peer2->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer2->listen_addr.sin6_addr))[0] = 0xfe;
    ((unsigned char *)(&peer2->listen_addr.sin6_addr))[1] = 0x80;
    ((unsigned char *)(&peer2->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer2->listen_addr.sin6_scope_id = 0;
    peer2->last_connected = time(NULL);
    return_code = linked_list_prepend(peer_info_list, peer2);
    assert_true(SUCCESS == return_code);
    return_code = linked_list_prepend(peer_info_list, peer1);
    assert_true(SUCCESS == return_code);
    run_consensus_peer_server_args_t args = {0};
    args.consensus_peer_server_addr.sin6_family = AF_INET6;
    args.consensus_peer_server_addr.sin6_port = htons(55554);
    ((unsigned char *)(&args.consensus_peer_server_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    args.sync = sync;
    args.peer_info_list = peer_info_list;
    pthread_mutex_init(&args.peer_info_list_mutex, NULL);
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    return_code = handle_one_consensus_request(&args, conn_fd);
    assert_true(SUCCESS == return_code);
    block_t *updated_genesis_block =
        (block_t *)args.sync->blockchain->block_list->head->data;
    // No change in blockchain because the peer's blockchain was not bigger.
    assert_true(100 == updated_genesis_block->created_at);
    free(send_blockchain_buffer);
    blockchain_destroy(blockchain);
    free(command_send_blockchain.blockchain_data);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    pthread_mutex_destroy(&args.peer_info_list_mutex);
    linked_list_destroy(args.peer_info_list);
    synchronized_blockchain_destroy(args.sync);
}

void test_handle_one_consensus_request_switches_to_longest_chain() {
    // TODO
    assert_true(false);
}

void test_handle_one_consensus_request_rejects_invalid_chain() {
    // TODO
    assert_true(false);
}

void test_handle_one_consensus_request_sends_longest_chain() {
    // TODO not sure if our mocking allows us to test this well
    assert_true(false);
}

void test_run_consensus_peer_server_exits_when_should_stop_is_set() {
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes_required = 2;
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
    peer1->last_connected = time(NULL);
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = htons(23456);
    peer2->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer2->listen_addr.sin6_addr))[0] = 0xfe;
    ((unsigned char *)(&peer2->listen_addr.sin6_addr))[1] = 0x80;
    ((unsigned char *)(&peer2->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer2->listen_addr.sin6_scope_id = 0;
    peer2->last_connected = time(NULL);
    return_code = linked_list_prepend(peer_info_list, peer2);
    assert_true(SUCCESS == return_code);
    return_code = linked_list_prepend(peer_info_list, peer1);
    assert_true(SUCCESS == return_code);
    run_consensus_peer_server_args_t args = {0};
    args.consensus_peer_server_addr.sin6_family = AF_INET6;
    args.consensus_peer_server_addr.sin6_port = htons(55554);
    ((unsigned char *)(&args.consensus_peer_server_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    args.sync = sync;
    args.peer_info_list = peer_info_list;
    pthread_mutex_init(&args.peer_info_list_mutex, NULL);
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    pthread_t thread;
    pthread_create(
        &thread, NULL, run_consensus_peer_server_pthread_wrapper, &args);
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
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    pthread_mutex_destroy(&args.peer_info_list_mutex);
    linked_list_destroy(args.peer_info_list);
    synchronized_blockchain_destroy(args.sync);
}
