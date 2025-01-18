#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "include/sleep.h"
#include "tests/test_sleep.h"

void test_sleep_microseconds_pauses_program() {
    struct timespec start_time;
    struct timespec end_time;
    uint64_t microseconds = 1000;
    clock_gettime(CLOCK_REALTIME, &start_time);
    return_code_t return_code = sleep_microseconds(microseconds);
    clock_gettime(CLOCK_REALTIME, &end_time);
    time_t nsec_diff = end_time.tv_nsec - start_time.tv_nsec;
    if (nsec_diff < 0) {
        nsec_diff += 1000000000L;
    }
    // When the sleep duration is low, the precision of the sleep is also low.
    // For that reason, we provide a wide margin for error in this test.
    assert_true(nsec_diff >= 10 * microseconds);
    assert_true(SUCCESS == return_code);
}
