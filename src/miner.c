/**
 * @brief Runs the miner.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "include/base64.h"
#include "include/blockchain.h"
#include "include/block.h"
#include "include/consensus_peer_client_thread.h"
#include "include/consensus_peer_server_thread.h"
#include "include/mining_thread.h"
#include "include/peer_discovery.h"
#include "include/peer_discovery_thread.h"
#include "include/transaction.h"

#define NUM_LEADING_ZERO_BYTES_IN_BLOCK_HASH 3
#define PRIVATE_KEY_ENVIRONMENT_VARIABLE "LEOCOIN_PRIVATE_KEY"
#define PUBLIC_KEY_ENVIRONMENT_VARIABLE "LEOCOIN_PUBLIC_KEY"

void print_usage_statement(char *program_name) {
    if (NULL == program_name) {
        goto end;
    }
    fprintf(
        stderr,
        "Usage: %s "
        "[bind_ipv6_address] "
        "[bind_port] "
        "[peer_discovery_bootstrap_server_ipv6_address] "
        "[peer_discovery_bootstrap_server_port] "
        "-i [communication_interval_seconds] "
        "-p [private_key_file_base64_encoded_contents] "
        "-k [public_key_file_base64_encoded_contents]\n",
        program_name);
    // TODO more detailed usage instructions
    fprintf(
        stderr,
        "Or supply keys as environment variables %s and %s\n",
        PRIVATE_KEY_ENVIRONMENT_VARIABLE,
        PUBLIC_KEY_ENVIRONMENT_VARIABLE);
end:
}

int main(int argc, char **argv) {
    return_code_t return_code = SUCCESS;
    size_t num_positional_args = 4;
    if (argc < num_positional_args + 1) {
        print_usage_statement(argv[0]);
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    uint64_t communication_interval_seconds =
        DEFAULT_COMMUNICATION_INTERVAL_SECONDS;
    char *ssh_private_key_contents_base64 = NULL;
    char *ssh_public_key_contents_base64 = NULL;
    int opt;
    while ((opt = getopt(
        argc - num_positional_args,
        argv + num_positional_args,
        "i:p:k:")) != -1) {
        switch (opt) {
            case 'i':
                communication_interval_seconds = strtol(optarg, NULL, 10);
                break;
            case 'p':
                printf("Using private key from argv\n");
                ssh_private_key_contents_base64 = optarg;
                break;
            case 'k':
                printf("Using public key from argv\n");
                ssh_public_key_contents_base64 = optarg;
                break;
            default:
                print_usage_statement(argv[0]);
                return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
                goto end;
        }
    }
    if (NULL == ssh_private_key_contents_base64) {
        printf(
            "No private key found in argv, searching env for %s\n",
            PRIVATE_KEY_ENVIRONMENT_VARIABLE);
        ssh_private_key_contents_base64 = getenv(
            PRIVATE_KEY_ENVIRONMENT_VARIABLE);
    }
    if (NULL == ssh_public_key_contents_base64) {
        printf(
            "No public key found in argv, searching env for %s\n",
            PUBLIC_KEY_ENVIRONMENT_VARIABLE);
        ssh_public_key_contents_base64 = getenv(
            PUBLIC_KEY_ENVIRONMENT_VARIABLE);
    }
    if (NULL == ssh_private_key_contents_base64 ||
        NULL == ssh_public_key_contents_base64) {
        print_usage_statement(argv[0]);
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    #ifdef _WIN32
        WSADATA wsaData;
        if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
            return_code = FAILURE_NETWORK_FUNCTION;
            goto end;
        }
    #endif
    // TODO rename these
    char *peer_ipv6_address = argv[1];
    uint16_t peer_port = strtol(argv[2], NULL, 10);
    char *server_ipv6_address = argv[3];
    uint16_t server_port = strtol(argv[4], NULL, 10);
    discover_peers_args_t discover_peers_args = {0};
    if (inet_pton(
        AF_INET6,
        server_ipv6_address,
        &discover_peers_args.peer_discovery_bootstrap_server_addr.sin6_addr) != 1) {
        fprintf(
            stderr, "Invalid server IPv6 address: %s\n", server_ipv6_address);
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    discover_peers_args.peer_discovery_bootstrap_server_addr.sin6_family = AF_INET6;
    discover_peers_args.peer_discovery_bootstrap_server_addr.sin6_port = htons(server_port);
    if (inet_pton(
        AF_INET6,
        peer_ipv6_address,
        &discover_peers_args.peer_addr.sin6_addr) != 1) {
        fprintf(stderr, "Invalid bind IPv6 address: %s\n", peer_ipv6_address);
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    discover_peers_args.peer_addr.sin6_family = AF_INET6;
    discover_peers_args.peer_addr.sin6_port = htons(peer_port);
    discover_peers_args.communication_interval_microseconds =
        communication_interval_seconds * 1e6;
    return_code = linked_list_create(
        &discover_peers_args.peer_info_list, free, compare_peer_info_t);
    if (SUCCESS != return_code) {
        goto end;
    }
    if (0 != pthread_mutex_init(&discover_peers_args.peer_info_list_mutex, NULL)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        linked_list_destroy(discover_peers_args.peer_info_list);
        goto end;
    }
    discover_peers_args.print_progress = true;
    atomic_bool discover_peers_should_stop = false;
    discover_peers_args.should_stop = &discover_peers_should_stop;
    bool discover_peers_exit_ready = false;
    discover_peers_args.exit_ready = &discover_peers_exit_ready;
    if (SUCCESS != pthread_cond_init(&discover_peers_args.exit_ready_cond, NULL)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    if (0 != pthread_mutex_init(&discover_peers_args.exit_ready_mutex, NULL)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    ssh_key_t miner_public_key = {0};
    size_t public_key_decoded_length =
        (size_t)ceil(strlen(ssh_public_key_contents_base64) * 3 / 4) + 1;
    if (public_key_decoded_length > sizeof(miner_public_key.bytes)) {
        printf("Public key is too long\n");
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    return_code = base64_decode(
        ssh_public_key_contents_base64,
        strlen(ssh_public_key_contents_base64),
        miner_public_key.bytes);
    if (SUCCESS != return_code) {
        goto end;
    }
    ssh_key_t miner_private_key = {0};
    size_t private_key_decoded_length =
        (size_t)ceil(strlen(ssh_private_key_contents_base64) * 3 / 4) + 1;
    if (private_key_decoded_length > sizeof(miner_private_key.bytes)) {
        printf("Private key is too long\n");
        return_code = FAILURE_INVALID_COMMAND_LINE_ARGS;
        goto end;
    }
    return_code = base64_decode(
        ssh_private_key_contents_base64,
        strlen(ssh_private_key_contents_base64),
        miner_private_key.bytes);
    if (SUCCESS != return_code) {
        goto end;
    }
    printf("Using public key: %s\n", miner_public_key.bytes);
    blockchain_t *blockchain = NULL;
    block_t *genesis_block = NULL;
    return_code = blockchain_create(
        &blockchain, NUM_LEADING_ZERO_BYTES_IN_BLOCK_HASH);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = block_create_genesis_block(&genesis_block);
    if (SUCCESS != return_code) {
        blockchain_destroy(blockchain);
        goto end;
    }
    return_code = blockchain_add_block(blockchain, genesis_block);
    if (SUCCESS != return_code) {
        block_destroy(genesis_block);
        blockchain_destroy(blockchain);
        goto end;
    }
    blockchain_print(blockchain);
    synchronized_blockchain_t *sync = NULL;
    return_code = synchronized_blockchain_create(&sync, blockchain);
    if (SUCCESS != return_code) {
        blockchain_destroy(blockchain);
        goto end;
    }
    atomic_bool should_stop = false;
    mine_blocks_args_t mine_blocks_args = {0};
    mine_blocks_args.sync = sync;
    mine_blocks_args.miner_public_key = &miner_public_key;
    mine_blocks_args.miner_private_key = &miner_private_key;
    mine_blocks_args.print_progress = true;
    mine_blocks_args.outfile = "blockchain.bin";
    mine_blocks_args.should_stop = &should_stop;
    bool exit_ready = false;
    mine_blocks_args.exit_ready = &exit_ready;
    return_code = pthread_cond_init(&mine_blocks_args.exit_ready_cond, NULL);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = pthread_mutex_init(&mine_blocks_args.exit_ready_mutex, NULL);
    if (SUCCESS != return_code) {
        goto end;
    }
    atomic_size_t sync_version_currently_mined = atomic_load(&sync->version);
    mine_blocks_args.sync_version_currently_mined = &sync_version_currently_mined;
    return_code = pthread_cond_init(
        &mine_blocks_args.sync_version_currently_mined_cond, NULL);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = pthread_mutex_init(
        &mine_blocks_args.sync_version_currently_mined_mutex, NULL);
    if (SUCCESS != return_code) {
        goto end;
    }
    run_consensus_peer_server_args_t run_consensus_peer_server_args = {0};
    memcpy(
        &run_consensus_peer_server_args.consensus_peer_server_addr.sin6_addr,
        &discover_peers_args.peer_addr.sin6_addr,
        sizeof(IN6_ADDR));
    run_consensus_peer_server_args.consensus_peer_server_addr.sin6_family = AF_INET6;
    run_consensus_peer_server_args.consensus_peer_server_addr.sin6_port = htons(peer_port);
    run_consensus_peer_server_args.sync = sync;
    run_consensus_peer_server_args.print_progress = true;
    atomic_bool run_consensus_peer_server_should_stop = false;
    run_consensus_peer_server_args.should_stop = &run_consensus_peer_server_should_stop;
    bool run_consensus_peer_server_exit_ready = false;
    run_consensus_peer_server_args.exit_ready = &run_consensus_peer_server_exit_ready;
    if (SUCCESS != pthread_cond_init(&run_consensus_peer_server_args.exit_ready_cond, NULL)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    if (0 != pthread_mutex_init(&run_consensus_peer_server_args.exit_ready_mutex, NULL)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    run_consensus_peer_client_args_t run_consensus_peer_client_args = {0};
    run_consensus_peer_client_args.sync = sync;
    run_consensus_peer_client_args.peer_info_list = discover_peers_args.peer_info_list;
    run_consensus_peer_client_args.peer_info_list_mutex = discover_peers_args.peer_info_list_mutex;
    run_consensus_peer_client_args.print_progress = true;
    atomic_bool run_consensus_peer_client_should_stop = false;
    run_consensus_peer_client_args.should_stop = &run_consensus_peer_client_should_stop;
    bool run_consensus_peer_client_exit_ready = false;
    run_consensus_peer_client_args.exit_ready = &run_consensus_peer_client_exit_ready;
    if (SUCCESS != pthread_cond_init(&run_consensus_peer_client_args.exit_ready_cond, NULL)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    if (0 != pthread_mutex_init(&run_consensus_peer_client_args.exit_ready_mutex, NULL)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    // TODO consensus peer should be in mining thread whenever new block is mined
    // TODO make sure we're sharing mutexes
    // TODO start all threads
    // TODO join later?
    pthread_t discover_peers_thread;
    return_code = pthread_create(
        &discover_peers_thread,
        NULL,
        discover_peers_pthread_wrapper,
        &discover_peers_args);
    if (SUCCESS != return_code) {
        goto end;
    }
    pthread_t run_consensus_peer_server_thread;
    return_code = pthread_create(
        &run_consensus_peer_server_thread,
        NULL,
        run_consensus_peer_server_pthread_wrapper,
        &run_consensus_peer_server_args);
    if (SUCCESS != return_code) {
        goto end;
    }
    pthread_t run_consensus_peer_client_thread;
    return_code = pthread_create(
        &run_consensus_peer_client_thread,
        NULL,
        run_consensus_peer_client_pthread_wrapper,
        &run_consensus_peer_client_args);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code_t *return_code_ptr = mine_blocks(&mine_blocks_args);
    return_code = *return_code_ptr;
    free(return_code_ptr);
    pthread_cond_destroy(&mine_blocks_args.exit_ready_cond);
    pthread_mutex_destroy(&mine_blocks_args.exit_ready_mutex);
    pthread_cond_destroy(&mine_blocks_args.sync_version_currently_mined_cond);
    pthread_mutex_destroy(&mine_blocks_args.sync_version_currently_mined_mutex);
    synchronized_blockchain_destroy(sync);
    #ifdef _WIN32
        WSACleanup();
    #endif
end:
    return return_code;
}
