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
#include "tests/mocks.h"
#include "tests/file_paths.h"

void test_run_consensus_peer_client_once_receives_peer_blockchain() {
    wrap_connect = mock_connect;
    wrap_recv = mock_recv;
    wrap_send = mock_send;
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    blockchain_t *peer_blockchain = NULL;
    size_t num_zero_bytes = 3;
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
    blockchain_t *blockchain = NULL;
    return_code = blockchain_create(&blockchain, num_zero_bytes);
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
    args.peer_info_list = &peer_info_list;
    pthread_mutex_t peer_info_list_mutex;
    pthread_mutex_init(&peer_info_list_mutex, NULL);
    args.peer_info_list_mutex = &peer_info_list_mutex;
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    return_code = run_consensus_peer_client_once(&args, peer1);
    assert_true(SUCCESS == return_code);
    block_t *updated_genesis_block =
        (block_t *)args.sync->blockchain->block_list->head->data;
    // No change in blockchain because the peer's blockchain was not bigger.
    assert_true(200 == updated_genesis_block->created_at);
    blockchain_destroy(peer_blockchain);
    free(send_blockchain_buffer);
    free(command_send_blockchain.blockchain_data);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    synchronized_blockchain_destroy(args.sync);
    linked_list_destroy(peer_info_list);
}

void test_run_consensus_peer_client_once_switches_to_longest_chain() {
    wrap_connect = mock_connect;
    wrap_recv = mock_recv;
    wrap_send = mock_send;
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    char fixture_directory[TESTS_MAX_PATH];
    get_fixture_directory(fixture_directory);
    char infile[TESTS_MAX_PATH];
    int return_value = snprintf(
        infile,
        TESTS_MAX_PATH,
        "%s/%s",
        fixture_directory,
        "blockchain_4_blocks_no_transactions");
    assert_true(return_value < TESTS_MAX_PATH);
    blockchain_t *peer_blockchain = NULL;
    return_code_t return_code = blockchain_read_from_file(
        &peer_blockchain, infile);
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
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes = 3;
    return_code = blockchain_create(&blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
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
    args.peer_info_list = &peer_info_list;
    pthread_mutex_t peer_info_list_mutex;
    pthread_mutex_init(&peer_info_list_mutex, NULL);
    args.peer_info_list_mutex = &peer_info_list_mutex;
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    atomic_size_t original_sync_version = atomic_load(&sync->version);
    return_code = run_consensus_peer_client_once(&args, peer1);
    assert_true(SUCCESS == return_code);
    block_t *updated_genesis_block =
        (block_t *)args.sync->blockchain->block_list->head->data;
    // Switched to peer blockchain.
    assert_true(200 != updated_genesis_block->created_at);
    uint64_t new_blockchain_len = 0;
    return_code = linked_list_length(
        args.sync->blockchain->block_list, &new_blockchain_len);
    assert_true(SUCCESS == return_code);
    assert_true(4 == new_blockchain_len);
    atomic_size_t new_sync_version = atomic_load(&sync->version);
    assert_true(new_sync_version > original_sync_version);
    blockchain_destroy(blockchain);
    blockchain_destroy(peer_blockchain);
    free(send_blockchain_buffer);
    free(command_send_blockchain.blockchain_data);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    synchronized_blockchain_destroy(args.sync);
    linked_list_destroy(peer_info_list);
}

void test_run_consensus_peer_client_once_rejects_invalid_chain() {
    wrap_connect = mock_connect;
    wrap_recv = mock_recv;
    wrap_send = mock_send;
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    char fixture_directory[TESTS_MAX_PATH];
    get_fixture_directory(fixture_directory);
    char infile[TESTS_MAX_PATH];
    int return_value = snprintf(
        infile,
        TESTS_MAX_PATH,
        "%s/%s",
        fixture_directory,
        "blockchain_4_blocks_no_transactions");
    assert_true(return_value < TESTS_MAX_PATH);
    blockchain_t *peer_blockchain = NULL;
    return_code_t return_code = blockchain_read_from_file(
        &peer_blockchain, infile);
    assert_true(SUCCESS == return_code);
    // Change something in the peer's blockchain to make it invalid.
    block_t *second_block =
        (block_t *)peer_blockchain->block_list->head->next->data;
    second_block->created_at++;
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
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes = 3;
    return_code = blockchain_create(&blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
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
    args.peer_info_list = &peer_info_list;
    pthread_mutex_t peer_info_list_mutex;
    pthread_mutex_init(&peer_info_list_mutex, NULL);
    args.peer_info_list_mutex = &peer_info_list_mutex;
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    return_code = run_consensus_peer_client_once(&args, peer1);
    assert_true(SUCCESS == return_code);
    block_t *updated_genesis_block =
        (block_t *)args.sync->blockchain->block_list->head->data;
    // No change in blockchain because the peer's blockchain was invalid.
    assert_true(200 == updated_genesis_block->created_at);
    blockchain_destroy(peer_blockchain);
    free(send_blockchain_buffer);
    free(command_send_blockchain.blockchain_data);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    synchronized_blockchain_destroy(args.sync);
    linked_list_destroy(peer_info_list);
}

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
    args.peer_info_list = &peer_info_list;
    pthread_mutex_t peer_info_list_mutex;
    pthread_mutex_init(&peer_info_list_mutex, NULL);
    args.peer_info_list_mutex = &peer_info_list_mutex;
    args.print_progress = false;
    atomic_bool should_stop = true;
    args.should_stop = &should_stop;
    bool exit_ready = false;
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
    pthread_mutex_destroy(args.peer_info_list_mutex);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    synchronized_blockchain_destroy(args.sync);
    linked_list_destroy(peer_info_list);
}
