#include <stdio.h>
#include <stdlib.h>
#include "include/block.h"
#include "include/blockchain.h"
#include "include/endian.h"
#include "include/linked_list.h"
#include "include/return_codes.h"
#include "include/transaction.h"

#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

return_code_t blockchain_create(
    blockchain_t **blockchain,
    size_t num_leading_zero_bytes_required_in_block_hash
) {
    return_code_t return_code = SUCCESS;
    if (NULL == blockchain) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    blockchain_t *new_blockchain = malloc(sizeof(blockchain_t));
    if (NULL == new_blockchain) {
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    linked_list_t *block_list = NULL;
    return_code = linked_list_create(
        &block_list,
        (free_function_t *)block_destroy,
        NULL);
    if (SUCCESS != return_code) {
        free(new_blockchain);
        goto end;
    }
    new_blockchain->block_list = block_list;
    new_blockchain->num_leading_zero_bytes_required_in_block_hash =
        num_leading_zero_bytes_required_in_block_hash;
    *blockchain = new_blockchain;
end:
    return return_code;
}

return_code_t blockchain_destroy(blockchain_t *blockchain) {
    return_code_t return_code = SUCCESS;
    if (NULL == blockchain) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    return_code = linked_list_destroy(blockchain->block_list);
    free(blockchain);
end:
    return return_code;
}

return_code_t blockchain_add_block(blockchain_t *blockchain, block_t *block) {
    return_code_t return_code = SUCCESS;
    if (NULL == blockchain || NULL == block) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    return_code = linked_list_append(blockchain->block_list, block);
end:
    return return_code;
}

return_code_t blockchain_is_valid_block_hash(
    blockchain_t *blockchain,
    sha_256_t block_hash,
    bool *is_valid_block_hash
) {
    return_code_t return_code = SUCCESS;
    if (NULL == blockchain || NULL == is_valid_block_hash) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    bool is_valid = true;
    for (size_t idx = 0;
        idx < blockchain->num_leading_zero_bytes_required_in_block_hash;
        idx++) {
        if (block_hash.digest[idx] != 0) {
            is_valid = false;
            break;
        }
    }
    *is_valid_block_hash = is_valid;
end:
    return return_code;
}

return_code_t blockchain_mine_block(
    blockchain_t *blockchain,
    block_t *block,
    bool print_progress
) {
    return_code_t return_code = SUCCESS;
    if (NULL == blockchain || NULL == block) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    size_t best_leading_zeroes = 0;
    size_t print_frequency = 20000;
    sha_256_t hash = {0};
    bool is_valid_block_hash = false;
    for (uint64_t new_proof = 0; new_proof < UINT64_MAX; new_proof++) {
        block->proof_of_work = new_proof;
        return_code = block_hash(block, &hash);
        if (SUCCESS != return_code) {
            goto end;
        }
        if (print_progress) {
            size_t num_zeroes = 0;
            for (size_t idx = 0; idx < sizeof(hash.digest); idx++) {
                unsigned char upper_nybble = hash.digest[idx] >> 4;
                if (upper_nybble != 0) {
                    break;
                }
                num_zeroes++;
                if (hash.digest[idx] != 0) {
                    break;
                }
                num_zeroes++;
            }
            bool print_this_iteration = new_proof % print_frequency == 0;
            if (num_zeroes > best_leading_zeroes) {
                best_leading_zeroes = num_zeroes;
                print_this_iteration = true;
            }
            if (print_this_iteration) {
                // Remove the previous block hash from output.
                printf("\rMining LeoCoin block: ");
                // Print the best number of leading zeroes.
                for (size_t idx = 0; idx < best_leading_zeroes; idx++) {
                    printf("0");
                }
                // Add the part of the new hash following the best number of
                // leading zeroes. It's not the accurate hash, but it does give
                // an idea of progress.
                for (size_t idx = best_leading_zeroes;
                    idx < 2 * sizeof(hash.digest);
                    idx++) {
                    size_t hash_idx = idx / 2;
                    unsigned char nybble = 0;
                    if (idx % 2 == 0) {
                        nybble = hash.digest[hash_idx] >> 4;
                    } else {
                        nybble = hash.digest[hash_idx] & 0x0f;
                    }
                    // To give a better sense of progress, don't allow a
                    // non-leading zero to follow the best number of leading
                    // zeroes.
                    if (idx == best_leading_zeroes && 0 == nybble) {
                        nybble += 1;
                    }
                    printf("%01x", nybble);
                }
                fflush(stdout);
            }
        }
        return_code = blockchain_is_valid_block_hash(
            blockchain, hash, &is_valid_block_hash);
        if (SUCCESS != return_code) {
            goto end;
        }
        if (is_valid_block_hash) {
            if (print_progress) {
                printf("\rMining LeoCoin block: %s", ANSI_COLOR_GREEN);
                for (size_t idx = 0; idx < sizeof(hash.digest); idx++) {
                    printf("%02x", hash.digest[idx]);
                }
                printf("%s\n", ANSI_COLOR_RESET);
            }
            goto end;
        }
    }
    return_code = FAILURE_COULD_NOT_FIND_VALID_PROOF_OF_WORK;
end:
    return return_code;
}

void blockchain_print(blockchain_t *blockchain) {
    if (NULL == blockchain) {
        return;
    }
    for (node_t *node = blockchain->block_list->head;
        NULL != node;
        node = node->next) {
        block_t *block = (block_t *)node->data;
        printf("%"PRIu64"->", block->proof_of_work);
    }
    printf("\n");
}

// TODO add issue to use JSON serialization rather than binary
return_code_t blockchain_serialize(
    blockchain_t *blockchain,
    unsigned char **buffer,
    uint64_t *buffer_size
) {
    return_code_t return_code = SUCCESS;
    if (NULL == blockchain || NULL == buffer || NULL == buffer_size) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    uint64_t num_blocks = 0;
    return_code = linked_list_length(blockchain->block_list, &num_blocks);
    if (SUCCESS != return_code) {
        goto end;
    }
    uint64_t size = 
        sizeof(blockchain->num_leading_zero_bytes_required_in_block_hash) +
        sizeof(num_blocks);
    unsigned char *serialization_buffer = calloc(1, size);
    if (NULL == serialization_buffer) {
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    unsigned char *next_spot_in_buffer = serialization_buffer;
    *(uint64_t *)next_spot_in_buffer = htobe64(
        blockchain->num_leading_zero_bytes_required_in_block_hash);
    next_spot_in_buffer += sizeof(
        blockchain->num_leading_zero_bytes_required_in_block_hash);
    *(uint64_t *)next_spot_in_buffer = htobe64(num_blocks);
    next_spot_in_buffer += sizeof(num_blocks);
    for (node_t *block_node = blockchain->block_list->head;
        NULL != block_node;
        block_node = block_node->next) {
        block_t *block = (block_t *)block_node->data;
        uint64_t num_transactions_in_block = 0;
        return_code = linked_list_length(
            block->transaction_list, &num_transactions_in_block);
        if (SUCCESS != return_code) {
            free(serialization_buffer);
            goto end;
        }
        uint64_t next_spot_in_buffer_offset = size;
        size += sizeof(block->created_at);
        size += sizeof(block->previous_block_hash);
        size += sizeof(block->proof_of_work);
        size += sizeof(num_transactions_in_block);
        serialization_buffer = realloc(serialization_buffer, size);
        if (NULL == serialization_buffer) {
            goto end;
        }
        // When we realloc, serialization_buffer may move. We need to use an
        // offset so we can get the next memory location to write.
        next_spot_in_buffer = serialization_buffer + next_spot_in_buffer_offset;
        *(uint64_t *)next_spot_in_buffer = htobe64(block->created_at);
        next_spot_in_buffer += sizeof(block->created_at);
        for (size_t idx = 0; idx < sizeof(block->previous_block_hash); idx++) {
            *next_spot_in_buffer = block->previous_block_hash.digest[idx];
            next_spot_in_buffer++;
        }
        *(uint64_t *)next_spot_in_buffer = htobe64(block->proof_of_work);
        next_spot_in_buffer += sizeof(block->proof_of_work);
        *(uint64_t *)next_spot_in_buffer = htobe64(num_transactions_in_block);
        next_spot_in_buffer += sizeof(num_transactions_in_block);
        for (node_t *transaction_node = block->transaction_list->head;
            NULL != transaction_node;
            transaction_node = transaction_node->next) {
            transaction_t *transaction = (transaction_t *)
                transaction_node->data;
            next_spot_in_buffer_offset = size;
            size += sizeof(transaction->created_at);
            size += sizeof(transaction->sender_public_key);
            size += sizeof(transaction->recipient_public_key);
            size += sizeof(transaction->amount);
            size += sizeof(transaction->sender_signature.length);
            size += sizeof(transaction->sender_signature.bytes);
            serialization_buffer = realloc(serialization_buffer, size);
            if (NULL == serialization_buffer) {
                goto end;
            }
            // When we realloc, serialization_buffer may move. We need to use an
            // offset so we can get the next memory location to write.
            next_spot_in_buffer =
                serialization_buffer + next_spot_in_buffer_offset;
            *(uint64_t *)next_spot_in_buffer = htobe64(transaction->created_at);
            next_spot_in_buffer += sizeof(transaction->created_at);
            for (size_t idx = 0;
                 idx < sizeof(transaction->sender_public_key);
                 idx++) {
                *next_spot_in_buffer =
                    transaction->sender_public_key.bytes[idx];
                next_spot_in_buffer++;
            }
            for (size_t idx = 0;
                 idx < sizeof(transaction->recipient_public_key);
                 idx++) {
                *next_spot_in_buffer =
                    transaction->recipient_public_key.bytes[idx];
                next_spot_in_buffer++;
            }
            *(uint64_t *)next_spot_in_buffer = htobe64(transaction->amount);
            next_spot_in_buffer += sizeof(transaction->amount);
            *(uint64_t *)next_spot_in_buffer = htobe64(
                transaction->sender_signature.length);
            next_spot_in_buffer += sizeof(transaction->sender_signature.length);
            for (size_t idx = 0;
                idx < sizeof(transaction->sender_signature.bytes);
                idx++) {
                *next_spot_in_buffer = transaction->sender_signature.bytes[idx];
                next_spot_in_buffer++;
            }
        }
    }
    *buffer = serialization_buffer;
    *buffer_size = size;
end:
    return return_code;
}

// TODO make issue that this function is too complex, too hard to error-check
return_code_t blockchain_deserialize(
    blockchain_t **blockchain,
    unsigned char *buffer,
    uint64_t buffer_size
) {
    return_code_t return_code = SUCCESS;
    if (NULL == blockchain || NULL == buffer) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    unsigned char *next_spot_in_buffer = buffer;
    ptrdiff_t total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    uint64_t num_leading_zero_bytes_required_in_block_hash = betoh64(
        *(uint64_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(
        num_leading_zero_bytes_required_in_block_hash);
    blockchain_t *new_blockchain = NULL;
    return_code = blockchain_create(
        &new_blockchain, num_leading_zero_bytes_required_in_block_hash);
    if (SUCCESS != return_code) {
        goto end;
    }
    total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
    if (total_read_size > buffer_size) {
        return_code = FAILURE_BUFFER_TOO_SMALL;
        goto end;
    }
    uint64_t num_blocks = betoh64(*(uint64_t *)next_spot_in_buffer);
    next_spot_in_buffer += sizeof(num_blocks);
    for (uint64_t block_idx = 0; block_idx < num_blocks; block_idx++) {
        block_t *block = NULL;
        total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
        if (total_read_size > buffer_size) {
            return_code = FAILURE_BUFFER_TOO_SMALL;
            blockchain_destroy(new_blockchain);
            goto end;
        }
        time_t block_created_at = betoh64(*(uint64_t *)next_spot_in_buffer);
        next_spot_in_buffer += sizeof(block_created_at);
        total_read_size = next_spot_in_buffer + sizeof(sha_256_t) - buffer;
        if (total_read_size > buffer_size) {
            return_code = FAILURE_BUFFER_TOO_SMALL;
            blockchain_destroy(new_blockchain);
            goto end;
        }
        sha_256_t previous_block_hash = {0};
        for (size_t idx = 0; idx < sizeof(previous_block_hash); idx++) {
            previous_block_hash.digest[idx] = *next_spot_in_buffer;
            next_spot_in_buffer++;
        }
        total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
        if (total_read_size > buffer_size) {
            return_code = FAILURE_BUFFER_TOO_SMALL;
            blockchain_destroy(new_blockchain);
            goto end;
        }
        uint64_t proof_of_work = betoh64(*(uint64_t *)next_spot_in_buffer);
        next_spot_in_buffer += sizeof(proof_of_work);
        total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
        if (total_read_size > buffer_size) {
            return_code = FAILURE_BUFFER_TOO_SMALL;
            blockchain_destroy(new_blockchain);
            goto end;
        }
        uint64_t num_transactions = betoh64(*(uint64_t *)next_spot_in_buffer);
        next_spot_in_buffer += sizeof(num_transactions);
        linked_list_t *transaction_list = NULL;
        return_code = linked_list_create(
            &transaction_list, (free_function_t *)transaction_destroy, NULL);
        if (SUCCESS != return_code) {
            blockchain_destroy(new_blockchain);
            goto end;
        }
        for (uint64_t transaction_idx = 0;
            transaction_idx < num_transactions;
            transaction_idx++) {
            // TODO transaction_create interface doesn't allow us to pass in a signature directly, so I manually alloc--not sure if good or bad
            transaction_t *transaction = calloc(1, sizeof(transaction_t));
            if (NULL == transaction) {
                return_code = FAILURE_COULD_NOT_MALLOC;
                blockchain_destroy(new_blockchain);
                linked_list_destroy(transaction_list);
                goto end;
            }
            total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
            if (total_read_size > buffer_size) {
                return_code = FAILURE_BUFFER_TOO_SMALL;
                blockchain_destroy(new_blockchain);
                linked_list_destroy(transaction_list);
                free(transaction);
                goto end;
            }
            transaction->created_at = betoh64(*(uint64_t *)next_spot_in_buffer);
            next_spot_in_buffer += sizeof(transaction->created_at);
            total_read_size = next_spot_in_buffer + sizeof(ssh_key_t) - buffer;
            if (total_read_size > buffer_size) {
                return_code = FAILURE_BUFFER_TOO_SMALL;
                blockchain_destroy(new_blockchain);
                linked_list_destroy(transaction_list);
                free(transaction);
                goto end;
            }
            for (size_t idx = 0;
                idx < sizeof(transaction->sender_public_key);
                idx++) {
                transaction->sender_public_key.bytes[idx] =
                    *next_spot_in_buffer;
                next_spot_in_buffer++;
            }
            total_read_size = next_spot_in_buffer + sizeof(ssh_key_t) - buffer;
            if (total_read_size > buffer_size) {
                return_code = FAILURE_BUFFER_TOO_SMALL;
                blockchain_destroy(new_blockchain);
                linked_list_destroy(transaction_list);
                free(transaction);
                goto end;
            }
            for (size_t idx = 0;
                idx < sizeof(transaction->recipient_public_key);
                idx++) {
                transaction->recipient_public_key.bytes[idx] =
                    *next_spot_in_buffer;
                next_spot_in_buffer++;
            }
            total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
            if (total_read_size > buffer_size) {
                return_code = FAILURE_BUFFER_TOO_SMALL;
                blockchain_destroy(new_blockchain);
                linked_list_destroy(transaction_list);
                free(transaction);
                goto end;
            }
            transaction->amount = betoh64(*(uint64_t *)next_spot_in_buffer);
            next_spot_in_buffer += sizeof(transaction->amount);
            total_read_size = next_spot_in_buffer + sizeof(uint64_t) - buffer;
            if (total_read_size > buffer_size) {
                return_code = FAILURE_BUFFER_TOO_SMALL;
                blockchain_destroy(new_blockchain);
                linked_list_destroy(transaction_list);
                free(transaction);
                goto end;
            }
            transaction->sender_signature.length = betoh64(
                *(uint64_t *)next_spot_in_buffer);
            next_spot_in_buffer += sizeof(transaction->sender_signature.length);
            if (transaction->sender_signature.length > MAX_SSH_KEY_LENGTH) {
                return_code = FAILURE_SIGNATURE_TOO_LONG;
                blockchain_destroy(new_blockchain);
                linked_list_destroy(transaction_list);
                free(transaction);
                goto end;
            }
            total_read_size =
                next_spot_in_buffer +
                transaction->sender_signature.length -
                buffer;
            if (total_read_size > buffer_size) {
                return_code = FAILURE_BUFFER_TOO_SMALL;
                blockchain_destroy(new_blockchain);
                linked_list_destroy(transaction_list);
                free(transaction);
                goto end;
            }
            for (size_t idx = 0;
                idx < transaction->sender_signature.length;
                idx++) {
                transaction->sender_signature.bytes[idx] = *next_spot_in_buffer;
                next_spot_in_buffer++;
            }
            return_code = linked_list_append(transaction_list, transaction);
            if (SUCCESS != return_code) {
                blockchain_destroy(new_blockchain);
                linked_list_destroy(transaction_list);
                free(transaction);
                goto end;
            }
        }
        return_code = block_create(
            &block, transaction_list, proof_of_work, previous_block_hash);
        if (SUCCESS != return_code) {
            blockchain_destroy(new_blockchain);
            linked_list_destroy(transaction_list);
            goto end;
        }
        return_code = blockchain_add_block(new_blockchain, block);
        if (SUCCESS != return_code) {
            blockchain_destroy(new_blockchain);
            block_destroy(block);
            goto end;
        }
    }
    *blockchain = new_blockchain;
end:
    return return_code;
}

return_code_t blockchain_write_to_file(
    blockchain_t *blockchain,
    char *outfile
) {
    return_code_t return_code = SUCCESS;
    if (NULL == blockchain || NULL == outfile) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    unsigned char *buffer = NULL;
    size_t buffer_size = 0;
    return_code = blockchain_serialize(blockchain, &buffer, &buffer_size);
    if (SUCCESS != return_code) {
        goto end;
    }
    FILE *f = fopen(outfile, "wb");
    if (NULL == f) {
        return_code = FAILURE_FILE_IO;
        goto end;
    }
    size_t bytes_written = fwrite(buffer, 1, buffer_size, f);
    fclose(f);
    free(buffer);
    if (bytes_written != buffer_size) {
        return_code = FAILURE_FILE_IO;
        goto end;
    }
end:
    return return_code;
}

return_code_t blockchain_read_from_file(
    blockchain_t **blockchain,
    char *infile
) {
    return_code_t return_code = SUCCESS;
    if (NULL == blockchain || NULL == infile) {
        return_code = FAILURE_INVALID_INPUT;
        goto end;
    }
    FILE *f = fopen(infile, "rb");
    if (NULL == f) {
        return_code = FAILURE_FILE_IO;
        goto end;
    }
    fseek(f, 0, SEEK_END);
    uint64_t buffer_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *buffer = malloc(buffer_size);
    if (NULL == buffer) {
        return_code = FAILURE_COULD_NOT_MALLOC;
        goto end;
    }
    size_t read_size = fread(buffer, 1, buffer_size, f);
    if (read_size != (size_t)buffer_size) {
        return_code = FAILURE_FILE_IO;
        goto cleanup;
    }
    return_code = blockchain_deserialize(blockchain, buffer, buffer_size);
cleanup:
    fclose(f);
    free(buffer);
end:
    return return_code;
}
