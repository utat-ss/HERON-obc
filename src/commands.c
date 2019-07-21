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

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            start_trans_tx_dec_msg();
            finish_trans_tx_dec_msg();
        }
        finish_current_cmd(true);
    }
    else if (current_cmd_arg1 == CMD_SUBSYS_PAY) {
        enqueue_pay_ctrl_tx_msg(CAN_PAY_CTRL_RESET, 0);

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            start_trans_tx_dec_msg();
            finish_trans_tx_dec_msg();
        }
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
