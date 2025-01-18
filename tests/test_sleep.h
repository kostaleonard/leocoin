/**
 * @brief Tests sleep.c
 */

#ifndef TESTS_TEST_SLEEP_H_
#define TESTS_TEST_SLEEP_H_
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

void test_sleep_microseconds_pauses_program();

#endif  // TESTS_TEST_SLEEP_H_
