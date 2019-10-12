#include "command_utilities.h"

// If you get an error here because `security.h` is not found, copy the dummy
// `security.h` file from https://github.com/HeronMkII/templates to your `src`
// folder and try again
// DO NOT COMMIT ANY `security.h` FILE TO GIT
#include "security.h"
// The super-secret password for sensitive commands
const uint8_t correct_pwd[] = SECURITY_CORRECT_PWD;

// Queue of commands that need to be executed but have not been executed yet
queue_t cmd_opcode_queue;
// Arguments corresponding to each command
queue_t cmd_args_queue;

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
            add_trans_tx_ack(0xFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x02);
            return;
        }

        // Use shorter variable names for convenience
        volatile uint8_t* msg = (volatile uint8_t*) trans_rx_dec_msg;
        uint8_t opcode = msg[0];
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

        cmd_t* cmd = cmd_opcode_to_cmd(opcode);
        if (cmd == &nop_cmd) {
            add_trans_tx_ack(opcode, arg1, arg2, 0x03);
            return;
        }

        // Password is indices 9-12 - check 4-byte password if necessary for the command
        if (cmd->pwd_protected) {
            for (uint8_t i = 0; i < 4; i++) {
                if (msg[9 + i] != correct_pwd[i]) {
                    // NACK
                    add_trans_tx_ack(opcode, arg1, arg2, 0x04);
                    return;
                }
            }
        }
        
        add_trans_tx_ack(opcode, arg1, arg2, 0x00);
        enqueue_cmd(cmd, arg1, arg2);
    }
}

void process_trans_tx_ack(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!trans_tx_ack_avail) {
            return;
        }
        trans_tx_ack_avail = false;

        // Can't use the standard trans_tx_dec functions because they use the current_cmd variables
        // TODO - better way?
        trans_tx_dec_msg[0] = trans_tx_ack_opcode | (0x1 << 7);
        trans_tx_dec_msg[1] = (trans_tx_ack_arg1 >> 24) & 0xFF;
        trans_tx_dec_msg[2] = (trans_tx_ack_arg1 >> 16) & 0xFF;
        trans_tx_dec_msg[3] = (trans_tx_ack_arg1 >> 8) & 0xFF;
        trans_tx_dec_msg[4] = (trans_tx_ack_arg1 >> 0) & 0xFF;
        trans_tx_dec_msg[5] = (trans_tx_ack_arg2 >> 24) & 0xFF;
        trans_tx_dec_msg[6] = (trans_tx_ack_arg2 >> 16) & 0xFF;
        trans_tx_dec_msg[7] = (trans_tx_ack_arg2 >> 8) & 0xFF;
        trans_tx_dec_msg[8] = (trans_tx_ack_arg2 >> 0) & 0xFF;
        trans_tx_dec_msg[9] = trans_tx_ack_status;

        trans_tx_dec_len = 10;
        trans_tx_dec_avail = true;
    }
}

// NOTE: these three functions should be used within the same atomic block

void start_trans_tx_dec_msg(void) {
    trans_tx_dec_msg[0] = current_cmd->opcode;
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




// We know that the microcontroller uses 16 bit addresses, so store a function
// pointer in the first 2 bytes of the queue entry (data[0] = MSB, data[1] = LSB)
// TODO - develop a harness test for enqueueing and dequeueing commands

/*
Enqueue the opcode instead of the function pointer because it's safer in case
something goes wrong.
cmd - Command to enqueue
*/
void enqueue_cmd(cmd_t* cmd, uint32_t arg1, uint32_t arg2) {
    print("enqueue_cmd: opcode = 0x%x, arg1 = 0x%lx, arg2 = 0x%lx\n", cmd->opcode, arg1, arg2);

    // Enqueue the command as an 8-byte array
    uint8_t opcode_data[8] = {0};
    opcode_data[0] = cmd->opcode;

    uint8_t args_data[8] = {0};
    args_data[0] = (arg1 >> 24) & 0xFF;
    args_data[1] = (arg1 >> 16) & 0xFF;
    args_data[2] = (arg1 >> 8) & 0xFF;
    args_data[3] = arg1 & 0xFF;
    args_data[4] = (arg2 >> 24) & 0xFF;
    args_data[5] = (arg2 >> 16) & 0xFF;
    args_data[6] = (arg2 >> 8) & 0xFF;
    args_data[7] = arg2 & 0xFF;

    // Make sure modifications to the states of the two queues are atomic/consistent
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        enqueue(&cmd_opcode_queue, opcode_data);
        enqueue(&cmd_args_queue, args_data);
    }
}

/*
cmd - The struct must already exist (be allocated) before calling this function,
      then this function sets the value of cmd->fn
      Use a double pointer because we need to set the value of the cmd pointer
*/
void dequeue_cmd(cmd_t** cmd, uint32_t* arg1, uint32_t* arg2) {
    // Dequeue the command as an 8-byte array
    uint8_t opcode_data[8] = {0};
    uint8_t args_data[8] = {0};

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&cmd_opcode_queue)) {
            return;
        }
        if (queue_empty(&cmd_args_queue)) {
            return;
        }
        dequeue(&cmd_opcode_queue, opcode_data);
        dequeue(&cmd_args_queue, args_data);
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        *cmd = cmd_opcode_to_cmd(opcode_data[0]);
        *arg1 =
            (((uint32_t) args_data[0]) << 24) |
            (((uint32_t) args_data[1]) << 16) |
            (((uint32_t) args_data[2]) << 8) |
            (((uint32_t) args_data[3]));
        *arg2 =
            (((uint32_t) args_data[4]) << 24) |
            (((uint32_t) args_data[5]) << 16) |
            (((uint32_t) args_data[6]) << 8) |
            (((uint32_t) args_data[7]));
    }

    print("dequeue_cmd: opcode = 0x%x, arg1 = 0x%lx, arg2 = 0x%lx\n", (*cmd)->opcode,
        *arg1, *arg2);
}

// If the command queue is not empty, dequeues the next command and executes it
void execute_next_cmd(void) {
    // TODO - maybe check if one queue has more items than the other and correct
    // if necessary?
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&cmd_opcode_queue)) {
            return;
        }
        if (queue_empty(&cmd_args_queue)) {
            return;
        }
        if (current_cmd != &nop_cmd) {
            return;
        }

        // Fetch the next command
        dequeue_cmd((cmd_t**) &current_cmd,
            (uint32_t*) &current_cmd_arg1, (uint32_t*) &current_cmd_arg2);
    }

    print("Starting cmd\n");

    // TODO - this should be moved to when receiving a transceiver packet instead
    // Restart the counter for not receiving communication
    restart_com_timeout();
    // Start timeout timer at 0
    cmd_timeout_count_s = 0;

    // Decide whether to use the primary or secondary command log
    mem_section_t* cmd_log_mem_section = &prim_cmd_log_mem_section;
    if (current_cmd == &read_data_block_cmd ||
            current_cmd == &read_prim_cmd_blocks_cmd ||
            current_cmd == &read_sec_cmd_blocks_cmd) {
        cmd_log_mem_section = &sec_cmd_log_mem_section;
    }

    // Log everything for the command (except the status byte)
    populate_header(&cmd_log_header, cmd_log_mem_section->curr_block, 0xFF);
    write_mem_cmd_block(cmd_log_mem_section, cmd_log_mem_section->curr_block,
        &cmd_log_header,
        current_cmd->opcode, current_cmd_arg1, current_cmd_arg2);
    inc_and_prepare_mem_section_curr_block(cmd_log_mem_section);

    // Execute the command's function
    (current_cmd->fn)();
}

// Finishes executing the current command and sets the succeeded flag
// TODO - integrate with success byte
void finish_current_cmd(uint8_t status) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // The erase flash command erases the command log as well, therefore re-write the command log
        // for the erase flash command
        if (current_cmd == &erase_all_mem_cmd) {
            write_mem_cmd_block(&prim_cmd_log_mem_section,
                prim_cmd_log_mem_section.curr_block - 1, &cmd_log_header,
                current_cmd->opcode, current_cmd_arg1, current_cmd_arg2);
        }

        if (current_cmd == &read_data_block_cmd || current_cmd == &read_prim_cmd_blocks_cmd
            || current_cmd == &read_sec_cmd_blocks_cmd) {
            write_mem_header_status(&sec_cmd_log_mem_section, sec_cmd_log_mem_section.curr_block - 1, status);
        } else {
            write_mem_header_status(&prim_cmd_log_mem_section, prim_cmd_log_mem_section.curr_block - 1, status);
        }
        current_cmd = &nop_cmd;
        current_cmd_arg1 = 0;
        current_cmd_arg2 = 0;
        cmd_timeout_count_s = 0;
    }
    print("Finished cmd\n");
}




void prepare_mem_section_curr_block(mem_section_t* section, uint32_t next_block) {
    // TODO - unit test properly, test edge cases

    // If the next block is going into a different memory sector,
    // erase it
    // Use the end address because it reaches the farthest possible address
    uint32_t curr_sector = mem_sector_for_addr(mem_block_end_addr(
        section, section->curr_block));
    uint32_t next_sector = mem_sector_for_addr(mem_block_end_addr(
        section, next_block));
    if (next_sector != curr_sector) {
        // TODO - should use enqueue_front to guaranteed to be executed next
        enqueue_cmd(&erase_mem_phy_sector_cmd,
            mem_addr_for_sector(next_sector), 1);    // auto
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


void append_header_to_tx_msg(mem_header_t* header) {
    append_to_trans_tx_dec_msg((header->block_num >> 16) & 0xFF);
    append_to_trans_tx_dec_msg((header->block_num >> 8) & 0xFF);
    append_to_trans_tx_dec_msg(header->block_num & 0xFF);
    append_to_trans_tx_dec_msg(header->date.yy);
    append_to_trans_tx_dec_msg(header->date.mm);
    append_to_trans_tx_dec_msg(header->date.dd);
    append_to_trans_tx_dec_msg(header->time.hh);
    append_to_trans_tx_dec_msg(header->time.mm);
    append_to_trans_tx_dec_msg(header->time.ss);
    append_to_trans_tx_dec_msg(header->status);
}

void append_fields_to_tx_msg(uint32_t* fields, uint8_t num_fields) {
    for (uint8_t i = 0; i < num_fields; i++) {
        append_to_trans_tx_dec_msg((fields[i] >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((fields[i] >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(fields[i] & 0xFF);
    }
}


void init_auto_data_col(void) {
    obc_hk_auto_data_col.enabled = read_eeprom_or_default(
        OBC_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    eps_hk_auto_data_col.enabled = read_eeprom_or_default(
        EPS_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    pay_hk_auto_data_col.enabled = read_eeprom_or_default(
        PAY_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    pay_opt_auto_data_col.enabled = read_eeprom_or_default(
        PAY_OPT_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, 0);
    
    obc_hk_auto_data_col.period = read_eeprom_or_default(
        OBC_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, OBC_HK_AUTO_DATA_COL_PERIOD);
    eps_hk_auto_data_col.period = read_eeprom_or_default(
        EPS_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, EPS_HK_AUTO_DATA_COL_PERIOD);
    pay_hk_auto_data_col.period = read_eeprom_or_default(
        PAY_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, PAY_HK_AUTO_DATA_COL_PERIOD);
    pay_opt_auto_data_col.period = read_eeprom_or_default(
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

// If a command (e.g. waiting for a CAN response) is not finished after the
// designated period, stop waiting to finish
void cmd_timeout_timer_cb(void) {
    if (current_cmd == &nop_cmd) {
        return;
    }

    cmd_timeout_count_s += 1;
    if (cmd_timeout_count_s >= cmd_timeout_period_s) {
        finish_current_cmd(CMD_STATUS_TIMED_OUT);
    }
}
