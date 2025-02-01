/**
 * @brief Contains networking functions.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/endian.h"
#include "include/networking.h"

recv_func_t wrap_recv = recv;

return_code_t command_header_serialize(
    command_header_t *command_header,
    unsigned char **buffer,
    uint64_t *buffer_size) {
    return_code_t return_code = SUCCESS;
    if (NULL == command_header || NULL == buffer || NULL == buffer_size) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    uint64_t size = sizeof(command_header_t);
    unsigned char *serialization_buffer = calloc(1, size);
    if (NULL == serialization_buffer) {
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    unsigned char *next_spot_in_buffer = serialization_buffer;
    for (size_t idx = 0; idx < COMMAND_PREFIX_LEN; idx++) {
        if (command_header->command_prefix[idx] != COMMAND_PREFIX[idx]) {
            return_code = FAILURE_INVALID_COMMAND_PREFIX;
            free(serialization_buffer);
            goto end;
        }
        *next_spot_in_buffer = command_header->command_prefix[idx];
        next_spot_in_buffer++;
    }
    *(uint32_t *)next_spot_in_buffer = htonl(command_header->command);
    next_spot_in_buffer += sizeof(command_header->command);
    *(uint64_t *)next_spot_in_buffer = htobe64(command_header->command_len);
    next_spot_in_buffer += sizeof(command_header->command_len);
    *buffer = serialization_buffer;
    *buffer_size = size;
end:
    return return_code;
}

return_code_t command_header_deserialize(
    command_header_t *command_header,
    unsigned char *buffer,
    uint64_t buffer_size) {
    return_code_t return_code = SUCCESS;
    if (NULL == command_header || NULL == buffer) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    command_header_t deserialized_command_header = {0};
    unsigned char *next_spot_in_buffer = buffer;
    ptrdiff_t total_read_size =
        next_spot_in_buffer + COMMAND_PREFIX_LEN - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    for (size_t idx = 0; idx < COMMAND_PREFIX_LEN; idx++) {
        if (*next_spot_in_buffer != COMMAND_PREFIX[idx]) {
            return_code = FAILURE_INVALID_COMMAND_PREFIX;
            goto end;
        }
        deserialized_command_header.command_prefix[idx] = *next_spot_in_buffer;
        next_spot_in_buffer++;
    }
    total_read_size = next_spot_in_buffer + sizeof(uint32_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    deserialized_command_header.command =
        ntohl(*(uint32_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(uint32_t);
    total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    deserialized_command_header.command_len =
        betoh64(*(uint64_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(uint64_t);
    memcpy(
        command_header,
        &deserialized_command_header,
        sizeof(command_header_t));
end:
    return return_code;
}

return_code_t command_register_peer_serialize(
    command_register_peer_t *command_register_peer,
    unsigned char **buffer,
    uint64_t *buffer_size) {
    return_code_t return_code = SUCCESS;
    if (NULL == command_register_peer ||
        NULL == buffer ||
        NULL == buffer_size) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    if (COMMAND_REGISTER_PEER != command_register_peer->header.command) {
        return_code = FAILURE_INVALID_COMMAND;
        goto end;
    }
    uint64_t register_peer_size =
        sizeof(command_register_peer_t) - sizeof(command_header_t);
    unsigned char *register_peer_buffer = calloc(1, register_peer_size);
    if (NULL == register_peer_buffer) {
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    unsigned char *next_spot_in_buffer = register_peer_buffer;
    *(uint16_t *)next_spot_in_buffer = htons(
        command_register_peer->sin6_family);
    next_spot_in_buffer += sizeof(uint16_t);
    *(uint16_t *)next_spot_in_buffer = htons(command_register_peer->sin6_port);
    next_spot_in_buffer += sizeof(uint16_t);
    *(uint32_t *)next_spot_in_buffer = htonl(
        command_register_peer->sin6_flowinfo);
    next_spot_in_buffer += sizeof(uint32_t);
    for (size_t idx = 0; idx < sizeof(IN6_ADDR); idx++) {
        *next_spot_in_buffer = command_register_peer->addr[idx];
        next_spot_in_buffer++;
    }
    *(uint32_t *)next_spot_in_buffer = htonl(
        command_register_peer->sin6_scope_id);
    next_spot_in_buffer += sizeof(uint32_t);
    command_register_peer->header.command_len = register_peer_size;
    unsigned char *header_buffer = NULL;
    uint64_t header_size = 0;
    return_code = command_header_serialize(
        &command_register_peer->header, &header_buffer, &header_size);
    if (SUCCESS != return_code) {
        free(register_peer_buffer);
        goto end;
    }
    uint64_t total_size = header_size + register_peer_size;
    unsigned char *total_buffer = calloc(1, total_size);
    if (NULL == total_buffer) {
        free(header_buffer);
        free(register_peer_buffer);
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    memcpy(total_buffer, header_buffer, header_size);
    memcpy(
        total_buffer + header_size, register_peer_buffer, register_peer_size);
    free(header_buffer);
    free(register_peer_buffer);
    *buffer = total_buffer;
    *buffer_size = total_size;
end:
    return return_code;
}

return_code_t command_register_peer_deserialize(
    command_register_peer_t *command_register_peer,
    unsigned char *buffer,
    uint64_t buffer_size) {
    return_code_t return_code = SUCCESS;
    if (NULL == command_register_peer || NULL == buffer) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    command_register_peer_t deserialized_command_register_peer = {0};
    return_code = command_header_deserialize(
        &deserialized_command_register_peer.header, buffer, buffer_size);
    if (SUCCESS != return_code) {
        goto end;
    }
    if (COMMAND_REGISTER_PEER !=
        deserialized_command_register_peer.header.command) {
        return_code = FAILURE_INVALID_COMMAND;
        goto end;
    }
    unsigned char *next_spot_in_buffer = buffer + sizeof(command_header_t);
    ptrdiff_t total_read_size = next_spot_in_buffer + sizeof(uint16_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    deserialized_command_register_peer.sin6_family = ntohs(
        *(uint16_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(uint16_t);
    total_read_size = next_spot_in_buffer + sizeof(uint16_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    deserialized_command_register_peer.sin6_port = ntohs(
        *(uint16_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(uint16_t);
    total_read_size = next_spot_in_buffer + sizeof(uint32_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    deserialized_command_register_peer.sin6_flowinfo = ntohl(
        *(uint32_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(uint32_t);
    total_read_size = next_spot_in_buffer + sizeof(IN6_ADDR) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    for (size_t idx = 0; idx < sizeof(IN6_ADDR); idx++) {
        deserialized_command_register_peer.addr[idx] = *next_spot_in_buffer;
        next_spot_in_buffer++;
    }
    total_read_size = next_spot_in_buffer + sizeof(uint32_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    deserialized_command_register_peer.sin6_scope_id = ntohl(
        *(uint32_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(uint32_t);
    memcpy(
        command_register_peer,
        &deserialized_command_register_peer,
        sizeof(command_register_peer_t));
end:
    return return_code;
}

return_code_t command_send_peer_list_serialize(
    command_send_peer_list_t *command_send_peer_list,
    unsigned char **buffer,
    uint64_t *buffer_size) {
    return_code_t return_code = SUCCESS;
    if (NULL == command_send_peer_list ||
        NULL == buffer ||
        NULL == buffer_size) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    if (COMMAND_SEND_PEER_LIST != command_send_peer_list->header.command) {
        return_code = FAILURE_INVALID_COMMAND;
        goto end;
    }
    uint64_t send_peer_list_size = sizeof(uint64_t);
    unsigned char *send_peer_list_buffer = calloc(1, send_peer_list_size);
    if (NULL == send_peer_list_buffer) {
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    unsigned char *next_spot_in_buffer = send_peer_list_buffer;
    *(uint64_t *)next_spot_in_buffer = htobe64(
        command_send_peer_list->peer_list_data_len);
    next_spot_in_buffer += sizeof(uint64_t);
    command_send_peer_list->header.command_len =
        send_peer_list_size + command_send_peer_list->peer_list_data_len;
    unsigned char *header_buffer = NULL;
    uint64_t header_size = 0;
    return_code = command_header_serialize(
        &command_send_peer_list->header, &header_buffer, &header_size);
    if (SUCCESS != return_code) {
        free(send_peer_list_buffer);
        goto end;
    }
    uint64_t total_size =
        header_size +
        send_peer_list_size +
        command_send_peer_list->peer_list_data_len;
    unsigned char *total_buffer = calloc(1, total_size);
    if (NULL == total_buffer) {
        free(header_buffer);
        free(send_peer_list_buffer);
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    memcpy(total_buffer, header_buffer, header_size);
    memcpy(
        total_buffer + header_size, send_peer_list_buffer, send_peer_list_size);
    memcpy(
        total_buffer + header_size + send_peer_list_size,
        command_send_peer_list->peer_list_data,
        command_send_peer_list->peer_list_data_len);
    free(header_buffer);
    free(send_peer_list_buffer);
    *buffer = total_buffer;
    *buffer_size = total_size;
end:
    return return_code;
}

return_code_t command_send_peer_list_deserialize(
    command_send_peer_list_t *command_send_peer_list,
    unsigned char *buffer,
    uint64_t buffer_size) {
    return_code_t return_code = SUCCESS;
    if (NULL == command_send_peer_list || NULL == buffer) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    command_send_peer_list_t deserialized_command_send_peer_list = {0};
    return_code = command_header_deserialize(
        &deserialized_command_send_peer_list.header, buffer, buffer_size);
    if (SUCCESS != return_code) {
        goto end;
    }
    if (COMMAND_SEND_PEER_LIST !=
        deserialized_command_send_peer_list.header.command) {
        return_code = FAILURE_INVALID_COMMAND;
        goto end;
    }
    unsigned char *next_spot_in_buffer = buffer + sizeof(command_header_t);
    ptrdiff_t total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    deserialized_command_send_peer_list.peer_list_data_len = betoh64(
        *(uint64_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(uint64_t);
    total_read_size += deserialized_command_send_peer_list.peer_list_data_len;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    deserialized_command_send_peer_list.peer_list_data = calloc(
        deserialized_command_send_peer_list.peer_list_data_len, 1);
    if (NULL == deserialized_command_send_peer_list.peer_list_data) {
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    for (
        uint64_t idx = 0;
        idx < deserialized_command_send_peer_list.peer_list_data_len;
        idx++) {
        deserialized_command_send_peer_list.peer_list_data[idx] =
            *next_spot_in_buffer;
        next_spot_in_buffer++;
    }
    memcpy(
        command_send_peer_list,
        &deserialized_command_send_peer_list,
        sizeof(command_send_peer_list_t));
end:
    return return_code;
}

return_code_t recv_all(int sockfd, void *buf, size_t len, int flags) {
    return_code_t return_code = SUCCESS;
    if (NULL == buf) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    size_t total_bytes_read = 0;
    while (total_bytes_read != len) {
        int bytes_read = wrap_recv(
            sockfd, buf + total_bytes_read, len - total_bytes_read, flags);
        if (bytes_read < 0) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
        total_bytes_read += bytes_read;
    }
    
end:
    return return_code;
}

return_code_t send_all(int sockfd, void *buf, size_t len, int flags) {
    return FAILURE_INVALID_INPUT;
}
