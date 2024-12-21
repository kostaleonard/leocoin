/**
 * @brief Contains networking functions.
 */

#ifndef INCLUDE_NETWORKING_H_
#define INCLUDE_NETWORKING_H_
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
#include <stdint.h>
#include "include/return_codes.h"

#define COMMAND_PREFIX "LEO\0"
#define COMMAND_PREFIX_LEN 4
#define COMMAND_ERROR_MESSAGE_LEN 256
#define COMMAND_HEADER_INITIALIZER {{'L', 'E', 'O', '\0'}, 0, 0}
// TODO remove COMMAND_REGISTER_PEER_LEN? We can't tell the length for all of the commands in advance, so this could cause confusion
#define COMMAND_REGISTER_PEER_LEN (sizeof(command_register_peer_t) - sizeof(command_header_t))

typedef enum command_t {
    COMMAND_OK,
    COMMAND_ERROR,
    COMMAND_REGISTER_PEER,
    COMMAND_SEND_PEER_LIST,
} command_t;

// TODO docstrings for data structures

typedef struct command_header_t {
    char command_prefix[COMMAND_PREFIX_LEN];
    uint32_t command;
    uint64_t command_len; // TODO this does not include the header length.
} command_header_t;

typedef struct command_ok_t {
    command_header_t header;
} command_ok_t;

typedef struct command_error_t {
    command_header_t header;
    char message[COMMAND_ERROR_MESSAGE_LEN];
} command_error_t;

typedef struct command_register_peer_t {
    command_header_t header;
    uint16_t sin6_family;
    uint16_t sin6_port;
    uint32_t sin6_flowinfo;
    unsigned char addr[sizeof(IN6_ADDR)];
    uint32_t sin6_scope_id;
} command_register_peer_t;

typedef struct command_send_peer_list_t {
    command_header_t header;
    uint64_t peer_list_data_len;
    unsigned char *peer_list_data;
} command_send_peer_list_t;

// TODO order these functions based on the command_t enum.
// TODO check return_codes.h for which error codes are needed to communicate what went wrong
// TODO send_all and recv_all functions for handling partial reads/writes
// TODO fix/add docstrings

// TODO docstring
return_code_t command_header_serialize(
    command_header_t *command_header,
    unsigned char **buffer,
    uint64_t *buffer_size);

/**
 * @brief Deserializes a command header from the buffer.
 * 
 * @param command_header A pointer to fill with the deserialized command header.
 * If the buffer contains a valid command header, this pointer will be filled
 * with data and the function will return SUCCESS.
 * @param buffer The buffer. The data in the buffer is in network byte order.
 * @param buffer_size The length of the buffer.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t command_header_deserialize(
    command_header_t *command_header,
    unsigned char *buffer,
    uint64_t buffer_size);

// TODO docstring
// TODO note that these serialization functions will set the command_len field in the struct because it may not be known in advance
return_code_t command_register_peer_serialize(
    command_register_peer_t *command_register_peer,
    unsigned char **buffer,
    uint64_t *buffer_size);

/**
 * @brief Deserializes a register peer command from the buffer.
 * 
 * @param command_register_peer A pointer to fill with the deserialized register
 * peer command data. If the buffer contains a valid register peer command, this
 * pointer will be filled with data and the function will return SUCCESS.
 * @param buffer The buffer. The data in the buffer is in network byte order.
 * @param buffer_size The length of the buffer.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t command_register_peer_deserialize(
    command_register_peer_t *command_register_peer,
    unsigned char *buffer,
    uint64_t buffer_size);

/**
 * @brief Serializes the send peer list command into a newly allocated buffer.
 * 
 * @param command_send_peer_list The send peer list command to serialize.
 * @param buffer A pointer to fill with a buffer that this function will allocate.
 * The buffer will contain the serialized command.
 * @param buffer_size A pointer to fill with the length of the allocated buffer.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t command_send_peer_list_serialize(
    command_send_peer_list_t *command_send_peer_list,
    unsigned char **buffer,
    uint64_t *buffer_size);

// TODO docstring
return_code_t command_send_peer_list_deserialize(
    command_send_peer_list_t *command_send_peer_list,
    unsigned char *buffer,
    uint64_t buffer_size);

// TODO not used yet
/*
return_code_t command_error_serialize(
    command_error_t *command_error,
    unsigned char **buffer,
    uint64_t *buffer_size);
*/

#endif  // INCLUDE_NETWORKING_H_
