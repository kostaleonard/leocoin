#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/blockchain.h"
#include "include/linked_list.h"
#include "include/networking.h"
#include "include/peer_discovery.h"
#include "tests/test_networking.h"
#include "tests/mocks.h"

void test_command_header_serialize_fails_on_invalid_input() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_header.command_len = 17;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_header_serialize(
        NULL, &buffer, &buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_header_serialize(
        &command_header, NULL, &buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_header_serialize(
        &command_header, &buffer, NULL);
    assert_true(FAILURE_INVALID_INPUT == return_code);
}

void test_command_header_serialize_fails_on_invalid_prefix() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command_prefix[2] = 'A';
    command_header.command = COMMAND_REGISTER_PEER;
    command_header.command_len = 17;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_header_serialize(
        &command_header, &buffer, &buffer_size);
    assert_true(FAILURE_INVALID_COMMAND_PREFIX == return_code);
}

void test_command_header_serialize_creates_nonempty_buffer() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_header.command_len = 17;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_header_serialize(
        &command_header, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    assert_true(NULL != buffer);
    assert_true(0 != buffer_size);
    unsigned char *empty_buffer = calloc(buffer_size, 1);
    assert_true(0 != memcmp(buffer, empty_buffer, buffer_size));
    free(buffer);
    free(empty_buffer);
}

void test_command_header_deserialize_reconstructs_command() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_header.command_len = 17;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_header_serialize(
        &command_header, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    command_header_t deserialized_command_header = {0};
    return_code = command_header_deserialize(
        &deserialized_command_header, buffer, buffer_size);
    assert_true(SUCCESS == return_code);
    assert_true(0 == memcmp(
        &command_header,
        &deserialized_command_header,
        sizeof(command_header_t)));
    free(buffer);
}

void test_command_header_deserialize_fails_on_read_past_buffer() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_header.command_len = 17;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_header_serialize(
        &command_header, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    command_header_t deserialized_command_header = {0};
    return_code = command_header_deserialize(
        &deserialized_command_header, buffer, buffer_size - 4);
    assert_true(FAILURE_BUFFER_TOO_SMALL == return_code);
    free(buffer);
}

void test_command_header_deserialize_fails_on_invalid_prefix() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_header.command_len = 17;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_header_serialize(
        &command_header, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    buffer[2] = 'A';
    command_header_t deserialized_command_header = {0};
    return_code = command_header_deserialize(
        &deserialized_command_header, buffer, buffer_size);
    assert_true(FAILURE_INVALID_COMMAND_PREFIX == return_code);
    free(buffer);
}

void test_command_header_deserialize_fails_on_invalid_input() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_header.command_len = 17;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_header_serialize(
        &command_header, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    command_header_t deserialized_command_header = {0};
    return_code = command_header_deserialize(
        NULL, buffer, buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_header_deserialize(
        &deserialized_command_header, NULL, buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    free(buffer);
}

void test_command_register_peer_serialize_fails_on_invalid_input() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_register_peer_t command_register_peer = {0};
    command_register_peer.header = command_header;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = 12345;
    command_register_peer.sin6_flowinfo = 0;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_scope_id = 0;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_register_peer_serialize(
        NULL, &buffer, &buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_register_peer_serialize(
        &command_register_peer, NULL, &buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_register_peer_serialize(
        &command_register_peer, &buffer, NULL);
    assert_true(FAILURE_INVALID_INPUT == return_code);
}

void test_command_register_peer_serialize_fails_on_invalid_prefix() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command_prefix[2] = 'A';
    command_header.command = COMMAND_REGISTER_PEER;
    command_register_peer_t command_register_peer = {0};
    command_register_peer.header = command_header;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = 12345;
    command_register_peer.sin6_flowinfo = 0;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_scope_id = 0;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_register_peer_serialize(
        &command_register_peer, &buffer, &buffer_size);
    assert_true(FAILURE_INVALID_COMMAND_PREFIX == return_code);
}

void test_command_register_peer_serialize_fails_on_invalid_command() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_OK;
    command_register_peer_t command_register_peer = {0};
    command_register_peer.header = command_header;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = 12345;
    command_register_peer.sin6_flowinfo = 0;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_scope_id = 0;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_register_peer_serialize(
        &command_register_peer, &buffer, &buffer_size);
    assert_true(FAILURE_INVALID_COMMAND == return_code);
}

void test_command_register_peer_serialize_creates_nonempty_buffer() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_register_peer_t command_register_peer = {0};
    command_register_peer.header = command_header;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = 12345;
    command_register_peer.sin6_flowinfo = 0;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_scope_id = 0;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_register_peer_serialize(
        &command_register_peer, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    assert_true(NULL != buffer);
    assert_true(0 != buffer_size);
    unsigned char *empty_buffer = calloc(buffer_size, 1);
    assert_true(0 != memcmp(buffer, empty_buffer, buffer_size));
    free(buffer);
    free(empty_buffer);
}

void test_command_register_peer_deserialize_reconstructs_command() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_register_peer_t command_register_peer = {0};
    command_register_peer.header = command_header;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = 12345;
    command_register_peer.sin6_flowinfo = 0;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_scope_id = 0;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_register_peer_serialize(
        &command_register_peer, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    command_register_peer_t deserialized_command_register_peer = {0};
    return_code = command_register_peer_deserialize(
        &deserialized_command_register_peer, buffer, buffer_size);
    assert_true(SUCCESS == return_code);
    assert_true(0 == memcmp(
        &command_register_peer,
        &deserialized_command_register_peer,
        sizeof(command_register_peer_t)));
    free(buffer);
}

void test_command_register_peer_deserialize_fails_on_read_past_buffer() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_register_peer_t command_register_peer = {0};
    command_register_peer.header = command_header;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = 12345;
    command_register_peer.sin6_flowinfo = 0;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_scope_id = 0;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_register_peer_serialize(
        &command_register_peer, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    command_register_peer_t deserialized_command_register_peer = {0};
    return_code = command_register_peer_deserialize(
        &deserialized_command_register_peer, buffer, buffer_size - 5);
    assert_true(FAILURE_BUFFER_TOO_SMALL == return_code);
    free(buffer);
}

void test_command_register_peer_deserialize_fails_on_invalid_prefix() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_register_peer_t command_register_peer = {0};
    command_register_peer.header = command_header;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = 12345;
    command_register_peer.sin6_flowinfo = 0;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_scope_id = 0;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_register_peer_serialize(
        &command_register_peer, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    buffer[2] = 'A';
    command_register_peer_t deserialized_command_register_peer = {0};
    return_code = command_register_peer_deserialize(
        &deserialized_command_register_peer, buffer, buffer_size);
    assert_true(FAILURE_INVALID_COMMAND_PREFIX == return_code);
    free(buffer);
}

void test_command_register_peer_deserialize_fails_on_invalid_command() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_register_peer_t command_register_peer = {0};
    command_register_peer.header = command_header;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = 12345;
    command_register_peer.sin6_flowinfo = 0;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_scope_id = 0;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_register_peer_serialize(
        &command_register_peer, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    buffer[7] = COMMAND_ERROR;
    command_register_peer_t deserialized_command_register_peer = {0};
    return_code = command_register_peer_deserialize(
        &deserialized_command_register_peer, buffer, buffer_size);
    assert_true(FAILURE_INVALID_COMMAND == return_code);
    free(buffer);
}

void test_command_register_peer_deserialize_fails_on_invalid_input() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_REGISTER_PEER;
    command_register_peer_t command_register_peer = {0};
    command_register_peer.header = command_header;
    command_register_peer.sin6_family = AF_INET6;
    command_register_peer.sin6_port = 12345;
    command_register_peer.sin6_flowinfo = 0;
    command_register_peer.addr[sizeof(IN6_ADDR) - 1] = 1;
    command_register_peer.sin6_scope_id = 0;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code_t return_code = command_register_peer_serialize(
        &command_register_peer, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    command_register_peer_t deserialized_command_register_peer = {0};
    return_code = command_register_peer_deserialize(
        NULL, buffer, buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_register_peer_deserialize(
        &deserialized_command_register_peer, NULL, buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    free(buffer);
}

void test_command_send_peer_list_serialize_fails_on_invalid_input() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_PEER_LIST;
    command_header.command_len = 0;
    command_send_peer_list_t command_send_peer_list = {0};
    command_send_peer_list.header = command_header;
    linked_list_t *peer_info_list = NULL;
    return_code_t return_code = linked_list_create(
        &peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    peer_info_t *peer1 = calloc(1, sizeof(peer_info_t));
    peer1->listen_addr.sin6_family = AF_INET6;
    peer1->listen_addr.sin6_port = 12345;
    peer1->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer1->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer1->listen_addr.sin6_scope_id = 0;
    peer1->last_connected = 100;
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = 23456;
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
    unsigned char *peer_list_buffer = NULL;
    uint64_t peer_list_buffer_size = 0;
    return_code = peer_info_list_serialize(
        peer_info_list, &peer_list_buffer, &peer_list_buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list.peer_list_data = peer_list_buffer;
    command_send_peer_list.peer_list_data_len = peer_list_buffer_size;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code = command_send_peer_list_serialize(
        NULL, &buffer, &buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list, NULL, &buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list, &buffer, NULL);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    free(peer_list_buffer);
    linked_list_destroy(peer_info_list);
}

void test_command_send_peer_list_serialize_fails_on_invalid_prefix() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command_prefix[2] = 'A';
    command_header.command = COMMAND_SEND_PEER_LIST;
    command_header.command_len = 0;
    command_send_peer_list_t command_send_peer_list = {0};
    command_send_peer_list.header = command_header;
    linked_list_t *peer_info_list = NULL;
    return_code_t return_code = linked_list_create(
        &peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    peer_info_t *peer1 = calloc(1, sizeof(peer_info_t));
    peer1->listen_addr.sin6_family = AF_INET6;
    peer1->listen_addr.sin6_port = 12345;
    peer1->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer1->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer1->listen_addr.sin6_scope_id = 0;
    peer1->last_connected = 100;
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = 23456;
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
    unsigned char *peer_list_buffer = NULL;
    uint64_t peer_list_buffer_size = 0;
    return_code = peer_info_list_serialize(
        peer_info_list, &peer_list_buffer, &peer_list_buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list.peer_list_data = peer_list_buffer;
    command_send_peer_list.peer_list_data_len = peer_list_buffer_size;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list, &buffer, &buffer_size);
    assert_true(FAILURE_INVALID_COMMAND_PREFIX == return_code);
    free(buffer);
    free(peer_list_buffer);
    linked_list_destroy(peer_info_list);
}

void test_command_send_peer_list_serialize_fails_on_invalid_command() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_OK;
    command_header.command_len = 0;
    command_send_peer_list_t command_send_peer_list = {0};
    command_send_peer_list.header = command_header;
    linked_list_t *peer_info_list = NULL;
    return_code_t return_code = linked_list_create(
        &peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    peer_info_t *peer1 = calloc(1, sizeof(peer_info_t));
    peer1->listen_addr.sin6_family = AF_INET6;
    peer1->listen_addr.sin6_port = 12345;
    peer1->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer1->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer1->listen_addr.sin6_scope_id = 0;
    peer1->last_connected = 100;
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = 23456;
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
    unsigned char *peer_list_buffer = NULL;
    uint64_t peer_list_buffer_size = 0;
    return_code = peer_info_list_serialize(
        peer_info_list, &peer_list_buffer, &peer_list_buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list.peer_list_data = peer_list_buffer;
    command_send_peer_list.peer_list_data_len = peer_list_buffer_size;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list, &buffer, &buffer_size);
    assert_true(FAILURE_INVALID_COMMAND == return_code);
    free(buffer);
    free(peer_list_buffer);
    linked_list_destroy(peer_info_list);
}

void test_command_send_peer_list_serialize_creates_nonempty_buffer() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_PEER_LIST;
    command_header.command_len = 0;
    command_send_peer_list_t command_send_peer_list = {0};
    command_send_peer_list.header = command_header;
    linked_list_t *peer_info_list = NULL;
    return_code_t return_code = linked_list_create(
        &peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    peer_info_t *peer1 = calloc(1, sizeof(peer_info_t));
    peer1->listen_addr.sin6_family = AF_INET6;
    peer1->listen_addr.sin6_port = 12345;
    peer1->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer1->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer1->listen_addr.sin6_scope_id = 0;
    peer1->last_connected = 100;
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = 23456;
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
    unsigned char *peer_list_buffer = NULL;
    uint64_t peer_list_buffer_size = 0;
    return_code = peer_info_list_serialize(
        peer_info_list, &peer_list_buffer, &peer_list_buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list.peer_list_data = peer_list_buffer;
    command_send_peer_list.peer_list_data_len = peer_list_buffer_size;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    unsigned char *empty_buffer = calloc(buffer_size, 1);
    assert_true(0 != memcmp(buffer, empty_buffer, buffer_size));
    free(buffer);
    free(empty_buffer);
    free(peer_list_buffer);
    linked_list_destroy(peer_info_list);
}

void test_command_send_peer_list_deserialize_reconstructs_command() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_PEER_LIST;
    command_header.command_len = 0;
    command_send_peer_list_t command_send_peer_list = {0};
    command_send_peer_list.header = command_header;
    linked_list_t *peer_info_list = NULL;
    return_code_t return_code = linked_list_create(
        &peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    peer_info_t *peer1 = calloc(1, sizeof(peer_info_t));
    peer1->listen_addr.sin6_family = AF_INET6;
    peer1->listen_addr.sin6_port = 12345;
    peer1->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer1->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer1->listen_addr.sin6_scope_id = 0;
    peer1->last_connected = 100;
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = 23456;
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
    unsigned char *peer_list_buffer = NULL;
    uint64_t peer_list_buffer_size = 0;
    return_code = peer_info_list_serialize(
        peer_info_list, &peer_list_buffer, &peer_list_buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list.peer_list_data = peer_list_buffer;
    command_send_peer_list.peer_list_data_len = peer_list_buffer_size;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list_t deserialized_command_send_peer_list = {0};
    return_code = command_send_peer_list_deserialize(
        &deserialized_command_send_peer_list, buffer, buffer_size);
    assert_true(SUCCESS == return_code);
    // Everything should be the same except the pointers to peer list data.
    assert_true(0 == memcmp(
        &command_send_peer_list,
        &deserialized_command_send_peer_list,
        sizeof(command_send_peer_list_t) - sizeof(unsigned char *)));
    // The pointers will be different, but have the same contents.
    assert_true(0 == memcmp(
        command_send_peer_list.peer_list_data,
        deserialized_command_send_peer_list.peer_list_data,
        command_send_peer_list.peer_list_data_len));
    free(buffer);
    free(peer_list_buffer);
    free(deserialized_command_send_peer_list.peer_list_data);
    linked_list_destroy(peer_info_list);
}

void test_command_send_peer_list_deserialize_fails_on_read_past_buffer() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_PEER_LIST;
    command_header.command_len = 0;
    command_send_peer_list_t command_send_peer_list = {0};
    command_send_peer_list.header = command_header;
    linked_list_t *peer_info_list = NULL;
    return_code_t return_code = linked_list_create(
        &peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    peer_info_t *peer1 = calloc(1, sizeof(peer_info_t));
    peer1->listen_addr.sin6_family = AF_INET6;
    peer1->listen_addr.sin6_port = 12345;
    peer1->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer1->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer1->listen_addr.sin6_scope_id = 0;
    peer1->last_connected = 100;
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = 23456;
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
    unsigned char *peer_list_buffer = NULL;
    uint64_t peer_list_buffer_size = 0;
    return_code = peer_info_list_serialize(
        peer_info_list, &peer_list_buffer, &peer_list_buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list.peer_list_data = peer_list_buffer;
    command_send_peer_list.peer_list_data_len = peer_list_buffer_size;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list_t deserialized_command_send_peer_list = {0};
    return_code = command_send_peer_list_deserialize(
        &deserialized_command_send_peer_list, buffer, buffer_size - 5);
    assert_true(FAILURE_BUFFER_TOO_SMALL == return_code);
    free(buffer);
    free(peer_list_buffer);
    linked_list_destroy(peer_info_list);
}

void test_command_send_peer_list_deserialize_fails_on_invalid_prefix() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_PEER_LIST;
    command_header.command_len = 0;
    command_send_peer_list_t command_send_peer_list = {0};
    command_send_peer_list.header = command_header;
    linked_list_t *peer_info_list = NULL;
    return_code_t return_code = linked_list_create(
        &peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    peer_info_t *peer1 = calloc(1, sizeof(peer_info_t));
    peer1->listen_addr.sin6_family = AF_INET6;
    peer1->listen_addr.sin6_port = 12345;
    peer1->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer1->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer1->listen_addr.sin6_scope_id = 0;
    peer1->last_connected = 100;
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = 23456;
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
    unsigned char *peer_list_buffer = NULL;
    uint64_t peer_list_buffer_size = 0;
    return_code = peer_info_list_serialize(
        peer_info_list, &peer_list_buffer, &peer_list_buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list.peer_list_data = peer_list_buffer;
    command_send_peer_list.peer_list_data_len = peer_list_buffer_size;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    buffer[2] = 'A';
    command_send_peer_list_t deserialized_command_send_peer_list = {0};
    return_code = command_send_peer_list_deserialize(
        &deserialized_command_send_peer_list, buffer, buffer_size);
    assert_true(FAILURE_INVALID_COMMAND_PREFIX == return_code);
    free(buffer);
    free(peer_list_buffer);
    linked_list_destroy(peer_info_list);
}

void test_command_send_peer_list_deserialize_fails_on_invalid_command() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_PEER_LIST;
    command_header.command_len = 0;
    command_send_peer_list_t command_send_peer_list = {0};
    command_send_peer_list.header = command_header;
    linked_list_t *peer_info_list = NULL;
    return_code_t return_code = linked_list_create(
        &peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    peer_info_t *peer1 = calloc(1, sizeof(peer_info_t));
    peer1->listen_addr.sin6_family = AF_INET6;
    peer1->listen_addr.sin6_port = 12345;
    peer1->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer1->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer1->listen_addr.sin6_scope_id = 0;
    peer1->last_connected = 100;
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = 23456;
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
    unsigned char *peer_list_buffer = NULL;
    uint64_t peer_list_buffer_size = 0;
    return_code = peer_info_list_serialize(
        peer_info_list, &peer_list_buffer, &peer_list_buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list.peer_list_data = peer_list_buffer;
    command_send_peer_list.peer_list_data_len = peer_list_buffer_size;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    buffer[7] = COMMAND_ERROR;
    command_send_peer_list_t deserialized_command_send_peer_list = {0};
    return_code = command_send_peer_list_deserialize(
        &deserialized_command_send_peer_list, buffer, buffer_size);
    assert_true(FAILURE_INVALID_COMMAND == return_code);
    free(buffer);
    free(peer_list_buffer);
    linked_list_destroy(peer_info_list);
}

void test_command_send_peer_list_deserialize_fails_on_invalid_input() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_PEER_LIST;
    command_header.command_len = 0;
    command_send_peer_list_t command_send_peer_list = {0};
    command_send_peer_list.header = command_header;
    linked_list_t *peer_info_list = NULL;
    return_code_t return_code = linked_list_create(
        &peer_info_list, free, compare_peer_info_t);
    assert_true(SUCCESS == return_code);
    peer_info_t *peer1 = calloc(1, sizeof(peer_info_t));
    peer1->listen_addr.sin6_family = AF_INET6;
    peer1->listen_addr.sin6_port = 12345;
    peer1->listen_addr.sin6_flowinfo = 0;
    ((unsigned char *)(&peer1->listen_addr.sin6_addr))[
        sizeof(IN6_ADDR) - 1] = 1;
    peer1->listen_addr.sin6_scope_id = 0;
    peer1->last_connected = 100;
    peer_info_t *peer2 = calloc(1, sizeof(peer_info_t));
    peer2->listen_addr.sin6_family = AF_INET6;
    peer2->listen_addr.sin6_port = 23456;
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
    unsigned char *peer_list_buffer = NULL;
    uint64_t peer_list_buffer_size = 0;
    return_code = peer_info_list_serialize(
        peer_info_list, &peer_list_buffer, &peer_list_buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list.peer_list_data = peer_list_buffer;
    command_send_peer_list.peer_list_data_len = peer_list_buffer_size;
    unsigned char *buffer = NULL;
    uint64_t buffer_size = 0;
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list, &buffer, &buffer_size);
    assert_true(SUCCESS == return_code);
    command_send_peer_list_t deserialized_command_send_peer_list = {0};
    return_code = command_send_peer_list_deserialize(
        NULL, buffer, buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_send_peer_list_deserialize(
        &deserialized_command_send_peer_list, NULL, buffer_size);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    free(buffer);
    free(peer_list_buffer);
    linked_list_destroy(peer_info_list);
}

void test_command_send_blockchain_serialize_fails_on_invalid_input() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes = 4;
    return_code_t return_code = blockchain_create(&blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_serialize(
        blockchain,
        &command_send_blockchain.blockchain_data,
        &command_send_blockchain.blockchain_data_len);
    assert_true(SUCCESS == return_code);
    unsigned char *send_blockchain_buffer = NULL;
    uint64_t send_blockchain_buffer_len = 0;
    return_code = command_send_blockchain_serialize(
        NULL,
        &send_blockchain_buffer,
        &send_blockchain_buffer_len);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_send_blockchain_serialize(
        &command_send_blockchain,
        NULL,
        &send_blockchain_buffer_len);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_send_blockchain_serialize(
        &command_send_blockchain,
        &send_blockchain_buffer,
        NULL);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    blockchain_destroy(blockchain);
    free(command_send_blockchain.blockchain_data);
}

void test_command_send_blockchain_serialize_fails_on_invalid_prefix() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command_prefix[2] = 'A';
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes = 4;
    return_code_t return_code = blockchain_create(&blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_serialize(
        blockchain,
        &command_send_blockchain.blockchain_data,
        &command_send_blockchain.blockchain_data_len);
    assert_true(SUCCESS == return_code);
    unsigned char *send_blockchain_buffer = NULL;
    uint64_t send_blockchain_buffer_len = 0;
    return_code = command_send_blockchain_serialize(
        &command_send_blockchain,
        &send_blockchain_buffer,
        &send_blockchain_buffer_len);
    assert_true(FAILURE_INVALID_COMMAND_PREFIX == return_code);
    blockchain_destroy(blockchain);
    free(command_send_blockchain.blockchain_data);
}

void test_command_send_blockchain_serialize_fails_on_invalid_command() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_OK;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes = 4;
    return_code_t return_code = blockchain_create(&blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_serialize(
        blockchain,
        &command_send_blockchain.blockchain_data,
        &command_send_blockchain.blockchain_data_len);
    assert_true(SUCCESS == return_code);
    unsigned char *send_blockchain_buffer = NULL;
    uint64_t send_blockchain_buffer_len = 0;
    return_code = command_send_blockchain_serialize(
        &command_send_blockchain,
        &send_blockchain_buffer,
        &send_blockchain_buffer_len);
    assert_true(FAILURE_INVALID_COMMAND == return_code);
    blockchain_destroy(blockchain);
    free(command_send_blockchain.blockchain_data);
}

void test_command_send_blockchain_serialize_creates_nonempty_buffer() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes = 4;
    return_code_t return_code = blockchain_create(&blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_serialize(
        blockchain,
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
    unsigned char *empty_buffer = calloc(send_blockchain_buffer_len, 1);
    assert_true(0 != memcmp(
        send_blockchain_buffer, empty_buffer, send_blockchain_buffer_len));
    free(send_blockchain_buffer);
    free(empty_buffer);
    blockchain_destroy(blockchain);
    free(command_send_blockchain.blockchain_data);
}

void test_command_send_blockchain_deserialize_reconstructs_command() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes = 4;
    return_code_t return_code = blockchain_create(&blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_serialize(
        blockchain,
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
    command_send_blockchain_t deserialized_command_send_blockchain = {0};
    return_code = command_send_blockchain_deserialize(
        &deserialized_command_send_blockchain,
        send_blockchain_buffer,
        send_blockchain_buffer_len);
    assert_true(SUCCESS == return_code);
    // Everything should be the same except the pointers to blockchain data.
    assert_true(0 == memcmp(
        &command_send_blockchain,
        &deserialized_command_send_blockchain,
        sizeof(command_send_blockchain) - sizeof(unsigned char *)));
    // The pointers will be different, but have the same contents.
    assert_true(0 == memcmp(
        command_send_blockchain.blockchain_data,
        deserialized_command_send_blockchain.blockchain_data,
        command_send_blockchain.blockchain_data_len));
    free(send_blockchain_buffer);
    blockchain_destroy(blockchain);
    free(command_send_blockchain.blockchain_data);
    free(deserialized_command_send_blockchain.blockchain_data);
}

void test_command_send_blockchain_deserialize_fails_on_read_past_buffer() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes = 4;
    return_code_t return_code = blockchain_create(&blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_serialize(
        blockchain,
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
    command_send_blockchain_t deserialized_command_send_blockchain = {0};
    return_code = command_send_blockchain_deserialize(
        &deserialized_command_send_blockchain,
        send_blockchain_buffer,
        send_blockchain_buffer_len - 5);
    assert_true(FAILURE_BUFFER_TOO_SMALL == return_code);
    free(send_blockchain_buffer);
    blockchain_destroy(blockchain);
    free(command_send_blockchain.blockchain_data);
}

void test_command_send_blockchain_deserialize_fails_on_invalid_prefix() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes = 4;
    return_code_t return_code = blockchain_create(&blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_serialize(
        blockchain,
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
    send_blockchain_buffer[2] = 'A';
    command_send_blockchain_t deserialized_command_send_blockchain = {0};
    return_code = command_send_blockchain_deserialize(
        &deserialized_command_send_blockchain,
        send_blockchain_buffer,
        send_blockchain_buffer_len);
    assert_true(FAILURE_INVALID_COMMAND_PREFIX == return_code);
    free(send_blockchain_buffer);
    blockchain_destroy(blockchain);
    free(command_send_blockchain.blockchain_data);
}

void test_command_send_blockchain_deserialize_fails_on_invalid_command() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes = 4;
    return_code_t return_code = blockchain_create(&blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_serialize(
        blockchain,
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
    send_blockchain_buffer[7] = COMMAND_ERROR;
    command_send_blockchain_t deserialized_command_send_blockchain = {0};
    return_code = command_send_blockchain_deserialize(
        &deserialized_command_send_blockchain,
        send_blockchain_buffer,
        send_blockchain_buffer_len);
    assert_true(FAILURE_INVALID_COMMAND == return_code);
    free(send_blockchain_buffer);
    blockchain_destroy(blockchain);
    free(command_send_blockchain.blockchain_data);
}

void test_command_send_blockchain_deserialize_fails_on_invalid_input() {
    command_header_t command_header = COMMAND_HEADER_INITIALIZER;
    command_header.command = COMMAND_SEND_BLOCKCHAIN;
    command_header.command_len = 0;
    command_send_blockchain_t command_send_blockchain = {0};
    command_send_blockchain.header = command_header;
    blockchain_t *blockchain = NULL;
    size_t num_zero_bytes = 4;
    return_code_t return_code = blockchain_create(&blockchain, num_zero_bytes);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block = NULL;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_serialize(
        blockchain,
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
    command_send_blockchain_t deserialized_command_send_blockchain = {0};
    return_code = command_send_blockchain_deserialize(
        NULL,
        send_blockchain_buffer,
        send_blockchain_buffer_len);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    return_code = command_send_blockchain_deserialize(
        &deserialized_command_send_blockchain,
        NULL,
        send_blockchain_buffer_len);
    assert_true(FAILURE_INVALID_INPUT == return_code);
    free(send_blockchain_buffer);
    blockchain_destroy(blockchain);
    free(command_send_blockchain.blockchain_data);
}

void test_recv_all_reads_data_from_socket() {
    wrap_recv = mock_recv;
    char *read_data = "hello recv";
    size_t read_data_len = strlen(read_data) + 1;
    will_return(mock_recv, read_data);
    will_return(mock_recv, read_data_len);
    char buf[BUFSIZ] = {0};
    return_code_t return_code = recv_all(MOCK_SOCKET, buf, read_data_len, 0);
    assert_true(SUCCESS == return_code);
    assert_true(0 == strncmp(buf, read_data, read_data_len));
}

void test_recv_all_handles_partial_read() {
    wrap_recv = mock_recv;
    char *read_data = "hello partial read";
    size_t total_len = strlen(read_data) + 1;
    size_t read_1_len = 5;
    size_t read_2_len = total_len - read_1_len;
    will_return(mock_recv, read_data);
    will_return(mock_recv, read_1_len);
    will_return(mock_recv, read_data + read_1_len);
    will_return(mock_recv, read_2_len);
    char buf[BUFSIZ] = {0};
    return_code_t return_code = recv_all(MOCK_SOCKET, buf, total_len, 0);
    assert_true(SUCCESS == return_code);
    assert_true(0 == strncmp(buf, read_data, total_len));
}

void test_recv_all_fails_on_recv_error() {
    wrap_recv = mock_recv;
    char *read_data = "hello recv error";
    size_t total_len = strlen(read_data) + 1;
    size_t read_1_len = 5;
    will_return(mock_recv, read_data);
    will_return(mock_recv, read_1_len);
    will_return(mock_recv, NULL);
    will_return(mock_recv, -1);
    char buf[BUFSIZ] = {0};
    return_code_t return_code = recv_all(MOCK_SOCKET, buf, total_len, 0);
    assert_true(FAILURE_NETWORK_FUNCTION == return_code);
}

void test_recv_all_fails_on_invalid_input() {
    return_code_t return_code = recv_all(MOCK_SOCKET, NULL, 100, 0);
    assert_true(FAILURE_INVALID_INPUT == return_code);
}

void test_send_all_sends_data_to_socket() {
    wrap_send = mock_send;
    char *send_data = "hello send";
    size_t send_data_len = strlen(send_data) + 1;
    will_return(mock_send, send_data_len);
    return_code_t return_code = send_all(
        MOCK_SOCKET, send_data, send_data_len, 0);
    assert_true(SUCCESS == return_code);
}

void test_send_all_handles_partial_write() {
    wrap_send = mock_send;
    char *send_data = "hello send";
    size_t send_data_len = strlen(send_data) + 1;
    size_t send_1_len = 5;
    size_t send_2_len = send_data_len - send_1_len;
    will_return(mock_send, send_1_len);
    will_return(mock_send, send_2_len);
    return_code_t return_code = send_all(
        MOCK_SOCKET, send_data, send_data_len, 0);
    assert_true(SUCCESS == return_code);
}

void test_send_all_fails_on_send_error() {
    wrap_send = mock_send;
    char *send_data = "hello send";
    size_t send_data_len = strlen(send_data) + 1;
    size_t send_1_len = 5;
    will_return(mock_send, send_1_len);
    will_return(mock_send, -1);
    return_code_t return_code = send_all(
        MOCK_SOCKET, send_data, send_data_len, 0);
    assert_true(FAILURE_NETWORK_FUNCTION == return_code);
}

void test_send_all_fails_on_invalid_input() {
    return_code_t return_code = send_all(MOCK_SOCKET, NULL, 100, 0);
    assert_true(FAILURE_INVALID_INPUT == return_code);
}
