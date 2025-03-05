/**
 * @brief Tests peer_discovery_bootstrap_server_thread.c.
 */

#ifndef TESTS_TEST_PEER_DISCOVERY_BOOTSTRAP_SERVER_THREAD_H_
#define TESTS_TEST_PEER_DISCOVERY_BOOTSTRAP_SERVER_THREAD_H_
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

void test_handle_one_peer_discovery_request_adds_to_peer_list();

void test_handle_one_peer_discovery_request_updates_peer_keepalive();

void test_handle_peer_discovery_requests_exits_when_should_stop_is_set();

#endif  // TESTS_TEST_PEER_DISCOVERY_BOOSTRAP_SERVER_THREAD_H_
