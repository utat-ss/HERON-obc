#include "commands.h"

void nop_fn(void);
void ping_fn(void);
void get_subsys_status_fn(void);
void get_rtc_fn(void);
void set_rtc_fn(void);
void read_mem_bytes_fn(void);
void erase_mem_phy_sector_fn(void);
void col_block_fn(void);
void read_loc_block_fn(void);
void read_mem_block_fn(void);
void auto_data_col_enable_fn(void);
void auto_data_col_period_fn(void);
void auto_data_col_resync_fn(void);
void pay_act_motors_fn(void);
void reset_subsys_fn(void);
void eps_can_fn(void);
void pay_can_fn(void);
void read_eeprom_fn(void);
void get_cur_block_num_fn(void);
void set_cur_block_num_fn(void);
void set_mem_sec_start_addr_fn(void);
void set_mem_sec_end_addr_fn(void);
void erase_eeprom_fn(void);
void erase_all_mem_fn(void);
void erase_mem_phy_block_fn(void);

// If true, the program will simulate local actions (i.e. simulates any
// operations with peripherals besides the microcontroller and CAN)
// This allows the software to be used on any microcontroller just to test
// the command handling and CAN functionality
bool sim_local_actions = false;

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

// TODO - put command number in struct?
// All possible commands
// Default no-op command
cmd_t nop_cmd = {
    .fn = nop_fn
};
cmd_t ping_cmd = {
    .fn = ping_fn
};
cmd_t get_restart_uptime_cmd = {
    .fn = get_subsys_status_fn
};
cmd_t get_rtc_cmd = {
    .fn = get_rtc_fn
};
cmd_t set_rtc_cmd = {
    .fn = set_rtc_fn
};
cmd_t read_mem_bytes_cmd = {
    .fn = read_mem_bytes_fn
};
cmd_t erase_mem_sector_cmd = {
    .fn = erase_mem_phy_sector_fn
};
cmd_t col_block_cmd = {
    .fn = col_block_fn
};
cmd_t read_loc_block_cmd = {
    .fn = read_loc_block_fn
};
cmd_t read_mem_block_cmd = {
    .fn = read_mem_block_fn
};
cmd_t auto_data_col_enable_cmd = {
    .fn = auto_data_col_enable_fn
};
cmd_t auto_data_col_period_cmd = {
    .fn = auto_data_col_period_fn
};
cmd_t auto_data_col_resync_cmd = {
    .fn = auto_data_col_resync_fn
};
cmd_t pay_act_motors_cmd = {
    .fn = pay_act_motors_fn
};
cmd_t reset_subsys_cmd = {
    .fn = reset_subsys_fn
};
cmd_t eps_can_cmd = {
    .fn = eps_can_fn
};
cmd_t pay_can_cmd = {
    .fn = pay_can_fn
};
cmd_t read_eeprom_cmd = {
    .fn = read_eeprom_fn
};
cmd_t get_cur_block_num_cmd = {
    .fn = get_cur_block_num_fn
};
cmd_t set_cur_block_num_cmd = {
    .fn = set_cur_block_num_fn
};
cmd_t set_mem_sec_start_addr_cmd = {
    .fn = set_mem_sec_start_addr_fn
};
cmd_t set_mem_sec_end_addr_cmd = {
    .fn = set_mem_sec_end_addr_fn
};
cmd_t erase_eeprom_cmd = {
    .fn = erase_eeprom_fn
};
cmd_t erase_all_mem_cmd = {
    .fn = erase_all_mem_fn
};
cmd_t erase_mem_phy_block_cmd = {
    .fn = erase_mem_phy_block_fn
};



// Command callback functions

void nop_fn(void) {
    // Don't need to do anything here because execute_next_cmd() in general.c
    // will start a newly requested command if nop is the current command
}

void ping_fn(void) {
    can_countdown = 30;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }
    finish_current_cmd(true);
}

void get_subsys_status_fn(void) {
    can_countdown = 30;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        append_to_trans_tx_dec_msg((restart_count >> 24) & 0xFF);
        append_to_trans_tx_dec_msg((restart_count >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((restart_count >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(restart_count & 0xFF);
        append_to_trans_tx_dec_msg(restart_date.yy);
        append_to_trans_tx_dec_msg(restart_date.mm);
        append_to_trans_tx_dec_msg(restart_date.dd);
        append_to_trans_tx_dec_msg(restart_time.hh);
        append_to_trans_tx_dec_msg(restart_time.mm);
        append_to_trans_tx_dec_msg(restart_time.ss);
        append_to_trans_tx_dec_msg((uptime_s >> 24) & 0xFF);
        append_to_trans_tx_dec_msg((uptime_s >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((uptime_s >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(uptime_s & 0xFF);
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);

}

void get_rtc_fn(void) {
    can_countdown = 30;
    rtc_date_t date = read_rtc_date();
    rtc_time_t time = read_rtc_time();

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        append_to_trans_tx_dec_msg(date.yy);
        append_to_trans_tx_dec_msg(date.mm);
        append_to_trans_tx_dec_msg(date.dd);
        append_to_trans_tx_dec_msg(time.hh);
        append_to_trans_tx_dec_msg(time.mm);
        append_to_trans_tx_dec_msg(time.ss);
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void set_rtc_fn(void) {
    can_countdown = 30;
    rtc_date_t date = {
        .yy = (current_cmd_arg1 >> 16) & 0xFF,
        .mm = (current_cmd_arg1 >> 8) & 0xFF,
        .dd = current_cmd_arg1 & 0xFF
    };
    rtc_time_t time = {
        .hh = (current_cmd_arg2 >> 16) & 0xFF,
        .mm = (current_cmd_arg2 >> 8) & 0xFF,
        .ss = current_cmd_arg2 & 0xFF
    };

    set_rtc_date(date);
    set_rtc_time(time);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void read_mem_bytes_fn(void) {
    can_countdown = 30;

    // Enforce max number of bytes
    if (current_cmd_arg2 > CMD_READ_MEM_MAX_COUNT) {
        finish_current_cmd(false);
        return;
    }

    uint8_t data[CMD_READ_MEM_MAX_COUNT] = { 0x00 };
    read_mem_bytes(current_cmd_arg1, data, (uint8_t) current_cmd_arg2);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        for (uint8_t i = 0; i < (uint8_t) current_cmd_arg2; i++) {
            append_to_trans_tx_dec_msg(data[i]);
        }
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void erase_mem_phy_sector_fn(void) {
    can_countdown = 30;

    erase_mem_sector(current_cmd_arg1);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

// Starts requesting block data (field 0)
// TODO - set error byte to error by default at beginning, clear error after receiving last field of data
void col_block_fn(void) {
    can_countdown = 30;
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

    // Will continue from CAN callbacks
}

void read_loc_block_fn(void) {
    can_countdown = 30;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();

        switch (current_cmd_arg1) {
            case CMD_BLOCK_EPS_HK:
                append_header_to_tx_msg(&eps_hk_header);
                append_fields_to_tx_msg(eps_hk_fields, CAN_EPS_HK_FIELD_COUNT);
                break;
            case CMD_BLOCK_PAY_HK:
                append_header_to_tx_msg(&pay_hk_header);
                append_fields_to_tx_msg(pay_hk_fields, CAN_PAY_HK_FIELD_COUNT);
                break;
            case CMD_BLOCK_PAY_OPT:
                append_header_to_tx_msg(&pay_opt_header);
                append_fields_to_tx_msg(pay_opt_fields, CAN_PAY_OPT_FIELD_COUNT);
                break;
            default:
                finish_current_cmd(false);
                return;
        }

        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void read_mem_block_fn(void) {
    can_countdown = 30;
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
                read_mem_data_block(&eps_hk_mem_section, current_cmd_arg2,
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
                read_mem_data_block(&pay_hk_mem_section, current_cmd_arg2,
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
                read_mem_data_block(&pay_opt_mem_section, current_cmd_arg2,
                    &pay_opt_header, pay_opt_fields);
            }
            break;

        default:
            finish_current_cmd(false);
            return;
    }

    // TODO - will this give the correct behavaiour? maybe refactor both with common functionality?
    read_loc_block_fn();

    // TODO - finish_current_cmd(true)?
}

void auto_data_col_enable_fn(void) {
    can_countdown = 30;
    switch (current_cmd_arg1) {
        case CMD_BLOCK_EPS_HK:
            eps_hk_auto_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            break;
        case CMD_BLOCK_PAY_HK:
            pay_hk_auto_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            break;
        case CMD_BLOCK_PAY_OPT:
            pay_opt_auto_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            break;
        default:
            finish_current_cmd(false);
            return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void auto_data_col_period_fn(void) {
    can_countdown = 30;
    switch (current_cmd_arg1) {
        case CMD_BLOCK_EPS_HK:
            eps_hk_auto_data_col.period = current_cmd_arg2;
            break;
        case CMD_BLOCK_PAY_HK:
            pay_hk_auto_data_col.period = current_cmd_arg2;
            break;
        case CMD_BLOCK_PAY_OPT:
            pay_opt_auto_data_col.period = current_cmd_arg2;
            break;
        default:
            finish_current_cmd(false);
            return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void auto_data_col_resync_fn(void) {
    can_countdown = 30;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        eps_hk_auto_data_col.count = 0;
        pay_hk_auto_data_col.count = 0;
        pay_opt_auto_data_col.count = 0;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void pay_act_motors_fn(void) {
    can_countdown = 30;
    // TODO - temp low-power
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

    // Continues from CAN callbacks
}

void reset_subsys_fn(void) {
    can_countdown = 30;

    if (current_cmd_arg1 == CMD_SUBSYS_OBC) {
        reset_self_mcu(UPTIME_RESTART_REASON_RESET_CMD);
        // Program should stop here and restart from the beginning

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            start_trans_tx_dec_msg();
            finish_trans_tx_dec_msg();
        }
        finish_current_cmd(true);
    }
    // PAY/EPS will not respond so don't expect a CAN message back
    // Just finish the current command
    else if (current_cmd_arg1 == CMD_SUBSYS_EPS) {
        enqueue_eps_ctrl_tx_msg(CAN_EPS_CTRL_RESET, 0);
        finish_current_cmd(true);
    }
    else if (current_cmd_arg1 == CMD_SUBSYS_PAY) {
        enqueue_pay_ctrl_tx_msg(CAN_PAY_CTRL_RESET, 0);
        finish_current_cmd(true);
    }
    else {
        finish_current_cmd(false);
    }
}

void eps_can_fn(void) {
    can_countdown = 30;
    enqueue_eps_tx_msg(current_cmd_arg1, current_cmd_arg2);
    // Will continue from CAN callbacks
}

void pay_can_fn(void) {
    can_countdown = 30;
    enqueue_pay_tx_msg(current_cmd_arg1, current_cmd_arg2);
    // Will continue from CAN callbacks
}

void read_eeprom_fn(void) {
    can_countdown = 30;

    // Need to represent address as uint32_t* for EEPROM function
    // Must first cast to uint16_t or else we get warning: cast to pointer
    // from integer of different size -Wint-to-pointer-cast]
    uint32_t data = eeprom_read_dword(
        (uint32_t*) ((uint16_t) current_cmd_arg2));

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        append_to_trans_tx_dec_msg((data >> 24) & 0xFF);
        append_to_trans_tx_dec_msg((data >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((data >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(data & 0xFF);
        finish_trans_tx_dec_msg();
    }
    finish_current_cmd(true);
}

void get_cur_block_num_fn(void) {
    can_countdown = 30;

    uint32_t block_num = 0;
    switch (current_cmd_arg1) {
        case CMD_BLOCK_EPS_HK:
            block_num = eps_hk_mem_section.curr_block;
            break;
        case CMD_BLOCK_PAY_HK:
            block_num = pay_hk_mem_section.curr_block;
            break;
        case CMD_BLOCK_PAY_OPT:
            block_num = pay_opt_mem_section.curr_block;
            break;
        default:
            break;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        append_to_trans_tx_dec_msg((block_num >> 24) & 0xFF);
        append_to_trans_tx_dec_msg((block_num >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((block_num >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(block_num & 0xFF);
        finish_trans_tx_dec_msg();
    }
    
    finish_current_cmd(true);
}

void set_cur_block_num_fn(void) {
    can_countdown = 30;

    switch (current_cmd_arg1) {
        case CMD_BLOCK_EPS_HK:
            eps_hk_mem_section.curr_block = current_cmd_arg2;
            break;
        case CMD_BLOCK_PAY_HK:
            pay_hk_mem_section.curr_block = current_cmd_arg2;
            break;
        case CMD_BLOCK_PAY_OPT:
            pay_opt_mem_section.curr_block = current_cmd_arg2;
            break;
        default:
            break;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }
    
    finish_current_cmd(true);
}

void set_mem_sec_start_addr_fn(void) {
    can_countdown = 30;

    switch (current_cmd_arg1) {
        case CMD_BLOCK_EPS_HK:
            eps_hk_mem_section.start_addr = current_cmd_arg2;
            break;
        case CMD_BLOCK_PAY_HK:
            pay_hk_mem_section.start_addr = current_cmd_arg2;
            break;
        case CMD_BLOCK_PAY_OPT:
            pay_opt_mem_section.start_addr = current_cmd_arg2;
            break;
        default:
            break;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }
    
    finish_current_cmd(true);
}

void set_mem_sec_end_addr_fn(void) {
    can_countdown = 30;

    switch (current_cmd_arg1) {
        case CMD_BLOCK_EPS_HK:
            eps_hk_mem_section.end_addr = current_cmd_arg2;
            break;
        case CMD_BLOCK_PAY_HK:
            pay_hk_mem_section.end_addr = current_cmd_arg2;
            break;
        case CMD_BLOCK_PAY_OPT:
            pay_opt_mem_section.end_addr = current_cmd_arg2;
            break;
        default:
            break;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }
    
    finish_current_cmd(true);
}

void erase_eeprom_fn(void) {
    can_countdown = 30;

    // Need to represent address as uint32_t* for EEPROM function
    // Must first cast to uint16_t or else we get warning: cast to pointer
    // from integer of different size -Wint-to-pointer-cast]
    eeprom_update_dword(
        (uint32_t*) ((uint16_t) current_cmd_arg2), EEPROM_DEF_DWORD);
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void erase_all_mem_fn(void) {
    can_countdown = 30;

    erase_mem();

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void erase_mem_phy_block_fn(void) {
    can_countdown = 30;

    erase_mem_block(current_cmd_arg1);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}






// Finishes executing the current command and sets the succeeded flag
void finish_current_cmd(bool succeeded) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        current_cmd = &nop_cmd;
        current_cmd_arg1 = 0;
        current_cmd_arg2 = 0;
        prev_cmd_succeeded = succeeded;
        can_countdown = 0;
    }
    print("Finished cmd\n");
}





// Automatic data collection timer callback (for 16-bit timer)
void auto_data_col_timer_cb(void) {
    // print("Aut data col timer cb\n");

    if (eps_hk_auto_data_col.enabled) {
        eps_hk_auto_data_col.count += 1;

        if (eps_hk_auto_data_col.count >= eps_hk_auto_data_col.period) {
            print("Auto collecting EPS_HK\n");
            eps_hk_auto_data_col.count = 0;
            enqueue_cmd(&col_block_cmd, CMD_BLOCK_EPS_HK, 0);
        }
    }

    if (pay_hk_auto_data_col.enabled) {
        pay_hk_auto_data_col.count += 1;

        if (pay_hk_auto_data_col.count >= pay_hk_auto_data_col.period) {
            print("Auto collecting PAY_HK\n");
            pay_hk_auto_data_col.count = 0;
            enqueue_cmd(&col_block_cmd, CMD_BLOCK_PAY_HK, 0);
        }
    }

    if (pay_opt_auto_data_col.enabled) {
        pay_opt_auto_data_col.count += 1;

        if (pay_opt_auto_data_col.count >= pay_opt_auto_data_col.period) {
            print("Auto collecting PAY_OPT\n");
            pay_opt_auto_data_col.count = 0;
            enqueue_cmd(&col_block_cmd, CMD_BLOCK_PAY_OPT, 0);
        }
    }
}


/*
Populates the block number, error, and current live date/time.
*/
void populate_header(mem_header_t* header, uint32_t block_num, uint8_t error) {
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


void append_header_to_tx_msg(mem_header_t* header) {
    append_to_trans_tx_dec_msg((header->block_num >> 16) & 0xFF);
    append_to_trans_tx_dec_msg((header->block_num >> 8) & 0xFF);
    append_to_trans_tx_dec_msg(header->block_num & 0xFF);
    append_to_trans_tx_dec_msg(header->error);
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
    }

    print("dequeue_cmd: cmd = 0x%x, arg1 = %lu, arg2 = %lu\n", current_cmd,
        current_cmd_arg1, current_cmd_arg2);
}


cmd_t* trans_msg_type_to_cmd(uint8_t msg_type) {
    switch (msg_type) {
        case TRANS_CMD_PING:
            return &ping_cmd;
        case TRANS_CMD_GET_RESTART_UPTIME:
            return &get_restart_uptime_cmd;
        case TRANS_CMD_GET_RTC:
            return &get_rtc_cmd;
        case TRANS_CMD_SET_RTC:
            return &set_rtc_cmd;
        case TRANS_CMD_READ_MEM_BYTES:
            return &read_mem_bytes_cmd;
        case TRANS_CMD_ERASE_MEM_SECTOR:
            return &erase_mem_sector_cmd;
        case TRANS_CMD_COL_BLOCK:
            return &col_block_cmd;
        case TRANS_CMD_READ_LOC_BLOCK:
            return &read_loc_block_cmd;
        case TRANS_CMD_READ_MEM_BLOCK:
            return &read_mem_block_cmd;
        case TRANS_CMD_AUTO_DATA_COL_ENABLE:
            return &auto_data_col_enable_cmd;
        case TRANS_CMD_AUTO_DATA_COL_PERIOD:
            return &auto_data_col_period_cmd;
        case TRANS_CMD_AUTO_DATA_COL_RESYNC:
            return &auto_data_col_resync_cmd;
        case TRANS_CMD_PAY_ACT_MOTORS:
            return &pay_act_motors_cmd;
        case TRANS_CMD_RESET_SUBSYS:
            return &reset_subsys_cmd;
        case TRANS_CMD_EPS_CAN:
            return &eps_can_cmd;
        case TRANS_CMD_PAY_CAN:
            return &pay_can_cmd;
        case TRANS_CMD_READ_EEPROM:
            return &read_eeprom_cmd;
        case TRANS_CMD_GET_CUR_BLOCK_NUM:
            return &get_cur_block_num_cmd;
        case TRANS_CMD_SET_CUR_BLOCK_NUM:
            return &set_cur_block_num_cmd;
        case TRANS_CMD_SET_MEM_SEC_START_ADDR:
            return &set_mem_sec_start_addr_cmd;
        case TRANS_CMD_SET_MEM_SEC_END_ADDR:
            return &set_mem_sec_end_addr_cmd;
        case TRANS_CMD_ERASE_EEPROM:
            return &erase_eeprom_cmd;
        case TRANS_CMD_ERASE_ALL_MEM:
            return &erase_all_mem_cmd;
        case TRANS_CMD_ERASE_MEM_PHY_BLOCK:
            return &erase_mem_phy_block_cmd;
        default:
            // TODO - return NOP?
            return NULL;
    }
}

uint8_t trans_cmd_to_msg_type(cmd_t* cmd) {
    // Can't use switch/case structure for pointers
    if (cmd == &ping_cmd) {
        return TRANS_CMD_PING;
    } else if (cmd == &get_restart_uptime_cmd) {
        return TRANS_CMD_GET_RESTART_UPTIME;
    } else if (cmd == &get_rtc_cmd) {
        return TRANS_CMD_GET_RTC;
    } else if (cmd == &set_rtc_cmd) {
        return TRANS_CMD_SET_RTC;
    } else if (cmd == &read_mem_bytes_cmd) {
        return TRANS_CMD_READ_MEM_BYTES;
    } else if (cmd == &erase_mem_sector_cmd) {
        return TRANS_CMD_ERASE_MEM_SECTOR;
    } else if (cmd == &col_block_cmd) {
        return TRANS_CMD_COL_BLOCK;
    } else if (cmd == &read_loc_block_cmd) {
        return TRANS_CMD_READ_LOC_BLOCK;
    } else if (cmd == &read_mem_block_cmd) {
        return TRANS_CMD_READ_MEM_BLOCK;
    } else if (cmd == &auto_data_col_enable_cmd) {
        return TRANS_CMD_AUTO_DATA_COL_ENABLE;
    } else if (cmd == &auto_data_col_period_cmd) {
        return TRANS_CMD_AUTO_DATA_COL_PERIOD;
    } else if (cmd == &auto_data_col_resync_cmd) {
        return TRANS_CMD_AUTO_DATA_COL_RESYNC;
    } else if (cmd == &pay_act_motors_cmd) {
        return TRANS_CMD_PAY_ACT_MOTORS;
    } else if (cmd == &reset_subsys_cmd) {
        return TRANS_CMD_RESET_SUBSYS;
    } else if (cmd == &eps_can_cmd) {
        return TRANS_CMD_EPS_CAN;
    } else if (cmd == &pay_can_cmd) {
        return TRANS_CMD_PAY_CAN;
    } else if (cmd == &read_eeprom_cmd) {
        return TRANS_CMD_READ_EEPROM;
    } else if (cmd == &get_cur_block_num_cmd) {
        return TRANS_CMD_GET_CUR_BLOCK_NUM;
    } else if (cmd == &set_cur_block_num_cmd) {
        return TRANS_CMD_SET_CUR_BLOCK_NUM;
    } else if (cmd == &set_mem_sec_start_addr_cmd) {
        return TRANS_CMD_SET_MEM_SEC_START_ADDR;
    } else if (cmd == &set_mem_sec_end_addr_cmd) {
        return TRANS_CMD_SET_MEM_SEC_END_ADDR;
    } else if (cmd == &erase_eeprom_cmd) {
        return TRANS_CMD_ERASE_EEPROM;
    } else if (cmd == &erase_all_mem_cmd) {
        return TRANS_CMD_ERASE_ALL_MEM;
    } else if (cmd == &erase_mem_phy_block_cmd) {
        return TRANS_CMD_ERASE_MEM_PHY_BLOCK;
    } else {
        // TODO - use NOP?
        return 0xFF;
    }
}
