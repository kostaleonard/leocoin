/**
 * @brief Tests mining_thread.c.
 */

#ifndef TESTS_TEST_MINING_THREAD_H_
#define TESTS_TEST_MINING_THREAD_H_
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

void test_mine_blocks_exits_when_should_stop_is_set();

void test_mine_blocks_mines_new_blockchain_when_version_incremented();

#endif  // TESTS_TEST_MINING_THREAD_H_
