#include <stdbool.h>
#include <stdio.h>
#include "include/networking.h"
#include "include/peer_discovery.h"
#include "include/peer_discovery_bootstrap_server_thread.h"
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
    args.print_progress = true;
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
}

void test_handle_one_peer_discovery_request_updates_peer_keepalive() {
    assert_true(false);
}

void test_handle_peer_discovery_requests_exits_when_should_stop_is_set() {
    assert_true(false);
}
