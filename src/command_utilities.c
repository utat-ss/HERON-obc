#include "command_utilities.h"

// Uncomment for extra debugging prints
#define COMMAND_UTILITIES_DEBUG
// #define COMMAND_UTILITIES_DEBUG_QUEUES
// #define COMMAND_UTILITIES_VERBOSE

// If you get an error here because `security.h` is not found, copy the dummy
// `security.h` file from https://github.com/HeronMkII/templates to your `src`
// folder and try again
// DO NOT COMMIT ANY `security.h` FILE TO GIT
#include "security.h"
// The super-secret password for sensitive commands
const uint8_t correct_pwd[] = SECURITY_CORRECT_PWD;

// Queue of commands that need to be executed but have not been executed yet
// Contains command ID and opcode
queue_t cmd_queue_1;
// Contains arguments 1 and 2
queue_t cmd_queue_2;

// Sequenced command ID from ground (or 0 for auto-scheduled)
// Default to 0xFFFF instead of 0x0000 because that represents an auto command
volatile uint16_t current_cmd_id = 0xFFFF;
// A pointer to the currently executing command (or nop_cmd for no command executing)
// Use double volatile just in case
volatile cmd_t* volatile current_cmd = &nop_cmd;
// Current command arguments
volatile uint32_t current_cmd_arg1 = 0;
volatile uint32_t current_cmd_arg2 = 0;
// Don't need to store a variable for the password because it is checked in the
// decoded message before enqueueing to the command queue

// For the uptime interrupt to cmd_timeout_cb to finish command if not completed
// after some number of time
volatile uint8_t cmd_timeout_count_s = 0;
uint8_t cmd_timeout_period_s = CMD_TIMEOUT_DEF_PERIOD_S;

// Must define these separately here because of different array sizes
uint32_t obc_hk_fields[CAN_OBC_HK_FIELD_COUNT] = { 0 };
uint32_t eps_hk_fields[CAN_EPS_HK_FIELD_COUNT] = { 0 };
uint32_t pay_hk_fields[CAN_PAY_HK_FIELD_COUNT] = { 0 };
uint32_t pay_opt_fields[CAN_PAY_OPT_FIELD_COUNT] = { 0 };

mem_header_t cmd_log_header;

// Don't need to initialize headers here
// Don't need to make these volatile because they are not modified
// within ISRs
data_col_t obc_hk_data_col = {
    .name = "OBC_HK",
    .auto_enabled = false,
    .auto_enabled_eeprom_addr = OBC_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR,
    .auto_period = OBC_HK_AUTO_DATA_COL_PERIOD,
    .auto_period_eeprom_addr = OBC_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR,
    .prev_auto_col_uptime_s = 0,
    .prev_field_col_uptime_s = 0,
    .fields = obc_hk_fields,
    .mem_section = &obc_hk_mem_section,
    .cmd_log_block_num = 0,
    .cmd_arg1 = CMD_OBC_HK,
    .can_tx_queue = &eps_tx_msg_queue,  // N/A for this data type
    .can_opcode = 0xFF, // N/A for this data type
};
data_col_t eps_hk_data_col = {
    .name = "EPS_HK",
    .auto_enabled = false,
    .auto_enabled_eeprom_addr = EPS_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR,
    .auto_period = EPS_HK_AUTO_DATA_COL_PERIOD,
    .auto_period_eeprom_addr = EPS_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR,
    .prev_auto_col_uptime_s = 0,
    .prev_field_col_uptime_s = 0,
    .fields = eps_hk_fields,
    .mem_section = &eps_hk_mem_section,
    .cmd_log_block_num = 0,
    .cmd_arg1 = CMD_EPS_HK,
    .can_tx_queue = &eps_tx_msg_queue,
    .can_opcode = CAN_EPS_HK,
};
data_col_t pay_hk_data_col = {
    .name = "PAY_HK",
    .auto_enabled = false,
    .auto_enabled_eeprom_addr = PAY_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR,
    .auto_period = PAY_HK_AUTO_DATA_COL_PERIOD,
    .auto_period_eeprom_addr = PAY_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR,
    .prev_auto_col_uptime_s = 0,
    .prev_field_col_uptime_s = 0,
    .fields = pay_hk_fields,
    .mem_section = &pay_hk_mem_section,
    .cmd_log_block_num = 0,
    .cmd_arg1 = CMD_PAY_HK,
    .can_tx_queue = &pay_tx_msg_queue,
    .can_opcode = CAN_PAY_HK,
};
data_col_t pay_opt_data_col = {
    .name = "PAY_OPT",
    .auto_enabled = false,
    .auto_enabled_eeprom_addr = PAY_OPT_AUTO_DATA_COL_ENABLED_EEPROM_ADDR,
    .auto_period = PAY_OPT_AUTO_DATA_COL_PERIOD,
    .auto_period_eeprom_addr = PAY_OPT_AUTO_DATA_COL_PERIOD_EEPROM_ADDR,
    .prev_auto_col_uptime_s = 0,
    .prev_field_col_uptime_s = 0,
    .fields = pay_opt_fields,
    .mem_section = &pay_opt_mem_section,
    .cmd_log_block_num = 0,
    .cmd_arg1 = CMD_PAY_OPT,
    .can_tx_queue = &pay_tx_msg_queue,
    .can_opcode = CAN_PAY_OPT,
};

data_col_t* all_data_cols[NUM_DATA_COL_SECTIONS] = {
    &obc_hk_data_col,
    &eps_hk_data_col,
    &pay_hk_data_col,
    &pay_opt_data_col,
};


// Date and time of the most recent restart
rtc_date_t restart_date = { .yy = 0, .mm = 0, .dd  = 0 };
rtc_time_t restart_time = { .hh = 0, .mm = 0, .ss  = 0 };


// Set to true to print commands and arguments
bool print_cmds = false;
// Set to true to print ACKs
bool print_trans_tx_acks = false;


/*
If there is a message in trans_rx_dec_msg, processes its components and enqueues the appropriate command and arguments.
*/
void handle_trans_rx_dec_msg(void) {
    // Need to put everything in an atomic block because the message is in a global array
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!trans_rx_dec_avail) {
            return;
        }
        trans_rx_dec_avail = false;

        if (print_trans_msgs) {
            print("Trans RX (Decoded): %u bytes: ", trans_rx_dec_len);
            print_bytes((uint8_t*) trans_rx_dec_msg, trans_rx_dec_len);
        }

        // Before checking for full message length, check for the reset command ID request
        // Only needs 2 bytes to work
        if (trans_rx_dec_len >= 2 &&
                trans_rx_dec_msg[0] == 0x00 &&
                trans_rx_dec_msg[1] == 0x00) {
            trans_last_cmd_id = 0x0000;
            add_trans_tx_ack(CMD_CMD_ID_UNKNOWN, CMD_ACK_STATUS_RESET_CMD_ID);
            // Return here because we don't want to continue and put anything
            // into the command queue
            return;
        }

        // Only accept 15 byte messages
        if (trans_rx_dec_len < 15) {
            // Don't know the opcode/args
            add_trans_tx_ack(CMD_CMD_ID_UNKNOWN, CMD_ACK_STATUS_INVALID_DEC_FMT);
            return;
        }

        // Use shorter variable names for convenience
        volatile uint8_t* msg = (volatile uint8_t*) trans_rx_dec_msg;

        uint16_t cmd_id =
            ((uint16_t) msg[0] << 8) |
            ((uint16_t) msg[1] << 0);
        uint8_t opcode = msg[2];
        uint32_t arg1 =
            ((uint32_t) msg[3] << 24) |
            ((uint32_t) msg[4] << 16) |
            ((uint32_t) msg[5] << 8) |
            ((uint32_t) msg[6]);
        uint32_t arg2 =
            ((uint32_t) msg[7] << 24) |
            ((uint32_t) msg[8] << 16) |
            ((uint32_t) msg[9] << 8) |
            ((uint32_t) msg[10]);
        uint8_t received_pwd[4];
        received_pwd[0] = msg[11];
        received_pwd[1] = msg[12];
        received_pwd[2] = msg[13];
        received_pwd[3] = msg[14];

        // NACK if the MSB of the command ID is 1
        // Send back the unknown command ID because if we sent back the actual
        // ID, the MSB would be 1 so ground would interpret it as a response
        // rather than an ACK packet
        if ((cmd_id >> 15) & 0x01) {
            add_trans_tx_ack(CMD_CMD_ID_UNKNOWN, CMD_ACK_STATUS_INVALID_CMD_ID);
            return;
        }

        // NACK if the command ID is less than the previous received ID
        if (cmd_id < trans_last_cmd_id) {
            add_trans_tx_ack(cmd_id, CMD_ACK_STATUS_DECREMENTED_CMD_ID);
            return;
        }

        // NACK if the command ID is equal to the previous received ID (repeated command)
        if (cmd_id == trans_last_cmd_id) {
            add_trans_tx_ack(cmd_id, CMD_ACK_STATUS_REPEATED_CMD_ID);
            return;
        }

        // Get the corresponding cmd_t struct for the opcode
        cmd_t* cmd = cmd_opcode_to_cmd(opcode);
        // Check if it is a valid opcode/command
        if (cmd == &nop_cmd) {
            add_trans_tx_ack(cmd_id, CMD_ACK_STATUS_INVALID_OPCODE);
            return;
        }

        // Check 4-byte password if necessary for the command
        if (cmd->pwd_protected) {
            for (uint8_t i = 0; i < 4; i++) {
                if (received_pwd[i] != correct_pwd[i]) {
                    // NACK
                    add_trans_tx_ack(cmd_id, CMD_ACK_STATUS_INVALID_PWD);
                    return;
                }
            }
        }

        // Check if either command queue is full (they should both be the same
        // size, but check both just in case)
        if (queue_full(&cmd_queue_1) || queue_full(&cmd_queue_2)) {
            add_trans_tx_ack(cmd_id, CMD_ACK_STATUS_FULL_CMD_QUEUE);
            return;
        }

        // If all the checks passed, the command is OK to put into the queue
        add_trans_tx_ack(cmd_id, CMD_ACK_STATUS_OK);
        enqueue_cmd(cmd_id, cmd, arg1, arg2);

        // Update the last command ID for the one we just received
        trans_last_cmd_id = cmd_id;

        // Restart the counter for not receiving communication from ground
        // By design, OBC will not reset the communication timeout when it
        // receives a reset command ID request
        restart_com_timeout();
    }
}

void process_trans_tx_ack(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!trans_tx_ack_avail) {
            return;
        }
        trans_tx_ack_avail = false;

        if (print_trans_tx_acks) {
            print("ACK: cmd_id = 0x%.4x, status = 0x%.2x\n",
                trans_tx_ack_cmd_id, trans_tx_ack_status);
        }

        // No mask on the command ID because this is an ACK
        uint16_t cmd_id = trans_tx_ack_cmd_id;
        uint8_t status = trans_tx_ack_status;

        // Can't use the standard trans_tx_dec functions because they use the current_cmd variables
        trans_tx_dec_msg[0] = (cmd_id >> 8) & 0xFF;
        trans_tx_dec_msg[1] = (cmd_id >> 0) & 0xFF;
        trans_tx_dec_msg[2] = status;

        trans_tx_dec_len = 3;
        trans_tx_dec_avail = true;
    }
}

// NOTE: these three functions should be used within the same atomic block

void start_trans_tx_resp(uint8_t status) {
    uint16_t cmd_id = current_cmd_id | CMD_RESP_CMD_ID_MASK;
    trans_tx_dec_msg[0] = (cmd_id >> 8) & 0xFF;
    trans_tx_dec_msg[1] = (cmd_id >> 0) & 0xFF;
    trans_tx_dec_msg[2] = status;

    trans_tx_dec_len = 3;
}

void append_to_trans_tx_resp(uint8_t byte) {
    if (trans_tx_dec_len < TRANS_TX_DEC_MSG_MAX_SIZE) {
        trans_tx_dec_msg[trans_tx_dec_len] = byte;
        trans_tx_dec_len++;
    }
}

void finish_trans_tx_resp(void) {
    trans_tx_dec_avail = true;
}

/*
Returns the cmd_t struct corresponding to the given opcode,
or &nop_cmd if not found.
*/
cmd_t* cmd_opcode_to_cmd(uint8_t opcode) {
    for (uint8_t i = 0; i < all_cmds_list_len; i++) {
        if (all_cmds_list[i]->opcode == opcode) {
            return all_cmds_list[i];
        }
    }

    return &nop_cmd;
}

mem_section_t* mem_section_for_cmd(cmd_t* cmd) {
    if (cmd == &get_cur_block_nums_cmd ||
            cmd == &read_data_block_cmd ||
            cmd == &read_prim_cmd_blocks_cmd ||
            cmd == &read_sec_cmd_blocks_cmd) {
        return &sec_cmd_log_mem_section;
    } else {
        return &prim_cmd_log_mem_section;
    }
}




// We know that the microcontroller uses 16 bit addresses, so store a function
// pointer in the first 2 bytes of the queue entry (data[0] = MSB, data[1] = LSB)

/*
Serialize command information to bytes.
Assumes bytes1 and bytes2 are each 8-byte arrays.
*/
void cmd_to_bytes(uint16_t cmd_id, cmd_t* cmd, uint32_t arg1, uint32_t arg2,
        uint8_t* bytes1, uint8_t* bytes2) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        bytes1[0] = (cmd_id >> 8) & 0xFF;
        bytes1[1] = (cmd_id >> 0) & 0xFF;
        bytes1[2] = cmd->opcode;
        // Fill unused bytes with zeros just in case it is uninitialized
        bytes1[3] = 0x00;
        bytes1[4] = 0x00;
        bytes1[5] = 0x00;
        bytes1[6] = 0x00;
        bytes1[7] = 0x00;

        bytes2[0] = (arg1 >> 24) & 0xFF;
        bytes2[1] = (arg1 >> 16) & 0xFF;
        bytes2[2] = (arg1 >> 8) & 0xFF;
        bytes2[3] = arg1 & 0xFF;
        bytes2[4] = (arg2 >> 24) & 0xFF;
        bytes2[5] = (arg2 >> 16) & 0xFF;
        bytes2[6] = (arg2 >> 8) & 0xFF;
        bytes2[7] = arg2 & 0xFF;
    }
}

/*
Enqueues a command and arguments to the back of the queues.
Enqueue the opcode instead of the function pointer because it's safer in case
something goes wrong.
*/
bool enqueue_cmd(uint16_t cmd_id, cmd_t* cmd, uint32_t arg1, uint32_t arg2) {
#ifdef COMMAND_UTILITIES_DEBUG_QUEUES
    print("enqueue_cmd: id = 0x%.4x, opcode = 0x%x, arg1 = 0x%lx, arg2 = 0x%lx\n",
        cmd_id, cmd->opcode, arg1, arg2);
#endif

    // Enqueue the command as two 8-byte arrays
    uint8_t bytes1[8] = {0};
    uint8_t bytes2[8] = {0};
    
    // Make sure modifications to the states of the two queues are atomic/consistent
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_full(&cmd_queue_1)) {
            return false;
        }
        if (queue_full(&cmd_queue_2)) {
            return false;
        }
        cmd_to_bytes(cmd_id, cmd, arg1, arg2, bytes1, bytes2);

        enqueue(&cmd_queue_1, bytes1);
        enqueue(&cmd_queue_2, bytes2);
    }

    return true;
}

/*
Enqueues a command and arguments to the front of the queues so it is guaranteed
to be the next executed command.
Enqueue the opcode instead of the function pointer because it's safer in case
something goes wrong.
*/
bool enqueue_cmd_front(uint16_t cmd_id, cmd_t* cmd, uint32_t arg1, uint32_t arg2) {
#ifdef COMMAND_UTILITIES_DEBUG_QUEUES
    print("enqueue_cmd_front: id = 0x%.4x, opcode = 0x%x, arg1 = 0x%lx, arg2 = 0x%lx\n",
        cmd_id, cmd->opcode, arg1, arg2);
#endif

    // Enqueue the command as two 8-byte arrays
    uint8_t bytes1[8] = {0};
    uint8_t bytes2[8] = {0};
    
    // Make sure modifications to the states of the two queues are atomic/consistent
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_full(&cmd_queue_1)) {
            return false;
        }
        if (queue_full(&cmd_queue_2)) {
            return false;
        }
        cmd_to_bytes(cmd_id, cmd, arg1, arg2, bytes1, bytes2);

        enqueue_front(&cmd_queue_1, bytes1);
        enqueue_front(&cmd_queue_2, bytes2);
    }

    return true;
}

/*
Deserialize command information from bytes.
Assumes bytes1 and bytes2 are each 8-byte arrays.
*/
void bytes_to_cmd(uint16_t* cmd_id, cmd_t** cmd, uint32_t* arg1, uint32_t* arg2,
        uint8_t* bytes1, uint8_t* bytes2) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        *cmd_id =
            ((uint16_t) bytes1[0] << 8) |
            ((uint16_t) bytes1[1] << 0);
        *cmd = cmd_opcode_to_cmd(bytes1[2]);

        *arg1 =
            (((uint32_t) bytes2[0]) << 24) |
            (((uint32_t) bytes2[1]) << 16) |
            (((uint32_t) bytes2[2]) << 8) |
            (((uint32_t) bytes2[3]));
        *arg2 =
            (((uint32_t) bytes2[4]) << 24) |
            (((uint32_t) bytes2[5]) << 16) |
            (((uint32_t) bytes2[6]) << 8) |
            (((uint32_t) bytes2[7]));
    }
}

/*
cmd - The struct must already exist (be allocated) before calling this function,
      then this function sets the value of cmd->fn
      Use a double pointer because we need to set the value of the cmd pointer
*/
bool dequeue_cmd(uint16_t* cmd_id, cmd_t** cmd, uint32_t* arg1, uint32_t* arg2) {
    // Dequeue the command as an 8-byte array
    uint8_t bytes1[8] = {0};
    uint8_t bytes2[8] = {0};

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&cmd_queue_1)) {
            return false;
        }
        if (queue_empty(&cmd_queue_2)) {
            return false;
        }
        dequeue(&cmd_queue_1, bytes1);
        dequeue(&cmd_queue_2, bytes2);

        bytes_to_cmd(cmd_id, cmd, arg1, arg2, bytes1, bytes2);
    }

#ifdef COMMAND_UTILITIES_DEBUG_QUEUES
    print("dequeue_cmd: id = 0x%.4x, opcode = 0x%x, arg1 = 0x%lx, arg2 = 0x%lx\n",
        *cmd_id, (*cmd)->opcode, *arg1, *arg2);
#endif

    return true;
}

// Returns true if the command queue(s) contains a collect data block command
// with the specified block type (arg 1) and field num (arg 2)
// TODO - unit test
bool cmd_queue_contains_col_data_block(uint8_t block_type) {
    // Assume that the head and tail are the same between queues 1 and 2,
    // since we always enqueue/dequeue to/from them at the same time atomically
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        for (uint8_t i = cmd_queue_1.head; i < cmd_queue_1.tail; i++) {
            uint16_t cmd_id = 0;
            cmd_t* cmd = NULL;
            uint32_t arg1 = 0;
            uint32_t arg2 = 0;
            bytes_to_cmd(&cmd_id, &cmd, &arg1, &arg2, cmd_queue_1.content[i], cmd_queue_2.content[i]);

            if (cmd == &col_data_block_cmd && arg1 == ((uint32_t) block_type)) {
#ifdef COMMAND_UTILITIES_VERBOSE
                print("Found col data cmd in queue (%lu, %lu)\n", arg1, arg2);
#endif
                return true;
            }
        }
    }

    return false;
}

// If the command queue is not empty, dequeues the next command and executes it
void execute_next_cmd(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&cmd_queue_1)) {
            return;
        }
        if (queue_empty(&cmd_queue_2)) {
            return;
        }
        // Only continue if we are open to start a new command
        if (current_cmd != &nop_cmd) {
            return;
        }

        // Fetch the next command
        dequeue_cmd((uint16_t*) &current_cmd_id, (cmd_t**) &current_cmd,
            (uint32_t*) &current_cmd_arg1, (uint32_t*) &current_cmd_arg2);
    }

    if (print_cmds) {
        print("Cmd: id = 0x%.4x, opcode = 0x%.2x, arg1 = 0x%lx, arg2 = 0x%lx\n",
            current_cmd_id, current_cmd->opcode, current_cmd_arg1, current_cmd_arg2);
    }

#ifdef COMMAND_UTILITIES_DEBUG
    print("Start cmd\n");
#endif

    // Start timeout timer at 0
    cmd_timeout_count_s = 0;

    // Decide whether to use the primary or secondary command log
    mem_section_t* cmd_log_mem_section = mem_section_for_cmd((cmd_t*) current_cmd);

    // Log everything for the command (except the status byte)
    // If we are running col_data_block_cmd, only populate the header and write
    // to the command log if it is field 0 (starting the command)
    if ((current_cmd != &col_data_block_cmd) ||
            (current_cmd == &col_data_block_cmd && current_cmd_arg2 == 0)) {
        populate_header(&cmd_log_header, cmd_log_mem_section->curr_block, CMD_RESP_STATUS_UNKNOWN);
        write_mem_cmd_block(cmd_log_mem_section, cmd_log_mem_section->curr_block,
            &cmd_log_header, current_cmd_id, current_cmd->opcode, current_cmd_arg1,
            current_cmd_arg2);
        
        // If we are starting a col_data_block_cmd (therefore must be field 0),
        // store the block number
        // in the command log (will be primary) so it can keep track of it
        // for writing the status byte later
        if (current_cmd == &col_data_block_cmd) {
            for (uint8_t i = 0; i < NUM_DATA_COL_SECTIONS; i++) {
                data_col_t* data_col = all_data_cols[i];
                if (current_cmd_arg1 == data_col->cmd_arg1) {
                    data_col->cmd_log_block_num = cmd_log_mem_section->curr_block;
                }
            }
        }

        inc_and_prepare_mem_section_curr_block(cmd_log_mem_section);
    }
    else {
#ifdef COMMAND_UTILITIES_VERBOSE
    print("Not writing to cmd log\n");
#endif
    }

    // Execute the command's function
    (current_cmd->fn)();
}

// Finishes executing the current command and writes the status byte in the command log
void finish_current_cmd(uint8_t status) {
#ifdef COMMAND_UTILITIES_VERBOSE
    print("%s: stat = 0x%.2x\n", __FUNCTION__, status);
#endif

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // The erase flash command erases the command log as well, therefore re-write the command log
        // for the erase flash command
        if (current_cmd == &erase_all_mem_cmd) {
            write_mem_cmd_block(&prim_cmd_log_mem_section,
                prim_cmd_log_mem_section.curr_block - 1, &cmd_log_header,
                current_cmd_id, current_cmd->opcode, current_cmd_arg1,
                current_cmd_arg2);
        }

        // If we are collecting a data block and it is the last field, write the
        // status byte to the header of the data section and the header of the
        // primary command log
        if (current_cmd == &col_data_block_cmd) {
            // Only do this if the status argument is not the dummy value that
            // indicates data collection is still in progress
            if (status != CMD_RESP_STATUS_DATA_COL_IN_PROGRESS) {
    #ifdef COMMAND_UTILITIES_VERBOSE
                print("Writing mem header status\n");
    #endif

                for (uint8_t i = 0; i < NUM_DATA_COL_SECTIONS; i++) {
                    data_col_t* data_col = all_data_cols[i];

                    // If we just finished the last field of a command
                    // or it was OBC_HK (is only enqueued and executed once)
                    if (current_cmd_arg1 == data_col->cmd_arg1 &&
                            (current_cmd_arg2 == data_col->mem_section->fields_per_block || current_cmd_arg1 == CMD_OBC_HK)) {
                        write_mem_header_status(
                            data_col->mem_section, data_col->header.block_num,
                            status);
                        write_mem_header_status(&prim_cmd_log_mem_section, data_col->cmd_log_block_num, status);
                    }
                }
            }
        }

        else {
            // Write the status byte to the appropriate command log (based on command)
            mem_section_t* section = mem_section_for_cmd((cmd_t*) current_cmd);
            write_mem_header_status(section, section->curr_block - 1, status);
        }

        current_cmd_id = 0xFFFF;
        current_cmd = &nop_cmd;
        current_cmd_arg1 = 0;
        current_cmd_arg2 = 0;

        cmd_timeout_count_s = 0;
    }

#ifdef COMMAND_UTILITIES_DEBUG
    print("Finish cmd: stat = 0x%.2x\n", status);
#endif
}




void prepare_mem_section_curr_block(mem_section_t* section, uint32_t next_block) {
    // If the next block is going into a different memory sector, erase it
    // Use the end address because it reaches the farthest possible address
    uint32_t curr_end_addr = mem_block_end_addr(section, section->curr_block);
    uint32_t curr_sector = mem_sector_for_addr(curr_end_addr);
    uint32_t next_end_addr = mem_block_end_addr(section, next_block);
    uint32_t next_sector = mem_sector_for_addr(next_end_addr);
    
#ifdef COMMAND_UTILITIES_VERBOSE
    print("Preparing mem section block\n");
    print("Current: block = 0x%lx, end_addr = 0x%lx, sector = 0x%lx\n",
        section->curr_block, curr_end_addr, curr_sector);
    print("Next: block = 0x%lx, end_addr = 0x%lx, sector = 0x%lx\n",
        next_block, next_end_addr, next_sector);
#endif

    if (next_sector != curr_sector) {
        // Enqueue to front to be guaranteed to be executed next
        enqueue_cmd_front(CMD_CMD_ID_AUTO_ENQUEUED, &erase_mem_phy_sector_cmd,
            mem_addr_for_sector(next_sector), 0);    // auto
    }

    // Set the new block number
    set_mem_section_curr_block(section, next_block);
}

void inc_and_prepare_mem_section_curr_block(mem_section_t* section) {
    // TODO - unit test properly, test edge cases

    uint32_t curr_block = section->curr_block;
    uint32_t next_block = curr_block + 1;

    // If the next block will go outside the bounds of the section,
    // go back to block 0
    // Use the end address because it reaches the farthest possible address
    uint32_t next_end_section_addr = mem_block_end_section_addr(
        section, next_block);
    uint32_t max_section_addr =
        section->end_addr - section->start_addr;
    if (next_end_section_addr > max_section_addr) {
        next_block = 0;
    }

    prepare_mem_section_curr_block(section, next_block);
}

/*
Populates the block number, success, and current live date/time.
*/
void populate_header(mem_header_t* header, uint32_t block_num, uint8_t status) {
    header->block_num = block_num;
    header->date = read_rtc_date();
    header->time = read_rtc_time();
    header->status = status;
}

void add_def_trans_tx_dec_msg(uint8_t status) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(status);
        finish_trans_tx_resp();
    }
}

void append_header_to_tx_msg(mem_header_t* header) {
    append_to_trans_tx_resp((header->block_num >> 16) & 0xFF);
    append_to_trans_tx_resp((header->block_num >> 8) & 0xFF);
    append_to_trans_tx_resp(header->block_num & 0xFF);
    append_to_trans_tx_resp(header->date.yy);
    append_to_trans_tx_resp(header->date.mm);
    append_to_trans_tx_resp(header->date.dd);
    append_to_trans_tx_resp(header->time.hh);
    append_to_trans_tx_resp(header->time.mm);
    append_to_trans_tx_resp(header->time.ss);
    append_to_trans_tx_resp(header->status);
}

void append_fields_to_tx_msg(uint32_t* fields, uint8_t num_fields) {
    for (uint8_t i = 0; i < num_fields; i++) {
        append_to_trans_tx_resp((fields[i] >> 16) & 0xFF);
        append_to_trans_tx_resp((fields[i] >> 8) & 0xFF);
        append_to_trans_tx_resp(fields[i] & 0xFF);
    }
}


void init_auto_data_col(void) {
    obc_hk_data_col.auto_enabled = read_eeprom_or_default(
        OBC_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    eps_hk_data_col.auto_enabled = read_eeprom_or_default(
        EPS_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    pay_hk_data_col.auto_enabled = read_eeprom_or_default(
        PAY_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    pay_opt_data_col.auto_enabled = read_eeprom_or_default(
        PAY_OPT_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    
    obc_hk_data_col.auto_period = read_eeprom_or_default(
        OBC_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, OBC_HK_AUTO_DATA_COL_PERIOD);
    eps_hk_data_col.auto_period = read_eeprom_or_default(
        EPS_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, EPS_HK_AUTO_DATA_COL_PERIOD);
    pay_hk_data_col.auto_period = read_eeprom_or_default(
        PAY_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, PAY_HK_AUTO_DATA_COL_PERIOD);
    pay_opt_data_col.auto_period = read_eeprom_or_default(
        PAY_OPT_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, PAY_OPT_AUTO_DATA_COL_PERIOD);
}

// Automatic data collection functionality to run in main loop
void run_auto_data_col(void) {
#ifdef COMMAND_UTILITIES_VERBOSE
    print("Auto data col\n");
#endif

    // Atomic because uptime_s could be changed by interrupt
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Could have multiple triggering at the same time
        for (uint8_t i = 0; i < NUM_DATA_COL_SECTIONS; i++) {
            data_col_t* data_col = all_data_cols[i];

            if (data_col->auto_enabled &&
                    (uptime_s >= data_col->prev_auto_col_uptime_s + data_col->auto_period)) {
#ifdef COMMAND_UTILITIES_DEBUG
                print("Auto %s\n", data_col->name);
#endif

                // To avoid filling up the command queue, only enqueue it if
                // the queue does not alreay contain a collect data block
                // command for this block type
                if (!cmd_queue_contains_col_data_block(data_col->cmd_arg1)) {
                    data_col->prev_auto_col_uptime_s = uptime_s;
                    enqueue_cmd(CMD_CMD_ID_AUTO_ENQUEUED, &col_data_block_cmd, data_col->cmd_arg1, 0);
                }
                else {
#ifdef COMMAND_UTILITIES_DEBUG
                    print("Already in cmd queue\n");
#endif
                }
            }
        }
    }
}

// If a command (e.g. waiting for a CAN response) is not finished after the
// designated period, stop waiting to finish
void cmd_timeout_timer_cb(void) {
    if (current_cmd == &nop_cmd) {
        return;
    }

    cmd_timeout_count_s += 1;

    if (cmd_timeout_count_s >= cmd_timeout_period_s) {
#ifdef COMMAND_UTILITIES_DEBUG
        print("CMD TIMED OUT\n");
#endif
        // Only add response packet if not auto command
        if (current_cmd_id != CMD_CMD_ID_AUTO_ENQUEUED) {
            add_def_trans_tx_dec_msg(CMD_RESP_STATUS_TIMED_OUT);
            finish_current_cmd(CMD_RESP_STATUS_TIMED_OUT);
        }

        cmd_timeout_count_s = 0;
    }
}
