#include "can_commands.h"

queue_t eps_tx_msg_queue;
queue_t pay_tx_msg_queue;
queue_t data_rx_msg_queue;

mem_header_t eps_hk_header;
uint32_t eps_hk_fields[CAN_EPS_HK_FIELD_COUNT] = { 0 };
mem_header_t pay_hk_header;
uint32_t pay_hk_fields[CAN_PAY_HK_FIELD_COUNT] = { 0 };
mem_header_t pay_opt_header;
uint32_t pay_opt_fields[CAN_PAY_OPT_FIELD_COUNT] = { 0 };

void handle_eps_hk(const uint8_t* data);
void handle_eps_ctrl(const uint8_t* data);
void handle_pay_hk(const uint8_t* data);
void handle_pay_opt(const uint8_t* data);
void handle_pay_ctrl(const uint8_t* data);


void handle_rx_msg(void) {
    // print("%s\n", __FUNCTION__);
    if (queue_empty(&data_rx_msg_queue)) {
        return;
    }

    else {
        uint8_t data[8] = {0x00};
        // print("Dequeued from data_rx_msg_queue\n");
        dequeue(&data_rx_msg_queue, data);

        uint8_t message_type = data[1];

        switch (message_type) {
            case CAN_EPS_HK:
                handle_eps_hk(data);
                break;
            case CAN_EPS_CTRL:
                handle_eps_ctrl(data);
                break;
            case CAN_PAY_HK:
                handle_pay_hk(data);
                break;
            case CAN_PAY_OPT:
                handle_pay_opt(data);
                break;
            case CAN_PAY_CTRL:
                handle_pay_ctrl(data);
                break;
            default:
                // print("Invalid RX\n");
                break;
        }
    }
}


void handle_eps_hk(const uint8_t* data){
    uint8_t field_num = data[2];

    // Save the data to the local array
    if (field_num < CAN_EPS_HK_FIELD_COUNT) {
        // print("Received EPS_HK #%u\n", field_num);
        eps_hk_fields[field_num] =
                (((uint32_t) data[3]) << 16) |
                (((uint32_t) data[4]) << 8) |
                ((uint32_t) data[5]);
    }

    // Request the next field (if not done yet)
    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_EPS_HK_FIELD_COUNT) {
        enqueue_eps_hk_tx_msg(next_field_num);
    }

    // If we have received all the fields
    if ((current_cmd.fn == req_eps_hk_cmd.fn) && (field_num == CAN_EPS_HK_FIELD_COUNT - 1)) {
        print("Done EPS_HK\n");
        finish_current_cmd(true);
    }
}

// TODO
void handle_eps_ctrl(const uint8_t* data){
    // uint8_t field_num = data[2];
    //
    // // If we have received the field
    // if ((current_cmd.fn == pop_blister_packs_cmd.fn) && (field_num == CAN_PAY_EXP_POP)) {
    //     print("Done PAY_EXP_POP\n");
    //     finish_current_cmd(true);
    // }
}

void handle_pay_hk(const uint8_t* data){
    uint8_t field_num = data[2];

    // Save the data to the local array
    if (field_num < CAN_PAY_HK_FIELD_COUNT) {
        // print("modifying pay_hk_fields[%u]\n", field_num);
        pay_hk_fields[field_num] =
                (((uint32_t) data[3]) << 16) |
                (((uint32_t) data[4]) << 8) |
                ((uint32_t) data[5]);
    }

    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_PAY_HK_FIELD_COUNT) {
        enqueue_pay_hk_tx_msg(next_field_num);
    }

    // If we have received all the fields
    if ((current_cmd.fn == req_pay_hk_cmd.fn) && (field_num == CAN_PAY_HK_FIELD_COUNT - 1)) {
        print("Done PAY_HK\n");
        finish_current_cmd(true);
    }
}

void handle_pay_opt(const uint8_t* data){
    uint8_t field_num = data[2];

    // Save the data to the local array
    if (field_num < CAN_PAY_OPT_FIELD_COUNT) {
        // Save data
        pay_opt_fields[field_num] =
                (((uint32_t) data[3]) << 16) |
                (((uint32_t) data[4]) << 8) |
                ((uint32_t) data[5]);
    }

    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_PAY_OPT_FIELD_COUNT){
        enqueue_pay_opt_tx_msg(next_field_num);
    }

    // If we have received all the fields
    if ((current_cmd.fn == req_pay_opt_cmd.fn) && (field_num == CAN_PAY_OPT_FIELD_COUNT - 1)) {
        print("Done PAY_OPT\n");
        finish_current_cmd(true);
    }
}

// TODO - fix actuation
void handle_pay_ctrl(const uint8_t* data){
    // uint8_t field_num = data[2];
    //
    // // If we have received the field
    // if ((current_cmd.fn == pop_blister_packs_cmd.fn) && (field_num == CAN_PAY_EXP_POP)) {
    //     print("Done PAY_EXP_POP\n");
    //     finish_current_cmd(true);
    // }
}



/*
Enqueues a CAN message onto the specified queue to request the specified message
    type and field number.
queue - Queue to enqueue the message to
msg_type - Message type to request (byte 1)
field_num - Field number to request (byte 2)
*/
void enqueue_tx_msg(queue_t* queue, uint8_t msg_type, uint8_t field_num) {
    uint8_t msg[8] = { 0 };
    msg[0] = 0;   // TODO
    msg[1] = msg_type;
    msg[2] = field_num;

    enqueue(queue, msg);
}

// Convenience functions to enqueue each of the message types
void enqueue_eps_hk_tx_msg(uint8_t field_num) {
    enqueue_tx_msg(&eps_tx_msg_queue, CAN_EPS_HK, field_num);
}
void enqueue_eps_ctrl_tx_msg(uint8_t field_num) {
    enqueue_tx_msg(&eps_tx_msg_queue, CAN_EPS_CTRL, field_num);
}
void enqueue_pay_hk_tx_msg(uint8_t field_num) {
    enqueue_tx_msg(&pay_tx_msg_queue, CAN_PAY_HK, field_num);
}
void enqueue_pay_opt_tx_msg(uint8_t field_num) {
    enqueue_tx_msg(&pay_tx_msg_queue, CAN_PAY_OPT, field_num);
}
void enqueue_pay_ctrl_tx_msg(uint8_t field_num) {
    enqueue_tx_msg(&pay_tx_msg_queue, CAN_PAY_CTRL, field_num);
}
