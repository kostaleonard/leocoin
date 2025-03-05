/**
 * @brief Runs the peer discovery bootstrap server.
 */

#include <stdio.h>
#include <stdlib.h>
#include "include/peer_discovery.h"
#include "include/peer_discovery_bootstrap_server_thread.h"

return_code_t run_peer_discovery_bootstrap_server(uint16_t port) {
    return_code_t return_code = SUCCESS;
    handle_peer_discovery_requests_args_t args = {0};
    args.peer_discovery_bootstrap_server_addr.sin6_family = AF_INET6;
    args.peer_discovery_bootstrap_server_addr.sin6_addr = in6addr_any;
    args.peer_discovery_bootstrap_server_addr.sin6_port = htons(port);
    args.peer_keepalive_microseconds = DEFAULT_PEER_KEEPALIVE_SECONDS * 1e6;
    return_code = linked_list_create(
        &args.peer_info_list, free, compare_peer_info_t);
    if (SUCCESS != return_code) {
        goto end;
    }
    if (0 != pthread_mutex_init(&args.peer_info_list_mutex, NULL)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        linked_list_destroy(args.peer_info_list);
        goto end;
    }
    args.print_progress = true;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    if (SUCCESS != pthread_cond_init(&args.exit_ready_cond, NULL)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    if (0 != pthread_mutex_init(&args.exit_ready_mutex, NULL)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    return_code_t *return_code_ptr = handle_peer_discovery_requests(&args);
    return_code = *return_code_ptr;
    free(return_code_ptr);
    linked_list_destroy(args.peer_info_list);
    pthread_mutex_destroy(&args.peer_info_list_mutex);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
end:
    return return_code;
}

int main(int argc, char **argv) {
    return_code_t return_code = SUCCESS;
    #ifdef _WIN32
        WSADATA wsaData;
        if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
    #endif
    if (2 != argc) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    uint16_t port = strtol(argv[1], NULL, 10);
    return_code = run_peer_discovery_bootstrap_server(port);
    #ifdef _WIN32
        WSACleanup();
    #endif
end:
    return return_code;
}
