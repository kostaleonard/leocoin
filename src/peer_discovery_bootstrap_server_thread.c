#include <stdio.h>
#include "include/networking.h"
#include "include/peer_discovery.h"
#include "include/peer_discovery_bootstrap_server_thread.h"

return_code_t handle_one_peer_discovery_request(handle_peer_discovery_requests_args_t *args, int conn_fd) {
    return_code_t return_code = SUCCESS;
    char *recv_buf = calloc(sizeof(command_header_t), 1);
    if (NULL == recv_buf) {
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    return_code = recv_all(conn_fd, recv_buf, sizeof(command_header_t), 0);
    if (SUCCESS != return_code) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    command_header_t command_header = {0};
    command_register_peer_t command_register_peer = {0};
    return_code = command_header_deserialize(
        &command_header, (unsigned char *)recv_buf, sizeof(command_header_t));
    if (SUCCESS != return_code) {
        goto end;
    }
    if (COMMAND_REGISTER_PEER != command_header.command) {
        return_code = FAILURE_INVALID_COMMAND;
        goto end;
    }
    recv_buf = realloc(
        recv_buf, sizeof(command_header_t) + command_header.command_len);
    return_code = recv_all(
        conn_fd,
        recv_buf + sizeof(command_header_t),
        command_header.command_len,
        0);
    if (SUCCESS != return_code) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    return_code = command_register_peer_deserialize(
        &command_register_peer,
        (unsigned char *)recv_buf,
        sizeof(command_header_t) + command_header.command_len);
    if (SUCCESS != return_code) {
        goto end;
    }
    peer_info_t *peer_info = malloc(sizeof(peer_info_t));
    peer_info->listen_addr.sin6_family = command_register_peer.sin6_family;
    peer_info->listen_addr.sin6_port = command_register_peer.sin6_port;
    peer_info->listen_addr.sin6_flowinfo =
        command_register_peer.sin6_flowinfo;
    memcpy(
        &peer_info->listen_addr.sin6_addr,
        command_register_peer.addr,
        sizeof(IN6_ADDR));
    peer_info->listen_addr.sin6_scope_id =
        command_register_peer.sin6_scope_id;
    peer_info->last_connected = time(NULL);
    node_t *found_node = NULL;
    return_code = pthread_mutex_lock(&args->peer_info_list_mutex);
    if (SUCCESS != return_code) {
        free(peer_info);
        goto end;
    }
    return_code = linked_list_find(args->peer_info_list, peer_info, &found_node);
    if (SUCCESS != return_code) {
        free(peer_info);
        pthread_mutex_unlock(&args->peer_info_list_mutex);
        goto end;
    }
    if (NULL == found_node) {
        return_code = linked_list_prepend(args->peer_info_list, peer_info);
        if (SUCCESS != return_code) {
            free(peer_info);
            pthread_mutex_unlock(&args->peer_info_list_mutex);
            goto end;
        }
        if (args->print_progress) {
            char peer_hostname[NI_MAXHOST] = {0};
            if (0 != getnameinfo(
                (struct sockaddr *)&peer_info->listen_addr,
                sizeof(struct sockaddr_in6),
                peer_hostname,
                NI_MAXHOST,
                NULL,
                0,
                0)) {
                return_code = FAILURE_NETWORK_FUNCTION;
                free(peer_info);
                pthread_mutex_unlock(&args->peer_info_list_mutex);
                goto end;
            }
            char peer_addr_str[INET6_ADDRSTRLEN] = {0};
            if (NULL == inet_ntop(
                AF_INET6,
                &peer_info->listen_addr.sin6_addr,
                peer_addr_str,
                INET6_ADDRSTRLEN)) {
                return_code = FAILURE_NETWORK_FUNCTION;
                free(peer_info);
                pthread_mutex_unlock(&args->peer_info_list_mutex);
                goto end;
            }
            printf(
                "Server added peer %s (%s):%d\n",
                peer_hostname,
                peer_addr_str,
                htons(peer_info->listen_addr.sin6_port));
        }
    } else {
        peer_info_t *found_peer_info = (peer_info_t *)found_node->data;
        found_peer_info->last_connected = peer_info->last_connected;
        if (args->print_progress) {
            char peer_hostname[NI_MAXHOST] = {0};
            if (0 != getnameinfo(
                (struct sockaddr *)&peer_info->listen_addr,
                sizeof(struct sockaddr_in6),
                peer_hostname,
                NI_MAXHOST,
                NULL,
                0,
                0)) {
                return_code = FAILURE_NETWORK_FUNCTION;
                free(peer_info);
                pthread_mutex_unlock(&args->peer_info_list_mutex);
                goto end;
            }
            char peer_addr_str[INET6_ADDRSTRLEN] = {0};
            if (NULL == inet_ntop(
                AF_INET6,
                &peer_info->listen_addr.sin6_addr,
                peer_addr_str,
                INET6_ADDRSTRLEN)) {
                return_code = FAILURE_NETWORK_FUNCTION;
                free(peer_info);
                pthread_mutex_unlock(&args->peer_info_list_mutex);
                goto end;
            }
            printf(
                "Server updated peer %s (%s):%d\n",
                peer_hostname,
                peer_addr_str,
                htons(peer_info->listen_addr.sin6_port));
        }
        free(peer_info);
    }
    // TODO filter out expired peers
    command_send_peer_list_t command_send_peer_list = {0};
    memcpy(
        command_send_peer_list.header.command_prefix,
        COMMAND_PREFIX,
        COMMAND_PREFIX_LEN);
    command_send_peer_list.header.command = COMMAND_SEND_PEER_LIST;
    return_code = peer_info_list_serialize(
        args->peer_info_list,
        &command_send_peer_list.peer_list_data,
        &command_send_peer_list.peer_list_data_len);
    if (SUCCESS != return_code) {
        pthread_mutex_unlock(&args->peer_info_list_mutex);
        goto end;
    }
    return_code = pthread_mutex_unlock(&args->peer_info_list_mutex);
    if (SUCCESS != return_code) {
        goto end;
    }
    unsigned char *command_send_peer_list_buffer = NULL;
    uint64_t command_send_peer_list_buffer_len = 0;
    return_code = command_send_peer_list_serialize(
        &command_send_peer_list,
        &command_send_peer_list_buffer,
        &command_send_peer_list_buffer_len);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = send_all(
        conn_fd,
        (char *)command_send_peer_list_buffer,
        command_send_peer_list_buffer_len,
        0);
    if (SUCCESS != return_code) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    free(command_send_peer_list.peer_list_data);
    free(command_send_peer_list_buffer);
    free(recv_buf);
end:
    return return_code;
}

return_code_t *handle_peer_discovery_requests(handle_peer_discovery_requests_args_t *args) {
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
        listen_fd, (struct sockaddr *)&args->peer_discovery_bootstrap_server_addr, sizeof(args->peer_discovery_bootstrap_server_addr)) < 0) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    if (listen(listen_fd, LISTEN_BACKLOG) < 0) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    bool should_stop = *args->should_stop;
    while (!should_stop) {
        // TODO have a timeout in here so we can check should_stop regularly?
        struct sockaddr_in6 client_addr = {0};
        socklen_t client_len = sizeof(client_addr);
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
        return_code = handle_one_peer_discovery_request(args, conn_fd);
        if (SUCCESS != return_code && args->print_progress) {
            printf("Error in peer discovery bootstrap server; retrying\n");
        }
        # ifdef _WIN32
            closesocket(conn_fd);
        # else
            close(conn_fd);
        # endif
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

void *handle_peer_discovery_requests_pthread_wrapper(void *args) {
    handle_peer_discovery_requests_args_t *server_args = (handle_peer_discovery_requests_args_t *)args;
    return_code_t *return_code_ptr = handle_peer_discovery_requests(server_args);
    return (void *)return_code_ptr;
}
