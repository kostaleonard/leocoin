/**
 * @brief Tests peer_discovery_thread.c.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/networking.h"
#include "include/peer_discovery.h"
#include "include/peer_discovery_thread.h"
#include "include/sleep.h"
#include "tests/mocks.h"
#include "tests/test_peer_discovery_thread.h"

void test_discover_peers_once_updates_peer_list() {
    wrap_connect = mock_connect;
    wrap_recv = mock_recv;
    wrap_send = mock_send;
    linked_list_t *peer_info_list = NULL;
    return_code_t return_code = linked_list_create(
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
    command_send_peer_list_t command_send_peer_list = {0};
    memcpy(
        command_send_peer_list.header.command_prefix,
        COMMAND_PREFIX,
        COMMAND_PREFIX_LEN);
    command_send_peer_list.header.command = COMMAND_SEND_PEER_LIST;
    return_code = peer_info_list_serialize(
        peer_info_list,
        &command_send_peer_list.peer_list_data,
        &command_send_peer_list.peer_list_data_len);
    assert_true(SUCCESS == return_code);
    unsigned char *command_send_peer_list_buffer = NULL;
    uint64_t command_send_peer_list_buffer_len = 0;
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list,
        &command_send_peer_list_buffer,
        &command_send_peer_list_buffer_len);
    assert_true(SUCCESS == return_code);
    will_return(mock_recv, command_send_peer_list_buffer);
    will_return(mock_recv, sizeof(command_header_t));
    will_return(
        mock_recv, command_send_peer_list_buffer + sizeof(command_header_t));
    will_return(
        mock_recv,
        command_send_peer_list_buffer_len - sizeof(command_header_t));
    size_t send_size = sizeof(command_register_peer_t);
    will_return(mock_send, send_size);
    discover_peers_args_t args = {0};
    args.peer_discovery_bootstrap_server_addr.sin6_addr.s6_addr[
        sizeof(IN6_ADDR) - 1] = 1;
    args.peer_discovery_bootstrap_server_addr.sin6_family = AF_INET6;
    args.peer_discovery_bootstrap_server_addr.sin6_port = htons(12345);
    args.peer_addr.sin6_addr.s6_addr[sizeof(IN6_ADDR) - 1] = 1;
    args.peer_addr.sin6_family = AF_INET6;
    args.peer_addr.sin6_port = htons(23456);
    args.communication_interval_microseconds = 100000;
    return_code = linked_list_create(
        &args.peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    pthread_mutex_init(&args.peer_info_list_mutex, NULL);
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    return_code = discover_peers_once(&args);
    assert_true(SUCCESS == return_code);
    uint64_t length = 0;
    return_code = linked_list_length(args.peer_info_list, &length);
    assert_true(SUCCESS == return_code);
    assert_true(2 == length);
    peer_info_t *p = (peer_info_t *)args.peer_info_list->head->data;
    assert_true(0 == compare_peer_info_t(peer1, p));
    p = (peer_info_t *)args.peer_info_list->head->next->data;
    assert_true(0 == compare_peer_info_t(peer2, p));
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    pthread_mutex_destroy(&args.peer_info_list_mutex);
    linked_list_destroy(args.peer_info_list);
}

void test_discover_peers_once_receives_large_peer_list() {
    wrap_connect = mock_connect;
    wrap_recv = mock_recv;
    wrap_send = mock_send;
    linked_list_t *peer_info_list = NULL;
    return_code_t return_code = linked_list_create(
        &peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    uint64_t num_test_peers = 1000;
    for (uint64_t idx = 0; idx < num_test_peers; idx++) {
        peer_info_t *peer1 = calloc(1, sizeof(peer_info_t));
        peer1->listen_addr.sin6_family = AF_INET6;
        peer1->listen_addr.sin6_port = htons(idx);
        peer1->listen_addr.sin6_flowinfo = 0;
        ((unsigned char *)(&peer1->listen_addr.sin6_addr))[
            sizeof(IN6_ADDR) - 1] = 1;
        peer1->listen_addr.sin6_scope_id = 0;
        peer1->last_connected = 100;
        return_code = linked_list_prepend(peer_info_list, peer1);
        assert_true(SUCCESS == return_code);
    }    
    command_send_peer_list_t command_send_peer_list = {0};
    memcpy(
        command_send_peer_list.header.command_prefix,
        COMMAND_PREFIX,
        COMMAND_PREFIX_LEN);
    command_send_peer_list.header.command = COMMAND_SEND_PEER_LIST;
    return_code = peer_info_list_serialize(
        peer_info_list,
        &command_send_peer_list.peer_list_data,
        &command_send_peer_list.peer_list_data_len);
    assert_true(SUCCESS == return_code);
    unsigned char *command_send_peer_list_buffer = NULL;
    uint64_t command_send_peer_list_buffer_len = 0;
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list,
        &command_send_peer_list_buffer,
        &command_send_peer_list_buffer_len);
    assert_true(SUCCESS == return_code);
    will_return(mock_recv, command_send_peer_list_buffer);
    will_return(mock_recv, sizeof(command_header_t));
    will_return(
        mock_recv, command_send_peer_list_buffer + sizeof(command_header_t));
    will_return(
        mock_recv,
        command_send_peer_list_buffer_len - sizeof(command_header_t));
    size_t send_size = sizeof(command_register_peer_t);
    will_return(mock_send, send_size);
    discover_peers_args_t args = {0};
    args.peer_discovery_bootstrap_server_addr.sin6_addr.s6_addr[
        sizeof(IN6_ADDR) - 1] = 1;
    args.peer_discovery_bootstrap_server_addr.sin6_family = AF_INET6;
    args.peer_discovery_bootstrap_server_addr.sin6_port = htons(12345);
    args.peer_addr.sin6_addr.s6_addr[sizeof(IN6_ADDR) - 1] = 1;
    args.peer_addr.sin6_family = AF_INET6;
    args.peer_addr.sin6_port = htons(23456);
    args.communication_interval_microseconds = 100000;
    return_code = linked_list_create(
        &args.peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    pthread_mutex_init(&args.peer_info_list_mutex, NULL);
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    return_code = discover_peers_once(&args);
    assert_true(SUCCESS == return_code);
    uint64_t length = 0;
    return_code = linked_list_length(args.peer_info_list, &length);
    assert_true(SUCCESS == return_code);
    assert_true(num_test_peers == length);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    pthread_mutex_destroy(&args.peer_info_list_mutex);
    linked_list_destroy(args.peer_info_list);
}

void test_discover_peers_exits_when_should_stop_is_set() {
    wrap_connect = mock_connect;
    discover_peers_args_t args = {0};
    args.peer_discovery_bootstrap_server_addr.sin6_addr.s6_addr[
        sizeof(IN6_ADDR) - 1] = 1;
    args.peer_discovery_bootstrap_server_addr.sin6_family = AF_INET6;
    args.peer_discovery_bootstrap_server_addr.sin6_port = htons(12345);
    args.peer_addr.sin6_addr.s6_addr[sizeof(IN6_ADDR) - 1] = 1;
    args.peer_addr.sin6_family = AF_INET6;
    args.peer_addr.sin6_port = htons(23456);
    args.communication_interval_microseconds = 10000;
    return_code_t return_code = linked_list_create(
        &args.peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    pthread_mutex_init(&args.peer_info_list_mutex, NULL);
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    pthread_t thread;
    pthread_create(&thread, NULL, discover_peers_pthread_wrapper, &args);
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
}
