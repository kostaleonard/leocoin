/**
 * @brief Contains functions for cross-platform sleep (pause the program).
 */

#ifndef INCLUDE_SLEEP_H_
#define INCLUDE_SLEEP_H_

#include <stdint.h>
#include "include/return_codes.h"

/**
 * @brief Pauses the program for the given number of microseconds.
 * 
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t sleep_microseconds(uint64_t microseconds);

#endif  // INCLUDE_SLEEP_H_
