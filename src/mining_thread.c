#include <stdio.h>
#include <stdlib.h>
#include "include/transaction.h"
#include "include/mining_thread.h"

return_code_t *mine_blocks(mine_blocks_args_t *args) {
    return_code_t return_code = SUCCESS;
    if (NULL == args) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    synchronized_blockchain_t *sync = args->sync;
    if (0 != pthread_mutex_lock(&sync->mutex)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    blockchain_t *blockchain = sync->blockchain;
    if (0 != pthread_mutex_unlock(&sync->mutex)) {
        return_code = FAILURE_PTHREAD_FUNCTION;
        goto end;
    }
    while (!*args->should_stop) {
        if (atomic_load(args->sync_version_currently_mined) !=
            atomic_load(&sync->version)) {
            blockchain_t *old_blockchain = blockchain;
            if (0 != pthread_mutex_lock(&sync->mutex)) {
                return_code = FAILURE_PTHREAD_FUNCTION;
                goto end;
            }
            blockchain = sync->blockchain;
            if (0 != pthread_mutex_unlock(&sync->mutex)) {
                return_code = FAILURE_PTHREAD_FUNCTION;
                goto end;
            }
            return_code = blockchain_destroy(old_blockchain);
            if (SUCCESS != return_code) {
                goto end;
            }
            atomic_store(
                args->sync_version_currently_mined,
                atomic_load(&sync->version));
            if (0 != pthread_mutex_lock(
                &args->sync_version_currently_mined_mutex)) {
                return_code = FAILURE_PTHREAD_FUNCTION;
                goto end;
            }
            pthread_cond_signal(&args->sync_version_currently_mined_cond);
            if (0 != pthread_mutex_unlock(
                &args->sync_version_currently_mined_mutex)) {
                return_code = FAILURE_PTHREAD_FUNCTION;
                goto end;
            }
        }
        bool is_valid_blockchain = false;
        block_t *first_invalid_block = NULL;
        return_code = blockchain_verify(
            blockchain, &is_valid_blockchain, &first_invalid_block);
        if (SUCCESS != return_code) {
            goto end;
        }
        if (!is_valid_blockchain) {
            printf(
                "Invalid blockchain detected. First invalid block follows.\n");
            printf("Created at: %"PRIu64"\n", first_invalid_block->created_at);
            printf(
                "Proof of work: %"PRIu64"\n",
                first_invalid_block->proof_of_work);
            printf("Block hash: ");
            hash_print(&first_invalid_block->previous_block_hash);
            return_code = FAILURE_INVALID_BLOCKCHAIN;
            goto end;
        }
        node_t *node = NULL;
        return_code = linked_list_get_last(blockchain->block_list, &node);
        if (SUCCESS != return_code) {
            goto end;
        }
        block_t *previous_block = (block_t *)node->data;
        sha_256_t previous_block_hash = {0};
        return_code = block_hash(previous_block, &previous_block_hash);
        if (SUCCESS != return_code) {
            goto end;
        }
        linked_list_t *transaction_list = NULL;
        return_code = linked_list_create(&transaction_list, free, NULL);
        if (SUCCESS != return_code) {
            goto end;
        }
        transaction_t *mint_coin_transaction = NULL;
        return_code = transaction_create(
            &mint_coin_transaction,
            args->miner_public_key,
            args->miner_public_key,
            AMOUNT_GENERATED_DURING_MINTING,
            args->miner_private_key);
        if (SUCCESS != return_code) {
            linked_list_destroy(transaction_list);
            goto end;
        }
        return_code = linked_list_prepend(
            transaction_list, mint_coin_transaction);
        if (SUCCESS != return_code) {
            transaction_destroy(mint_coin_transaction);
            linked_list_destroy(transaction_list);
            goto end;
        }
        block_t *next_block = NULL;
        return_code = block_create(
            &next_block, transaction_list, 0, previous_block_hash);
        if (SUCCESS != return_code) {
            linked_list_destroy(transaction_list);
            goto end;
        }
        return_code = synchronized_blockchain_mine_block(
            sync,
            next_block,
            args->print_progress,
            args->should_stop,
            args->sync_version_currently_mined);
        if (SUCCESS != return_code) {
            block_destroy(next_block);
            if (FAILURE_COULD_NOT_FIND_VALID_PROOF_OF_WORK == return_code) {
                if (args->print_progress) {
                    printf("\nCouldn't find valid proof of work for block; "
                           "generating new block.\n");
                }
            } else if (FAILURE_LONGER_BLOCKCHAIN_DETECTED == return_code) {
                if (args->print_progress) {
                    printf("Longer chain detected; switching to new chain.\n");
                }
                return_code = SUCCESS;
            } else if (FAILURE_STOPPED_EARLY == return_code) {
                if (args->print_progress) {
                    printf("Stopping miner.\n");
                }
                return_code = SUCCESS;
            } else {
                goto end;
            }
        } else {
            return_code = blockchain_add_block(blockchain, next_block);
            if (SUCCESS != return_code) {
                block_destroy(next_block);
                goto end;
            }
            if (args->print_progress) {
                blockchain_print(blockchain);
            }
            if (NULL != args->outfile) {
                blockchain_write_to_file(blockchain, args->outfile);
            }
            // TODO spin up client thread here to broadcast blockchain
        }
    }
    pthread_mutex_lock(&args->exit_ready_mutex);
    *args->exit_ready = true;
    pthread_cond_signal(&args->exit_ready_cond);
    pthread_mutex_unlock(&args->exit_ready_mutex);
end:
    return_code_t *return_code_ptr = malloc(sizeof(return_code_t));
    *return_code_ptr = return_code;
    return return_code_ptr;
}

void *mine_blocks_pthread_wrapper(void *args) {
    mine_blocks_args_t *mine_blocks_args = (mine_blocks_args_t *)args;
    return_code_t *return_code_ptr = mine_blocks(mine_blocks_args);
    return (void *)return_code_ptr;
}
