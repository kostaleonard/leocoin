/**
 * @brief Contains functions for peer discovery.
 */

#ifndef INCLUDE_PEER_DISCOVERY_H_
#define INCLUDE_PEER_DISCOVERY_H_

#include <time.h>
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

/**
 * @brief Contains peer connection information.
 * 
 * @param listen_addr The peer's address. The peer is listening for connections
 * on this address.
 * @param last_connected The time at which the peer last connected to the
 * bootstrap server. The bootstrap server uses this information to filter out
 * inactive peers.
 */
typedef struct peer_info_t {
    struct sockaddr_in6 listen_addr;
    time_t last_connected;
} peer_info_t;

/**
 * @brief Compares the listen_addr of two peer_info_t structs.
 * 
 * @return int 0 if peer1 and peer2 have the same listen_addr, otherwise some
 * other number. This function does not compare the peers' last_connected
 * values.
 */
int compare_peer_info_t(void *peer1, void *peer2);

/**
 * @brief Serializes the peer info list into a buffer.
 * 
 * @param peer_info_list A linked_list_t containing peer_info_t structs.
 * @param buffer A pointer to fill with the bytes representing the list. Callers
 * must free the buffer.
 * @param buffer_size A pointer to fill with the final size of the buffer.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t peer_info_list_serialize(
    linked_list_t *peer_info_list,
    unsigned char **buffer,
    uint64_t *buffer_size
);

/**
 * @brief Deserializes a peer info list from the buffer.
 * 
 * @param peer_info_list A pointer to fill with the deserialized linked_list_t
 * containing peer_info_t structs. If the buffer contains a valid peer info list,
 * this pointer will be filled with data and the function will return SUCCESS.
 * @param buffer The buffer. The data in the buffer is in network byte order.
 * @param buffer_size The length of the buffer.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t peer_info_list_deserialize(
    linked_list_t **peer_info_list,
    unsigned char *buffer,
    uint64_t buffer_size
);

#endif  // INCLUDE_PEER_DISCOVERY_H_
