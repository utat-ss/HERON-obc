#include <stdlib.h>
#include <test/test.h>

#include "../../src/general.h"

#define TEST_SIZE 5
#define NUM_EPS_HK_FIELDS 23
#define NUM_EPS_CTRL_FIELDS 14
#define NUM_PAY_HK_FIELDS 16
#define NUM_PAY_OPT_FIELDS 32
#define NUM_PAY_CTRL_FIELDS 12

/* Positions denoted by 1 expect non-zero data */
uint8_t eps_ctrl_data[14] = {0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0};
uint8_t pay_ctrl_data[12] = {0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0};

/* Send a message to request data from all EPS fields */
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

/* Send a message to request data from pay optical */
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


test_t t1 = { .name = "EPS Housekeeping Test", .fn = eps_hk_test };
test_t t2 = { .name = "EPS Control Test", .fn = eps_ctrl_test };
test_t t3 = { .name = "PAY Housekeeping Test", .fn = pay_hk_test };
test_t t4 = { .name = "PAY Optical Test", .fn = pay_opt_test };
test_t t5 = { .name = "PAY Control Test", .fn = pay_ctrl_test };

test_t* suite[] = { &t1, &t2, &t3, &t4, &t5};

int main(void) {
    init_obc_core();
    run_tests(suite, TEST_SIZE);
    return 0;
}
