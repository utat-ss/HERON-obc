/**
 * Harness tests for the command code on OBC. Tests that OBC responds and 
 * processes transceiver commands.
 */

#include <test/test.h>
#include "../../src/command_utilities.h"
#include "../../src/commands.h"
#include "../../src/general.h"
#include "../../src/transceiver.h"

void basic_commands_test (void) {
    enqueue_cmd(&set_rtc_cmd, 0, 0);
    enqueue_cmd(&get_rtc_cmd, 0, 0);
    
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);
    
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 6);
        if (trans_tx_dec_len == 6) {
            ASSERT_EQ(trans_tx_dec_msg[1], 0);
            ASSERT_EQ(trans_tx_dec_msg[2], 0);
            ASSERT_EQ(trans_tx_dec_msg[3], 0);
            ASSERT_EQ(trans_tx_dec_msg[4], 0);
            ASSERT_EQ(trans_tx_dec_msg[5], 0);
        }
    }

    int curr_block_num = 10;
    enqueue_cmd(&set_cur_block_num_cmd, 1, 10);
    enqueue_cmd(&get_cur_block_num_cmd, 1, 0);
    
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);
    
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 4);
        if (trans_tx_dec_len == 4) {
            ASSERT_EQ(trans_tx_dec_msg[0], 10);
            ASSERT_EQ(trans_tx_dec_msg[1], 0);
            ASSERT_EQ(trans_tx_dec_msg[2], 0);
            ASSERT_EQ(trans_tx_dec_msg[3], 0);
        }
    }

    enqueue_cmd(&read_raw_mem_bytes_cmd, 0x200, 12);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail){
        ASSERT_EQ(trans_tx_dec_len, 12);
    }

    enqueue_cmd(&read_rec_status_info_cmd, 0, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 27);
    }

    // Collect a data block and make sure block number increments
    enqueue_cmd(&col_data_block_cmd, 0, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 4);
    }
    ++curr_block_num;   // The block number should increase with a read

    enqueue_cmd(&get_cur_block_num_cmd, 0, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 4);
        ASSERT_EQ(trans_tx_dec_msg[0], curr_block_num);
    }
}

/**
 * Check memory commands:
 * - Setting and getting memory addresses
 * - Check corner cases such as setting invalid memory addresses (out of 
 *      range and end address < start address)
 */
void mem_commands_test() {
    int valid_start = 1000;
    int valid_end = 2000;
    // Set the memory addresses
    enqueue_cmd(&set_mem_sec_start_addr_cmd, 1, valid_start);
    enqueue_cmd(&set_mem_sec_end_addr_cmd, 1, valid_end);
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);

    // Get the memory addresses
    enqueue_cmd(&get_mem_sec_start_addr_cmd, 1, 0);
    enqueue_cmd(&get_mem_sec_end_addr_cmd, 1, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        // TODO: Check message, return data not yet implemented
    }
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        // TODO: Check message, return data not yet implemented
    }

    // Try setting the memory addresses to invalid address, it shouldn't change
    // Set the end address to be before the start
    enqueue_cmd(&set_mem_sec_end_addr_cmd, 1, 100);
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);
    
    // Set the start address to an out of range address
    enqueue_cmd(&set_mem_sec_start_addr_cmd, 1, 128000);
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);
}

test_t t1 = { .name = "basic commands test", .fn = basic_commands_test };
test_t t2 = { .name = "memory commands test", .fn = mem_commands_test };

test_t* suite[] = {&t1, &t2};

int main( void ) {
    init_obc_core();

    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
