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

void test_peer_info_list_serialize_fails_on_invalid_inputs();

#endif  // TESTS_TEST_PEER_DISCOVERY_H_
