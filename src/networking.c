/**
 * @brief Contains networking functions.
 */

#include <stdio.h>
#include "include/endian.h"
#include "include/networking.h"

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
    ptrdiff_t total_read_size = next_spot_in_buffer + COMMAND_PREFIX_LEN - buffer;
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
    deserialized_command_header.command = ntohl(*(uint32_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(uint32_t);
    total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    deserialized_command_header.command_len = betoh64(*(uint64_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(uint64_t);
    memcpy(command_header, &deserialized_command_header, sizeof(command_header_t));
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
    unsigned char *next_spot_in_buffer = buffer + sizeof(command_header_t);
    ptrdiff_t total_read_size = next_spot_in_buffer + sizeof(uint16_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    // TODO check for invalid deserialized command?
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
    // TODO serialize list
    // TODO once we know the size, set command_send_peer_list->header->command_len to the right length, serialize the header, and combine the buffers
    return FAILURE_INVALID_INPUT;
}

return_code_t command_send_peer_list_deserialize(
    command_send_peer_list_t *command_send_peer_list,
    unsigned char *buffer,
    uint64_t buffer_size) {
    return FAILURE_INVALID_INPUT;
}
