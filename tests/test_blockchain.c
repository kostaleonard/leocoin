#include <stdbool.h>
#include "include/blockchain.h"
#include "tests/test_blockchain.h"

void test_blockchain_create_gives_blockchain() {
    blockchain_t *blockchain = NULL;
    return_code_t return_code = blockchain_create(&blockchain);
    assert_true(SUCCESS == return_code);
    assert_true(NULL != blockchain);
    assert_true(NULL != blockchain->block_list);
    blockchain_destroy(blockchain);
}

void test_blockchain_create_fails_on_invalid_input() {
    return_code_t return_code = blockchain_create(NULL);
    assert_true(FAILURE_INVALID_INPUT == return_code);
}

void test_blockchain_destroy_returns_success() {
    blockchain_t *blockchain = NULL;
    return_code_t return_code = blockchain_create(&blockchain);
    if (SUCCESS != return_code) {
        assert_true(false);
        goto end;
    }
    return_code = blockchain_destroy(blockchain);
    assert_true(SUCCESS == return_code);
end:
}

void test_blockchain_destroy_fails_on_invalid_input() {
    return_code_t return_code = blockchain_destroy(NULL);
    assert_true(FAILURE_INVALID_INPUT == return_code);
}
