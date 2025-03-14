/**
 * @brief Defines the blockchain.
 * 
 * See https://github.com/dvf/blockchain for a helpful Python blockchain
 * implementation.
 */

#ifndef INCLUDE_BLOCKCHAIN_H_
#define INCLUDE_BLOCKCHAIN_H_

#include <stdint.h>
#include <sys/time.h>
#include <stdatomic.h>
#include <pthread.h>
#include "include/block.h"
#include "include/return_codes.h"

/**
 * @brief Represents a blockchain.
 * 
 * @param block_list The list of blocks in the chain.
 */
typedef struct blockchain_t {
    linked_list_t *block_list;
    size_t num_leading_zero_bytes_required_in_block_hash;
} blockchain_t;

/**
 * @brief A synchronized blockchain.
 * 
 * @param blockchain The blockchain.
 * @param version A number indicating how many times the blockchain pointer has
 * been changed. It is initially zero. Writer threads can lock, update the
 * blockchain pointer, increment this number to signal an update, and unlock.
 * Reader threads can check this value without locking to know whether they need
 * to lock and update their view of the blockchain.
 * @param mutex The mutex protecting access to this data structure's fields.
 */
typedef struct synchronized_blockchain_t {
    blockchain_t *blockchain;
    atomic_size_t version;
    pthread_mutex_t mutex;
} synchronized_blockchain_t;

/**
 * @brief Fills blockchain with a pointer to the newly allocated blockchain.
 * 
 * @param blockchain A pointer to fill with the blockchain's address.
 * @param num_leading_zero_bytes_required_in_block_hash The number of leading
 * zero bytes to make a block hash a valid proof of work.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t blockchain_create(
    blockchain_t **blockchain,
    size_t num_leading_zero_bytes_required_in_block_hash
);

/**
 * @brief Frees all memory associated with a blockchain.
 * 
 * @param blockchain The blockchain to destroy.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t blockchain_destroy(blockchain_t *blockchain);

/**
 * @brief Fills sync with a newly allocated synchronized blockchain.
 * 
 * @param sync A pointer to fill with the synchronized blockchain's address.
 * @param initial_blockchain A pointer to the initial blockchain that should be
 * put into the data structure.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t synchronized_blockchain_create(
    synchronized_blockchain_t **sync,
    blockchain_t *initial_blockchain
);

/**
 * @brief Frees all memory associated with a synchronized blockchain.
 * 
 * @param sync The synchronized blockchain to destroy.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t synchronized_blockchain_destroy(synchronized_blockchain_t *sync);

/**
 * @brief Appends a block to the blockchain.
 * 
 * @param blockchain The blockchain.
 * @param block The block to add.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t blockchain_add_block(blockchain_t *blockchain, block_t *block);

/**
 * @brief Fills is_valid_block_hash with true or false.
 * 
 * A valid block hash begins with the number of leading zeroes specified in the
 * blockchain struct.
 * 
 * @param blockchain The blockchain.
 * @param block_hash The block hash to check.
 * @param is_valid_block_hash A pointer to fill with the result.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t blockchain_is_valid_block_hash(
    blockchain_t *blockchain,
    sha_256_t block_hash,
    bool *is_valid_block_hash
);

/**
 * @brief Fills block's proof_of_work with a number that produces a valid hash.
 * 
 * @param blockchain The blockchain.
 * @param block The block for which to calculate a proof of work.
 * @param print_progress If true, display progress on the screen.
 * @param should_stop This should initially be false. Setting this flag while
 * the function is running requests that the function terminate gracefully.
 * Users should expect the function to terminate in a timely manner (on the
 * order of seconds), but not necessarily immediately.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t blockchain_mine_block(
    blockchain_t *blockchain,
    block_t *block,
    bool print_progress,
    atomic_bool *should_stop
);

/**
 * @brief Fills block's proof_of_work with a number that produces a valid hash.
 * 
 * @param sync The synchronized blockchain.
 * @param block The block for which to calculate a proof of work.
 * @param print_progress If true, display progress on the screen.
 * @param should_stop This should initially be false. Setting this flag while
 * the function is running requests that the function terminate gracefully.
 * Users should expect the function to terminate in a timely manner (on the
 * order of seconds), but not necessarily immediately.
 * @param sync_version_currently_mined Contains the version number of the
 * synchronized blockchain that this function is currently mining. When the user
 * updates this number (from another thread), this function stops and returns
 * FAILURE_LONGER_BLOCKCHAIN_DETECTED.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t synchronized_blockchain_mine_block(
    synchronized_blockchain_t *sync,
    block_t *block,
    bool print_progress,
    atomic_bool *should_stop,
    atomic_size_t *sync_version_currently_mined
);

/**
 * @brief Prints the blockchain.
 */
void blockchain_print(blockchain_t *blockchain);

/**
 * @brief Verifies that the blockchain is valid.
 * 
 * A blockchain is valid if it meets the following conditions.
 * 
 * 1. Every block has a valid proof of work. For every block other than the
 * genesis block, the block must hash to a value that contains the number of
 * leading zero bytes in the blockchain data structure. The genesis block must
 * have the magic number proof of work GENESIS_BLOCK_PROOF_OF_WORK and be the
 * first block.
 * 2. Every block must have a correct previous block hash. For the genesis
 * block, the previous block hash must be zero.
 * 3. Every block except the genesis block must have a minting transaction as
 * its first transaction. The minting transaction has an amount of 1 and has
 * both sender and recipient keys set to the miner.
 * 4. Every transaction in every block must have a valid digital signature.
 * 
 * @param blockchain The blockchain.
 * @param is_valid_blockchain A pointer to fill with the result.
 * @param first_invalid_block If the blockchain is invalid and this argument is
 * not NULL, the function fills this pointer with the first invalid block.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t blockchain_verify(
    blockchain_t *blockchain,
    bool *is_valid_blockchain,
    block_t **first_invalid_block
);

/**
 * @brief Serializes the blockchain into a buffer for file or network I/O.
 * 
 * @param blockchain The blockchain.
 * @param buffer A pointer to fill with the bytes representing the blockchain.
 * Callers can write the bytes to a file or send on the network and reconstruct
 * the original blockchain with blockchain_deserialize. Callers are responsible
 * for freeing the buffer.
 * @param buffer_size A pointer to fill with the final size of the buffer.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t blockchain_serialize(
    blockchain_t *blockchain,
    unsigned char **buffer,
    uint64_t *buffer_size
);

/**
 * @brief Reconstructs the blockchain from a buffer.
 * 
 * @param blockchain A pointer to fill with the reconstructed blockchain.
 * Callers are responsible for calling blockchain_destroy when finished.
 * @param buffer An array containing the serialized blockchain.
 * @param buffer_size The length of the serialized blockchain.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t blockchain_deserialize(
    blockchain_t **blockchain,
    unsigned char *buffer,
    uint64_t buffer_size
);

/**
 * @brief Saves the blockchain to a file.
 * 
 * @param blockchain The blockchain.
 * @param outfile The path to the file to which to write the blockchain.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t blockchain_write_to_file(
    blockchain_t *blockchain,
    char *outfile
);

/**
 * @brief Reads the blockchain from a file.
 * 
 * @param blockchain A pointer to fill with the reconstructed blockchain.
 * Callers are responsible for calling blockchain_destroy when finished.
 * @param infile The path to the file from which to read the blockchain.
 * @return return_code_t A return code indicating success or failure.
 */
return_code_t blockchain_read_from_file(
    blockchain_t **blockchain,
    char *infile
);

#endif  // INCLUDE_BLOCKCHAIN_H_
