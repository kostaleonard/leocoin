/**
 * @brief Runs the peer discovery client.
 */

// TODO this file is primarily for testing, it's not intended to be run standalone by actual users

#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include "include/peer_discovery.h"
#include "include/peer_discovery_thread.h"
#include "include/return_codes.h"

void print_usage_statement(char *program_name) {
    if (NULL == program_name) {
        goto end;
    }
    fprintf(
        stderr,
        "Usage: %s "
        "[server_ipv6_address] "
        "[server_port] "
        "[peer_ipv6_address] "
        "[peer_port] "
        "-i [communication_interval_seconds]\n",
        program_name);
end:
}

int main(int argc, char **argv) {
    return_code_t return_code = SUCCESS;
    size_t num_positional_args = 4;
    if (argc < num_positional_args + 1) {
        print_usage_statement(argv[0]);
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    uint64_t communication_interval_seconds = 
        DEFAULT_COMMUNICATION_INTERVAL_SECONDS;
    int opt;
    while ((opt = getopt(
        argc - num_positional_args, argv + num_positional_args, "i:")) != -1) {
        switch (opt) {
            case 'i':
                communication_interval_seconds = strtol(optarg, NULL, 10);
                break;
            default:
                print_usage_statement(argv[0]);
                return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
                goto end;
        }
    }
    char *server_ipv6_address = argv[1];
    uint16_t server_port = strtol(argv[2], NULL, 10);
    char *peer_ipv6_address = argv[3];
    uint16_t peer_port = strtol(argv[4], NULL, 10);
    discover_peers_args_t args = {0};
    if (inet_pton(
        AF_INET6,
        server_ipv6_address,
        &args.peer_discovery_bootstrap_server_addr.sin6_addr) != 1) {
        fprintf(stderr, "Invalid server IPv6 address: %s\n", server_ipv6_address);
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    args.peer_discovery_bootstrap_server_addr.sin6_family = AF_INET6;
    args.peer_discovery_bootstrap_server_addr.sin6_port = htons(server_port);
     if (inet_pton(
        AF_INET6,
        peer_ipv6_address,
        &args.peer_addr.sin6_addr) != 1) {
        fprintf(stderr, "Invalid peer IPv6 address: %s\n", peer_ipv6_address);
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    args.peer_addr.sin6_family = AF_INET6;
    args.peer_addr.sin6_port = htons(peer_port);
    args.communication_interval_seconds = communication_interval_seconds;
    return_code = linked_list_create(
        &args.peer_info_list, free, compare_peer_info_t);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = pthread_mutex_init(&args.peer_info_list_mutex, NULL);
    if (SUCCESS != return_code) {
        linked_list_destroy(args.peer_info_list);
        goto end;
    }
    args.print_progress = true;
    atomic_bool should_stop = false;
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    return_code = pthread_cond_init(&args.exit_ready_cond, NULL);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = pthread_mutex_init(&args.exit_ready_mutex, NULL);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code_t *return_code_ptr = discover_peers(&args);
    return_code = *return_code_ptr;
    free(return_code_ptr);
    linked_list_destroy(args.peer_info_list);
    pthread_mutex_destroy(&args.peer_info_list_mutex);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
end:
    return return_code;
}
