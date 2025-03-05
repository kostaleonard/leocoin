#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/networking.h"
#include "include/peer_discovery.h"
#include "include/peer_discovery_bootstrap_server_thread.h"
#include "include/sleep.h"
#include "tests/mocks.h"
#include "tests/test_peer_discovery_bootstrap_server_thread.h"

void test_handle_one_peer_discovery_request_adds_to_peer_list() {
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
    handle_peer_discovery_requests_args_t args = {0};
    args.peer_discovery_bootstrap_server_addr.sin6_family = AF_INET6;
    args.peer_discovery_bootstrap_server_addr.sin6_port = htons(55555);
    ((unsigned char *)(&args.peer_discovery_bootstrap_server_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    args.peer_keepalive_microseconds = 1e6;
    args.peer_info_list = peer_info_list;
    pthread_mutex_init(&args.peer_info_list_mutex, NULL);
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    int conn_fd = 99;
    command_register_peer_t command_register_peer = {0};
    memcpy(
        command_register_peer.header.command_prefix,
        COMMAND_PREFIX,
        COMMAND_PREFIX_LEN);
    command_register_peer.header.command = COMMAND_REGISTER_PEER;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = htons(44444);
    unsigned char *command_register_peer_buffer = NULL;
    uint64_t command_register_peer_buffer_len = 0;
    return_code = command_register_peer_serialize(
        &command_register_peer,
        &command_register_peer_buffer,
        &command_register_peer_buffer_len);
    assert_true(SUCCESS == return_code);
    will_return(mock_recv, command_register_peer_buffer);
    will_return(mock_recv, sizeof(command_header_t));
    will_return(
        mock_recv, command_register_peer_buffer + sizeof(command_header_t));
    will_return(
        mock_recv,
        command_register_peer_buffer_len - sizeof(command_header_t));
    will_return_always(mock_send, 1);
    return_code = handle_one_peer_discovery_request(&args, conn_fd);
    assert_true(SUCCESS == return_code);
    uint64_t length = 0;
    return_code = linked_list_length(peer_info_list, &length);
    assert_true(SUCCESS == return_code);
    assert_true(3 == length);
    peer_info_t peer3 = {0};
    ((unsigned char *)(&peer3.listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer3.listen_addr.sin6_family = AF_INET6;
    peer3.listen_addr.sin6_port = htons(44444);
    node_t *found_node = NULL;
    return_code = linked_list_find(peer_info_list, &peer3, &found_node);
    assert_true(SUCCESS == return_code);
    assert_true(NULL != found_node);
    free(command_register_peer_buffer);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    pthread_mutex_destroy(&args.peer_info_list_mutex);
    linked_list_destroy(args.peer_info_list);
}

void test_handle_one_peer_discovery_request_updates_peer_keepalive() {
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
    peer2->last_connected = time(NULL);
    return_code = linked_list_prepend(peer_info_list, peer2);
    assert_true(SUCCESS == return_code);
    return_code = linked_list_prepend(peer_info_list, peer1);
    assert_true(SUCCESS == return_code);
    handle_peer_discovery_requests_args_t args = {0};
    args.peer_discovery_bootstrap_server_addr.sin6_family = AF_INET6;
    args.peer_discovery_bootstrap_server_addr.sin6_port = htons(55555);
    ((unsigned char *)(&args.peer_discovery_bootstrap_server_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    args.peer_keepalive_microseconds = 1e6;
    args.peer_info_list = peer_info_list;
    pthread_mutex_init(&args.peer_info_list_mutex, NULL);
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    int conn_fd = 99;
    command_register_peer_t command_register_peer = {0};
    memcpy(
        command_register_peer.header.command_prefix,
        COMMAND_PREFIX,
        COMMAND_PREFIX_LEN);
    command_register_peer.header.command = COMMAND_REGISTER_PEER;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = htons(12345);
    unsigned char *command_register_peer_buffer = NULL;
    uint64_t command_register_peer_buffer_len = 0;
    return_code = command_register_peer_serialize(
        &command_register_peer,
        &command_register_peer_buffer,
        &command_register_peer_buffer_len);
    assert_true(SUCCESS == return_code);
    will_return(mock_recv, command_register_peer_buffer);
    will_return(mock_recv, sizeof(command_header_t));
    will_return(
        mock_recv, command_register_peer_buffer + sizeof(command_header_t));
    will_return(
        mock_recv,
        command_register_peer_buffer_len - sizeof(command_header_t));
    will_return_always(mock_send, 1);
    return_code = handle_one_peer_discovery_request(&args, conn_fd);
    assert_true(SUCCESS == return_code);
    uint64_t length = 0;
    return_code = linked_list_length(peer_info_list, &length);
    assert_true(SUCCESS == return_code);
    assert_true(2 == length);
    peer_info_t peer3 = {0};
    ((unsigned char *)(&peer3.listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer3.listen_addr.sin6_family = AF_INET6;
    peer3.listen_addr.sin6_port = htons(12345);
    node_t *found_node = NULL;
    return_code = linked_list_find(peer_info_list, &peer3, &found_node);
    assert_true(SUCCESS == return_code);
    assert_true(NULL != found_node);
    peer_info_t *found_peer = (peer_info_t *)found_node->data;
    bool connected_in_last_second = time(NULL) - found_peer->last_connected < 1;
    assert_true(connected_in_last_second);
    free(command_register_peer_buffer);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    pthread_mutex_destroy(&args.peer_info_list_mutex);
    linked_list_destroy(args.peer_info_list);
}

void test_handle_one_peer_discovery_request_removes_expired_peers() {
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
    handle_peer_discovery_requests_args_t args = {0};
    args.peer_discovery_bootstrap_server_addr.sin6_family = AF_INET6;
    args.peer_discovery_bootstrap_server_addr.sin6_port = htons(55555);
    ((unsigned char *)(&args.peer_discovery_bootstrap_server_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    args.peer_keepalive_microseconds = 1e6;
    args.peer_info_list = peer_info_list;
    pthread_mutex_init(&args.peer_info_list_mutex, NULL);
    args.print_progress = false;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    int conn_fd = 99;
    command_register_peer_t command_register_peer = {0};
    memcpy(
        command_register_peer.header.command_prefix,
        COMMAND_PREFIX,
        COMMAND_PREFIX_LEN);
    command_register_peer.header.command = COMMAND_REGISTER_PEER;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = htons(44444);
    unsigned char *command_register_peer_buffer = NULL;
    uint64_t command_register_peer_buffer_len = 0;
    return_code = command_register_peer_serialize(
        &command_register_peer,
        &command_register_peer_buffer,
        &command_register_peer_buffer_len);
    assert_true(SUCCESS == return_code);
    will_return(mock_recv, command_register_peer_buffer);
    will_return(mock_recv, sizeof(command_header_t));
    will_return(
        mock_recv, command_register_peer_buffer + sizeof(command_header_t));
    will_return(
        mock_recv,
        command_register_peer_buffer_len - sizeof(command_header_t));
    will_return_always(mock_send, 1);
    return_code = handle_one_peer_discovery_request(&args, conn_fd);
    assert_true(SUCCESS == return_code);
    uint64_t length = 0;
    return_code = linked_list_length(peer_info_list, &length);
    assert_true(SUCCESS == return_code);
    assert_true(1 == length);
    peer_info_t peer3 = {0};
    ((unsigned char *)(&peer3.listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer3.listen_addr.sin6_family = AF_INET6;
    peer3.listen_addr.sin6_port = htons(44444);
    node_t *found_node = NULL;
    return_code = linked_list_find(peer_info_list, &peer3, &found_node);
    assert_true(SUCCESS == return_code);
    assert_true(NULL != found_node);
    free(command_register_peer_buffer);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    pthread_mutex_destroy(&args.peer_info_list_mutex);
    linked_list_destroy(args.peer_info_list);
}

void test_handle_peer_discovery_requests_exits_when_should_stop_is_set() {
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
    handle_peer_discovery_requests_args_t args = {0};
    args.peer_discovery_bootstrap_server_addr.sin6_family = AF_INET6;
    args.peer_discovery_bootstrap_server_addr.sin6_port = htons(55555);
    ((unsigned char *)(&args.peer_discovery_bootstrap_server_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    args.peer_keepalive_microseconds = 1e6;
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
        &thread, NULL, handle_peer_discovery_requests_pthread_wrapper, &args);
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
