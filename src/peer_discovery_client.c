/**
 * @brief Runs the peer discovery client.
 */

// TODO this file is primarily for testing, it's not intended to be run standalone by actual users

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
        "-i [communication_interval_seconds]\n",
        program_name);
end:
}

int main(int argc, char **argv) {
    return_code_t return_code = SUCCESS;
    size_t num_positional_args = 3;
    if (argc < num_positional_args) {
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
    char *ipv6_address = argv[1];
    uint16_t port = strtol(argv[2], NULL, 10);
    discover_peers_args_t args = {0};
    if (inet_pton(
        AF_INET6,
        ipv6_address,
        &args.peer_discovery_bootstrap_server_addr.sin6_addr) != 1) {
        fprintf(stderr, "Invalid IPv6 address: %s\n", ipv6_address);
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    args.peer_discovery_bootstrap_server_addr.sin6_family = AF_INET6;
    args.peer_discovery_bootstrap_server_addr.sin6_port = htons(port);
    args.communication_interval_seconds = communication_interval_seconds;
    return_code = linked_list_create(
        &args.peer_info_list, free, compare_peer_info_t);
    if (SUCCESS != return_code) {
        goto end;
    }
    // TODO fill in args
    // TODO launch thread
end:
    return return_code;
}
