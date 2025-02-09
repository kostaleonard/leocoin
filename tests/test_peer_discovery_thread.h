/**
 * @brief Tests peer_discovery_thread.c.
 */

#ifndef TESTS_TEST_PEER_DISCOVERY_THREAD_H_
#define TESTS_TEST_PEER_DISCOVERY_THREAD_H_
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

void test_discover_peers_once_updates_peer_list();

void test_discover_peers_once_receives_large_peer_list();

void test_discover_peers_exits_when_should_stop_is_set();

#endif  // TESTS_TEST_PEER_DISCOVERY_THREAD_H_
