// TODO registers with peer discovery server and maintains peer list.

#include <stdio.h>
#include "include/networking.h"
#include "include/peer_discovery_thread.h"
#include "include/sleep.h"

return_code_t *discover_peers(discover_peers_args_t *args) {
    return_code_t return_code = SUCCESS;
    #ifdef _WIN32
        WSADATA wsaData;
        if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
    #endif
    bool should_stop = *args->should_stop;
    while (!should_stop) {
        if (args->print_progress) {
            printf("Attempting to connect to peer discovery server.\n");
        }
        int client_fd = socket(AF_INET6, SOCK_STREAM, 0);
        if (client_fd < 0) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
        if (SUCCESS != connect(
            client_fd,
            (struct sockaddr *)&args->peer_discovery_bootstrap_server_addr,
            sizeof(struct sockaddr_in6))) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
        command_header_t command_header = COMMAND_HEADER_INITIALIZER;
        command_header.command = COMMAND_REGISTER_PEER;
        command_register_peer_t command_register_peer = {0};
        command_register_peer.header = command_header;
        command_register_peer.sin6_family = args->peer_addr.sin6_family;
        command_register_peer.sin6_port = args->peer_addr.sin6_port;
        command_register_peer.sin6_flowinfo = args->peer_addr.sin6_flowinfo;
        memcpy(&command_register_peer.addr, &args->peer_addr.sin6_addr, sizeof(IN6_ADDR));
        command_register_peer.sin6_scope_id = args->peer_addr.sin6_scope_id;
        unsigned char *send_buffer = NULL;
        uint64_t send_buffer_size = 0;
        command_register_peer_serialize(&command_register_peer, &send_buffer, &send_buffer_size);
        send(client_fd, (char *)send_buffer, send_buffer_size, 0);
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
    // TODO cleanup open socket
    pthread_mutex_lock(&args->exit_ready_mutex);
    *args->exit_ready = true;
    pthread_cond_signal(&args->exit_ready_cond);
    pthread_mutex_unlock(&args->exit_ready_mutex);
end:
    return_code_t *return_code_ptr = malloc(sizeof(return_code_t));
    *return_code_ptr = return_code;
    return return_code_ptr;
}

void *discover_peers_pthread_wrapper(void *args) {
    discover_peers_args_t *discover_peers_args = (discover_peers_args_t *)args;
    return_code_t *return_code_ptr = discover_peers(discover_peers_args);
    return (void *)return_code_ptr;
}
