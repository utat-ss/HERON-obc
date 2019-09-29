#include "commands.h"


void nop_fn(void);

void ping_obc_fn(void);
void get_rtc_fn(void);
void set_rtc_fn(void);
void read_obc_eeprom_fn(void);
void erase_obc_eeprom_fn(void);
void read_obc_ram_byte_fn(void);
void send_eps_can_msg_fn(void);
void send_pay_can_msg_fn(void);
void act_pay_motors_fn(void);
void reset_subsys_fn(void);
void set_indef_lpm_enable_fn(void);

void read_rec_status_info_fn(void);
void read_data_block_fn(void);
void read_rec_loc_data_block_fn(void);
void read_prim_cmd_blocks_fn(void);
void read_sec_cmd_blocks_fn(void);
void read_raw_mem_bytes_fn(void);
void erase_mem_phy_sector_fn(void);
void erase_mem_phy_block_fn(void);
void erase_all_mem_fn(void);

void col_data_block_fn(void);
void get_cur_block_num_fn(void);
void set_cur_block_num_fn(void);
void get_mem_sec_start_addr_fn(void);
void set_mem_sec_start_addr_fn(void);
void get_mem_sec_end_addr_fn(void);
void set_mem_sec_end_addr_fn(void);
void get_auto_data_col_enable_fn(void);
void set_auto_data_col_enable_fn(void);
void get_auto_data_col_period_fn(void);
void set_auto_data_col_period_fn(void);
void get_auto_data_col_timers_fn(void);
void resync_auto_data_col_timers_fn(void);




// All possible commands

// Default no-op command
// Don't care about `num`
cmd_t nop_cmd = {
    .fn = nop_fn,
    .opcode = 0xFF,
    .pwd_protected = false
};


cmd_t ping_obc_cmd = {
    .fn = ping_obc_fn,
    .opcode = CMD_PING_OBC,
    .pwd_protected = false
};
cmd_t get_rtc_cmd = {
    .fn = get_rtc_fn,
    .opcode = CMD_GET_RTC,
    .pwd_protected = false
};
cmd_t set_rtc_cmd = {
    .fn = set_rtc_fn,
    .opcode = CMD_SET_RTC,
    .pwd_protected = true
};
cmd_t read_obc_eeprom_cmd = {
    .fn = read_obc_eeprom_fn,
    .opcode = CMD_READ_OBC_EEPROM,
    .pwd_protected = true
};
cmd_t erase_obc_eeprom_cmd = {
    .fn = erase_obc_eeprom_fn,
    .opcode = CMD_ERASE_OBC_EEPROM,
    .pwd_protected = true
};
cmd_t read_obc_ram_byte_cmd = {
    .fn = read_obc_ram_byte_fn,
    .opcode = CMD_READ_OBC_RAM_BYTE,
    .pwd_protected = true
};
cmd_t send_eps_can_msg_cmd = {
    .fn = send_eps_can_msg_fn,
    .opcode = CMD_SEND_EPS_CAN_MSG,
    .pwd_protected = true
};
cmd_t send_pay_can_msg_cmd = {
    .fn = send_pay_can_msg_fn,
    .opcode = CMD_SEND_PAY_CAN_MSG,
    .pwd_protected = true
};
cmd_t act_pay_motors_cmd = {
    .fn = act_pay_motors_fn,
    .opcode = CMD_ACT_PAY_MOTORS,
    .pwd_protected = true
};
cmd_t reset_subsys_cmd = {
    .fn = reset_subsys_fn,
    .opcode = CMD_RESET_SUBSYS,
    .pwd_protected = true
};
cmd_t set_indef_lpm_enable_cmd = {
    .fn = set_indef_lpm_enable_fn,
    .opcode = CMD_SET_INDEF_LPM_ENABLE,
    .pwd_protected = true
};


cmd_t read_rec_status_info_cmd = {
    .fn = read_rec_status_info_fn,
    .opcode = CMD_READ_REC_STATUS_INFO,
    .pwd_protected = false
};
cmd_t read_data_block_cmd = {
    .fn = read_data_block_fn,
    .opcode = CMD_READ_DATA_BLOCK,
    .pwd_protected = false
};
cmd_t read_rec_loc_data_block_cmd = {
    .fn = read_rec_loc_data_block_fn,
    .opcode = CMD_READ_REC_LOC_DATA_BLOCK,
    .pwd_protected = false
};
cmd_t read_prim_cmd_blocks_cmd = {
    .fn = read_prim_cmd_blocks_fn,
    .opcode = CMD_READ_PRIM_CMD_BLOCKS,
    .pwd_protected = false
};
cmd_t read_sec_cmd_blocks_cmd = {
    .fn = read_sec_cmd_blocks_fn,
    .opcode = CMD_READ_SEC_CMD_BLOCKS,
    .pwd_protected = false
};
cmd_t read_raw_mem_bytes_cmd = {
    .fn = read_raw_mem_bytes_fn,
    .opcode = CMD_READ_RAW_MEM_BYTES,
    .pwd_protected = true
};
cmd_t erase_mem_phy_sector_cmd = {
    .fn = erase_mem_phy_sector_fn,
    .opcode = CMD_ERASE_MEM_PHY_SECTOR,
    .pwd_protected = true
};
cmd_t erase_mem_phy_block_cmd = {
    .fn = erase_mem_phy_block_fn,
    .opcode = CMD_ERASE_MEM_PHY_BLOCK,
    .pwd_protected = true
};
cmd_t erase_all_mem_cmd = {
    .fn = erase_all_mem_fn,
    .opcode = CMD_ERASE_ALL_MEM,
    .pwd_protected = true
};


cmd_t col_data_block_cmd = {
    .fn = col_data_block_fn,
    .opcode = CMD_COL_DATA_BLOCK,
    .pwd_protected = false
};
cmd_t get_cur_block_num_cmd = {
    .fn = get_cur_block_num_fn,
    .opcode = CMD_GET_CUR_BLOCK_NUM,
    .pwd_protected = false
};
cmd_t set_cur_block_num_cmd = {
    .fn = set_cur_block_num_fn,
    .opcode = CMD_SET_CUR_BLOCK_NUM,
    .pwd_protected = true
};
cmd_t get_mem_sec_start_addr_cmd = {
    .fn = get_mem_sec_start_addr_fn,
    .opcode = CMD_GET_MEM_SEC_START_ADDR,
    .pwd_protected = true
};
cmd_t set_mem_sec_start_addr_cmd = {
    .fn = set_mem_sec_start_addr_fn,
    .opcode = CMD_SET_MEM_SEC_START_ADDR,
    .pwd_protected = true
};
cmd_t get_mem_sec_end_addr_cmd = {
    .fn = get_mem_sec_end_addr_fn,
    .opcode = CMD_GET_MEM_SEC_END_ADDR,
    .pwd_protected = true
};
cmd_t set_mem_sec_end_addr_cmd = {
    .fn = set_mem_sec_end_addr_fn,
    .opcode = CMD_SET_MEM_SEC_END_ADDR,
    .pwd_protected = true
};
cmd_t get_auto_data_col_enable_cmd = {
    .fn = get_auto_data_col_enable_fn,
    .opcode = CMD_GET_AUTO_DATA_COL_ENABLE,
    .pwd_protected = false
};
cmd_t set_auto_data_col_enable_cmd = {
    .fn = set_auto_data_col_enable_fn,
    .opcode = CMD_SET_AUTO_DATA_COL_ENABLE,
    .pwd_protected = true
};
cmd_t get_auto_data_col_period_cmd = {
    .fn = get_auto_data_col_period_fn,
    .opcode = CMD_GET_AUTO_DATA_COL_PERIOD,
    .pwd_protected = false
};
cmd_t set_auto_data_col_period_cmd = {
    .fn = set_auto_data_col_period_fn,
    .opcode = CMD_SET_AUTO_DATA_COL_PERIOD,
    .pwd_protected = true
};
cmd_t get_auto_data_col_timers_cmd = {
    .fn = get_auto_data_col_timers_fn,
    .opcode = CMD_GET_AUTO_DATA_COL_TIMERS,
    .pwd_protected = false
};
cmd_t resync_auto_data_col_timers_cmd = {
    .fn = resync_auto_data_col_timers_fn,
    .opcode = CMD_RESYNC_AUTO_DATA_COL_TIMERS,
    .pwd_protected = true
};




// Should not include nop_cmd
// Explicitly specify the length because using sizeof(all_cmds_list) gives an error
// "invalid application of 'sizeof' to incomplete type 'cmd_t *[]' {aka 'struct
// <anonymous> *[]'}"
// If the ALL_CMDS_LEN is too small, gives warning "excess elements in array initializer"
// If the ALL_CMDS_LEN is too big, no warnings
// NOTE: MAKE SURE TO UPDATE ALL_CMDS_LEN WHEN ADDING/DELETING
// TODO - refactor to use sizeof
cmd_t* all_cmds_list[ALL_CMDS_LEN] = {
    &ping_obc_cmd,
    &get_rtc_cmd,
    &set_rtc_cmd,
    &read_obc_eeprom_cmd,
    &erase_obc_eeprom_cmd,
    &read_obc_ram_byte_cmd,
    &send_eps_can_msg_cmd,
    &send_pay_can_msg_cmd,
    &act_pay_motors_cmd,
    &reset_subsys_cmd,
    &set_indef_lpm_enable_cmd,
    &read_rec_status_info_cmd,
    &read_data_block_cmd,
    &read_rec_loc_data_block_cmd,
    &read_prim_cmd_blocks_cmd,
    &read_sec_cmd_blocks_cmd,
    &read_raw_mem_bytes_cmd,
    &erase_mem_phy_sector_cmd,
    &erase_mem_phy_block_cmd,
    &erase_all_mem_cmd,
    &col_data_block_cmd,
    &get_cur_block_num_cmd,
    &set_cur_block_num_cmd,
    &get_mem_sec_start_addr_cmd,
    &set_mem_sec_start_addr_cmd,
    &get_mem_sec_end_addr_cmd,
    &set_mem_sec_end_addr_cmd,
    &get_auto_data_col_enable_cmd,
    &set_auto_data_col_enable_cmd,
    &get_auto_data_col_period_cmd,
    &set_auto_data_col_period_cmd,
    &get_auto_data_col_timers_cmd,
    &resync_auto_data_col_timers_cmd,
};


// Command callback functions

void nop_fn(void) {
    // Don't need to do anything here because execute_next_cmd() in general.c
    // will start a newly requested command if nop is the current command
}

void ping_obc_fn(void) {
    can_countdown = 30;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
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

void read_obc_eeprom_fn(void) {
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

void erase_obc_eeprom_fn(void) {
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

void read_obc_ram_byte_fn(void) {
    can_countdown = 30;

    // Need to represent address as uint8_t* to read RAM
    // Must first cast to uint16_t or else we get warning: cast to pointer
    // from integer of different size -Wint-to-pointer-cast]
    uint8_t* pointer = (uint8_t*) ((uint16_t) current_cmd_arg2);
    uint8_t data = *pointer;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        append_to_trans_tx_dec_msg(data);
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void send_eps_can_msg_fn(void) {
    can_countdown = 30;
    enqueue_eps_tx_msg(current_cmd_arg1, current_cmd_arg2);
    // Will continue from CAN callbacks
}

void send_pay_can_msg_fn(void) {
    can_countdown = 30;
    enqueue_pay_tx_msg(current_cmd_arg1, current_cmd_arg2);
    // Will continue from CAN callbacks
}

void act_pay_motors_fn(void) {
    can_countdown = 30;

    // Enqueue temporary low-power mode CAN commands
    // These will both be sent before the actuate motors CAN command
    enqueue_eps_ctrl_tx_msg(CAN_EPS_CTRL_START_TEMP_LPM, 0);
    enqueue_pay_ctrl_tx_msg(CAN_PAY_CTRL_START_TEMP_LPM, 0);

    // TODO - what if not a valid motor field number?
    enqueue_pay_ctrl_tx_msg(current_cmd_arg1, 0);

    // Continues from CAN callbacks
}

void reset_subsys_fn(void) {
    can_countdown = 30;

    if (current_cmd_arg1 == CMD_OBC) {
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
    else if (current_cmd_arg1 == CMD_EPS) {
        enqueue_eps_ctrl_tx_msg(CAN_EPS_CTRL_RESET, 0);

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            start_trans_tx_dec_msg();
            finish_trans_tx_dec_msg();
        }
        finish_current_cmd(true);
    }
    else if (current_cmd_arg1 == CMD_PAY) {
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

// TODO - should only send TX packet after receiving both responses
void set_indef_lpm_enable_fn(void) {
    can_countdown = 30;

    enqueue_eps_ctrl_tx_msg(CAN_EPS_CTRL_ENABLE_INDEF_LPM, 0);
    enqueue_pay_ctrl_tx_msg(CAN_PAY_CTRL_ENABLE_INDEF_LPM, 0);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void read_rec_status_info_fn(void) {
    can_countdown = 30;
    
    uint32_t obc_block = obc_hk_mem_section.curr_block;
    obc_block = (obc_block > 0) ? (obc_block - 1) : obc_block;
    uint32_t eps_block = eps_hk_mem_section.curr_block;
    eps_block = (eps_block > 0) ? (eps_block - 1) : eps_block;
    uint32_t pay_block = pay_hk_mem_section.curr_block;
    pay_block = (pay_block > 0) ? (pay_block - 1) : pay_block;

    read_mem_data_block(&obc_hk_mem_section, obc_block, &obc_hk_header, obc_hk_fields);
    read_mem_data_block(&eps_hk_mem_section, eps_block, &eps_hk_header, eps_hk_fields);
    read_mem_data_block(&pay_hk_mem_section, pay_block, &pay_hk_header, pay_hk_fields);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();

        for (uint8_t i = CAN_OBC_HK_UPTIME; i <= CAN_OBC_HK_RESTART_TIME; i++) {
            append_to_trans_tx_dec_msg((obc_hk_fields[i] >> 16) & 0xFF);
            append_to_trans_tx_dec_msg((obc_hk_fields[i] >> 8) & 0xFF);
            append_to_trans_tx_dec_msg(obc_hk_fields[i] & 0xFF);
        }
        for (uint8_t i = CAN_EPS_HK_UPTIME; i <= CAN_EPS_HK_RESTART_REASON; i++) {
            append_to_trans_tx_dec_msg((eps_hk_fields[i] >> 16) & 0xFF);
            append_to_trans_tx_dec_msg((eps_hk_fields[i] >> 8) & 0xFF);
            append_to_trans_tx_dec_msg(eps_hk_fields[i] & 0xFF);
        }
        for (uint8_t i = CAN_PAY_HK_UPTIME; i <= CAN_PAY_HK_RESTART_REASON; i++) {
            append_to_trans_tx_dec_msg((pay_hk_fields[i] >> 16) & 0xFF);
            append_to_trans_tx_dec_msg((pay_hk_fields[i] >> 8) & 0xFF);
            append_to_trans_tx_dec_msg(pay_hk_fields[i] & 0xFF);
        }
        
        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void read_data_block_fn(void) {
    can_countdown = 30;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();

        switch (current_cmd_arg1) {
            case CMD_OBC_HK:
                read_mem_data_block(&obc_hk_mem_section, current_cmd_arg2,
                    &obc_hk_header, obc_hk_fields);
                append_header_to_tx_msg(&obc_hk_header);
                append_fields_to_tx_msg(obc_hk_fields, CAN_OBC_HK_FIELD_COUNT);
                break;

            case CMD_EPS_HK:
                read_mem_data_block(&eps_hk_mem_section, current_cmd_arg2,
                    &eps_hk_header, eps_hk_fields);
                append_header_to_tx_msg(&eps_hk_header);
                append_fields_to_tx_msg(eps_hk_fields, CAN_EPS_HK_FIELD_COUNT);
                break;

            case CMD_PAY_HK:
                read_mem_data_block(&pay_hk_mem_section, current_cmd_arg2,
                    &pay_hk_header, pay_hk_fields);
                append_header_to_tx_msg(&pay_hk_header);
                append_fields_to_tx_msg(pay_hk_fields, CAN_PAY_HK_FIELD_COUNT);
                break;

            case CMD_PAY_OPT:
                read_mem_data_block(&pay_opt_mem_section, current_cmd_arg2,
                    &pay_opt_header, pay_opt_fields);
                append_header_to_tx_msg(&pay_opt_header);
                append_fields_to_tx_msg(pay_opt_fields, CAN_PAY_OPT_FIELD_COUNT);
                break;

            default:
                finish_current_cmd(false);
                // TODO - will it properly terminate the trans msg?
                return;
        }

        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void read_rec_loc_data_block_fn(void) {
    can_countdown = 30;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();

        switch (current_cmd_arg1) {
            case CMD_OBC_HK:
                append_header_to_tx_msg(&obc_hk_header);
                append_fields_to_tx_msg(obc_hk_fields, CAN_OBC_HK_FIELD_COUNT);
                break;
            case CMD_EPS_HK:
                append_header_to_tx_msg(&eps_hk_header);
                append_fields_to_tx_msg(eps_hk_fields, CAN_EPS_HK_FIELD_COUNT);
                break;
            case CMD_PAY_HK:
                append_header_to_tx_msg(&pay_hk_header);
                append_fields_to_tx_msg(pay_hk_fields, CAN_PAY_HK_FIELD_COUNT);
                break;
            case CMD_PAY_OPT:
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

void read_prim_cmd_blocks_fn(void) {
    can_countdown = 30;

    if (current_cmd_arg2 > 5) {
        finish_current_cmd(false);
        return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();

        for (uint32_t block_num = current_cmd_arg1;
            block_num < current_cmd_arg1 + current_cmd_arg2;
            block_num++) {
            
            mem_header_t header;
            uint8_t opcode = 0;
            uint32_t arg1 = 0;
            uint32_t arg2 = 0;
            read_mem_cmd_block(&prim_cmd_log_mem_section, block_num,
                &header, &opcode, &arg1, &arg2);

            append_header_to_tx_msg(&header);
            append_to_trans_tx_dec_msg(opcode);
            append_to_trans_tx_dec_msg((arg1 >> 24) & 0xFF);
            append_to_trans_tx_dec_msg((arg1 >> 16) & 0xFF);
            append_to_trans_tx_dec_msg((arg1 >> 8) & 0xFF);
            append_to_trans_tx_dec_msg((arg1 >> 0) & 0xFF);
            append_to_trans_tx_dec_msg((arg2 >> 24) & 0xFF);
            append_to_trans_tx_dec_msg((arg2 >> 16) & 0xFF);
            append_to_trans_tx_dec_msg((arg2 >> 8) & 0xFF);
            append_to_trans_tx_dec_msg((arg2 >> 0) & 0xFF);
        }

        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void read_sec_cmd_blocks_fn(void) {
    // TODO - refactor common with primary command?
    can_countdown = 30;

    if (current_cmd_arg2 > 5) {
        finish_current_cmd(false);
        return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();

        for (uint32_t block_num = current_cmd_arg1;
            block_num < current_cmd_arg1 + current_cmd_arg2;
            block_num++) {
            
            mem_header_t header;
            uint8_t opcode = 0;
            uint32_t arg1 = 0;
            uint32_t arg2 = 0;
            read_mem_cmd_block(&sec_cmd_log_mem_section, block_num,
                &header, &opcode, &arg1, &arg2);

            append_header_to_tx_msg(&header);
            append_to_trans_tx_dec_msg(opcode);
            append_to_trans_tx_dec_msg((arg1 >> 24) & 0xFF);
            append_to_trans_tx_dec_msg((arg1 >> 16) & 0xFF);
            append_to_trans_tx_dec_msg((arg1 >> 8) & 0xFF);
            append_to_trans_tx_dec_msg((arg1 >> 0) & 0xFF);
            append_to_trans_tx_dec_msg((arg2 >> 24) & 0xFF);
            append_to_trans_tx_dec_msg((arg2 >> 16) & 0xFF);
            append_to_trans_tx_dec_msg((arg2 >> 8) & 0xFF);
            append_to_trans_tx_dec_msg((arg2 >> 0) & 0xFF);
        }

        finish_trans_tx_dec_msg();
    }

    finish_current_cmd(true);
}

void read_raw_mem_bytes_fn(void) {
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

void erase_mem_phy_block_fn(void) {
    can_countdown = 30;

    erase_mem_block(current_cmd_arg1);

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

// Starts requesting block data (field 0)
// TODO - don't write success byte later, write at end in CAN RX processing
void col_data_block_fn(void) {
    can_countdown = 30;
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            print("Start OBC_HK\n");

            populate_header(&obc_hk_header, obc_hk_mem_section.curr_block, 0x00);
            obc_hk_fields[CAN_OBC_HK_UPTIME] = uptime_s;
            obc_hk_fields[CAN_OBC_HK_RESTART_COUNT] = restart_count;
            obc_hk_fields[CAN_OBC_HK_RESTART_REASON] = restart_reason;
            obc_hk_fields[CAN_OBC_HK_RESTART_DATE] =
                ((uint32_t) restart_date.yy << 16) |
                ((uint32_t) restart_date.mm << 8) |
                ((uint32_t) restart_date.dd << 0);
            obc_hk_fields[CAN_OBC_HK_RESTART_TIME] =
                ((uint32_t) restart_time.hh << 16) |
                ((uint32_t) restart_time.mm << 8) |
                ((uint32_t) restart_time.ss << 0);

            // Write data to the section and increment the block number
            write_mem_data_block(&obc_hk_mem_section, obc_hk_mem_section.curr_block,
                &obc_hk_header, obc_hk_fields);
            inc_and_prepare_mem_section_curr_block(&obc_hk_mem_section);

            // Only send back a transceiver packet if the command was sent from
            // ground (arg2 = 0)
            if (current_cmd_arg2 == 0) {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                    start_trans_tx_dec_msg();
                    append_to_trans_tx_dec_msg((obc_hk_mem_section.curr_block >> 24) & 0xFF);
                    append_to_trans_tx_dec_msg((obc_hk_mem_section.curr_block >> 16) & 0xFF);
                    append_to_trans_tx_dec_msg((obc_hk_mem_section.curr_block >> 8) & 0xFF);
                    append_to_trans_tx_dec_msg((obc_hk_mem_section.curr_block >> 0) & 0xFF);
                    finish_trans_tx_dec_msg();
                }
            }

            print("Done OBC_HK\n");
            finish_current_cmd(true);
            
            // Don't use CAN
            return;

        case CMD_EPS_HK:
            print("Start EPS_HK\n");
            populate_header(&eps_hk_header, eps_hk_mem_section.curr_block, 0x00);
            enqueue_eps_hk_tx_msg(0);
            break;

        case CMD_PAY_HK:
            print ("Start PAY_HK\n");
            populate_header(&pay_hk_header, pay_hk_mem_section.curr_block, 0x00);
            enqueue_pay_hk_tx_msg(0);
            break;

        case CMD_PAY_OPT:
            print ("Start PAY_OPT\n");
            populate_header(&pay_opt_header, pay_opt_mem_section.curr_block, 0x00);
            enqueue_pay_opt_tx_msg(0);
            break;

        default:
            finish_current_cmd(false);
            return;
    }

    // Will continue from CAN callbacks
}

void get_cur_block_num_fn(void) {
    can_countdown = 30;

    uint32_t block_num = 0;
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            block_num = obc_hk_mem_section.curr_block;
            break;
        case CMD_EPS_HK:
            block_num = eps_hk_mem_section.curr_block;
            break;
        case CMD_PAY_HK:
            block_num = pay_hk_mem_section.curr_block;
            break;
        case CMD_PAY_OPT:
            block_num = pay_opt_mem_section.curr_block;
            break;
        case CMD_PRIM_CMD_LOG:
            block_num = prim_cmd_log_mem_section.curr_block;
            break;
        case CMD_SEC_CMD_LOG:
            block_num = sec_cmd_log_mem_section.curr_block;
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

// TODO - OBC needs to erase memory sectors automatically when resetting current
// block numbers on its own or writing to a previously written address
void set_cur_block_num_fn(void) {
    can_countdown = 30;

    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            obc_hk_mem_section.curr_block = current_cmd_arg2;
            break;
        case CMD_EPS_HK:
            eps_hk_mem_section.curr_block = current_cmd_arg2;
            break;
        case CMD_PAY_HK:
            pay_hk_mem_section.curr_block = current_cmd_arg2;
            break;
        case CMD_PAY_OPT:
            pay_opt_mem_section.curr_block = current_cmd_arg2;
            break;
        case CMD_PRIM_CMD_LOG:
            prim_cmd_log_mem_section.curr_block = current_cmd_arg2;
            break;
        case CMD_SEC_CMD_LOG:
            sec_cmd_log_mem_section.curr_block = current_cmd_arg2;
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

void get_mem_sec_start_addr_fn(void) {
    can_countdown = 30;

    uint32_t start_addr = 0;
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            start_addr = obc_hk_mem_section.start_addr;
            break;
        case CMD_EPS_HK:
            start_addr = eps_hk_mem_section.start_addr;
            break;
        case CMD_PAY_HK:
            start_addr = pay_hk_mem_section.start_addr;
            break;
        case CMD_PAY_OPT:
            start_addr = pay_opt_mem_section.start_addr;
            break;
        case CMD_PRIM_CMD_LOG:
            start_addr = prim_cmd_log_mem_section.start_addr;
            break;
        case CMD_SEC_CMD_LOG:
            start_addr = sec_cmd_log_mem_section.start_addr;
            break;
        default:
            break;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        append_to_trans_tx_dec_msg((start_addr >> 24) & 0xFF);
        append_to_trans_tx_dec_msg((start_addr >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((start_addr >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(start_addr & 0xFF);
        finish_trans_tx_dec_msg();
    }
    
    finish_current_cmd(true);
}

void set_mem_sec_start_addr_fn(void) {
    can_countdown = 30;

    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            obc_hk_mem_section.start_addr = current_cmd_arg2;
            break;
        case CMD_EPS_HK:
            eps_hk_mem_section.start_addr = current_cmd_arg2;
            break;
        case CMD_PAY_HK:
            pay_hk_mem_section.start_addr = current_cmd_arg2;
            break;
        case CMD_PAY_OPT:
            pay_opt_mem_section.start_addr = current_cmd_arg2;
            break;
        case CMD_PRIM_CMD_LOG:
            prim_cmd_log_mem_section.start_addr = current_cmd_arg2;
            break;
        case CMD_SEC_CMD_LOG:
            sec_cmd_log_mem_section.start_addr = current_cmd_arg2;
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

void get_mem_sec_end_addr_fn(void) {
    can_countdown = 30;

    uint32_t end_addr = 0;
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            end_addr = obc_hk_mem_section.end_addr;
            break;
        case CMD_EPS_HK:
            end_addr = eps_hk_mem_section.end_addr;
            break;
        case CMD_PAY_HK:
            end_addr = pay_hk_mem_section.end_addr;
            break;
        case CMD_PAY_OPT:
            end_addr = pay_opt_mem_section.end_addr;
            break;
        case CMD_PRIM_CMD_LOG:
            end_addr = prim_cmd_log_mem_section.end_addr;
            break;
        case CMD_SEC_CMD_LOG:
            end_addr = sec_cmd_log_mem_section.end_addr;
            break;
        default:
            break;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        append_to_trans_tx_dec_msg((end_addr >> 24) & 0xFF);
        append_to_trans_tx_dec_msg((end_addr >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((end_addr >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(end_addr & 0xFF);
        finish_trans_tx_dec_msg();
    }
    
    finish_current_cmd(true);
}

void set_mem_sec_end_addr_fn(void) {
    can_countdown = 30;

    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            obc_hk_mem_section.end_addr = current_cmd_arg2;
            break;
        case CMD_EPS_HK:
            eps_hk_mem_section.end_addr = current_cmd_arg2;
            break;
        case CMD_PAY_HK:
            pay_hk_mem_section.end_addr = current_cmd_arg2;
            break;
        case CMD_PAY_OPT:
            pay_opt_mem_section.end_addr = current_cmd_arg2;
            break;
        case CMD_PRIM_CMD_LOG:
            prim_cmd_log_mem_section.end_addr = current_cmd_arg2;
            break;
        case CMD_SEC_CMD_LOG:
            sec_cmd_log_mem_section.end_addr = current_cmd_arg2;
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

void get_auto_data_col_enable_fn(void) {
    can_countdown = 30;

    bool enabled = false;
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            enabled = obc_hk_auto_data_col.enabled;
            break;
        case CMD_EPS_HK:
            enabled = eps_hk_auto_data_col.enabled;
            break;
        case CMD_PAY_HK:
            enabled = pay_hk_auto_data_col.enabled;
            break;
        case CMD_PAY_OPT:
            enabled = pay_opt_auto_data_col.enabled;
            break;
        default:
            break;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        append_to_trans_tx_dec_msg((uint8_t) enabled);
        finish_trans_tx_dec_msg();
    }
    
    finish_current_cmd(true);
}

void set_auto_data_col_enable_fn(void) {
    can_countdown = 30;
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            obc_hk_auto_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            eeprom_update_dword(OBC_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR,  obc_hk_auto_data_col.enabled);
            break;
        case CMD_EPS_HK:
            eps_hk_auto_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            eeprom_update_dword(EPS_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR,  eps_hk_auto_data_col.enabled);
            break;
        case CMD_PAY_HK:
            pay_hk_auto_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            eeprom_update_dword(PAY_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR,  pay_hk_auto_data_col.enabled);
            break;
        case CMD_PAY_OPT:
            pay_opt_auto_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            eeprom_update_dword(PAY_OPT_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, pay_opt_auto_data_col.enabled);
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

void get_auto_data_col_period_fn(void) {
    can_countdown = 30;

    uint32_t period = 0;
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            period = obc_hk_auto_data_col.period;
            break;
        case CMD_EPS_HK:
            period = eps_hk_auto_data_col.period;
            break;
        case CMD_PAY_HK:
            period = pay_hk_auto_data_col.period;
            break;
        case CMD_PAY_OPT:
            period = pay_opt_auto_data_col.period;
            break;
        default:
            break;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        append_to_trans_tx_dec_msg((period >> 24) & 0xFF);
        append_to_trans_tx_dec_msg((period >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((period >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(period & 0xFF);
        finish_trans_tx_dec_msg();
    }
    
    finish_current_cmd(true);
}

void set_auto_data_col_period_fn(void) {
    can_countdown = 30;
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            obc_hk_auto_data_col.period = current_cmd_arg2;
            eeprom_update_dword(OBC_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR,  obc_hk_auto_data_col.period);
            break;
        case CMD_EPS_HK:
            eps_hk_auto_data_col.period = current_cmd_arg2;
            eeprom_update_dword(EPS_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR,  eps_hk_auto_data_col.period);
            break;
        case CMD_PAY_HK:
            pay_hk_auto_data_col.period = current_cmd_arg2;
            eeprom_update_dword(PAY_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR,  pay_hk_auto_data_col.period);
            break;
        case CMD_PAY_OPT:
            pay_opt_auto_data_col.period = current_cmd_arg2;
            eeprom_update_dword(PAY_OPT_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, pay_opt_auto_data_col.period);
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

void get_auto_data_col_timers_fn(void) {
    can_countdown = 30;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_dec_msg();
        append_to_trans_tx_dec_msg((obc_hk_auto_data_col.count >> 24) & 0xFF);
        append_to_trans_tx_dec_msg((obc_hk_auto_data_col.count >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((obc_hk_auto_data_col.count >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(obc_hk_auto_data_col.count & 0xFF);
        append_to_trans_tx_dec_msg((eps_hk_auto_data_col.count >> 24) & 0xFF);
        append_to_trans_tx_dec_msg((eps_hk_auto_data_col.count >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((eps_hk_auto_data_col.count >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(eps_hk_auto_data_col.count & 0xFF);
        append_to_trans_tx_dec_msg((pay_hk_auto_data_col.count >> 24) & 0xFF);
        append_to_trans_tx_dec_msg((pay_hk_auto_data_col.count >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((pay_hk_auto_data_col.count >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(pay_hk_auto_data_col.count & 0xFF);
        append_to_trans_tx_dec_msg((pay_opt_auto_data_col.count >> 24) & 0xFF);
        append_to_trans_tx_dec_msg((pay_opt_auto_data_col.count >> 16) & 0xFF);
        append_to_trans_tx_dec_msg((pay_opt_auto_data_col.count >> 8) & 0xFF);
        append_to_trans_tx_dec_msg(pay_opt_auto_data_col.count & 0xFF);
        finish_trans_tx_dec_msg();
    }
    
    finish_current_cmd(true);
}

void resync_auto_data_col_timers_fn(void) {
    can_countdown = 30;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        obc_hk_auto_data_col.count = 0;
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
