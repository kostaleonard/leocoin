// TODO registers with peer discovery server and maintains peer list.

#include "include/peer_discovery_thread.h"
#include "include/sleep.h"

return_code_t *discover_peers(discover_peers_args_t *args) {
    return_code_t return_code = SUCCESS;
    bool should_stop = *args->should_stop;
    while (!should_stop) {
        if (args->print_progress) {
            printf("Attempting to connect to peer discovery server.\n");
        }
        // TODO connect to peer discovery server

        // TODO register self as peer
        // TODO request peer list
        
        sleep_microseconds(args->communication_interval_seconds * 1000000);
        should_stop = *args->should_stop;
    }
    if (should_stop) {
        return_code = FAILURE_STOPPED_EARLY;
        if (args->print_progress) {
            printf("Stopping peer discovery.\n");
        }
    }
    pthread_mutex_lock(&args->exit_ready_mutex);
    *args->exit_ready = true;
    pthread_cond_signal(&args->exit_ready_cond);
    pthread_mutex_unlock(&args->exit_ready_mutex);
end:
    return_code_t *return_code_ptr = malloc(sizeof(return_code_t));
    *return_code_ptr = return_code;
    return return_code_ptr;
}
