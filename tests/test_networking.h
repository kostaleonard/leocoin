/**
 * @brief Tests networking.c.
 */

#ifndef TESTS_TEST_NETWORKING_H_
#define TESTS_TEST_NETWORKING_H_
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

void test_command_header_serialize_fails_on_invalid_input();

void test_command_header_serialize_fails_on_invalid_prefix();

void test_command_header_serialize_creates_nonempty_buffer();

void test_command_header_deserialize_reconstructs_command();

void test_command_header_deserialize_fails_on_read_past_buffer();

void test_command_header_deserialize_fails_on_invalid_prefix();

void test_command_header_deserialize_fails_on_invalid_input();

void test_command_register_peer_serialize_fails_on_invalid_input();

void test_command_register_peer_serialize_fails_on_invalid_prefix();

void test_command_register_peer_serialize_fails_on_invalid_command();

void test_command_register_peer_serialize_creates_nonempty_buffer();

void test_command_register_peer_deserialize_reconstructs_command();

void test_command_register_peer_deserialize_fails_on_read_past_buffer();

void test_command_register_peer_deserialize_fails_on_invalid_prefix();

void test_command_register_peer_deserialize_fails_on_invalid_command();

void test_command_register_peer_deserialize_fails_on_invalid_input();

void test_command_send_peer_list_serialize_fails_on_invalid_input();

void test_command_send_peer_list_serialize_fails_on_invalid_prefix();

void test_command_send_peer_list_serialize_fails_on_invalid_command();

void test_command_send_peer_list_serialize_creates_nonempty_buffer();

void test_command_send_peer_list_deserialize_reconstructs_command();

void test_command_send_peer_list_deserialize_fails_on_read_past_buffer();

void test_command_send_peer_list_deserialize_fails_on_invalid_prefix();

void test_command_send_peer_list_deserialize_fails_on_invalid_command();

void test_command_send_peer_list_deserialize_fails_on_invalid_input();

void test_command_send_blockchain_serialize_fails_on_invalid_input();

void test_command_send_blockchain_serialize_fails_on_invalid_prefix();

void test_command_send_blockchain_serialize_fails_on_invalid_command();

void test_command_send_blockchain_serialize_creates_nonempty_buffer();

void test_command_send_blockchain_deserialize_reconstructs_command();

void test_command_send_blockchain_deserialize_fails_on_read_past_buffer();

void test_command_send_blockchain_deserialize_fails_on_invalid_prefix();

void test_command_send_blockchain_deserialize_fails_on_invalid_command();

void test_command_send_blockchain_deserialize_fails_on_invalid_input();

void test_recv_all_reads_data_from_socket();

void test_recv_all_handles_partial_read();

void test_recv_all_fails_on_recv_error();

void test_recv_all_fails_on_invalid_input();

void test_send_all_sends_data_to_socket();

void test_send_all_handles_partial_write();

void test_send_all_fails_on_send_error();

void test_send_all_fails_on_invalid_input();

#endif  // TESTS_TEST_NETWORKING_H_
