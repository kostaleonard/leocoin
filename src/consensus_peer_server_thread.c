#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/networking.h"
#include "include/consensus_peer_server_thread.h"

#define LISTEN_BACKLOG 5
#define POLL_TIMEOUT_MILLISECONDS 100

return_code_t handle_one_consensus_request(run_consensus_peer_server_args_t *args, int conn_fd) {
    // TODO receive command send blockchain
    // TODO make sure peer chain is valid and has same number of leading zeros required
    // TODO update to longest chain
    // TODO send back command send blockchain with new longest chain
    return FAILURE_INVALID_INPUT;
}
 
return_code_t *run_consensus_peer_server(
    run_consensus_peer_server_args_t *args) {
    return_code_t return_code = SUCCESS;
    int listen_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    int optval = 1;
    if (0 != setsockopt(
        listen_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        (const void *)&optval,
        sizeof(optval))) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    int ipv6_v6only = 1;
    if (0 != setsockopt(
        listen_fd,
        IPPROTO_IPV6,
        IPV6_V6ONLY,
        (const void *)&ipv6_v6only,
        sizeof(ipv6_v6only))) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    if (bind(
        listen_fd,
        (struct sockaddr *)&args->consensus_peer_server_addr,
        sizeof(args->consensus_peer_server_addr)) < 0) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    if (listen(listen_fd, LISTEN_BACKLOG) < 0) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    bool should_stop = *args->should_stop;
    while (!should_stop) {
        struct sockaddr_in6 client_addr = {0};
        socklen_t client_len = sizeof(client_addr);
        #ifdef _WIN32
            WSAPOLLFD fds;
            fds.fd = listen_fd;
            fds.events = POLLRDNORM;
            int retval = WSAPoll(&fds, 1, POLL_TIMEOUT_MILLISECONDS);
        #else
            struct pollfd fds;
            fds.fd = listen_fd;
            fds.events = POLLIN;
            int retval = poll(&fds, 1, POLL_TIMEOUT_MILLISECONDS);
        #endif
        if (-1 == retval) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        } else if (0 != retval) {
            int conn_fd = accept(
                listen_fd, (struct sockaddr *)&client_addr, &client_len);
            if (conn_fd < 0) {
                return_code = FAILURE_NETWORK_FUNCTION;
                goto end;
            }
            if (sizeof(client_addr) != client_len) {
                return_code = FAILURE_BUFFER_TOO_SMALL;
                goto end;
            }
            if (args->print_progress) {
                char client_hostname[NI_MAXHOST] = {0};
                if (0 != getnameinfo(
                    (struct sockaddr *)&client_addr,
                    client_len,
                    client_hostname,
                    NI_MAXHOST,
                    NULL,
                    0,
                    0)) {
                    return_code = FAILURE_NETWORK_FUNCTION;
                    goto end;
                }
                char client_addr_str[INET6_ADDRSTRLEN] = {0};
                if (NULL == inet_ntop(
                    AF_INET6,
                    &client_addr.sin6_addr,
                    client_addr_str,
                    INET6_ADDRSTRLEN)) {
                    return_code = FAILURE_NETWORK_FUNCTION;
                    goto end;
                }
                printf(
                    "Server established connection with %s (%s)\n",
                    client_hostname,
                    client_addr_str);
            }
            return_code = handle_one_consensus_request(args, conn_fd);
            if (SUCCESS != return_code && args->print_progress) {
                printf("Error handling peer consensus request; retrying\n");
            }
            # ifdef _WIN32
                closesocket(conn_fd);
            # else
                close(conn_fd);
            # endif
        }
        should_stop = *args->should_stop;
    }
end:
    pthread_mutex_lock(&args->exit_ready_mutex);
    *args->exit_ready = true;
    pthread_cond_signal(&args->exit_ready_cond);
    pthread_mutex_unlock(&args->exit_ready_mutex);
    return_code_t *return_code_ptr = malloc(sizeof(return_code_t));
    *return_code_ptr = return_code;
    return return_code_ptr;
}

void *run_consensus_peer_server_pthread_wrapper(void *args) {
    run_consensus_peer_server_args_t *server_args =
        (run_consensus_peer_server_args_t *)args;
    return_code_t *return_code_ptr = run_consensus_peer_server(server_args);
    return (void *)return_code_ptr;
}
