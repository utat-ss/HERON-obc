#include "can_commands.h"

// Uncomment for extra debugging prints
// #define CAN_COMMANDS_DEBUG

queue_t eps_tx_msg_queue;
queue_t pay_tx_msg_queue;
queue_t data_rx_msg_queue;


// Set to true to print TX and RX CAN messages
bool print_can_msgs = false;


void process_eps_hk(uint8_t field_num, uint32_t data);
void process_pay_hk(uint8_t field_num, uint32_t data);
void process_pay_opt(uint8_t field_num, uint32_t data);
void process_pay_ctrl(uint8_t field_num);


// If there is an RX messsage in the queue, process it
void process_next_rx_msg(void) {
    uint8_t msg[8] = {0x00};
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&data_rx_msg_queue)) {
            return;
        }
        dequeue(&data_rx_msg_queue, msg);
    }

    if (print_can_msgs) {
        // Extra spaces to align with CAN TX messages
        print("CAN RX:       ");
        print_bytes(msg, 8);
    }

    // Break down the message into components
    uint8_t opcode = msg[2];
    uint8_t field_num = msg[3];
    uint32_t data =
        ((uint32_t) msg[4] << 24) |
        ((uint32_t) msg[5] << 16) |
        ((uint32_t) msg[6] << 8) |
        ((uint32_t) msg[7]);

    //General CAN command-Intercept and send back data
    if ((current_cmd == &send_eps_can_msg_cmd) || (current_cmd == &send_pay_can_msg_cmd)) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            start_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
            for (uint8_t i = 0; i < 8; i++) {
                append_to_trans_tx_dec_msg(msg[i]);
            }
            finish_trans_tx_dec_msg();
        }
        finish_current_cmd(CMD_RESP_STATUS_OK);
    }
    else {
        switch (opcode) {
            case CAN_EPS_HK:
                process_eps_hk(field_num, data);
                break;
            // Don't need an EPS CTRL handler
            case CAN_PAY_HK:
                process_pay_hk(field_num, data);
                break;
            case CAN_PAY_OPT:
                process_pay_opt(field_num, data);
                break;
            case CAN_PAY_CTRL:
                process_pay_ctrl(field_num);
                break;
            default:
                break;
        }
    }
}


void process_eps_hk(uint8_t field_num, uint32_t data){
    if (current_cmd != &col_data_block_cmd) {
        return;
    }
    if (current_cmd_arg1 != CMD_EPS_HK) {
        return;
    }

    // Note that eps_hk_mem_section.curr_block has already been incremented,
    // so we need to use the block number from the header that was populated
    // at the start of this command

    // Save the data to the local array and flash memory
    if (field_num < CAN_EPS_HK_FIELD_COUNT) {
        eps_hk_fields[field_num] = data;
        write_mem_field(&eps_hk_mem_section, eps_hk_header.block_num, field_num,
            data);
    }

    // Request the next field (if not done yet)
    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_EPS_HK_FIELD_COUNT) {
        enqueue_eps_tx_msg(CAN_EPS_HK, next_field_num, 0);
    }

    // If we have received all the fields
    if (field_num == CAN_EPS_HK_FIELD_COUNT - 1) {
        // Successfully finished command
        write_mem_header_status(&eps_hk_mem_section, eps_hk_header.block_num,
            CMD_RESP_STATUS_OK);

        // Only send back a transceiver packet if the command was sent from
        // ground (arg2 = 0)
        if (current_cmd_arg2 == 0) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                start_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
                append_to_trans_tx_dec_msg((eps_hk_header.block_num >> 24) & 0xFF);
                append_to_trans_tx_dec_msg((eps_hk_header.block_num >> 16) & 0xFF);
                append_to_trans_tx_dec_msg((eps_hk_header.block_num >> 8) & 0xFF);
                append_to_trans_tx_dec_msg((eps_hk_header.block_num >> 0) & 0xFF);
                finish_trans_tx_dec_msg();
            }
        }

#ifdef CAN_COMMANDS_DEBUG
        print("Done EPS_HK\n");
#endif
        finish_current_cmd(CMD_RESP_STATUS_OK);
    }
}


void process_pay_hk(uint8_t field_num, uint32_t data){
    if (current_cmd != &col_data_block_cmd) {
        return;
    }
    if (current_cmd_arg1 != CMD_PAY_HK) {
        return;
    }

    // Save the data to the local array and flash memory
    if (field_num < CAN_PAY_HK_FIELD_COUNT) {
        pay_hk_fields[field_num] = data;
        write_mem_field(&pay_hk_mem_section, pay_hk_header.block_num, field_num,
            data);
    }

    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_PAY_HK_FIELD_COUNT) {
        enqueue_pay_tx_msg(CAN_PAY_HK, next_field_num, 0);
    }

    // If we have received all the fields
    if (field_num == CAN_PAY_HK_FIELD_COUNT - 1) {
        // Successfully finished command
        write_mem_header_status(&pay_hk_mem_section, pay_hk_header.block_num,
            CMD_RESP_STATUS_OK);
        
        // Only send back a transceiver packet if the command was sent from
        // ground (arg2 = 0)
        if (current_cmd_arg2 == 0) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                start_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
                append_to_trans_tx_dec_msg((pay_hk_header.block_num >> 24) & 0xFF);
                append_to_trans_tx_dec_msg((pay_hk_header.block_num >> 16) & 0xFF);
                append_to_trans_tx_dec_msg((pay_hk_header.block_num >> 8) & 0xFF);
                append_to_trans_tx_dec_msg((pay_hk_header.block_num >> 0) & 0xFF);
                finish_trans_tx_dec_msg();
            }
        }

#ifdef CAN_COMMANDS_DEBUG
        print("Done PAY_HK\n");
#endif
        finish_current_cmd(CMD_RESP_STATUS_OK);
    }
}

void process_pay_opt(uint8_t field_num, uint32_t data){
    if (current_cmd != &col_data_block_cmd) {
        return;
    }
    if (current_cmd_arg1 != CMD_PAY_OPT) {
        return;
    }

    // Save the data to the local array and flash memory
    if (field_num < CAN_PAY_OPT_FIELD_COUNT) {
        // Save data
        pay_opt_fields[field_num] = data;
        write_mem_field(&pay_opt_mem_section, pay_opt_header.block_num,
            field_num, data);
    }

    uint8_t next_field_num = field_num + 1;
    if (next_field_num < CAN_PAY_OPT_FIELD_COUNT){
        enqueue_pay_tx_msg(CAN_PAY_OPT, next_field_num, 0);
    }

    // If we have received all the fields
    if (field_num == CAN_PAY_OPT_FIELD_COUNT - 1) {
        // Successfully finished command
        write_mem_header_status(&pay_opt_mem_section, pay_opt_header.block_num,
            CMD_RESP_STATUS_OK);
        
        // Only send back a transceiver packet if the command was sent from
        // ground (arg2 = 0)
        if (current_cmd_arg2 == 0) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                start_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
                append_to_trans_tx_dec_msg((pay_opt_header.block_num >> 24) & 0xFF);
                append_to_trans_tx_dec_msg((pay_opt_header.block_num >> 16) & 0xFF);
                append_to_trans_tx_dec_msg((pay_opt_header.block_num >> 8) & 0xFF);
                append_to_trans_tx_dec_msg((pay_opt_header.block_num >> 0) & 0xFF);
                finish_trans_tx_dec_msg();
            }
        }

#ifdef CAN_COMMANDS_DEBUG
        print("Done PAY_OPT\n");
#endif
        finish_current_cmd(CMD_RESP_STATUS_OK);
    }
}

void process_pay_ctrl(uint8_t field_num) {
    if (current_cmd == &act_pay_motors_cmd &&
            (field_num == CAN_PAY_CTRL_ACT_UP ||
            field_num == CAN_PAY_CTRL_ACT_DOWN ||
            field_num == CAN_PAY_CTRL_BLIST_DEP_SEQ)) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
        finish_current_cmd(CMD_RESP_STATUS_OK);
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
    if (queue_empty(&eps_tx_msg_queue)) {
        return;
    }

    if (print_can_msgs) {
        uint8_t tx_msg[8] = { 0x00 };
        peek_queue(&eps_tx_msg_queue, tx_msg);
        print("CAN TX (EPS): ");
        print_bytes(tx_msg, 8);
    }

    resume_mob(&eps_cmd_tx_mob);
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
    if (queue_empty(&pay_tx_msg_queue)) {
        return;
    }

    if (print_can_msgs) {
        uint8_t tx_msg[8] = { 0x00 };
        peek_queue(&pay_tx_msg_queue, tx_msg);
        print("CAN TX (PAY): ");
        print_bytes(tx_msg, 8);
    }

    resume_mob(&pay_cmd_tx_mob);
}




// Enqueues a CAN message given a general set of 8 bytes data
void enqueue_tx_msg_bytes(queue_t* queue, uint32_t data1, uint32_t data2) {
    uint8_t msg[8] = { 0x00 };
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

/*
Enqueues a CAN message onto the specified queue to request the specified message
    type and field number.
queue - Queue to enqueue the message to
opcode - Message type to request (byte 2)
field_num - Field number to request (byte 3)
*/
void enqueue_tx_msg(queue_t* queue, uint8_t opcode, uint8_t field_num, uint32_t data) {
    uint8_t msg[8] = { 0x00 };
    msg[0] = 0x00;
    msg[1] = 0x00;
    msg[2] = opcode;
    msg[3] = field_num;
    msg[4] = (data >> 24) & 0xFF;
    msg[5] = (data >> 16) & 0xFF;
    msg[6] = (data >> 8) & 0xFF;
    msg[7] = data & 0xFF;

    enqueue(queue, msg);
}

void enqueue_eps_tx_msg(uint8_t opcode, uint8_t field_num, uint32_t data) {
    enqueue_tx_msg(&eps_tx_msg_queue, opcode, field_num, data);
}

void enqueue_pay_tx_msg(uint8_t opcode, uint8_t field_num, uint32_t data) {
    enqueue_tx_msg(&pay_tx_msg_queue, opcode, field_num, data);
}
