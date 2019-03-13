#include "commands.h"

void nop_fn(void);
void ping_fn(void);
void get_restart_uptime_fn(void);
void get_rtc_fn(void);
void set_rtc_fn(void);
void read_mem_fn(void);
void erase_mem_fn(void);
void collect_block_fn(void);
void read_local_block_fn(void);
void read_mem_block_fn(void);
void set_aut_data_col_enable_fn(void);
void set_aut_data_col_period_fn(void);
void resync_aut_data_col_fn(void);
void set_eps_heater_sp_fn(void);
void set_pay_heater_sp_fn(void);
void actuate_motors_fn(void);
void reset_fn(void);


// If true, the program will simulate local actions (i.e. simulates any
// operations with peripherals besides the microcontroller and CAN)
// This allows the software to be used on any microcontroller just to test
// the command handling and CAN functionality
bool sim_local_actions = false;

// TODO - define default period constants
volatile aut_data_col_t eps_hk_aut_data_col = {
    .enabled = false,
    .period = 60,
    .count = 0
};
volatile aut_data_col_t pay_hk_aut_data_col = {
    .enabled = false,
    .period = 60,
    .count = 0
};
volatile aut_data_col_t pay_opt_aut_data_col = {
    .enabled = false,
    .period = 60,
    .count = 0
};

// Queue of commands that need to be executed but have not been executed yet
queue_t cmd_queue;
// Arguments corresponding to each command
queue_t cmd_args_queue;

// The currently executing command (or nop_fn for no command executing)
// NOTE: need to compare the function pointer, not the command pointer (could have a duplicate of the command struct but the function pointer will always be the same for the same command)
// TODO - check volatile
volatile cmd_t* volatile current_cmd = &nop_cmd;
// Current command arguments
volatile uint32_t current_cmd_arg1 = 0;
volatile uint32_t current_cmd_arg2 = 0;
// true if the previous command succeeded or false if it failed
volatile bool prev_cmd_succeeded = false;

// All possible commands
// Default no-op command
cmd_t nop_cmd = {
    .fn = nop_fn
};
cmd_t ping_cmd = {
    .fn = ping_fn
};
cmd_t get_restart_uptime_cmd = {
    .fn = get_restart_uptime_fn
};
cmd_t get_rtc_cmd = {
    .fn = get_rtc_fn
};
cmd_t set_rtc_cmd = {
    .fn = set_rtc_fn
};
cmd_t read_mem_cmd = {
    .fn = read_mem_fn
};
cmd_t erase_mem_cmd = {
    .fn = erase_mem_fn
};
cmd_t collect_block_cmd = {
    .fn = collect_block_fn
};
cmd_t read_local_block_cmd = {
    .fn = read_local_block_fn
};
cmd_t read_mem_block_cmd = {
    .fn = read_mem_block_fn
};
cmd_t set_aut_data_col_enable_cmd = {
    .fn = set_aut_data_col_enable_fn
};
cmd_t set_aut_data_col_period_cmd = {
    .fn = set_aut_data_col_period_fn
};
cmd_t resync_aut_data_col_cmd = {
    .fn = resync_aut_data_col_fn
};
cmd_t set_eps_heater_sp_cmd = {
    .fn = set_eps_heater_sp_fn
};
cmd_t set_pay_heater_sp_cmd = {
    .fn = set_pay_heater_sp_fn
};
cmd_t actuate_motors_cmd = {
    .fn = actuate_motors_fn
};
cmd_t reset_cmd = {
    .fn = reset_fn
};





// Command callback functions

void nop_fn(void) {}

void ping_fn(void) {
    print("Ping TODO\n");
    finish_current_cmd(true);
}

void get_restart_uptime_fn(void) {
    print("Get restart uptime TODO\n");
    finish_current_cmd(true);
}

void get_rtc_fn(void) {
    print("Get RTC TODO\n");
    finish_current_cmd(true);
}

void set_rtc_fn(void) {
    print("Set RTC TODO\n");
    finish_current_cmd(true);
}

void read_mem_fn(void) {
    print("Read mem TODO\n");
    finish_current_cmd(true);
}

void erase_mem_fn(void) {
    print("Erase mem TODO\n");
    finish_current_cmd(true);
}

// Starts requesting block data (field 0)
void collect_block_fn(void) {
    switch (current_cmd_arg1) {
        case CMD_BLOCK_EPS_HK:
            print("Starting EPS_HK\n");
            populate_header(&eps_hk_header, eps_hk_mem_section.curr_block, 0x00);
            enqueue_eps_hk_tx_msg(0);
            break;
        case CMD_BLOCK_PAY_HK:
            print ("Starting PAY_HK\n");
            populate_header(&pay_hk_header, pay_hk_mem_section.curr_block, 0x00);
            enqueue_pay_hk_tx_msg(0);
            break;
        case CMD_BLOCK_PAY_OPT:
            print ("Starting PAY_OPT\n");
            populate_header(&pay_opt_header, pay_opt_mem_section.curr_block, 0x00);
            enqueue_pay_opt_tx_msg(0);
            break;
        default:
            break;
    }
}

void read_local_block_fn(void) {
    print("Read local TODO\n");
    finish_current_cmd(true);
}

void read_mem_block_fn(void) {
    switch (current_cmd_arg1) {
        case CMD_BLOCK_EPS_HK:
            if (sim_local_actions) {
                // Random values
                eps_hk_header.block_num = 3;
                eps_hk_header.error = 0x00;
                eps_hk_header.date.yy = 14;
                eps_hk_header.date.mm = 11;
                eps_hk_header.date.dd = 7;
                eps_hk_header.time.hh = 18;
                eps_hk_header.time.mm = 5;
                eps_hk_header.time.ss = 20;
                for (uint8_t i = 0; i < CAN_EPS_HK_FIELD_COUNT; i++) {
                    eps_hk_fields[i] = i + 4;
                }
            }
            else {
                read_mem_block(&eps_hk_mem_section, current_cmd_arg2,
                    &eps_hk_header, eps_hk_fields);
            }
            break;

        case CMD_BLOCK_PAY_HK:
            if (sim_local_actions) {
                pay_hk_header.block_num = 15;
                pay_hk_header.error = 0x00;
                pay_hk_header.date.yy = 13;
                pay_hk_header.date.mm = 11;
                pay_hk_header.date.dd = 7;
                pay_hk_header.time.hh = 21;
                pay_hk_header.time.mm = 5;
                pay_hk_header.time.ss = 20;
                for (uint8_t i = 0; i < CAN_PAY_HK_FIELD_COUNT; i++) {
                    pay_hk_fields[i] = i + 17;
                }
            }
            else {
                read_mem_block(&pay_hk_mem_section, current_cmd_arg2,
                    &pay_hk_header, pay_hk_fields);
            }
            break;

        case CMD_BLOCK_PAY_OPT:
            if (sim_local_actions) {
                pay_opt_header.block_num = 29;
                pay_opt_header.error = 0x00;
                pay_opt_header.date.yy = 79;
                pay_opt_header.date.mm = 11;
                pay_opt_header.date.dd = 7;
                pay_opt_header.time.hh = 4;
                pay_opt_header.time.mm = 5;
                pay_opt_header.time.ss = 20;
                for (uint8_t i = 0; i < CAN_PAY_OPT_FIELD_COUNT; i++) {
                    pay_opt_fields[i] = i + 11;
                }
            }
            else {
                read_mem_block(&pay_opt_mem_section, current_cmd_arg2,
                    &pay_opt_header, pay_opt_fields);
            }
            break;

        default:
            break;
    }

    // TODO - create transceiver message

    print("Read memory block\n");

    finish_current_cmd(true);
}

void set_aut_data_col_enable_fn(void) {
    switch (current_cmd_arg1) {
        case CMD_BLOCK_EPS_HK:
            eps_hk_aut_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            break;
        case CMD_BLOCK_PAY_HK:
            pay_hk_aut_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            break;
        case CMD_BLOCK_PAY_OPT:
            pay_opt_aut_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            break;
        default:
            break;
    }
    finish_current_cmd(true);
}

void set_aut_data_col_period_fn(void) {
    switch (current_cmd_arg1) {
        case CMD_BLOCK_EPS_HK:
            eps_hk_aut_data_col.period = current_cmd_arg2;
            break;
        case CMD_BLOCK_PAY_HK:
            pay_hk_aut_data_col.period = current_cmd_arg2;
            break;
        case CMD_BLOCK_PAY_OPT:
            pay_opt_aut_data_col.period = current_cmd_arg2;
            break;
        default:
            break;
    }
    finish_current_cmd(true);
}

void resync_aut_data_col_fn(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        eps_hk_aut_data_col.count = 0;
        pay_hk_aut_data_col.count = 0;
        pay_opt_aut_data_col.count = 0;
    }
    finish_current_cmd(true);
}

void set_eps_heater_sp_fn(void) {
    switch (current_cmd_arg1) {
        case 0:
            enqueue_eps_ctrl_tx_msg(CAN_EPS_CTRL_HEAT_SP1, current_cmd_arg2);
            break;
        case 1:
            enqueue_eps_ctrl_tx_msg(CAN_EPS_CTRL_HEAT_SP2, current_cmd_arg2);
            break;
        default:
            finish_current_cmd(false);
            break;
    }
}

void set_pay_heater_sp_fn(void) {
    switch (current_cmd_arg1) {
        case 0:
            enqueue_pay_ctrl_tx_msg(CAN_PAY_CTRL_HEAT_SP1, current_cmd_arg2);
            break;
        case 1:
            enqueue_pay_ctrl_tx_msg(CAN_PAY_CTRL_HEAT_SP2, current_cmd_arg2);
            break;
        default:
            finish_current_cmd(false);
            break;
    }
}

void actuate_motors_fn(void) {
    switch (current_cmd_arg1) {
        case 1:
            enqueue_pay_ctrl_tx_msg(CAN_PAY_CTRL_ACT_UP, 0);
            break;
        case 2:
            enqueue_pay_ctrl_tx_msg(CAN_PAY_CTRL_ACT_DOWN, 0);
            break;
        default:
            finish_current_cmd(false);
            break;
    }
}

void reset_fn(void) {
    print("Reset TODO\n");
}

// Finishes executing the current command and sets the succeeded flag
void finish_current_cmd(bool succeeded) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        current_cmd = &nop_cmd;
        current_cmd_arg1 = 0;
        current_cmd_arg2 = 0;
        prev_cmd_succeeded = succeeded;
    }
    print("Finished cmd\n");
}





// Automatic data collection timer callback (for 16-bit timer)
void aut_data_col_timer_cb(void) {
    print("Aut data col timer cb\n");

    if (eps_hk_aut_data_col.enabled) {
        eps_hk_aut_data_col.count += 1;

        if (eps_hk_aut_data_col.count >= eps_hk_aut_data_col.period) {
            eps_hk_aut_data_col.count = 0;
            enqueue_cmd(&collect_block_cmd, CMD_BLOCK_EPS_HK, 0);
        }
    }

    if (pay_hk_aut_data_col.enabled) {
        pay_hk_aut_data_col.count += 1;

        if (pay_hk_aut_data_col.count >= pay_hk_aut_data_col.period) {
            pay_hk_aut_data_col.count = 0;
            enqueue_cmd(&collect_block_cmd, CMD_BLOCK_PAY_HK, 0);
        }
    }

    if (pay_opt_aut_data_col.enabled) {
        pay_opt_aut_data_col.count += 1;

        if (pay_opt_aut_data_col.count >= pay_opt_aut_data_col.period) {
            pay_opt_aut_data_col.count = 0;
            enqueue_cmd(&collect_block_cmd, CMD_BLOCK_PAY_OPT, 0);
        }
    }
}


/*
Populates the block number, error, and current live date/time.
*/
void populate_header(mem_header_t* header, uint8_t block_num, uint8_t error) {
    header->block_num = block_num;
    header->error = error;
    if (sim_local_actions) {
        header->date.yy = 5;
        header->date.mm = 9;
        header->date.dd = 4;
        header->time.hh = 15;
        header->time.mm = 48;
        header->time.ss = 58;
    } else {
        header->date = read_rtc_date();
        header->time = read_rtc_time();
    }
}


// We know that the microcontroller uses 16 bit addresses, so store a function
// pointer in the first 2 bytes of the queue entry (data[0] = MSB, data[1] = LSB)
// TODO - develop a func test and a harness test for enqueueing and dequeueing cmd_t structs

/*
cmd - Command (with cmd->fn already set before calling this function) to enqueue
*/
void enqueue_cmd(cmd_t* cmd, uint32_t arg1, uint32_t arg2) {
    // Cast the cmd_t command pointer to a uint16
    uint16_t cmd_ptr = (uint16_t) cmd;

    // print("enqueue: fn_ptr = %x\n", fn_ptr);

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
    if (queue_empty(&cmd_queue)) {
        return;
    }
    if (queue_empty(&cmd_args_queue)) {
        return;
    }

    // Dequeue the command as an 8-byte array
    uint8_t cmd_data[8] = {0};
    uint8_t args_data[8] = {0};

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
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

    // print("dequeue: fn_ptr = %x\n", fn_ptr);

    // Cast the uint16 to a cmd_t command pointer
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Set the global current command to prevent other commands from running
        current_cmd = (cmd_t*) cmd_ptr;
        current_cmd_arg1 = arg1;
        current_cmd_arg2 = arg2;
    }
}
