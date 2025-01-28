/**
 * @brief Runs the unit test suite.
 */

#include <ftw.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>
#include <cmocka.h>
#ifndef _WIN32
    #include <sys/stat.h>
    #include <unistd.h>
#endif
#include "include/networking.h"
#include "include/return_codes.h"
#include "tests/file_paths.h"
#include "tests/test_linked_list.h"
#include "tests/test_block.h"
#include "tests/test_blockchain.h"
#include "tests/test_transaction.h"
#include "tests/test_base64.h"
#include "tests/test_endian.h"
#include "tests/test_mining_thread.h"
#include "tests/test_peer_discovery.h"
#include "tests/test_networking.h"
#include "tests/test_sleep.h"
#include "tests/test_peer_discovery_thread.h"

int _unlink_callback(
    const char *fpath,
    const struct stat *sb,
    int typeflag,
    struct FTW *ftwbuf) {
    struct stat st;
    int return_value = stat(fpath, &st);
    if (0 != return_value) {
        goto end;
    }
    if (S_ISDIR(st.st_mode)) {
        return_value = rmdir(fpath);
    } else {
        return_value = remove(fpath);
    }
    if (0 != return_value) {
        perror(fpath);
        goto end;
    }
end:
    return return_value;
}

return_code_t _create_empty_output_directory(char *dirname) {
    struct stat st;
    int return_value = stat(dirname, &st);
    // If the directory exists, delete it.
    if (0 == return_value) {
        int nopenfd = 64;
        return_value = nftw(
            dirname,
            _unlink_callback,
            nopenfd,
            FTW_DEPTH | FTW_PHYS);
        if (0 != return_value) {
            perror(dirname);
            goto end;
        }
    }
    #ifdef _WIN32
        return_value = mkdir(dirname);
    #else
        return_value = mkdir(dirname, 0755);
    #endif
    if (0 != return_value) {
        perror(dirname);
    }
end:
    return_code_t return_code = return_value == 0 ? SUCCESS : FAILURE_FILE_IO;
    return return_code;
}

int teardown() {
    wrap_recv = recv;
    return 0;
}

int main(int argc, char **argv) {
    char output_directory[TESTS_MAX_PATH] = {0};
    return_code_t return_code = get_output_directory(output_directory);
    if (0 != return_code) {
        printf("Failed to get output directory\n");
        goto end;
    }
    return_code = _create_empty_output_directory(output_directory);
    if (0 != return_code) {
        printf("Failed to create output directory\n");
        goto end;
    }
    const struct CMUnitTest tests[] = {
        // test_linked_list.h
        cmocka_unit_test(test_linked_list_create_gives_linked_list),
        cmocka_unit_test(test_linked_list_create_fails_on_invalid_input),
        cmocka_unit_test(test_linked_list_create_compare_function_may_be_null),
        cmocka_unit_test(test_linked_list_destroy_empty_list_returns_success),
        cmocka_unit_test(
            test_linked_list_destroy_nonempty_list_returns_success),
        cmocka_unit_test(test_linked_list_destroy_fails_on_invalid_input),
        cmocka_unit_test(test_linked_list_prepend_adds_node_to_front),
        cmocka_unit_test(test_linked_list_prepend_fails_on_invalid_input),
        cmocka_unit_test(test_linked_list_get_first_fails_on_empty_list),
        cmocka_unit_test(test_linked_list_get_first_gives_head_of_list),
        cmocka_unit_test(test_linked_list_get_first_fails_on_invalid_input),
        cmocka_unit_test(test_linked_list_remove_first_fails_on_empty_list),
        cmocka_unit_test(test_linked_list_remove_first_removes_head),
        cmocka_unit_test(test_linked_list_remove_first_fails_on_invalid_input),
        cmocka_unit_test(test_linked_list_is_empty_gives_true_if_no_elements),
        cmocka_unit_test(
            test_linked_list_is_empty_gives_false_if_list_has_elements),
        cmocka_unit_test(test_linked_list_is_empty_fails_on_invalid_input),
        cmocka_unit_test(
            test_linked_list_find_succeeds_and_gives_null_if_list_empty),
        cmocka_unit_test(
            test_linked_list_find_succeeds_and_gives_null_if_no_match),
        cmocka_unit_test(test_linked_list_find_gives_first_matching_element),
        cmocka_unit_test(test_linked_list_find_fails_on_invalid_input),
        cmocka_unit_test(test_linked_list_find_fails_on_null_compare_function),
        cmocka_unit_test(test_linked_list_append_adds_node_to_back),
        cmocka_unit_test(test_linked_list_append_fails_on_invalid_input),
        cmocka_unit_test(test_linked_list_get_last_fails_on_empty_list),
        cmocka_unit_test(test_linked_list_get_last_gives_last_element),
        cmocka_unit_test(test_linked_list_get_last_fails_on_invalid_input),
        cmocka_unit_test(test_linked_list_length_gives_zero_on_empty_list),
        cmocka_unit_test(
            test_linked_list_length_gives_num_elements_on_nonempty_list),
        cmocka_unit_test(test_linked_list_length_fails_on_invalid_input),
        // test_block.h
        cmocka_unit_test(test_block_create_gives_block),
        cmocka_unit_test(test_block_create_fails_on_invalid_input),
        cmocka_unit_test(
            test_create_genesis_block_gives_block_with_genesis_values),
        cmocka_unit_test(test_create_genesis_block_fails_on_invalid_input),
        cmocka_unit_test(test_block_destroy_returns_success),
        cmocka_unit_test(test_block_destroy_fails_on_invalid_input),
        cmocka_unit_test(test_block_hash_gives_nonempty_hash),
        cmocka_unit_test(test_block_hash_same_fields_gives_same_hash),
        cmocka_unit_test(test_block_hash_created_at_included_in_hash),
        cmocka_unit_test(test_block_hash_transactions_included_in_hash),
        cmocka_unit_test(
            test_block_hash_multiple_transactions_included_in_hash),
        cmocka_unit_test(test_block_hash_proof_of_work_included_in_hash),
        cmocka_unit_test(test_block_hash_previous_block_hash_included_in_hash),
        cmocka_unit_test(test_block_hash_fails_on_invalid_input),
        // test_blockchain.h
        cmocka_unit_test(test_blockchain_create_gives_blockchain),
        cmocka_unit_test(test_blockchain_create_fails_on_invalid_input),
        cmocka_unit_test(test_blockchain_destroy_returns_success),
        cmocka_unit_test(test_blockchain_destroy_fails_on_invalid_input),
        cmocka_unit_test(test_synchronized_blockchain_create_gives_blockchain),
        cmocka_unit_test(
            test_synchronized_blockchain_create_fails_on_invalid_input),
        cmocka_unit_test(test_synchronized_blockchain_destroy_returns_success),
        cmocka_unit_test(
            test_synchronized_blockchain_destroy_fails_on_invalid_input),
        cmocka_unit_test(test_blockchain_add_block_appends_block),
        cmocka_unit_test(test_blockchain_add_block_fails_on_invalid_input),
        cmocka_unit_test(
            test_blockchain_is_valid_block_hash_true_on_valid_hash),
        cmocka_unit_test(
            test_blockchain_is_valid_block_hash_false_on_invalid_hash),
        cmocka_unit_test(
            test_blockchain_is_valid_block_hash_fails_on_invalid_input),
        cmocka_unit_test(
            test_blockchain_mine_block_produces_block_with_valid_hash),
        cmocka_unit_test(test_blockchain_mine_block_fails_on_invalid_input),
        cmocka_unit_test(test_blockchain_verify_succeeds_on_valid_blockchain),
        cmocka_unit_test(test_blockchain_verify_fails_on_invalid_genesis_block),
        cmocka_unit_test(test_blockchain_verify_fails_on_invalid_proof_of_work),
        cmocka_unit_test(
            test_blockchain_verify_fails_on_invalid_previous_block_hash),
        cmocka_unit_test(
            test_blockchain_verify_fails_on_invalid_transaction_signature),
        cmocka_unit_test(test_blockchain_verify_fails_on_invalid_input),
        cmocka_unit_test(test_blockchain_serialize_creates_nonempty_buffer),
        cmocka_unit_test(test_blockchain_serialize_fails_on_invalid_input),
        cmocka_unit_test(test_blockchain_deserialize_reconstructs_blockchain),
        cmocka_unit_test(
            test_blockchain_deserialize_fails_on_attempted_read_past_buffer),
        cmocka_unit_test(test_blockchain_deserialize_fails_on_invalid_input),
        cmocka_unit_test(test_blockchain_write_to_file_creates_nonempty_file),
        cmocka_unit_test(test_blockchain_write_to_file_fails_on_invalid_input),
        cmocka_unit_test(
            test_blockchain_read_from_file_reconstructs_blockchain),
        cmocka_unit_test(test_blockchain_read_from_file_fails_on_invalid_input),
        cmocka_unit_test(
            test_blockchain_serialization_does_not_alter_block_hash),
        // test_transaction.h
        cmocka_unit_test(test_transaction_create_gives_transaction),
        cmocka_unit_test(test_transaction_create_fails_on_invalid_input),
        cmocka_unit_test(test_transaction_destroy_returns_success),
        cmocka_unit_test(test_transaction_destroy_fails_on_invalid_input),
        cmocka_unit_test(test_transaction_generate_signature_gives_signature),
        cmocka_unit_test(
            test_transaction_generate_signature_fails_on_invalid_input),
        cmocka_unit_test(
            test_transaction_verify_signature_identifies_valid_signature),
        cmocka_unit_test(
            test_transaction_verify_signature_identifies_invalid_signature),
        cmocka_unit_test(
            test_transaction_verify_signature_fails_on_invalid_input),
        // test_base64.h
        cmocka_unit_test(test_base64_decode_correctly_decodes),
        cmocka_unit_test(test_base64_decode_fails_on_invalid_input),
        // test_endian.h
        cmocka_unit_test(test_htobe64_correctly_encodes_data),
        cmocka_unit_test(test_betoh64_correctly_decodes_data),
        // test_mining_thread.h
        // These multithreaded tests are incredibly slow in valgrind.
        // They run very fast outside of valgrind.
        # ifdef RUN_SLOWTESTS
            cmocka_unit_test(test_mine_blocks_exits_when_should_stop_is_set),
            cmocka_unit_test(
                test_mine_blocks_mines_new_blockchain_when_version_incremented),
        # endif
        // test_peer_discovery.h
        cmocka_unit_test(test_compare_peer_info_t_compares_ip_addresses),
        cmocka_unit_test(test_peer_info_list_serialize_fails_on_invalid_input),
        cmocka_unit_test(test_peer_info_list_serialize_creates_nonempty_buffer),
        cmocka_unit_test(test_peer_info_list_deserialize_reconstructs_list),
        cmocka_unit_test(
            test_peer_info_list_deserialize_fails_on_read_past_buffer),
        cmocka_unit_test(
            test_peer_info_list_deserialize_fails_on_invalid_input),
        // test_networking.h
        cmocka_unit_test(test_command_header_serialize_fails_on_invalid_input),
        cmocka_unit_test(test_command_header_serialize_fails_on_invalid_prefix),
        cmocka_unit_test(test_command_header_serialize_creates_nonempty_buffer),
        cmocka_unit_test(test_command_header_deserialize_reconstructs_command),
        cmocka_unit_test(
            test_command_header_deserialize_fails_on_read_past_buffer),
        cmocka_unit_test(
            test_command_header_deserialize_fails_on_invalid_prefix),
        cmocka_unit_test(
            test_command_header_deserialize_fails_on_invalid_input),
        cmocka_unit_test(
            test_command_register_peer_serialize_fails_on_invalid_input),
        cmocka_unit_test(
            test_command_register_peer_serialize_fails_on_invalid_prefix),
        cmocka_unit_test(
            test_command_register_peer_serialize_fails_on_invalid_command),
        cmocka_unit_test(
            test_command_register_peer_serialize_creates_nonempty_buffer),
        cmocka_unit_test(
            test_command_register_peer_deserialize_reconstructs_command),
        cmocka_unit_test(
            test_command_register_peer_deserialize_fails_on_read_past_buffer),
        cmocka_unit_test(
            test_command_register_peer_deserialize_fails_on_invalid_prefix),
        cmocka_unit_test(
            test_command_register_peer_deserialize_fails_on_invalid_command),
        cmocka_unit_test(
            test_command_register_peer_deserialize_fails_on_invalid_input),
        cmocka_unit_test(
            test_command_send_peer_list_serialize_fails_on_invalid_input),
        cmocka_unit_test(
            test_command_send_peer_list_serialize_fails_on_invalid_prefix),
        cmocka_unit_test(
            test_command_send_peer_list_serialize_fails_on_invalid_command),
        cmocka_unit_test(
            test_command_send_peer_list_serialize_creates_nonempty_buffer),
        cmocka_unit_test(
            test_command_send_peer_list_deserialize_reconstructs_command),
        cmocka_unit_test(
            test_command_send_peer_list_deserialize_fails_on_read_past_buffer),
        cmocka_unit_test(
            test_command_send_peer_list_deserialize_fails_on_invalid_prefix),
        cmocka_unit_test(
            test_command_send_peer_list_deserialize_fails_on_invalid_command),
        cmocka_unit_test(
            test_command_send_peer_list_deserialize_fails_on_invalid_input),
        cmocka_unit_test(test_recv_all_mock_works),
        // test_sleep.h
        cmocka_unit_test(test_sleep_microseconds_pauses_program),
        // test_peer_discovery_thread.h
        cmocka_unit_test(test_discover_peers_exits_when_should_stop_is_set),
    };
    return_code = cmocka_run_group_tests(tests, NULL, teardown);
end:
    return return_code;
}
