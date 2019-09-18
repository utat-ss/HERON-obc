#include "can_commands.h"

queue_t eps_tx_msg_queue;
queue_t pay_tx_msg_queue;
queue_t data_rx_msg_queue;


void handle_eps_hk(uint8_t field_num, uint32_t data);
void handle_pay_hk(uint8_t field_num, uint32_t data);
void handle_pay_opt(uint8_t field_num, uint32_t data);
void handle_pay_ctrl(uint8_t field_num);


void handle_rx_msg(void) {
    // print("%s\n", __FUNCTION__);
    if (queue_empty(&data_rx_msg_queue)) {
        return;
    }

    can_countdown = 0; // Received message
    uint8_t msg[8] = {0x00};
    // print("Dequeued from data_rx_msg_queue\n");
    dequeue(&data_rx_msg_queue, msg);

    uint8_t msg_type = msg[2];
    uint8_t field_num = msg[3];
    uint32_t data =
        ((uint32_t) msg[4] << 24) |
        ((uint32_t) msg[5] << 16) |
        ((uint32_t) msg[6] << 8) |
        ((uint32_t) msg[7]);

    //General CAN command-Send back data
    if ((current_cmd == &send_eps_can_msg_cmd) || (current_cmd == &send_pay_can_msg_cmd)) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            start_trans_tx_dec_msg();
            for (uint8_t i = 0; i < 8; i++) {
                append_to_trans_tx_dec_msg(msg[i]);
            }
            finish_trans_tx_dec_msg();
        }
        finish_current_cmd(true);
    }
    else {
        switch (msg_type) {
            case CAN_EPS_HK:
                handle_eps_hk(field_num, data);
                break;
            // Don't need an EPS CTRL handler
            case CAN_PAY_HK:
                handle_pay_hk(field_num, data);
                break;
            case CAN_PAY_OPT:
                handle_pay_opt(field_num, data);
                break;
            case CAN_PAY_CTRL:
                handle_pay_ctrl(field_num);
                break;
            default:
                break;
        }
    }
}


void handle_eps_hk(uint8_t field_num, uint32_t data){
    // Save the data to the local array
    if (field_num < CAN_EPS_HK_FIELD_COUNT) {
        // print("Received EPS_HK #%u\n", field_num);
        eps_hk_fields[field_num] = data;
    }

    // Request the next field (if not done yet)
    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_EPS_HK_FIELD_COUNT) {
        enqueue_eps_hk_tx_msg(next_field_num);
    }

    // If we have received all the fields
    if ((field_num == CAN_EPS_HK_FIELD_COUNT - 1) &&
        (current_cmd == &col_data_block_cmd) &&
        (current_cmd_arg1 == CMD_EPS_HK)) {

        // Increment the current block and then write to the section
        write_mem_data_block(&eps_hk_mem_section, eps_hk_mem_section.curr_block,
            &eps_hk_header, eps_hk_fields);
        inc_mem_section_curr_block(&eps_hk_mem_section);

        // Only send back a transceiver packet if the command was sent from
        // ground (arg2 = 0)
        if (current_cmd_arg2 == 0) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                start_trans_tx_dec_msg();
                append_to_trans_tx_dec_msg((eps_hk_mem_section.curr_block >> 24) & 0xFF);
                append_to_trans_tx_dec_msg((eps_hk_mem_section.curr_block >> 16) & 0xFF);
                append_to_trans_tx_dec_msg((eps_hk_mem_section.curr_block >> 8) & 0xFF);
                append_to_trans_tx_dec_msg(eps_hk_mem_section.curr_block & 0xFF);
                finish_trans_tx_dec_msg();
            }
        }

        print("Done EPS_HK\n");
        finish_current_cmd(true);
    }
}


void handle_pay_hk(uint8_t field_num, uint32_t data){
    // Save the data to the local array
    if (field_num < CAN_PAY_HK_FIELD_COUNT) {
        // print("modifying pay_hk_fields[%u]\n", field_num);
        pay_hk_fields[field_num] = data;
    }

    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_PAY_HK_FIELD_COUNT) {
        enqueue_pay_hk_tx_msg(next_field_num);
    }

    // If we have received all the fields
    if ((field_num == CAN_PAY_HK_FIELD_COUNT - 1) &&
        (current_cmd == &col_data_block_cmd) &&
        (current_cmd_arg1 == CMD_PAY_HK)) {

        // Increment the current block and then write to the section
        write_mem_data_block(&pay_hk_mem_section, pay_hk_mem_section.curr_block,
            &pay_hk_header, pay_hk_fields);
        inc_mem_section_curr_block(&pay_hk_mem_section);

        // Only send back a transceiver packet if the command was sent from
        // ground (arg2 = 0)
        if (current_cmd_arg2 == 0) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                start_trans_tx_dec_msg();
                append_to_trans_tx_dec_msg((pay_hk_mem_section.curr_block >> 24) & 0xFF);
                append_to_trans_tx_dec_msg((pay_hk_mem_section.curr_block >> 16) & 0xFF);
                append_to_trans_tx_dec_msg((pay_hk_mem_section.curr_block >> 8) & 0xFF);
                append_to_trans_tx_dec_msg(pay_hk_mem_section.curr_block & 0xFF);
                finish_trans_tx_dec_msg();
            }
        }

        print("Done PAY_HK\n");
        finish_current_cmd(true);
    }
}

void handle_pay_opt(uint8_t field_num, uint32_t data){
    // Save the data to the local array
    if (field_num < CAN_PAY_OPT_FIELD_COUNT) {
        // Save data
        pay_opt_fields[field_num] = data;
    }

    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_PAY_OPT_FIELD_COUNT){
        enqueue_pay_opt_tx_msg(next_field_num);
    }

    // If we have received all the fields
    if ((field_num == CAN_PAY_OPT_FIELD_COUNT - 1) &&
        (current_cmd == &col_data_block_cmd) &&
        (current_cmd_arg1 == CMD_PAY_OPT)) {

        // Increment the current block and then write to the section
        write_mem_data_block(&pay_opt_mem_section, pay_opt_mem_section.curr_block,
            &pay_opt_header, pay_opt_fields);
        inc_mem_section_curr_block(&pay_opt_mem_section);

        // Only send back a transceiver packet if the command was sent from
        // ground (arg2 = 0)
        if (current_cmd_arg2 == 0) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                start_trans_tx_dec_msg();
                append_to_trans_tx_dec_msg((pay_opt_mem_section.curr_block >> 24) & 0xFF);
                append_to_trans_tx_dec_msg((pay_opt_mem_section.curr_block >> 16) & 0xFF);
                append_to_trans_tx_dec_msg((pay_opt_mem_section.curr_block >> 8) & 0xFF);
                append_to_trans_tx_dec_msg(pay_opt_mem_section.curr_block & 0xFF);
                finish_trans_tx_dec_msg();
            }
        }

        print("Done PAY_OPT\n");
        finish_current_cmd(true);
    }
}

void handle_pay_ctrl(uint8_t field_num) {
    if ((field_num == CAN_PAY_CTRL_ACT_UP ||
        field_num == CAN_PAY_CTRL_ACT_DOWN) &&
        current_cmd == &act_pay_motors_cmd) {

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            start_trans_tx_dec_msg();
            finish_trans_tx_dec_msg();
        }

        finish_current_cmd(true);
    }
}


// If there is an RX messsage in the queue, handle it
void process_next_rx_msg(void) {
    if (!queue_empty(&data_rx_msg_queue)) {
        handle_rx_msg();
    }
}

/*
If there is a TX message in the EPS queue, sends it

When resume_mob(mob name) is called, it:
1) resumes the MOB
2) triggers an interrupt (callback function) to get the data to transmit
3) sends the data
4) pauses the mob
*/
void send_next_eps_tx_msg(void) {
    if (!queue_empty(&eps_tx_msg_queue)) {
        resume_mob(&eps_cmd_tx_mob);
    }
}

/*
If there is a TX message in the PAY queue, sends it

When resume_mob(mob name) is called, it:
1) resumes the MOB
2) triggers an interrupt (callback function) to get the data to transmit
3) sends the data
4) pauses the mob
*/
void send_next_pay_tx_msg(void) {
    if (!queue_empty(&pay_tx_msg_queue)) {
        resume_mob(&pay_cmd_tx_mob);
    }
}




// Enqueues a CAN message given a general set of 8 bytes data
void enqueue_tx_msg_general(queue_t* queue, uint32_t data1, uint32_t data2) {
    uint8_t msg[8] = { 0x00 };
    msg[0] = (data1 >> 24) & 0xFF;
    msg[1] = (data1 >> 16) & 0xFF;
    msg[2] = (data1 >> 8) & 0xFF;
    msg[3] = data1 & 0xFF;
    msg[4] = (data2 >> 24) & 0xFF;
    msg[5] = (data2 >> 16) & 0xFF;
    msg[6] = (data2 >> 8) & 0xFF;
    msg[7] = data2 & 0xFF;

    can_countdown = 5;
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
    uint8_t msg[8] = { 0x00 };
    msg[0] = 0x00;
    msg[1] = 0x00;
    msg[2] = msg_type;
    msg[3] = field_num;
    msg[4] = (data >> 24) & 0xFF;
    msg[5] = (data >> 16) & 0xFF;
    msg[6] = (data >> 8) & 0xFF;
    msg[7] = data & 0xFF;

    can_countdown = 5; // Wait 30 seconds for return message
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
