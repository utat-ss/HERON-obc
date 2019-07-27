#include <stdlib.h>
#include <test/test.h>
#include <uptime/uptime.h>

#include "../../src/general.h"


#define TEST_SIZE 7

#define NUM_EPS_HK_FIELDS 23
#define NUM_EPS_CTRL_FIELDS 14
#define NUM_PAY_HK_FIELDS 16
#define NUM_PAY_OPT_FIELDS 32
#define NUM_PAY_CTRL_FIELDS 12

#define EPS_RESTART_COUNT 10
#define EPS_RESET_REQUEST 7
#define EPS_RESET_REASON 11

#define PAY_RESTART_COUNT 8
#define PAY_RESET_REQUEST 5
#define PAY_RESET_REASON 9

/* Positions denoted by 1 expect non-zero data */
uint8_t eps_ctrl_data[NUM_EPS_CTRL_FIELDS] = {0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0};
uint8_t pay_ctrl_data[NUM_PAY_CTRL_FIELDS] = {0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0};

/* Sends a message to request data from all EPS Housekeeping fields */
void eps_hk_test(void){
    for (uint8_t field = 0; field < NUM_EPS_HK_FIELDS; field++){
        uint8_t msg[8] = {0x00};
        enqueue_eps_hk_tx_msg(field);
        send_next_eps_tx_msg();
        /* Delay to give time to send and receive message */
        _delay_ms(100);
        ASSERT_FALSE(queue_empty(&data_rx_msg_queue));
        dequeue(&data_rx_msg_queue, msg);
        ASSERT_EQ(msg[0], 0x00);
        ASSERT_EQ(msg[1], 0x00);
        ASSERT_EQ(msg[2], CAN_EPS_HK);
        ASSERT_EQ(msg[3], field);
        /* Assert that data portion of message is not all zero */
        ASSERT_NEQ(msg[4] | msg[5]| msg[6] | msg[7], 0x00);
    }
}

/* Sends a message to request data from all EPS Control fields and
    verifies that data is sent back when expected */
void eps_ctrl_test(void){
    uint32_t data = 0x00;
    for (uint8_t field = 0; field < NUM_EPS_CTRL_FIELDS; field++){
        uint8_t msg[8] = {0x00};
        enqueue_eps_ctrl_tx_msg(field, data);
        send_next_eps_tx_msg();
        /* Delay to give time to send and receive message */
        _delay_ms(100);
        ASSERT_FALSE(queue_empty(&data_rx_msg_queue));
        dequeue(&data_rx_msg_queue, msg);
        ASSERT_EQ(msg[0], 0x00);
        ASSERT_EQ(msg[1], 0x00);
        ASSERT_EQ(msg[2], CAN_EPS_CTRL);
        ASSERT_EQ(msg[3], field);
        /* Assert that data portion of message is not all zero if applicable*/
        if (eps_ctrl_data[field]){
            ASSERT_NEQ(msg[4] | msg[5]| msg[6] | msg[7], 0x00);
        }
        else {
            ASSERT_EQ(msg[4] | msg[5]| msg[6] | msg[7], 0x00);
        }
    }
}

/* Sends a message to request data from all PAY Housekeeping fields and
    verifies that data is sent back when expected */
void pay_hk_test(void){
    for (uint8_t field = 0; field < NUM_PAY_HK_FIELDS; field++){
        uint8_t msg[8] = {0x00};
        enqueue_pay_hk_tx_msg(field);
        send_next_pay_tx_msg();
        /* Delay to give time to send and receive message */
        _delay_ms(100);
        ASSERT_FALSE(queue_empty(&data_rx_msg_queue));
        dequeue(&data_rx_msg_queue, msg);
        ASSERT_EQ(msg[0], 0x00);
        ASSERT_EQ(msg[1], 0x00);
        ASSERT_EQ(msg[2], CAN_PAY_HK);
        ASSERT_EQ(msg[3], field);
        /* Assert that data portion of message is not all zero */
        ASSERT_NEQ(msg[4] | msg[5]| msg[6] | msg[7], 0x00);
    }
}

/* Sends a message to request data from all PAY Optical fields and
    verifies that data is sent back when expected */
void pay_opt_test(void){
    for (uint8_t field = 0; field < NUM_PAY_OPT_FIELDS; field++){
        uint8_t msg[8] = {0x00};
        enqueue_pay_opt_tx_msg(field);
        send_next_pay_tx_msg();
        /* Delay to give time to send and receive message */
        _delay_ms(100);
        ASSERT_FALSE(queue_empty(&data_rx_msg_queue));
        dequeue(&data_rx_msg_queue, msg);
        ASSERT_EQ(msg[0], 0x00);
        ASSERT_EQ(msg[1], 0x00);
        ASSERT_EQ(msg[2], CAN_PAY_OPT);
        ASSERT_EQ(msg[3], field);
        /* Assert that data portion of message is not all zero */
        ASSERT_NEQ(msg[4] | msg[5]| msg[6] | msg[7], 0x00);
    }
}

/* Sends a message to request data from all PAY Control fields and
    verifies that data is sent back when expected */
void pay_ctrl_test(void){
    uint32_t data = 0x00;
    for (uint8_t field = 0; field < NUM_PAY_CTRL_FIELDS; field++){
        uint8_t msg[8] = {0x00};
        enqueue_pay_ctrl_tx_msg(field, data);
        send_next_pay_tx_msg();
        /* Delay to give time to send and receive message */
        _delay_ms(100);
        ASSERT_FALSE(queue_empty(&data_rx_msg_queue));
        dequeue(&data_rx_msg_queue, msg);
        ASSERT_EQ(msg[0], 0x00);
        ASSERT_EQ(msg[1], 0x00);
        ASSERT_EQ(msg[2], CAN_PAY_CTRL);
        ASSERT_EQ(msg[3], field);
        /* Assert that data portion of message is not all zero if applicable */
        if (pay_ctrl_data[field]){
            ASSERT_NEQ(msg[4] | msg[5]| msg[6] | msg[7], 0x00);
        }
        else {
            ASSERT_EQ(msg[4] | msg[5]| msg[6] | msg[7], 0x00);
        }
    }
}

/* Resets the PAY SSM and verifies that the reset counter increments correctly
    and the restart reason is correct */
void pay_reset_test(void){
    uint32_t num_resets = 0x00;
    uint32_t num_resets_new = 0x00;
    uint8_t msg[8] = {0x00};
    uint8_t data = 0;

    /* Request and store current amount of resets */
    enqueue_pay_ctrl_tx_msg(PAY_RESTART_COUNT, data);
    send_next_pay_tx_msg();
    _delay_ms(100);
    dequeue(&data_rx_msg_queue, msg);
    num_resets = (uint32_t) msg[4] << 24| (uint32_t) msg[5] << 16 | (uint32_t) msg[6] << 8 | (uint32_t) msg[7];

    /* Request reset */
    enqueue_pay_ctrl_tx_msg(PAY_RESET_REQUEST, data);
    send_next_pay_tx_msg();
    _delay_ms(100);
    dequeue(&data_rx_msg_queue, msg);

    /* Get number of resets */
    enqueue_pay_ctrl_tx_msg(PAY_RESTART_COUNT, data);
    send_next_pay_tx_msg();
    _delay_ms(100);
    dequeue(&data_rx_msg_queue, msg);
    num_resets_new = (uint32_t) msg[4] << 24| (uint32_t) msg[5] << 16 | (uint32_t) msg[6] << 8 | (uint32_t) msg[7];
    ASSERT_EQ(num_resets_new, num_resets + 1);

    /* Get reset reason and assert that it is due to wdt timeout */
    enqueue_pay_ctrl_tx_msg(PAY_RESET_REASON, data);
    send_next_pay_tx_msg();
    _delay_ms(100);
    dequeue(&data_rx_msg_queue, msg);
    restart_reason = (uint32_t) msg[4] << 24| (uint32_t) msg[5] << 16 | (uint32_t) msg[6] << 8 | (uint32_t) msg[7];
    ASSERT_EQ(restart_reason, UPTIME_RESTART_REASON_WDT_TIMEOUT);
}

/* Resets the EPS SSM and verifies that the reset counter increments correctly
    and the restart reason is correct */
void eps_reset_test(void){
    uint32_t num_resets = 0x00;
    uint32_t num_resets_new = 0x00;
    uint8_t msg[8] = {0x00};
    uint8_t data = 0x00;
    uint32_t restart_reason = 0x00;

    /* Request and store current amount of resets */
    enqueue_eps_ctrl_tx_msg(EPS_RESTART_COUNT, data);
    send_next_eps_tx_msg();
    _delay_ms(100);
    dequeue(&data_rx_msg_queue, msg);
    num_resets = (uint32_t) msg[4] << 24| (uint32_t) msg[5] << 16 | (uint32_t) msg[6] << 8 | (uint32_t) msg[7];

    /* Request reset */
    enqueue_eps_ctrl_tx_msg(EPS_RESET_REQUEST, data);
    send_next_eps_tx_msg();
    _delay_ms(100);
    dequeue(&data_rx_msg_queue, msg);

    /* Get number of resets */
    enqueue_eps_ctrl_tx_msg(EPS_RESTART_COUNT, data);
    send_next_eps_tx_msg();
    _delay_ms(100);
    dequeue(&data_rx_msg_queue, msg);
    num_resets_new = (uint32_t) msg[4] << 24| (uint32_t) msg[5] << 16 | (uint32_t) msg[6] << 8 | (uint32_t) msg[7];
    ASSERT_EQ(num_resets_new, num_resets + 1);

    /* Get reset reason and assert that it is due to wdt timeout */
    enqueue_eps_ctrl_tx_msg(EPS_RESET_REASON, data);
    send_next_eps_tx_msg();
    _delay_ms(100);
    dequeue(&data_rx_msg_queue, msg);
    restart_reason = (uint32_t) msg[4] << 24| (uint32_t) msg[5] << 16 | (uint32_t) msg[6] << 8 | (uint32_t) msg[7];
    ASSERT_EQ(restart_reason, UPTIME_RESTART_REASON_WDT_TIMEOUT);
}


test_t t1 = { .name = "EPS Housekeeping Test", .fn = eps_hk_test };
test_t t2 = { .name = "EPS Control Test", .fn = eps_ctrl_test };
test_t t3 = { .name = "PAY Housekeeping Test", .fn = pay_hk_test };
test_t t4 = { .name = "PAY Optical Test", .fn = pay_opt_test };
test_t t5 = { .name = "PAY Control Test", .fn = pay_ctrl_test };
test_t t6 = { .name = "PAY Reset Test", .fn = pay_reset_test };
test_t t7 = { .name = "EPS Reset Test", .fn = pay_reset_test };

test_t* suite[] = { &t1, &t2, &t3, &t4, &t5, &t6, &t7};

int main(void) {
    init_obc_core();
    run_tests(suite, TEST_SIZE);
    return 0;
}
