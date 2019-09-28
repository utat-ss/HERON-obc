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
    enqueue_cmd(&set_rtc_cmd, 0xff0401, 0); 
    enqueue_cmd(&get_rtc_cmd, 0, 0);
    
    execute_next_cmd();
    ASSERT_EQ(RTC_MIN_R, 0);
    ASSERT_EQ(RTC_HOUR_R, 0);
    ASSERT_EQ(RTC_WKDAY_R, 0);
    ASSERT_EQ(RTC_DAY_R, 1);
    ASSERT_EQ(RTC_MONTH_R, 4);
    ASSERT_EQ(RTC_YEAR_R, 0xff);

    ASSERT_FALSE(trans_tx_dec_avail);
    
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 6);
        if (trans_tx_dec_len == 6) {
            // ignore the seconds
            ASSERT_EQ(trans_tx_dec_msg[1], 0);
            ASSERT_EQ(trans_tx_dec_msg[2], 0);
            ASSERT_EQ(trans_tx_dec_msg[3], 1);
            ASSERT_EQ(trans_tx_dec_msg[4], 4);
            ASSERT_EQ(trans_tx_dec_msg[5], 0xff);
        }
    }

    // Try to read 12 bytes of raw memory
    enqueue_cmd(&read_raw_mem_bytes_cmd, 0x200, 12);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail){
        ASSERT_EQ(trans_tx_dec_len, 12);
    }

    // Read the recent status info, just confirm length, content doesn't matter
    enqueue_cmd(&read_rec_status_info_cmd, 0, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 27);
    }

    // Test get and set for data collection, data period
    enqueue_cmd(&set_auto_data_col_period_cmd, 1, 40);
    enqueue_cmd(&get_auto_data_col_period_cmd, 1, 0);

    execute_next_cmd();
    ASSERT_EQ(obc_hk_auto_data_col.period, 40);
    ASSERT_FALSE(trans_tx_dec_avail);

    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 1); // Assuming size of period var is 1 byte
        ASSERT_EQ(trans_tx_dec_msg[0], 40);
    }

    // Enable the data collection
    enqueue_cmd(&set_auto_data_col_enable_cmd, 1, 1);
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);
    ASSERT_TRUE(obc_hk_auto_data_col.enabled);
    enqueue_cmd(&get_auto_data_col_enable_cmd, 1, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 1);
        ASSERT_EQ(trans_tx_dec_msg[0], 1);
    }

    // Disable the data collection
    enqueue_cmd(&set_auto_data_col_enable_cmd, 0, 0);
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);
    ASSERT_FALSE(obc_hk_auto_data_col.enabled);
    enqueue_cmd(&get_auto_data_col_enable_cmd, 1, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 1);
        ASSERT_EQ(trans_tx_dec_msg[0], 0);
    }

    enqueue_cmd(&read_prim_cmd_blocks_cmd, 0, 5);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 5*19);
        if (trans_tx_dec_len == 5*19) {
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
void mem_commands_test(void) {
    int valid_start = 0x3e8;    // 1000
    int valid_end = 0x7d0;      // 2000
    // Set the memory addresses
    enqueue_cmd(&set_mem_sec_start_addr_cmd, 1, valid_start);
    enqueue_cmd(&set_mem_sec_end_addr_cmd, 1, valid_end);
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);
    ASSERT_EQ(obc_hk_mem_section.start_addr, 0x3e8);
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);
    ASSERT_EQ(obc_hk_mem_section.end_addr, 0x7d0);


    // Get the memory addresses
    enqueue_cmd(&get_mem_sec_start_addr_cmd, 1, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 8);
        if (trans_tx_dec_len == 8) {
            ASSERT_EQ(trans_tx_dec_msg[0], 8);
            ASSERT_EQ(trans_tx_dec_msg[1], 0xe);
            ASSERT_EQ(trans_tx_dec_msg[2], 3);
            for (int i = 3; i < 8; ++i) {
                ASSERT_EQ(trans_tx_dec_msg[i], 0);
            }
        }
    }

    enqueue_cmd(&get_mem_sec_end_addr_cmd, 1, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    if (trans_tx_dec_avail) {
        ASSERT_EQ(trans_tx_dec_len, 8);
        if (trans_tx_dec_len == 8) {
            ASSERT_EQ(trans_tx_dec_msg[0], 0);
            ASSERT_EQ(trans_tx_dec_msg[1], 0xd);
            ASSERT_EQ(trans_tx_dec_msg[2], 7);
            for (int i = 3; i < 8; ++i) {
                ASSERT_EQ(trans_tx_dec_msg[i], 0);
            }
        }
    }

    // Try setting the memory addresses to invalid address, it shouldn't change
    // Set the end address to be before the start
    enqueue_cmd(&set_mem_sec_end_addr_cmd, 1, 100);
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);
    ASSERT_EQ(obc_hk_mem_section.end_addr, 0x7d0);
    
    // Set the start address to an out of range address
    enqueue_cmd(&set_mem_sec_start_addr_cmd, 1, 128000);
    execute_next_cmd();
    ASSERT_FALSE(trans_tx_dec_avail);
    ASSERT_EQ(obc_hk_mem_section.start_addr, 0x3e8);
}

test_t t1 = { .name = "basic commands test", .fn = basic_commands_test };
test_t t2 = { .name = "data collection test", .fn = data_collection_test };
test_t t3 = { .name = "memory commands test", .fn = mem_commands_test };

test_t* suite[] = {&t1, &t2};

int main( void ) {
    init_obc_core();

    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
