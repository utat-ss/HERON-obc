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

    enqueue_cmd(&read_mem_bytes_cmd, 0x200, 12);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    ASSERT_EQ(trans_tx_dec_len, 12);

    enqueue_cmd(&get_subsys_status_cmd, 0, 0);
    execute_next_cmd();
    ASSERT_TRUE(trans_tx_dec_avail);
    ASSERT_EQ(trans_tx_dec_len, 27);

}

test_t t1 = { .name = "basic commands test", .fn = basic_commands_test };

test_t* suite[] = {};

int main( void ) {
    init_obc_core();

    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
