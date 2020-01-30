/*
 * Harness test for ensuring functionality of command utilities code
 * Tests include:
 *  - Correctness of enqueue_cmd and dequeue_cmd
 */

#include <test/test.h>
#include "../../src/general.h"
#include "../../src/commands.h"
#include "../../src/command_utilities.h"

void dequeue_empty_test(void) {
    uint16_t dq_cmd_id = 0;
    uint32_t dq_arg1 = 0;
    uint32_t dq_arg2 = 0;

    ASSERT_FALSE(dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2));
}


// Enqueue a bunch of commmands in a row, then dequeue them
void triangle_queue_test(void) {
    enqueue_cmd(1, &ping_obc_cmd, 1, 1);
    enqueue_cmd(2, &get_rtc_cmd, 2, 2);
    enqueue_cmd(3, &set_rtc_cmd, 3, 3);
    enqueue_cmd(4, &read_obc_eeprom_cmd, 4, 4);
    enqueue_cmd(5, &erase_obc_eeprom_cmd, 5, 5);

    uint16_t dq_cmd_id = 0;
    uint32_t dq_arg1 = 0;
    uint32_t dq_arg2 = 0;
    uint16_t eq_cmd_id = 1;

    dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2);
    ASSERT_EQ(eq_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &ping_obc_cmd, (uint16_t)current_cmd);
    ASSERT_EQ(eq_cmd_id, dq_arg1);
    ASSERT_EQ(eq_cmd_id++, dq_arg2);


    dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2);
    ASSERT_EQ(eq_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &get_rtc_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(eq_cmd_id, dq_arg1);
    ASSERT_EQ(eq_cmd_id++, dq_arg2);


    dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2);
    ASSERT_EQ(eq_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &set_rtc_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(eq_cmd_id, dq_arg1);
    ASSERT_EQ(eq_cmd_id++, dq_arg2);

    dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2);
    ASSERT_EQ(eq_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &read_obc_eeprom_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(eq_cmd_id, dq_arg1);
    ASSERT_EQ(eq_cmd_id++, dq_arg2);

    dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2);
    ASSERT_EQ(eq_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &erase_obc_eeprom_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(eq_cmd_id, dq_arg1);
    ASSERT_EQ(eq_cmd_id++, dq_arg2);
}

/* Enqueue a few commands, then dequeue less commands than enqueued and 
 * repeat.
 * At the end, dequeue all remaining commands */
void stair_queue_test(void) {
    uint16_t dq_cmd_id = 0;
    uint32_t dq_arg1 = 0;
    uint32_t dq_arg2 = 0;
    uint16_t eq_cmd_id = 1;
    uint16_t check_cmd_id = 1;

    ASSERT_TRUE(enqueue_cmd(eq_cmd_id, &ping_obc_cmd, eq_cmd_id, eq_cmd_id));
    ++eq_cmd_id;
    ASSERT_TRUE(enqueue_cmd(eq_cmd_id, &get_rtc_cmd, eq_cmd_id, eq_cmd_id));
    ++eq_cmd_id;

    ASSERT_TRUE(dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2));
    ASSERT_EQ(check_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &ping_obc_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(check_cmd_id, dq_arg1);
    ASSERT_EQ(check_cmd_id++, dq_arg2);

    ASSERT_TRUE(enqueue_cmd(eq_cmd_id, &set_rtc_cmd, eq_cmd_id, eq_cmd_id));
    ++eq_cmd_id;

    ASSERT_TRUE(enqueue_cmd(eq_cmd_id, &read_obc_eeprom_cmd, eq_cmd_id, eq_cmd_id));
    ++eq_cmd_id;

    ASSERT_TRUE(dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2));
    ASSERT_EQ(check_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &get_rtc_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(check_cmd_id, dq_arg1);
    ASSERT_EQ(check_cmd_id++, dq_arg2);

    ASSERT_TRUE(enqueue_cmd(eq_cmd_id, &erase_obc_eeprom_cmd, eq_cmd_id, eq_cmd_id));
    ++eq_cmd_id;
    ASSERT_TRUE(enqueue_cmd(eq_cmd_id, &read_obc_ram_byte_cmd, eq_cmd_id, eq_cmd_id));
    ++eq_cmd_id;

    ASSERT_TRUE(dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2));
    ASSERT_EQ(check_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &set_rtc_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(check_cmd_id, dq_arg1);
    ASSERT_EQ(check_cmd_id++, dq_arg2);

    ASSERT_TRUE(enqueue_cmd(eq_cmd_id, &set_beacon_inhibit_enable_cmd, eq_cmd_id, eq_cmd_id));
    ++eq_cmd_id;
    ASSERT_TRUE(enqueue_cmd(eq_cmd_id, &send_eps_can_msg_cmd, eq_cmd_id, eq_cmd_id));
    ++eq_cmd_id;

    ASSERT_TRUE(dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2));
    ASSERT_EQ(check_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &read_obc_eeprom_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(check_cmd_id, dq_arg1);
    ASSERT_EQ(check_cmd_id++, dq_arg2);

    ASSERT_TRUE(enqueue_cmd(eq_cmd_id, &send_pay_can_msg_cmd, eq_cmd_id, eq_cmd_id));
    ++eq_cmd_id;
    
    // Queue should be full
    ASSERT_FALSE(enqueue_cmd(eq_cmd_id, &act_pay_motors_cmd, eq_cmd_id, eq_cmd_id));
    ++eq_cmd_id;

    dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2);
    ASSERT_EQ(check_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &erase_obc_eeprom_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(check_cmd_id, dq_arg1);
    ASSERT_EQ(check_cmd_id++, dq_arg2);

    dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2);
    ASSERT_EQ(check_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &read_obc_ram_byte_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(check_cmd_id, dq_arg1);
    ASSERT_EQ(check_cmd_id++, dq_arg2);

    dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2);
    ASSERT_EQ(check_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &set_beacon_inhibit_enable_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(check_cmd_id, dq_arg1);
    ASSERT_EQ(check_cmd_id++, dq_arg2);

    dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2);
    ASSERT_EQ(check_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &send_eps_can_msg_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(check_cmd_id, dq_arg1);
    ASSERT_EQ(check_cmd_id++, dq_arg2);

    dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2);
    ASSERT_EQ(check_cmd_id, dq_cmd_id);
    ASSERT_EQ((uint16_t) &send_pay_can_msg_cmd, (uint16_t) current_cmd);
    ASSERT_EQ(check_cmd_id, dq_arg1);
    ASSERT_EQ(check_cmd_id++, dq_arg2);

    // Queue should be empty
    ASSERT_FALSE(dequeue_cmd(&dq_cmd_id, (cmd_t **) &current_cmd, &dq_arg1, &dq_arg2));
}

// Miscellaneous constants and configuration parameters
void params_test(void) {
    ASSERT_EQ(CAN_PAY_OPT_OD_FIELD_COUNT, CAN_PAY_OPT_FL_FIELD_COUNT);
    ASSERT_EQ(CAN_PAY_OPT_OD_FIELD_COUNT + CAN_PAY_OPT_FL_FIELD_COUNT,
        CAN_PAY_OPT_TOT_FIELD_COUNT);

    ASSERT_EQ((CAN_PAY_OPT_OD_FIELD_COUNT * MEM_BYTES_PER_FIELD) + MEM_BYTES_PER_HEADER,
        CMD_READ_MEM_MAX_COUNT);

    // TODO - assert memory addresses are consecutive between sections and match intended sizes
}

test_t t1 = {.name = "dequeue empty test", .fn = dequeue_empty_test}; 
test_t t2 = {.name = "triangle_queue test", .fn = triangle_queue_test};
test_t t3 = {.name = "stair_queue test", .fn = stair_queue_test};
test_t t4 = {.name = "params test", .fn = params_test};

test_t* suite[] = { &t1, &t2, &t3, &t4 };

int main( void ) {
    init_obc_phase1();

    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
