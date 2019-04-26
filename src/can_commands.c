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

        //General CAN command-Send back data
        if ((current_cmd == &send_eps_can_cmd) ||
            (current_cmd == &send_pay_can_cmd)) {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                    start_trans_decoded_tx_msg();
                    for (uint8_t i = 0; i < 8; i++) {
                        append_to_trans_decoded_tx_msg(data[i]);
                    }
                    finish_trans_decoded_tx_msg();
                }
                finish_current_cmd(true);
            }
        else {
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
                    break;
            }
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
    if ((field_num == CAN_EPS_HK_FIELD_COUNT - 1) &&
        (current_cmd == &collect_block_cmd) &&
        (current_cmd_arg1 == CMD_BLOCK_EPS_HK)) {

        if (!sim_local_actions) {
            write_mem_block(&eps_hk_mem_section, eps_hk_mem_section.curr_block,
                &eps_hk_header, eps_hk_fields);
        }
        inc_mem_section_curr_block(&eps_hk_mem_section);
        write_mem_section_eeprom(&eps_hk_mem_section);

        print("Done EPS_HK\n");
        finish_current_cmd(true);
    }
}

void handle_eps_ctrl(const uint8_t* data){
    uint8_t field_num = data[2];

    if ((field_num == CAN_EPS_CTRL_HEAT_SP1 ||
        field_num == CAN_EPS_CTRL_HEAT_SP2) &&
        current_cmd == &set_eps_heater_sp_cmd) {

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            start_trans_decoded_tx_msg();
            finish_trans_decoded_tx_msg();
        }

        finish_current_cmd(true);
    }
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
    if ((field_num == CAN_PAY_HK_FIELD_COUNT - 1) &&
        (current_cmd == &collect_block_cmd) &&
        (current_cmd_arg1 == CMD_BLOCK_PAY_HK)) {

        if (!sim_local_actions) {
            write_mem_block(&pay_hk_mem_section, pay_hk_mem_section.curr_block,
                &pay_hk_header, pay_hk_fields);
        }
        inc_mem_section_curr_block(&pay_hk_mem_section);
        write_mem_section_eeprom(&pay_hk_mem_section);

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
    if ((field_num == CAN_PAY_OPT_FIELD_COUNT - 1) &&
        (current_cmd == &collect_block_cmd) &&
        (current_cmd_arg1 == CMD_BLOCK_PAY_OPT)) {

        if (!sim_local_actions) {
            write_mem_block(&pay_opt_mem_section, pay_opt_mem_section.curr_block,
                &pay_opt_header, pay_opt_fields);
        }
        inc_mem_section_curr_block(&pay_opt_mem_section);
        write_mem_section_eeprom(&pay_opt_mem_section);

        print("Done PAY_OPT\n");
        finish_current_cmd(true);
    }
}

void handle_pay_ctrl(const uint8_t* data) {
    uint8_t field_num = data[2];

    if ((field_num == CAN_PAY_CTRL_HEAT_SP1 ||
        field_num == CAN_PAY_CTRL_HEAT_SP2) &&
        current_cmd == &set_pay_heater_sp_cmd) {

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            start_trans_decoded_tx_msg();
            finish_trans_decoded_tx_msg();
        }

        finish_current_cmd(true);
    }

    else if ((field_num == CAN_PAY_CTRL_ACT_UP ||
        field_num == CAN_PAY_CTRL_ACT_DOWN) &&
        current_cmd == &actuate_pay_motors_cmd) {

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            start_trans_decoded_tx_msg();
            finish_trans_decoded_tx_msg();
        }

        finish_current_cmd(true);
    }
}


// Enqueues a CAN message given a general set of 8 bytes data
void enqueue_tx_msg_general(queue_t* queue, uint32_t data1, uint32_t data2) {
    uint8_t msg[8] = { 0 };
    msg[0] = (data1 >> 24) & 0xFF;
    msg[1] = (data1 >> 16) & 0xFF;
    msg[2] = (data1 >> 8) & 0xFF;
    msg[3] = data1 & 0xFF;
    msg[4] = (data2 >> 24) & 0xFF;
    msg[5] = (data2 >> 16) & 0xFF;
    msg[6] = (data2 >> 8) & 0xFF;
    msg[7] = data2 & 0xFF;

    enqueue(queue, msg);
}

void enqueue_eps_tx_msg(uint32_t data1, uint32_t data2) {
    enqueue_tx_msg_general(&eps_tx_msg_queue, data1, data2);
}
void enqueue_pay_tx_msg(uint32_t data1, uint32_t data2) {
    enqueue_tx_msg_general(&pay_tx_msg_queue, data1, data2);
}

/*
Enqueues a CAN message onto the specified queue to request the specified message
    type and field number.
queue - Queue to enqueue the message to
msg_type - Message type to request (byte 1)
field_num - Field number to request (byte 2)
*/
void enqueue_tx_msg(queue_t* queue, uint8_t msg_type, uint8_t field_num, uint32_t data) {
    uint8_t msg[8] = { 0 };
    msg[0] = 0;   // TODO
    msg[1] = msg_type;
    msg[2] = field_num;
    msg[3] = (data >> 16) & 0xFF;
    msg[4] = (data >> 8) & 0xFF;
    msg[5] = data & 0xFF;

    enqueue(queue, msg);
}

// Convenience functions to enqueue each of the message types
void enqueue_eps_hk_tx_msg(uint8_t field_num) {
    enqueue_tx_msg(&eps_tx_msg_queue, CAN_EPS_HK, field_num, 0);
}
void enqueue_eps_ctrl_tx_msg(uint8_t field_num, uint32_t data) {
    enqueue_tx_msg(&eps_tx_msg_queue, CAN_EPS_CTRL, field_num, data);
}
void enqueue_pay_hk_tx_msg(uint8_t field_num) {
    enqueue_tx_msg(&pay_tx_msg_queue, CAN_PAY_HK, field_num, 0);
}
void enqueue_pay_opt_tx_msg(uint8_t field_num) {
    enqueue_tx_msg(&pay_tx_msg_queue, CAN_PAY_OPT, field_num, 0);
}
void enqueue_pay_ctrl_tx_msg(uint8_t field_num, uint32_t data) {
    enqueue_tx_msg(&pay_tx_msg_queue, CAN_PAY_CTRL, field_num, data);
}
