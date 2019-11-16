/**
 * Harness tests for the command code on OBC. Tests that OBC responds and 
 * processes transceiver commands.
 * 
 * Not included in test as of yet:
 *  - The erase commands
 *  - Actuate pay motors
 *  - 
 *  - Reset subsystem
 *  - Resync data collection timers
 */

#include <test/test.h>
#include "../../src/command_utilities.h"
#include "../../src/commands.h"
#include "../../src/general.h"
#include "../../src/transceiver.h"

/**
 * Test some basic set and get commands
 */
void basic_commands_test (void) {
    // Set the date to yr = 255, month = 4, date = 1, disregard time
    enqueue_cmd(&set_rtc_cmd, 0x210401, 0); 
    enqueue_cmd(&get_rtc_cmd, 0, 0);
    
    execute_next_cmd();
    rtc_date_t date = read_rtc_date();
    rtc_time_t time = read_rtc_time();
    ASSERT_EQ(time.ss, 0);  // This could fail
    ASSERT_EQ(time.mm, 0);
    ASSERT_EQ(time.hh, 0);
    ASSERT_EQ(date.dd, 1);
    ASSERT_EQ(date.mm, 4);
    ASSERT_EQ(date.yy, 0x21);

    ASSERT_TRUE(trans_tx_dec_avail);
    
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 15);
        if (trans_tx_dec_len == 6) {
            ASSERT_EQ(trans_tx_dec_msg[9], 0);  // This might fail
            ASSERT_EQ(trans_tx_dec_msg[10], 0);
            ASSERT_EQ(trans_tx_dec_msg[11], 0);
            ASSERT_EQ(trans_tx_dec_msg[12], 1);
            ASSERT_EQ(trans_tx_dec_msg[13], 4);
            ASSERT_EQ(trans_tx_dec_msg[14], 0xff);
        }
    }

    // Try to read 12 bytes of raw memory
    enqueue_cmd(&read_raw_mem_bytes_cmd, 0x200, 12);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail){
        ASSERT_EQ(trans_tx_dec_len, 21);
    }

    // Read the recent status info, just confirm length, content doesn't matter
    enqueue_cmd(&read_rec_status_info_cmd, 0, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 42);    // OBC has 5 field --> 33 bytes in total + 9
    }

    // Test get and set for data collection, data period
    enqueue_cmd(&set_auto_data_col_period_cmd, 1, 40);
    enqueue_cmd(&get_auto_data_col_settings_cmd, 1, 0);

    execute_next_cmd();
    ASSERT_EQ(obc_hk_auto_data_col.period, 40);
    ASSERT_TRUE(trans_tx_dec_avail);

    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 13); // Assuming size of period var is 1 byte
        ASSERT_EQ(trans_tx_dec_msg[12], 40);
    }

    // Enable the data collection
    enqueue_cmd(&set_auto_data_col_enable_cmd, 1, 1);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    ASSERT_TRUE(obc_hk_auto_data_col.enabled);
    enqueue_cmd(&get_auto_data_col_settings_cmd, 1, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 10);
        ASSERT_EQ(trans_tx_dec_msg[9], 1);
    }

    // Disable the data collection
    enqueue_cmd(&set_auto_data_col_enable_cmd, CMD_OBC_HK, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    ASSERT_FALSE(obc_hk_auto_data_col.enabled);
    enqueue_cmd(&get_auto_data_col_settings_cmd, CMD_OBC_HK, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 10);
        ASSERT_EQ(trans_tx_dec_msg[9], 0);
    }

    enqueue_cmd(&read_prim_cmd_blocks_cmd, 0, 5);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 5*19+9);
        if (trans_tx_dec_len == 5*19+9) {
            // TODO: Figure out how to check that the commands match up
        }
    }
}

/**
 *  Set the current block number for OBC, then read a data block.
 *  The current block number should increment after the read
 */
void data_collection_test(void) {
    int curr_block_num = 10;
    enqueue_cmd(&set_cur_block_num_cmd, CMD_OBC_HK, 10);
    enqueue_cmd(&get_cur_block_nums_cmd, CMD_OBC_HK, 0);
    
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 13);
        if (trans_tx_dec_len == 13) {
            ASSERT_EQ(trans_tx_dec_msg[9], 0);
            ASSERT_EQ(trans_tx_dec_msg[10], 0);
            ASSERT_EQ(trans_tx_dec_msg[11], 0);
            ASSERT_EQ(trans_tx_dec_msg[12], 10);
        }
    }

    // Collect a data block and make sure block number increments
    enqueue_cmd(&col_data_block_cmd, CMD_OBC_HK, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 13);
    }
    ++curr_block_num;   // The block number should increase with a read

    enqueue_cmd(&get_cur_block_nums_cmd, CMD_OBC_HK, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 13);
        ASSERT_EQ(trans_tx_dec_msg[12], curr_block_num);
    }
}

/**
 * Check memory commands:
 * - Setting and getting memory addresses
 * - Check corner cases such as setting invalid memory addresses (out of 
 *      range and end address < start address)
 */
void mem_commands_test(void) {
    int valid_start = 0x3e8;    // 1000
    int valid_end = 0x7d0;      // 2000
    // Set the memory addresses
    enqueue_cmd(&set_mem_sec_start_addr_cmd, 1, valid_start);
    enqueue_cmd(&set_mem_sec_end_addr_cmd, 1, valid_end);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    ASSERT_EQ(obc_hk_mem_section.start_addr, 0x3e8);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    ASSERT_EQ(obc_hk_mem_section.end_addr, 0x7d0);


    // Get the memory addresses
    enqueue_cmd(&get_mem_sec_addrs_cmd, CMD_OBC_HK, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 13);
        ASSERT_EQ(trans_tx_dec_msg[9], 0);
        ASSERT_EQ(trans_tx_dec_msg[10], 0);
        ASSERT_EQ(trans_tx_dec_msg[11], 3);
        ASSERT_EQ(trans_tx_dec_msg[12], 0xe8);

    }

    enqueue_cmd(&get_mem_sec_addrs_cmd, CMD_OBC_HK, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 13);
        ASSERT_EQ(trans_tx_dec_msg[9], 0);
        ASSERT_EQ(trans_tx_dec_msg[10], 0);
        ASSERT_EQ(trans_tx_dec_msg[11], 7);
        ASSERT_EQ(trans_tx_dec_msg[12], 0xd0);
    }

    // Try setting the memory addresses to invalid address, it shouldn't change
    // Set the end address to be before the start
    enqueue_cmd(&set_mem_sec_end_addr_cmd, 1, 100);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    ASSERT_EQ(obc_hk_mem_section.end_addr, 0x7d0);
    
    // Set the start address to an out of range address
    enqueue_cmd(&set_mem_sec_start_addr_cmd, 1, 0x600001);  // TODO: Command implementation needs to be fixed to check for out of bounds
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    ASSERT_EQ(obc_hk_mem_section.start_addr, 0x3e8);
}

test_t t1 = { .name = "basic commands test", .fn = basic_commands_test };
test_t t2 = { .name = "data collection test", .fn = data_collection_test };
test_t t3 = { .name = "memory commands test", .fn = mem_commands_test };

test_t* suite[] = {&t1, &t2, &t3};

int main( void ) {
    init_obc_phase1();

    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
