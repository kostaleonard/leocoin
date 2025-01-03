/**
 * @brief Runs the miner.
 */

//TODO update Dockerfile to run either miner or peer discovery server
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "include/base64.h"
#include "include/blockchain.h"
#include "include/block.h"
#include "include/mining_thread.h"
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
        "-p [private_key_file_base64_encoded_contents] "
        "-k [public_key_file_base64_encoded_contents]\n",
        program_name);
    fprintf(
        stderr,
        "Or supply keys as environment variables %s and %s\n",
        PRIVATE_KEY_ENVIRONMENT_VARIABLE,
        PUBLIC_KEY_ENVIRONMENT_VARIABLE);
end:
}

int main(int argc, char **argv) {
    return_code_t return_code = SUCCESS;
    blockchain_t *blockchain = NULL;
    block_t *genesis_block = NULL;
    char *ssh_private_key_contents_base64 = NULL;
    char *ssh_public_key_contents_base64 = NULL;
    int opt;
    while ((opt = getopt(argc, argv, "p:k:")) != -1) {
        switch (opt) {
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
    mine_blocks_args_t args = {0};
    args.sync = sync;
    args.miner_public_key = &miner_public_key;
    args.miner_private_key = &miner_private_key;
    args.print_progress = true;
    args.outfile = "blockchain.bin";
    args.should_stop = &should_stop;
    bool exit_ready = false;
    args.exit_ready = &exit_ready;
    return_code = pthread_cond_init(&args.exit_ready_cond, NULL);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = pthread_mutex_init(&args.exit_ready_mutex, NULL);
    if (SUCCESS != return_code) {
        goto end;
    }
    atomic_size_t sync_version_currently_mined = atomic_load(&sync->version);
    args.sync_version_currently_mined = &sync_version_currently_mined;
    return_code = pthread_cond_init(
        &args.sync_version_currently_mined_cond, NULL);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code = pthread_mutex_init(
        &args.sync_version_currently_mined_mutex, NULL);
    if (SUCCESS != return_code) {
        goto end;
    }
    return_code_t *return_code_ptr = mine_blocks(&args);
    return_code = *return_code_ptr;
    free(return_code_ptr);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    pthread_cond_destroy(&args.sync_version_currently_mined_cond);
    pthread_mutex_destroy(&args.sync_version_currently_mined_mutex);
    synchronized_blockchain_destroy(sync);
end:
    return return_code;
}
