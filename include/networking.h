/**
 * @brief Contains networking functions.
 */

#ifndef INCLUDE_NETWORKING_H_
#define INCLUDE_NETWORKING_H_
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int (*recv_func_t)(SOCKET, char *, int, int);
#else
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
    #ifndef IN6_ADDR
        typedef struct in6_addr IN6_ADDR;
    #endif
    typedef ssize_t (*recv_func_t)(int, void *, ssize_t, int);
#endif
#include <stdint.h>
#include "include/return_codes.h"

#define COMMAND_PREFIX "LEO\0"
#define COMMAND_PREFIX_LEN 4
#define COMMAND_ERROR_MESSAGE_LEN 256
#define COMMAND_HEADER_INITIALIZER {{'L', 'E', 'O', '\0'}, 0, 0}

//TODO make sure everyone uses wrap_recv
// This function is for mocking in unit tests.
extern recv_func_t wrap_recv;

/**
 * @brief Represents valid command codes for network communication.
 */
typedef enum command_t {
    COMMAND_OK,
    COMMAND_ERROR,
    COMMAND_REGISTER_PEER,
    COMMAND_SEND_PEER_LIST,
} command_t;

/**
 * @brief The header with which every command begins.
 * 
 * @param command_prefix A short string used to signal to the recipient that the
 * message is using the LeoCoin network communication protocol.
 * @param command The command code, which indicates the content of the message.
 * @param command_len The length of the payload, in bytes. This length excludes
 * the header itself. The header data structure is embedded in actual command
 * data structures, which have additional contents (the payload). Every command
 * message begins with the fixed-size command header, so the recipient knows
 * that they will receive the header bytes. This field in the header indicates
 * how many additional bytes the recipient must be prepared to receive. The
 * command serialization functions usually fill in this field since it may not
 * be known in advance.
 */
typedef struct command_header_t {
    char command_prefix[COMMAND_PREFIX_LEN];
    uint32_t command;
    uint64_t command_len;
} command_header_t;

/**
 * @brief Represents "acknowledge" or "success."
 */
typedef struct command_ok_t {
    command_header_t header;
} command_ok_t;

/**
 * @brief Contains an error message.
 */
typedef struct command_error_t {
    command_header_t header;
    char message[COMMAND_ERROR_MESSAGE_LEN];
} command_error_t;

/**
 * @brief Requests that the recipient register a new peer.
 */
typedef struct command_register_peer_t {
    command_header_t header;
    uint16_t sin6_family;
    uint16_t sin6_port;
    uint32_t sin6_flowinfo;
    unsigned char addr[sizeof(IN6_ADDR)];
    uint32_t sin6_scope_id;
} command_register_peer_t;

/**
 * @brief Contains the serialized peer list.
 * 
 * @param header The command header.
 * @param peer_list_data_len The number of bytes in peer_list_data.
 * @param peer_list_data The serialized linked list of peer_info_t data
 * structures.
 */
typedef struct command_send_peer_list_t {
    command_header_t header;
    uint64_t peer_list_data_len;
    unsigned char *peer_list_data;
} command_send_peer_list_t;

/**
 * @brief Serializes the header into a buffer.
 * 
 * @param command_header The header.
 * @param buffer A pointer to fill with the bytes representing the header.
 * Callers must free the buffer.
 * @param buffer_size A pointer to fill with the final size of the buffer.
 * @return return_code_t A return code indicating success or failure.
 */
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

/**
 * @brief Serializes the register peer command into a buffer.
 * 
 * @param command_register_peer The command. This function will set the
 * command_len field in the command's header.
 * @param buffer A pointer to fill with the bytes representing the command.
 * Callers must free the buffer.
 * @param buffer_size A pointer to fill with the final size of the buffer.
 * @return return_code_t A return code indicating success or failure.
 */
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
 * @brief Serializes the send peer list command into a buffer.
 * 
 * @param command_send_peer_list The command. This function will set the
 * command_len field in the command's header.
 * @param buffer A pointer to fill with the bytes representing the command.
 * Callers must free the buffer.
 * @param buffer_size A pointer to fill with the final size of the buffer.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t command_send_peer_list_serialize(
    command_send_peer_list_t *command_send_peer_list,
    unsigned char **buffer,
    uint64_t *buffer_size);

/**
 * @brief Deserializes a send peer list command from the buffer.
 * 
 * @param command_send_peer_list A pointer to fill with the deserialized send
 * peer list command data. If the buffer contains a valid send peer list
 * command, this pointer will be filled with data and the function will return
 * SUCCESS.
 * @param buffer The buffer. The data in the buffer is in network byte order.
 * @param buffer_size The length of the buffer.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t command_send_peer_list_deserialize(
    command_send_peer_list_t *command_send_peer_list,
    unsigned char *buffer,
    uint64_t buffer_size);

/**
 * @brief Receives exactly len bytes from sockfd into buf.
 * 
 * @param sockfd The socket from which to read.
 * @param buf The buffer to fill with data.
 * @param len The exact number of bytes to receive. This function will block
 * until it receives exactly this many bytes. Callers should set a timeout on
 * their sockets to prevent blocking indefinitely in the case that the sender
 * does not or cannot transmit a complete message.
 * @param flags Receive flags.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t recv_all(int sockfd, void *buf, size_t len, int flags);

/**
 * @brief Sends exactly len bytes from buf through sockfd.
 * 
 * @param sockfd The socket to which to send.
 * @param buf The buffer containing the data to send.
 * @param len The exact number of bytes to send. This function may block while
 * sending the data. Callers should consider setting a timeout on their sockets.
 * @param flags Send flags.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t send_all(int sockfd, void *buf, size_t len, int flags);

#endif  // INCLUDE_NETWORKING_H_
