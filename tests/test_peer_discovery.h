/**
 * @brief Tests peer_discovery.c.
 */

#ifndef TESTS_TEST_PEER_DISCOVERY_H_
#define TESTS_TEST_PEER_DISCOVERY_H_
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

void test_compare_peer_info_t_compares_ip_addresses();

void test_peer_info_list_serialize_fails_on_invalid_input();

void test_peer_info_list_serialize_creates_nonempty_buffer();

void test_peer_info_list_deserialize_reconstructs_list();

void test_peer_info_list_deserialize_fails_on_read_past_buffer();

void test_peer_info_list_deserialize_fails_on_invalid_input();

#endif  // TESTS_TEST_PEER_DISCOVERY_H_
