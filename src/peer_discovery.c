/**
 * @brief Contains functions for peer discovery.
 */

#include <stdio.h>
#include "include/peer_discovery.h"
#include "include/endian.h"

int compare_peer_info_t(void *peer1, void *peer2) {
    if (NULL == peer1 || NULL == peer2) {
        return 0;
    }
    peer_info_t *p1 = (peer_info_t *)peer1;
    peer_info_t *p2 = (peer_info_t *)peer2;
    return memcmp(
        &p1->listen_addr,
        &p2->listen_addr,
        sizeof(struct sockaddr_in6));
}

return_code_t peer_info_list_serialize(
    linked_list_t *peer_info_list,
    unsigned char **buffer,
    uint64_t *buffer_size
) {
    return_code_t return_code = SUCCESS;
    if (NULL == peer_info_list || NULL == buffer || NULL == buffer_size) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    uint64_t num_peers = 0;
    return_code = linked_list_length(peer_info_list, &num_peers);
    if (SUCCESS != return_code) {
        goto end;
    }
    uint64_t size = sizeof(num_peers);
    unsigned char *serialization_buffer = calloc(1, size);
    if (NULL == serialization_buffer) {
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    unsigned char *next_spot_in_buffer = serialization_buffer;
    *(uint64_t *)next_spot_in_buffer = htobe64(num_peers);
    next_spot_in_buffer += sizeof(num_peers);
    for (node_t *node = peer_info_list->head; NULL != node; node = node->next) {
        peer_info_t *peer = (peer_info_t *)node->data;
        uint64_t next_spot_in_buffer_offset = size;
        size += sizeof(peer->listen_addr.sin6_family);
        size += sizeof(peer->listen_addr.sin6_port);
        size += sizeof(peer->listen_addr.sin6_flowinfo);
        size += sizeof(peer->listen_addr.sin6_addr);
        size += sizeof(peer->listen_addr.sin6_scope_id);
        size += sizeof(peer->last_connected);
        serialization_buffer = realloc(serialization_buffer, size);
        if (NULL == serialization_buffer) {
            return_code = FAILURE_COULD_NOT_MALLOC;
            goto end;
        }
        next_spot_in_buffer = serialization_buffer + next_spot_in_buffer_offset;
        *(uint16_t *)next_spot_in_buffer = htons(peer->listen_addr.sin6_family);
        next_spot_in_buffer += sizeof(peer->listen_addr.sin6_family);
        *(uint16_t *)next_spot_in_buffer = htons(peer->listen_addr.sin6_port);
        next_spot_in_buffer += sizeof(peer->listen_addr.sin6_port);
        *(uint32_t *)next_spot_in_buffer = htonl(
            peer->listen_addr.sin6_flowinfo);
        next_spot_in_buffer += sizeof(peer->listen_addr.sin6_flowinfo);
        for (size_t idx = 0; idx < sizeof(IN6_ADDR); idx++) {
            *next_spot_in_buffer =
                ((unsigned char *)(&peer->listen_addr.sin6_addr))[idx];
            next_spot_in_buffer++;
        }
        *(uint32_t *)next_spot_in_buffer = htonl(
            peer->listen_addr.sin6_scope_id);
        next_spot_in_buffer += sizeof(peer->listen_addr.sin6_scope_id);
        *(uint64_t *)next_spot_in_buffer = htobe64(peer->last_connected);
        next_spot_in_buffer += sizeof(peer->last_connected);
    }
    *buffer = serialization_buffer;
    *buffer_size = size;
end:
    return return_code;
}

return_code_t peer_info_list_deserialize(
    linked_list_t **peer_info_list,
    unsigned char *buffer,
    uint64_t buffer_size
) {
    return_code_t return_code = SUCCESS;
    if (NULL == peer_info_list || NULL == buffer) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    unsigned char *next_spot_in_buffer = buffer;
    ptrdiff_t total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    uint64_t num_peers = betoh64(*(uint64_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(num_peers);
    linked_list_t *new_peer_info_list = NULL;
    return_code = linked_list_create(
        &new_peer_info_list, free, compare_peer_info_t);
    if (SUCCESS != return_code) {
        goto end;
    }
    for (uint64_t peer_idx = 0; peer_idx < num_peers; peer_idx++) {
        total_read_size = next_spot_in_buffer + sizeof(uint16_t) - buffer;
        if (total_read_size > buffer_size) {
            return_code = FAILURE_BUFFER_TOO_SMALL;
            linked_list_destroy(new_peer_info_list);
            goto end;
        }
        uint16_t sin6_family = ntohs(*(uint16_t *)next_spot_in_buffer);
        next_spot_in_buffer += sizeof(sin6_family);
        total_read_size = next_spot_in_buffer + sizeof(uint16_t) - buffer;
        if (total_read_size > buffer_size) {
            return_code = FAILURE_BUFFER_TOO_SMALL;
            linked_list_destroy(new_peer_info_list);
            goto end;
        }
        uint16_t sin6_port = ntohs(*(uint16_t *)next_spot_in_buffer);
        next_spot_in_buffer += sizeof(sin6_port);
        total_read_size = next_spot_in_buffer + sizeof(uint32_t) - buffer;
        if (total_read_size > buffer_size) {
            return_code = FAILURE_BUFFER_TOO_SMALL;
            linked_list_destroy(new_peer_info_list);
            goto end;
        }
        uint32_t sin6_flowinfo = ntohl(*(uint32_t *)next_spot_in_buffer);
        next_spot_in_buffer += sizeof(sin6_flowinfo);
        total_read_size = next_spot_in_buffer + sizeof(IN6_ADDR) - buffer;
        if (total_read_size > buffer_size) {
            return_code = FAILURE_BUFFER_TOO_SMALL;
            linked_list_destroy(new_peer_info_list);
            goto end;
        }
        struct in6_addr addr = {0};
        for (size_t idx = 0; idx < sizeof(IN6_ADDR); idx++) {
            ((unsigned char *)&addr)[idx] = *next_spot_in_buffer;
            next_spot_in_buffer++;
        }
        total_read_size = next_spot_in_buffer + sizeof(uint32_t) - buffer;
        if (total_read_size > buffer_size) {
            return_code = FAILURE_BUFFER_TOO_SMALL;
            linked_list_destroy(new_peer_info_list);
            goto end;
        }
        uint32_t sin6_scope_id = ntohl(*(uint32_t *)next_spot_in_buffer);
        next_spot_in_buffer += sizeof(sin6_scope_id);
        total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
        if (total_read_size > buffer_size) {
            return_code = FAILURE_BUFFER_TOO_SMALL;
            linked_list_destroy(new_peer_info_list);
            goto end;
        }
        uint64_t last_connected = betoh64(*(uint64_t *)next_spot_in_buffer);
        next_spot_in_buffer += sizeof(last_connected);
        peer_info_t *peer = malloc(sizeof(peer_info_t));
        if (NULL == peer) {
            return_code = FAILURE_COULD_NOT_MALLOC;
            linked_list_destroy(new_peer_info_list);
            goto end;
        }
        peer->listen_addr.sin6_family = sin6_family;
        peer->listen_addr.sin6_port = sin6_port;
        peer->listen_addr.sin6_flowinfo = sin6_flowinfo;
        memcpy(&peer->listen_addr.sin6_addr, &addr, sizeof(IN6_ADDR));
        peer->listen_addr.sin6_scope_id = sin6_scope_id;
        peer->last_connected = last_connected;
        return_code = linked_list_append(new_peer_info_list, peer);
        if (SUCCESS != return_code) {
            free(peer);
            linked_list_destroy(new_peer_info_list);
            goto end;
        }
    }
    *peer_info_list = new_peer_info_list;
end:
    return return_code;
}
