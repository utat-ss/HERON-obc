#include "command_utilities.h"

// If you get an error here because `security.h` is not found, copy the dummy
// `security.h` file from https://github.com/HeronMkII/templates to your `src`
// folder and try again
// DO NOT COMMIT ANY `security.h` FILE TO GIT
#include "security.h"
// The super-secret password for sensitive commands
const uint8_t correct_pwd[] = SECURITY_CORRECT_PWD;

// Queue of commands that need to be executed but have not been executed yet
queue_t cmd_queue;
// Arguments corresponding to each command
queue_t cmd_args_queue;

// A pointer to the currently executing command (or nop_cmd for no command executing)
// Use double volatile just in case
volatile cmd_t* volatile current_cmd = &nop_cmd;
// Current command arguments
volatile uint32_t current_cmd_arg1 = 0;
volatile uint32_t current_cmd_arg2 = 0;
// true if the previous command succeeded or false if it failed
volatile bool prev_cmd_succeeded = false;
// Don't need to store a variable for the password because it is checked in the
// decoded message before enqueueing to the command queue

// For the uptime interrupt to can_timer_cb to finish command if no response
// received after 30 seconds
volatile uint8_t can_countdown = 0;

mem_header_t obc_hk_header;
uint32_t obc_hk_fields[CAN_OBC_HK_FIELD_COUNT] = { 0 };
mem_header_t eps_hk_header;
uint32_t eps_hk_fields[CAN_EPS_HK_FIELD_COUNT] = { 0 };
mem_header_t pay_hk_header;
uint32_t pay_hk_fields[CAN_PAY_HK_FIELD_COUNT] = { 0 };
mem_header_t pay_opt_header;
uint32_t pay_opt_fields[CAN_PAY_OPT_FIELD_COUNT] = { 0 };
mem_header_t cmd_log_header;


volatile auto_data_col_t obc_hk_auto_data_col = {
    .enabled = false,
    .period = OBC_HK_AUTO_DATA_COL_PERIOD,
    .count = 0
};
volatile auto_data_col_t eps_hk_auto_data_col = {
    .enabled = false,
    .period = EPS_HK_AUTO_DATA_COL_PERIOD,
    .count = 0
};
volatile auto_data_col_t pay_hk_auto_data_col = {
    .enabled = false,
    .period = PAY_HK_AUTO_DATA_COL_PERIOD,
    .count = 0
};
volatile auto_data_col_t pay_opt_auto_data_col = {
    .enabled = false,
    .period = PAY_OPT_AUTO_DATA_COL_PERIOD,
    .count = 0
};


// Date and time of the most recent restart
rtc_date_t restart_date = { .yy = 0, .mm = 0, .dd  = 0 };
rtc_time_t restart_time = { .hh = 0, .mm = 0, .ss  = 0 };




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
        // Only accept 13 byte messages
        if (trans_rx_dec_len < 13) {
            return;
        }

        // Use shorter names for now
        uint8_t* msg = (uint8_t*) trans_rx_dec_msg;
        uint8_t msg_type = msg[0];
        uint32_t arg1 =
            ((uint32_t) msg[1] << 24) |
            ((uint32_t) msg[2] << 16) |
            ((uint32_t) msg[3] << 8) |
            ((uint32_t) msg[4]);
        uint32_t arg2 =
            ((uint32_t) msg[5] << 24) |
            ((uint32_t) msg[6] << 16) |
            ((uint32_t) msg[7] << 8) |
            ((uint32_t) msg[8]);

        cmd_t* cmd = trans_msg_type_to_cmd(msg_type);
        if (cmd == NULL) {
            return;
        }

        // Password is indices 9-12 - check 4-byte password if necessary for the command
        if (cmd->pwd_protected) {
            for (uint8_t i = 0; i < 4; i++) {
                if (msg[9 + i] != correct_pwd[i]) {
                    // TODO - NACK
                    return;
                }
            }
        }
        
        enqueue_cmd(cmd, arg1, arg2);
    }
}

// NOTE: these three functions should be used within the same atomic block

void start_trans_tx_dec_msg(void) {
    trans_tx_dec_msg[0] = trans_cmd_to_msg_type((cmd_t*) current_cmd);
    trans_tx_dec_msg[1] = (current_cmd_arg1 >> 24) & 0xFF;
    trans_tx_dec_msg[2] = (current_cmd_arg1 >> 16) & 0xFF;
    trans_tx_dec_msg[3] = (current_cmd_arg1 >> 8) & 0xFF;
    trans_tx_dec_msg[4] = current_cmd_arg1 & 0xFF;
    trans_tx_dec_msg[5] = (current_cmd_arg2 >> 24) & 0xFF;
    trans_tx_dec_msg[6] = (current_cmd_arg2 >> 16) & 0xFF;
    trans_tx_dec_msg[7] = (current_cmd_arg2 >> 8) & 0xFF;
    trans_tx_dec_msg[8] = current_cmd_arg2 & 0xFF;

    trans_tx_dec_len = 9;
}

void append_to_trans_tx_dec_msg(uint8_t byte) {
    if (trans_tx_dec_len < TRANS_TX_DEC_MSG_MAX_SIZE) {
        trans_tx_dec_msg[trans_tx_dec_len] = byte;
        trans_tx_dec_len++;
    }
}

void finish_trans_tx_dec_msg(void) {
    trans_tx_dec_avail = true;
}


cmd_t* trans_msg_type_to_cmd(uint8_t msg_type) {
    for (uint8_t i = 0; i < ALL_CMDS_LEN; i++) {
        if (all_cmds_list[i]->opcode == msg_type) {
            return all_cmds_list[i];
        }
    }

    return &nop_cmd;
}

uint8_t trans_cmd_to_msg_type(cmd_t* cmd) {
    return cmd->opcode;
}




// We know that the microcontroller uses 16 bit addresses, so store a function
// pointer in the first 2 bytes of the queue entry (data[0] = MSB, data[1] = LSB)
// TODO - develop a func test and a harness test for enqueueing and dequeueing cmd_t structs

/*
cmd - Command (with cmd->fn already set before calling this function) to enqueue
*/
void enqueue_cmd(cmd_t* cmd, uint32_t arg1, uint32_t arg2) {
    print("enqueue_cmd: cmd = 0x%x, arg1 = %lu, arg2 = %lu\n", cmd, arg1, arg2);

    // Cast the cmd_t command pointer to a uint16
    uint16_t cmd_ptr = (uint16_t) cmd;

    // Enqueue the command as an 8-byte array
    uint8_t cmd_data[8] = {0};
    cmd_data[0] = (cmd_ptr >> 8) & 0xFF;
    cmd_data[1] = cmd_ptr & 0xFF;

    uint8_t args_data[8] = {0};
    args_data[0] = (arg1 >> 24) & 0xFF;
    args_data[1] = (arg1 >> 16) & 0xFF;
    args_data[2] = (arg1 >> 8) & 0xFF;
    args_data[3] = arg1 & 0xFF;
    args_data[4] = (arg2 >> 24) & 0xFF;
    args_data[5] = (arg2 >> 16) & 0xFF;
    args_data[6] = (arg2 >> 8) & 0xFF;
    args_data[7] = arg2 & 0xFF;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        enqueue(&cmd_queue, cmd_data);
        enqueue(&cmd_args_queue, args_data);
    }
}

/*
cmd - The struct must already exist (be allocated) before calling this function,
      then this function sets the value of cmd->fn
*/
void dequeue_cmd(void) {
    // Dequeue the command as an 8-byte array
    uint8_t cmd_data[8] = {0};
    uint8_t args_data[8] = {0};

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&cmd_queue)) {
            return;
        }
        if (queue_empty(&cmd_args_queue)) {
            return;
        }
        dequeue(&cmd_queue, cmd_data);
        dequeue(&cmd_args_queue, args_data);
    }

    uint16_t cmd_ptr =
        (((uint16_t) cmd_data[0]) << 8) |
        (((uint16_t) cmd_data[1]));
    uint32_t arg1 =
        (((uint32_t) args_data[0]) << 24) |
        (((uint32_t) args_data[1]) << 16) |
        (((uint32_t) args_data[2]) << 8) |
        (((uint32_t) args_data[3]));
    uint32_t arg2 =
        (((uint32_t) args_data[4]) << 24) |
        (((uint32_t) args_data[5]) << 16) |
        (((uint32_t) args_data[6]) << 8) |
        (((uint32_t) args_data[7]));

    // Cast the uint16 to a cmd_t command pointer
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Set the global current command to prevent other commands from running
        current_cmd = (cmd_t*) cmd_ptr;
        current_cmd_arg1 = arg1;
        current_cmd_arg2 = arg2;
        populate_header(&cmd_log_header, cmd_log_mem_section.curr_block, 0xFF);
        write_mem_cmd_block(cmd_log_mem_section.curr_block, &cmd_log_header,
            trans_cmd_to_msg_type((cmd_t*) current_cmd), current_cmd_arg1, current_cmd_arg2);
        inc_mem_section_curr_block(&cmd_log_mem_section);
    }

    print("dequeue_cmd: cmd = 0x%x, arg1 = 0x%lx, arg2 = 0x%lx\n", current_cmd,
        current_cmd_arg1, current_cmd_arg2);
}

// If the command queue is not empty, dequeues the next command and executes it
void execute_next_cmd(void) {
    if (!queue_empty(&cmd_queue) && current_cmd == &nop_cmd) {
        print("Starting cmd\n");
        // Fetch the next command
        dequeue_cmd();
        // Run the command's function
        (current_cmd->fn)();
    }
}

// Finishes executing the current command and sets the succeeded flag
void finish_current_cmd(bool succeeded) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // The erase flash command erases the command log as well, therefore re-write the command log
        // for the erase flash command
        if (current_cmd == &erase_all_mem_cmd) {
            write_mem_cmd_block(cmd_log_mem_section.curr_block - 1, &cmd_log_header,
                trans_cmd_to_msg_type((cmd_t*) current_cmd), current_cmd_arg1, current_cmd_arg2);
        }
        current_cmd = &nop_cmd;
        current_cmd_arg1 = 0;
        current_cmd_arg2 = 0;
        prev_cmd_succeeded = succeeded;
        can_countdown = 0;
        if (succeeded == true) {
            write_mem_cmd_success(cmd_log_mem_section.curr_block - 1, 0x01);
        } else {
            write_mem_cmd_success(cmd_log_mem_section.curr_block - 1, 0x00);
        }
    }
    print("Finished cmd\n");
}




/*
Populates the block number, success, and current live date/time.
*/
void populate_header(mem_header_t* header, uint32_t block_num, uint8_t success) {
    header->block_num = block_num;
    header->success = success;
    header->date = read_rtc_date();
    header->time = read_rtc_time();
}


void append_header_to_tx_msg(mem_header_t* header) {
    append_to_trans_tx_dec_msg((header->block_num >> 16) & 0xFF);
    append_to_trans_tx_dec_msg((header->block_num >> 8) & 0xFF);
    append_to_trans_tx_dec_msg(header->block_num & 0xFF);
    append_to_trans_tx_dec_msg(header->success);
    append_to_trans_tx_dec_msg(header->date.yy);
    append_to_trans_tx_dec_msg(header->date.mm);
    append_to_trans_tx_dec_msg(header->date.dd);
    append_to_trans_tx_dec_msg(header->time.hh);
    append_to_trans_tx_dec_msg(header->time.mm);
    append_to_trans_tx_dec_msg(header->time.ss);
}

void append_fields_to_tx_msg(uint32_t* fields, uint8_t num_fields) {
    for (uint8_t i = 0; i < num_fields; i++) {
        append_to_trans_tx_dec_msg((fields[i] >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((fields[i] >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(fields[i] & 0xFF);
    }
}


// TODO - put in utilities?
uint32_t read_eeprom(uint32_t* addr, uint32_t default_val) {
    uint32_t value = eeprom_read_dword(addr);
    if (value == EEPROM_DEF_DWORD) {
        return default_val;
    }
    return value;
}


void init_auto_data_col(void) {
    obc_hk_auto_data_col.enabled = read_eeprom(
        OBC_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    eps_hk_auto_data_col.enabled = read_eeprom(
        EPS_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    pay_hk_auto_data_col.enabled = read_eeprom(
        PAY_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    pay_opt_auto_data_col.enabled = read_eeprom(
        PAY_OPT_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    
    obc_hk_auto_data_col.period = read_eeprom(
        OBC_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, OBC_HK_AUTO_DATA_COL_PERIOD);
    eps_hk_auto_data_col.period = read_eeprom(
        EPS_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, EPS_HK_AUTO_DATA_COL_PERIOD);
    pay_hk_auto_data_col.period = read_eeprom(
        PAY_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, PAY_HK_AUTO_DATA_COL_PERIOD);
    pay_opt_auto_data_col.period = read_eeprom(
        PAY_OPT_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, PAY_OPT_AUTO_DATA_COL_PERIOD);

    add_uptime_callback(auto_data_col_timer_cb);
}

// Automatic data collection timer callback (for 16-bit timer)
void auto_data_col_timer_cb(void) {
    // print("Auto data col timer cb\n");

    if (obc_hk_auto_data_col.enabled) {
        obc_hk_auto_data_col.count += 1;

        if (obc_hk_auto_data_col.count >= obc_hk_auto_data_col.period) {
            print("Auto OBC_HK\n");
            obc_hk_auto_data_col.count = 0;
            enqueue_cmd(&col_data_block_cmd, CMD_OBC_HK, 1);    // auto
        }
    }

    if (eps_hk_auto_data_col.enabled) {
        eps_hk_auto_data_col.count += 1;

        if (eps_hk_auto_data_col.count >= eps_hk_auto_data_col.period) {
            print("Auto EPS_HK\n");
            eps_hk_auto_data_col.count = 0;
            enqueue_cmd(&col_data_block_cmd, CMD_EPS_HK, 1);    // auto
        }
    }

    if (pay_hk_auto_data_col.enabled) {
        pay_hk_auto_data_col.count += 1;

        if (pay_hk_auto_data_col.count >= pay_hk_auto_data_col.period) {
            print("Auto PAY_HK\n");
            pay_hk_auto_data_col.count = 0;
            enqueue_cmd(&col_data_block_cmd, CMD_PAY_HK, 1);    // auto
        }
    }

    if (pay_opt_auto_data_col.enabled) {
        pay_opt_auto_data_col.count += 1;

        if (pay_opt_auto_data_col.count >= pay_opt_auto_data_col.period) {
            print("Auto PAY_OPT\n");
            pay_opt_auto_data_col.count = 0;
            enqueue_cmd(&col_data_block_cmd, CMD_PAY_OPT, 1);   // auto
        }
    }
}

// If no CAN response is received after 30 seconds, stop waiting for command
void can_timer_cb(void) {
    if (can_countdown > 155) {
        finish_current_cmd(false);
    }
    else if (can_countdown > 0) {
        can_countdown -= 1;
        if (can_countdown == 0) {
            finish_current_cmd(false);
        }
    }
}
