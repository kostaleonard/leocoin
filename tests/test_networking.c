#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/linked_list.h"
#include "include/networking.h"
#include "include/peer_discovery.h"
#include "tests/test_networking.h"

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
