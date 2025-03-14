/**
 * @brief Tests consensus_peer_server_thread.c.
 */

#ifndef TESTS_TEST_CONSENSUS_PEER_SERVER_THREAD_H_
#define TESTS_TEST_CONSENSUS_PEER_SERVER_THREAD_H_
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

void test_handle_one_consensus_request_receives_peer_blockchain();

void test_handle_one_consensus_request_switches_to_longest_chain();

void test_handle_one_consensus_request_rejects_invalid_chain();

void test_run_consensus_peer_server_exits_when_should_stop_is_set();

#endif  // TESTS_TEST_CONSENSUS_PEER_SERVER_THREAD_H_
