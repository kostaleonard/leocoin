#include <time.h>
#include "include/sleep.h"

return_code_t sleep_microseconds(uint64_t microseconds) {
    return_code_t return_code = SUCCESS;
    struct timespec ts;
    ts.tv_sec = microseconds / 1000000;
    ts.tv_nsec = (microseconds % 1000000) * 1000;
    int result = nanosleep(&ts, NULL);
    if (0 != result) {
        return_code = FAILURE_SLEEP;
        goto end;
    }
end:
    return return_code;
}
