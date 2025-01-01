/**
 * @brief Runs the peer discovery bootstrap server.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif
#include "include/linked_list.h"
#include "include/networking.h"
#include "include/peer_discovery.h"
#include "include/return_codes.h"

#define LISTEN_BACKLOG 5

int main(int argc, char **argv) {
    return_code_t return_code = SUCCESS;
    printf("Hello peer discovery server\n");
    // TODO accepts connections, adds IP info to peer list, sends current peer list to peer that just connected, closes connection with peer
    // TODO IP information in peer list expires after 60 seconds, and peers are responsible for reconnecting with the server every now and then to refresh.
    // TODO just before sending the current peer list, filter out any expired peer addresses.
    if (2 != argc) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    linked_list_t *peer_list = NULL;
    return_code = linked_list_create(&peer_list, free, compare_peer_info_t);
    if (SUCCESS != return_code) {
        goto end;
    }
    int port = strtol(argv[1], NULL, 10);
    #ifdef _WIN32
        WSADATA wsaData;
        if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
    #endif
    struct sockaddr_in6 server_addr = {0};
    struct sockaddr_in6 client_addr = {0};
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
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons((unsigned short)port);
    if (bind(
        listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }
    if (listen(listen_fd, LISTEN_BACKLOG) < 0) {
        return_code = FAILURE_NETWORK_FUNCTION;
        goto end;
    }    
    while (true) {
        socklen_t client_len = sizeof(client_addr);
        int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        if (conn_fd < 0) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
        if (sizeof(client_addr) != client_len) {
            return_code = FAILURE_BUFFER_TOO_SMALL;
            goto end;
        }
        char client_hostname[NI_MAXHOST] = {0};
        if (0 != getnameinfo((struct sockaddr *)&client_addr, client_len, client_hostname, NI_MAXHOST, NULL, 0, 0)) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
        char client_addr_str[INET6_ADDRSTRLEN] = {0};
        if (NULL == inet_ntop(AF_INET6, &client_addr.sin6_addr, client_addr_str, INET6_ADDRSTRLEN)) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
        printf("Server established connection with %s (%s)\n", client_hostname, client_addr_str);
        char recv_buf[BUFSIZ] = {0};
        // TODO use send_buf or remove
        //char send_buf[BUFSIZ] = {0};
        // TODO need timeout or new thread to prevent resources getting taken up
        int bytes_received = recv(conn_fd, recv_buf, sizeof(command_header_t), 0);
        if (bytes_received < 0) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
        printf("Server received %d bytes: %s\n", bytes_received, recv_buf);
        command_header_t command_header = {0};
        command_register_peer_t command_register_peer = {0};
        // TODO use error command to send info back to client
        //command_error_t command_error = {0};
        // TODO don't need this because we know we're expecting a peer list command and it includes header deserialization.
        return_code = command_header_deserialize(
            &command_header, (unsigned char *)recv_buf, bytes_received);
        if (SUCCESS != return_code) {
            // TODO send error command based on return code--probably just FAILURE_INVALID_COMMAND_PREFIX
            printf("Header deserialization error\n");
        }
        if (COMMAND_REGISTER_PEER != command_header.command) {
            // TODO send error command that the server expected a register peer command
            printf("Expected register peer command\n");
        }
        // TODO handle partial read/write based on length of command
        // TODO malloc/realloc as the message gets filled in to handle messages longer than BUFSIZ?
        // TODO be careful of buffer overflow on this line because the user could specify a command_len greater than BUFSIZ--just malloc for size of command_len plus header, memcpy the header from recv_buf, or alloc/realloc for recv_buf and free at end
        bytes_received = recv(conn_fd, recv_buf + sizeof(command_header_t), command_header.command_len, 0);
        if (bytes_received < 0) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
        printf("Server received %d bytes: %s\n", bytes_received, recv_buf);
        return_code = command_register_peer_deserialize(
            &command_register_peer,
            (unsigned char *)recv_buf,
            bytes_received);
        // TODO send back peer list or error message
        if (SUCCESS != return_code) {
            // TODO send error message that the server couldn't deserialize the register peer command
            printf("Register peer deserialization error\n");   
        }
        peer_info_t *peer_info = malloc(sizeof(peer_info_t));
        peer_info->listen_addr.sin6_family = command_register_peer.sin6_family;
        peer_info->listen_addr.sin6_port = command_register_peer.sin6_port;
        peer_info->listen_addr.sin6_flowinfo = command_register_peer.sin6_flowinfo;
        memcpy(&peer_info->listen_addr.sin6_addr, command_register_peer.addr, sizeof(IN6_ADDR));
        peer_info->listen_addr.sin6_scope_id = command_register_peer.sin6_scope_id;
        peer_info->last_connected = time(NULL);
        node_t *found_node = NULL;
        return_code = linked_list_find(peer_list, peer_info, &found_node);
        // TODO this error handling is sketchy
        if (SUCCESS != return_code) {
            printf("linked_list_find error\n");
            free(peer_info);
            goto end;
        }
        if (NULL == found_node) {
            return_code = linked_list_prepend(peer_list, peer_info);
            if (SUCCESS != return_code) {
                printf("linked_list_prepend error\n");
                free(peer_info);
                goto end;
            }
        } else {
            peer_info_t *found_peer_info = (peer_info_t *)found_node->data;
            found_peer_info->last_connected = peer_info->last_connected;
            free(peer_info);
        }
        // TODO filter peer list for last_connected > 60 seconds old
        command_send_peer_list_t command_send_peer_list = {0};
        // TODO can use the header initializer
        memcpy(command_send_peer_list.header.command_prefix, COMMAND_PREFIX, COMMAND_PREFIX_LEN);
        command_send_peer_list.header.command = COMMAND_SEND_PEER_LIST;
        return_code = peer_info_list_serialize(peer_list, &command_send_peer_list.peer_list_data, &command_send_peer_list.peer_list_data_len);
        if (SUCCESS != return_code) {
            // TODO
            printf("peer_info_list_serialize error\n");
            goto end;
        }
        unsigned char *command_send_peer_list_buffer = NULL;
        uint64_t command_send_peer_list_buffer_len = 0;
        return_code = command_send_peer_list_serialize(&command_send_peer_list, &command_send_peer_list_buffer, &command_send_peer_list_buffer_len);
        if (SUCCESS != return_code) {
            // TODO
            printf("command_send_peer_list_serialize error\n");
            goto end;
        }
        // TODO handle partial read/write
        int bytes_sent = send(conn_fd, (char *)command_send_peer_list_buffer, command_send_peer_list_buffer_len, 0);
        if (bytes_sent < 0) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
        free(command_send_peer_list.peer_list_data);
        free(command_send_peer_list_buffer);
        # ifdef _WIN32
            closesocket(conn_fd);
        # else
            close(conn_fd);
        # endif
    }
    #ifdef _WIN32
        closesocket(listen_fd);
        WSACleanup();
    # else
        close(listen_fd);
    #endif
end:
    return return_code;
}
