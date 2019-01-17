#include "can_commands.h"

queue_t eps_tx_msg_queue;
queue_t pay_tx_msg_queue;
queue_t data_rx_msg_queue;

uint32_t eps_hk_data[CAN_EPS_HK_GET_COUNT] = { 0 };
uint32_t pay_hk_data[CAN_PAY_HK_GET_COUNT] = { 0 };
uint32_t pay_sci_data[CAN_PAY_SCI_GET_COUNT] = { 0 };


void handle_pay_hk(const uint8_t* data);
void handle_pay_sci(const uint8_t* data);
void handle_eps_hk(const uint8_t* data);
void handle_pay_motor(const uint8_t* data);


// TODO
void handle_rx_msg(void) {
    print("%s\n", __FUNCTION__);
    if (queue_empty(&data_rx_msg_queue)) {
        return;
    }

    else {
        uint8_t data[8];
        // print("Dequeued from data_rx_msg_queue\n");
        dequeue(&data_rx_msg_queue, data);

        uint8_t message_type = data[1];

        switch (message_type) {
            case CAN_EPS_HK:
                handle_eps_hk(data);
                break;
            case CAN_PAY_HK:
                handle_pay_hk(data);
                break;
            case CAN_PAY_OPT:
                handle_pay_sci(data);
                break;
            case CAN_PAY_EXP:
                handle_pay_motor(data);
                break;
            default:
                print("Invalid RX\n");
                break;
        }
    }
}


void handle_eps_hk(const uint8_t* data){
    uint8_t field_num = data[2];

    // Save the data to the local array
    if (field_num < CAN_EPS_HK_GET_COUNT) {
        eps_hk_data[field_num] =
                (((uint32_t) data[3]) << 16) |
                (((uint32_t) data[4]) << 8) |
                ((uint32_t) data[5]);
    }

    // Request the next field (if not done yet)
    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_EPS_HK_GET_COUNT) {
        enqueue_eps_hk_req_can_msg(next_field_num);
    }
}


void handle_pay_hk(const uint8_t* data){
    uint8_t field_num = data[2];

    // Save the data to the local array
    if (field_num < CAN_PAY_HK_GET_COUNT) {
        // print("modifying pay_hk_data[%u]\n", field_num);
        pay_hk_data[field_num] =
                (((uint32_t) data[3]) << 16) |
                (((uint32_t) data[4]) << 8) |
                ((uint32_t) data[5]);
    }

    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_PAY_HK_GET_COUNT) {
        enqueue_pay_hk_req_can_msg(next_field_num);
    }
}

void handle_pay_sci(const uint8_t* data){
    uint8_t field_num = data[2];

    // Save the data to the local array
    if (field_num < CAN_PAY_SCI_GET_COUNT) {
        // Save data
        pay_sci_data[field_num] =
                (((uint32_t) data[3]) << 16) |
                (((uint32_t) data[4]) << 8) |
                ((uint32_t) data[5]);
    }

    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_PAY_SCI_GET_COUNT){
        enqueue_pay_sci_req_can_msg(next_field_num);
    }
}

void handle_pay_motor(const uint8_t* data){
    uint8_t field_num = data[2];

    // TODO - what to do here?
    if (field_num == CAN_PAY_EXP_POP) {
        print("PAY_MOTOR done\n");
    }
}



/*
Enqueues a CAN message onto eps_tx_msg_queue to request the specified HK field number.
field_num - Field number to request
*/
void enqueue_eps_hk_req_can_msg(uint8_t field_num) {
    uint8_t msg[8] = { 0 };
    msg[0] = 0;   // TODO
    msg[1] = CAN_EPS_HK;
    msg[2] = field_num;

    enqueue(&eps_tx_msg_queue, msg);
}

/*
Enqueues a CAN message onto pay_tx_msg_queue to request the specified HK field number.
field_num - Field number to request
*/
void enqueue_pay_hk_req_can_msg(uint8_t field_num) {
    uint8_t msg[8] = { 0 };
    msg[0] = 0;   // TODO
    msg[1] = CAN_PAY_HK;
    msg[2] = field_num;

    enqueue(&pay_tx_msg_queue, msg);
}

/*
Enqueues a CAN message onto pay_tx_msg_queue to request the specified SCI field number.
field_num - Field number to request
*/
void enqueue_pay_sci_req_can_msg(uint8_t field_num) {
    uint8_t msg[8] = { 0 };
    msg[0] = 0;   // TODO
    msg[1] = CAN_PAY_OPT;
    msg[2] = field_num;

    enqueue(&pay_tx_msg_queue, msg);
}

/*
Enqueues a CAN message onto pay_tx_msg_queue to command actuating the motors.
*/
void enqueue_actuate_motor_can_msg(void) {
    uint8_t msg[8] = { 0 };
    msg[0] = 0;   // TODO
    msg[1] = CAN_PAY_EXP;
    msg[2] = CAN_PAY_EXP_POP;

    enqueue(&pay_tx_msg_queue, msg);
}
