#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "include/base64.h"
#include "include/blockchain.h"
#include "include/mining_thread.h"
#include "include/sleep.h"
#include "tests/test_cryptography.h"
#include "tests/test_mining_thread.h"

#define NUM_LEADING_ZERO_BYTES_IN_BLOCK_HASH 2

void test_mine_blocks_exits_when_should_stop_is_set() {
    blockchain_t *blockchain = NULL;
    return_code_t return_code = blockchain_create(
        &blockchain, NUM_LEADING_ZERO_BYTES_IN_BLOCK_HASH);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    synchronized_blockchain_t *sync = NULL;
    return_code = synchronized_blockchain_create(&sync, blockchain);
    assert_true(SUCCESS == return_code);
    atomic_bool should_stop = false;
    ssh_key_t miner_public_key = {0};
    ssh_key_t miner_private_key = {0};
    mine_blocks_args_t args = {0};
    args.sync = sync;
    char *ssh_public_key_contents_base64 = getenv(
        TEST_PUBLIC_KEY_ENVIRONMENT_VARIABLE);
    return_code = base64_decode(
        ssh_public_key_contents_base64,
        strlen(ssh_public_key_contents_base64),
        miner_public_key.bytes);
    char *ssh_private_key_contents_base64 = getenv(
        TEST_PRIVATE_KEY_ENVIRONMENT_VARIABLE);
    return_code = base64_decode(
        ssh_private_key_contents_base64,
        strlen(ssh_private_key_contents_base64),
        miner_private_key.bytes);
    bool exit_ready = false;
    atomic_size_t sync_version_currently_mined = atomic_load(&sync->version);
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    pthread_cond_init(&args.sync_version_currently_mined_cond, NULL);
    pthread_mutex_init(&args.sync_version_currently_mined_mutex, NULL);
    args.miner_public_key = &miner_public_key;
    args.miner_private_key = &miner_private_key;
    args.print_progress = false;
    args.outfile = NULL;
    args.should_stop = &should_stop;
    args.exit_ready = &exit_ready;
    args.sync_version_currently_mined = &sync_version_currently_mined;
    pthread_t thread;
    pthread_create(&thread, NULL, mine_blocks_pthread_wrapper, &args);
    // Pause for a short period to allow the miner to start.
    sleep_microseconds(100000);
    *args.should_stop = true;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    // One second timeout.
    ts.tv_sec += 1;
    pthread_mutex_lock(&args.exit_ready_mutex);
    while (!*args.exit_ready) {
        int result = pthread_cond_timedwait(
            &args.exit_ready_cond, &args.exit_ready_mutex, &ts);
        if (ETIMEDOUT == result) {
            assert_true(false);
        }
    }
    pthread_mutex_unlock(&args.exit_ready_mutex);
    void *retval = NULL;
    pthread_join(thread, &retval);
    return_code_t *return_code_ptr = (return_code_t *)retval;
    assert_true(NULL != return_code_ptr);
    return_code = *return_code_ptr;
    assert_true(SUCCESS == return_code);
    free(return_code_ptr);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    pthread_cond_destroy(&args.sync_version_currently_mined_cond);
    pthread_mutex_destroy(&args.sync_version_currently_mined_mutex);
    synchronized_blockchain_destroy(sync);
}

void test_mine_blocks_mines_new_blockchain_when_version_incremented() {
    blockchain_t *blockchain = NULL;
    return_code_t return_code = blockchain_create(
        &blockchain, NUM_LEADING_ZERO_BYTES_IN_BLOCK_HASH);
    assert_true(SUCCESS == return_code);
    block_t *genesis_block;
    return_code = block_create_genesis_block(&genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(blockchain, genesis_block);
    assert_true(SUCCESS == return_code);
    synchronized_blockchain_t *sync = NULL;
    return_code = synchronized_blockchain_create(&sync, blockchain);
    assert_true(SUCCESS == return_code);
    atomic_bool should_stop = false;
    ssh_key_t miner_public_key = {0};
    ssh_key_t miner_private_key = {0};
    mine_blocks_args_t args = {0};
    args.sync = sync;
    char *ssh_public_key_contents_base64 = getenv(
        TEST_PUBLIC_KEY_ENVIRONMENT_VARIABLE);
    return_code = base64_decode(
        ssh_public_key_contents_base64,
        strlen(ssh_public_key_contents_base64),
        miner_public_key.bytes);
    char *ssh_private_key_contents_base64 = getenv(
        TEST_PRIVATE_KEY_ENVIRONMENT_VARIABLE);
    return_code = base64_decode(
        ssh_private_key_contents_base64,
        strlen(ssh_private_key_contents_base64),
        miner_private_key.bytes);
    bool exit_ready = false;
    atomic_size_t sync_version_currently_mined = atomic_load(&sync->version);
    pthread_cond_init(&args.exit_ready_cond, NULL);
    pthread_mutex_init(&args.exit_ready_mutex, NULL);
    pthread_cond_init(&args.sync_version_currently_mined_cond, NULL);
    pthread_mutex_init(&args.sync_version_currently_mined_mutex, NULL);
    args.miner_public_key = &miner_public_key;
    args.miner_private_key = &miner_private_key;
    args.print_progress = false;
    args.outfile = NULL;
    args.should_stop = &should_stop;
    args.exit_ready = &exit_ready;
    args.sync_version_currently_mined = &sync_version_currently_mined;
    pthread_t thread;
    pthread_create(&thread, NULL, mine_blocks_pthread_wrapper, &args);
    // Pause for a short period to allow the miner to start.
    sleep_microseconds(100000);
    blockchain_t *new_blockchain = NULL;
    return_code = blockchain_create(
        &new_blockchain, NUM_LEADING_ZERO_BYTES_IN_BLOCK_HASH);
    assert_true(SUCCESS == return_code);
    block_t *new_genesis_block = NULL;
    return_code = block_create_genesis_block(&new_genesis_block);
    assert_true(SUCCESS == return_code);
    return_code = blockchain_add_block(new_blockchain, new_genesis_block);
    assert_true(SUCCESS == return_code);
    pthread_mutex_lock(&sync->mutex);
    sync->blockchain = new_blockchain;
    pthread_mutex_unlock(&sync->mutex);
    atomic_fetch_add(&sync->version, 1);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    // One second timeout.
    ts.tv_sec += 1;
    pthread_mutex_lock(&args.sync_version_currently_mined_mutex);
    while (
        atomic_load(args.sync_version_currently_mined) !=
        atomic_load(&sync->version)) {
        int result = pthread_cond_timedwait(
            &args.sync_version_currently_mined_cond,
            &args.sync_version_currently_mined_mutex,
            &ts);
        if (ETIMEDOUT == result) {
            assert_true(false);
        }
    }
    pthread_mutex_unlock(&args.sync_version_currently_mined_mutex);
    *args.should_stop = true;
    clock_gettime(CLOCK_REALTIME, &ts);
    // One second timeout.
    ts.tv_sec += 1;
    pthread_mutex_lock(&args.exit_ready_mutex);
    while (!*args.exit_ready) {
        int result = pthread_cond_timedwait(
            &args.exit_ready_cond, &args.exit_ready_mutex, &ts);
        if (ETIMEDOUT == result) {
            assert_true(false);
        }
    }
    pthread_mutex_unlock(&args.exit_ready_mutex);
    void *retval = NULL;
    pthread_join(thread, &retval);
    return_code_t *return_code_ptr = (return_code_t *)retval;
    assert_true(NULL != return_code_ptr);
    return_code = *return_code_ptr;
    assert_true(SUCCESS == return_code);
    free(return_code_ptr);
    pthread_cond_destroy(&args.exit_ready_cond);
    pthread_mutex_destroy(&args.exit_ready_mutex);
    pthread_cond_destroy(&args.sync_version_currently_mined_cond);
    pthread_mutex_destroy(&args.sync_version_currently_mined_mutex);
    synchronized_blockchain_destroy(sync);
}
