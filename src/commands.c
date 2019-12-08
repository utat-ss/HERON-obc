#include "commands.h"

// Uncomment for extra debugging prints
// #define COMMANDS_DEBUG


void nop_fn(void);

void ping_obc_fn(void);
void get_rtc_fn(void);
void set_rtc_fn(void);
void read_obc_eeprom_fn(void);
void erase_obc_eeprom_fn(void);
void read_obc_ram_byte_fn(void);
void set_beacon_inhibit_enable_fn(void);
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
void get_cur_block_nums_fn(void);
void set_cur_block_num_fn(void);
void get_mem_sec_addrs_fn(void);
void set_mem_sec_start_addr_fn(void);
void set_mem_sec_end_addr_fn(void);
void get_auto_data_col_settings_fn(void);
void set_auto_data_col_enable_fn(void);
void set_auto_data_col_period_fn(void);
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
cmd_t set_beacon_inhibit_enable_cmd = {
    .fn = set_beacon_inhibit_enable_fn,
    .opcode = CMD_SET_BEACON_INHIBIT_ENABLE,
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
cmd_t get_cur_block_nums_cmd = {
    .fn = get_cur_block_nums_fn,
    .opcode = CMD_GET_CUR_BLOCK_NUMS,
    .pwd_protected = false
};
cmd_t set_cur_block_num_cmd = {
    .fn = set_cur_block_num_fn,
    .opcode = CMD_SET_CUR_BLOCK_NUM,
    .pwd_protected = true
};
cmd_t get_mem_sec_addrs_cmd = {
    .fn = get_mem_sec_addrs_fn,
    .opcode = CMD_GET_MEM_SEC_ADDRS,
    .pwd_protected = true
};
cmd_t set_mem_sec_start_addr_cmd = {
    .fn = set_mem_sec_start_addr_fn,
    .opcode = CMD_SET_MEM_SEC_START_ADDR,
    .pwd_protected = true
};
cmd_t set_mem_sec_end_addr_cmd = {
    .fn = set_mem_sec_end_addr_fn,
    .opcode = CMD_SET_MEM_SEC_END_ADDR,
    .pwd_protected = true
};
cmd_t get_auto_data_col_settings_cmd = {
    .fn = get_auto_data_col_settings_fn,
    .opcode = CMD_GET_AUTO_DATA_COL_SETTINGS,
    .pwd_protected = false
};
cmd_t set_auto_data_col_enable_cmd = {
    .fn = set_auto_data_col_enable_fn,
    .opcode = CMD_SET_AUTO_DATA_COL_ENABLE,
    .pwd_protected = true
};
cmd_t set_auto_data_col_period_cmd = {
    .fn = set_auto_data_col_period_fn,
    .opcode = CMD_SET_AUTO_DATA_COL_PERIOD,
    .pwd_protected = true
};
cmd_t resync_auto_data_col_timers_cmd = {
    .fn = resync_auto_data_col_timers_fn,
    .opcode = CMD_RESYNC_AUTO_DATA_COL_TIMERS,
    .pwd_protected = true
};




// List of all command structs
// Should not include nop_cmd
cmd_t* all_cmds_list[] = {
    &ping_obc_cmd,
    &get_rtc_cmd,
    &set_rtc_cmd,
    &read_obc_eeprom_cmd,
    &erase_obc_eeprom_cmd,
    &read_obc_ram_byte_cmd,
    &set_beacon_inhibit_enable_cmd,
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
    &get_cur_block_nums_cmd,
    &set_cur_block_num_cmd,
    &get_mem_sec_addrs_cmd,
    &set_mem_sec_start_addr_cmd,
    &set_mem_sec_end_addr_cmd,
    &get_auto_data_col_settings_cmd,
    &set_auto_data_col_enable_cmd,
    &set_auto_data_col_period_cmd,
    &resync_auto_data_col_timers_cmd,
};

// Length of `all_cmds_list` array
const uint8_t all_cmds_list_len = sizeof(all_cmds_list) / sizeof(all_cmds_list[0]);


// Command callback functions

void nop_fn(void) {
    // Don't need to do anything here because execute_next_cmd() in general.c
    // will start a newly requested command if nop is the current command
}

void ping_obc_fn(void) {
    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void get_rtc_fn(void) {
    rtc_date_t date = read_rtc_date();
    rtc_time_t time = read_rtc_time();

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);
        append_to_trans_tx_resp(date.yy);
        append_to_trans_tx_resp(date.mm);
        append_to_trans_tx_resp(date.dd);
        append_to_trans_tx_resp(time.hh);
        append_to_trans_tx_resp(time.mm);
        append_to_trans_tx_resp(time.ss);
        finish_trans_tx_resp();
    }
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void set_rtc_fn(void) {
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

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void read_obc_eeprom_fn(void) {
    uint32_t data = read_eeprom((uint16_t) current_cmd_arg2);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);
        append_to_trans_tx_resp((data >> 24) & 0xFF);
        append_to_trans_tx_resp((data >> 16) & 0xFF);
        append_to_trans_tx_resp((data >> 8) & 0xFF);
        append_to_trans_tx_resp(data & 0xFF);
        finish_trans_tx_resp();
    }
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void erase_obc_eeprom_fn(void) {
    write_eeprom((uint16_t) current_cmd_arg2, EEPROM_DEF_DWORD);
    
    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void read_obc_ram_byte_fn(void) {
    // See lib-common/examples/read_registers for an MMIO example
    // https://arduino.stackexchange.com/questions/56304/how-do-i-directly-access-a-memory-mapped-register-of-avr-with-c
    // http://download.mikroe.com/documents/compilers/mikroc/avr/help/avr_memory_organization.htm

    // Need to represent address as volatile uint8_t* to read RAM
    // Must first cast to uint16_t or else we get warning: cast to pointer
    // from integer of different size -Wint-to-pointer-cast]
    volatile uint8_t* pointer = (volatile uint8_t*) ((uint16_t) current_cmd_arg1);
    uint8_t data = *pointer;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);
        append_to_trans_tx_resp(data);
        finish_trans_tx_resp();
    }
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void set_beacon_inhibit_enable_fn(void) {
    if (current_cmd_arg1 == 0) {
        // Stop inhibiting
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            turn_on_trans_beacon();
            beacon_inhibit_enabled = false;
            beacon_inhibit_count_s = 0;
        }
    } else if (current_cmd_arg1 == 1) {
        // Start inhibiting
        turn_off_trans_beacon();
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            beacon_inhibit_enabled = true;
            beacon_inhibit_count_s = 0;
        }
    } else {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
    }
    
    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void send_eps_can_msg_fn(void) {
    enqueue_tx_msg_bytes(&eps_tx_msg_queue, current_cmd_arg1, current_cmd_arg2);
    // Will continue from CAN callbacks
}

void send_pay_can_msg_fn(void) {
    enqueue_tx_msg_bytes(&pay_tx_msg_queue, current_cmd_arg1, current_cmd_arg2);
    // Will continue from CAN callbacks
}

void act_pay_motors_fn(void) {
    // Enqueue temporary low-power mode CAN commands
    // These will both be sent before the actuate motors CAN command
    enqueue_eps_tx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_START_TEMP_LPM, 0);
    enqueue_pay_tx_msg(CAN_PAY_CTRL, CAN_PAY_CTRL_START_TEMP_LPM, 0);

    // TODO - what if not a valid motor field number?
    enqueue_pay_tx_msg(CAN_PAY_CTRL, current_cmd_arg1, 0);

    // Continues from CAN callbacks
}

void reset_subsys_fn(void) {
    if (current_cmd_arg1 == CMD_OBC) {
        reset_self_mcu(UPTIME_RESTART_REASON_RESET_CMD);
        // Program should stop here and restart from the beginning

        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
        finish_current_cmd(CMD_RESP_STATUS_OK);
    }
    // PAY/EPS will not respond so don't expect a CAN message back
    // Just finish the current command
    else if (current_cmd_arg1 == CMD_EPS) {
        enqueue_eps_tx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_RESET, 0);

        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
        finish_current_cmd(CMD_RESP_STATUS_OK);
    }
    else if (current_cmd_arg1 == CMD_PAY) {
        enqueue_pay_tx_msg(CAN_PAY_CTRL, CAN_PAY_CTRL_RESET, 0);

        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
        finish_current_cmd(CMD_RESP_STATUS_OK);
    }
    else {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
    }
}

// TODO - should only send TX packet after receiving both responses
void set_indef_lpm_enable_fn(void) {
    enqueue_eps_tx_msg(CAN_EPS_CTRL, CAN_EPS_CTRL_ENABLE_INDEF_LPM, 0);
    enqueue_pay_tx_msg(CAN_PAY_CTRL, CAN_PAY_CTRL_ENABLE_INDEF_LPM, 0);

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void read_rec_status_info_fn(void) {    
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
        start_trans_tx_resp(CMD_RESP_STATUS_OK);

        for (uint8_t i = CAN_OBC_HK_UPTIME; i <= CAN_OBC_HK_RESTART_TIME; i++) {
            append_to_trans_tx_resp((obc_hk_fields[i] >> 16) & 0xFF);
            append_to_trans_tx_resp((obc_hk_fields[i] >> 8) & 0xFF);
            append_to_trans_tx_resp(obc_hk_fields[i] & 0xFF);
        }
        for (uint8_t i = CAN_EPS_HK_UPTIME; i <= CAN_EPS_HK_RESTART_REASON; i++) {
            append_to_trans_tx_resp((eps_hk_fields[i] >> 16) & 0xFF);
            append_to_trans_tx_resp((eps_hk_fields[i] >> 8) & 0xFF);
            append_to_trans_tx_resp(eps_hk_fields[i] & 0xFF);
        }
        for (uint8_t i = CAN_PAY_HK_UPTIME; i <= CAN_PAY_HK_RESTART_REASON; i++) {
            append_to_trans_tx_resp((pay_hk_fields[i] >> 16) & 0xFF);
            append_to_trans_tx_resp((pay_hk_fields[i] >> 8) & 0xFF);
            append_to_trans_tx_resp(pay_hk_fields[i] & 0xFF);
        }
        
        finish_trans_tx_resp();
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void read_data_block_fn(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);

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
                add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
                finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
                return;
        }

        finish_trans_tx_resp();
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void read_rec_loc_data_block_fn(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);

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
                add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
                finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
                return;
        }

        finish_trans_tx_resp();
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
}

// Common functionality for primary and secondary blocks
void read_cmd_blocks(mem_section_t* section) {
    if (current_cmd_arg2 > 5) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
        return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);

        for (uint32_t block_num = current_cmd_arg1;
            block_num < current_cmd_arg1 + current_cmd_arg2;
            block_num++) {
            
            mem_header_t header;
            uint16_t cmd_id = 0;
            uint8_t opcode = 0;
            uint32_t arg1 = 0;
            uint32_t arg2 = 0;
            read_mem_cmd_block(section, block_num,
                &header, &cmd_id, &opcode, &arg1, &arg2);

            append_header_to_tx_msg(&header);
            append_to_trans_tx_resp((cmd_id >> 8) & 0xFF);
            append_to_trans_tx_resp((cmd_id >> 0) & 0xFF);
            append_to_trans_tx_resp(opcode);
            append_to_trans_tx_resp((arg1 >> 24) & 0xFF);
            append_to_trans_tx_resp((arg1 >> 16) & 0xFF);
            append_to_trans_tx_resp((arg1 >> 8) & 0xFF);
            append_to_trans_tx_resp((arg1 >> 0) & 0xFF);
            append_to_trans_tx_resp((arg2 >> 24) & 0xFF);
            append_to_trans_tx_resp((arg2 >> 16) & 0xFF);
            append_to_trans_tx_resp((arg2 >> 8) & 0xFF);
            append_to_trans_tx_resp((arg2 >> 0) & 0xFF);
        }

        finish_trans_tx_resp();
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void read_prim_cmd_blocks_fn(void) {
    read_cmd_blocks(&prim_cmd_log_mem_section);
}

void read_sec_cmd_blocks_fn(void) {
    read_cmd_blocks(&sec_cmd_log_mem_section);
}

void read_raw_mem_bytes_fn(void) {
    // Enforce max number of bytes
    if (current_cmd_arg2 > CMD_READ_MEM_MAX_COUNT) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
        return;
    }

    uint8_t data[CMD_READ_MEM_MAX_COUNT] = { 0x00 };
    read_mem_bytes(current_cmd_arg1, data, (uint8_t) current_cmd_arg2);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);
        for (uint8_t i = 0; i < (uint8_t) current_cmd_arg2; i++) {
            append_to_trans_tx_resp(data[i]);
        }
        finish_trans_tx_resp();
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void erase_mem_phy_sector_fn(void) {
    erase_mem_sector(current_cmd_arg1);

    // Only send a transceiver packet if the erase was initiated by the ground
    // station
    if (current_cmd_id != CMD_CMD_ID_AUTO_ENQUEUED) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void erase_mem_phy_block_fn(void) {
    erase_mem_block(current_cmd_arg1);

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void erase_all_mem_fn(void) {
    erase_mem();

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

// Starts requesting block data (field 0)
void col_data_block_fn(void) {
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
#ifdef COMMANDS_DEBUG
            print("Start OBC_HK\n");
#endif

            populate_header(&obc_hk_header, obc_hk_mem_section.curr_block, CMD_RESP_STATUS_UNKNOWN);
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

            // Write header (except status)
            write_mem_header_main(&obc_hk_mem_section, obc_hk_mem_section.curr_block, &obc_hk_header);
            // Write data fields
            for (uint8_t i = 0; i < obc_hk_mem_section.fields_per_block; i++) {
                write_mem_field(&obc_hk_mem_section, obc_hk_mem_section.curr_block, i, obc_hk_fields[i]);
                // i is field number; fields[i] corresponds to associated field data
            }

            // Increment the block number
            inc_and_prepare_mem_section_curr_block(&obc_hk_mem_section);

            // Only send back a transceiver packet if the command was sent from
            // ground
            if (current_cmd_arg2 != CMD_CMD_ID_AUTO_ENQUEUED) {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                    start_trans_tx_resp(CMD_RESP_STATUS_OK);
                    append_to_trans_tx_resp((obc_hk_mem_section.curr_block >> 24) & 0xFF);
                    append_to_trans_tx_resp((obc_hk_mem_section.curr_block >> 16) & 0xFF);
                    append_to_trans_tx_resp((obc_hk_mem_section.curr_block >> 8) & 0xFF);
                    append_to_trans_tx_resp((obc_hk_mem_section.curr_block >> 0) & 0xFF);
                    finish_trans_tx_resp();
                }
            }
#ifdef COMMANDS_DEBUG
            print("Done OBC_HK\n");
#endif
            finish_current_cmd(CMD_RESP_STATUS_OK);
            
            // Don't use CAN
            return;

        case CMD_EPS_HK:
#ifdef COMMANDS_DEBUG
            print("Start EPS_HK\n");
#endif
            populate_header(&eps_hk_header, eps_hk_mem_section.curr_block, CMD_RESP_STATUS_UNKNOWN);
            write_mem_header_main(&eps_hk_mem_section, eps_hk_mem_section.curr_block, &eps_hk_header);
            // This increment invalidates the current block number for the
            // memory section struct for the current command, so the command
            // will need to fetch the block number from the header
            inc_and_prepare_mem_section_curr_block(&eps_hk_mem_section);
            enqueue_eps_tx_msg(CAN_EPS_HK, 0, 0);
            break;

        case CMD_PAY_HK:
#ifdef COMMANDS_DEBUG
            print ("Start PAY_HK\n");
#endif
            populate_header(&pay_hk_header, pay_hk_mem_section.curr_block, CMD_RESP_STATUS_UNKNOWN);
            write_mem_header_main(&pay_hk_mem_section, pay_hk_mem_section.curr_block, &pay_hk_header);
            inc_and_prepare_mem_section_curr_block(&pay_hk_mem_section);
            enqueue_pay_tx_msg(CAN_PAY_HK, 0, 0);
            break;

        case CMD_PAY_OPT:
#ifdef COMMANDS_DEBUG
            print ("Start PAY_OPT\n");
#endif
            populate_header(&pay_opt_header, pay_opt_mem_section.curr_block, CMD_RESP_STATUS_UNKNOWN);
            write_mem_header_main(&pay_opt_mem_section, pay_opt_mem_section.curr_block, &pay_opt_header);
            inc_and_prepare_mem_section_curr_block(&pay_opt_mem_section);
            enqueue_pay_tx_msg(CAN_PAY_OPT, 0, 0);
            break;

        default:
            add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
            finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
            return;
    }

    // Will continue from CAN callbacks
}

void get_cur_block_nums_fn(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);

        for (uint8_t i = 0; i < MEM_NUM_SECTIONS; i++) {
            append_to_trans_tx_resp((all_mem_sections[i]->curr_block >> 24) & 0xFF);
            append_to_trans_tx_resp((all_mem_sections[i]->curr_block >> 16) & 0xFF);
            append_to_trans_tx_resp((all_mem_sections[i]->curr_block >> 8) & 0xFF);
            append_to_trans_tx_resp((all_mem_sections[i]->curr_block >> 0) & 0xFF);
        }
        
        finish_trans_tx_resp();
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void set_cur_block_num_fn(void) {
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            prepare_mem_section_curr_block(&obc_hk_mem_section, current_cmd_arg2);
            break;
        case CMD_EPS_HK:
            prepare_mem_section_curr_block(&eps_hk_mem_section, current_cmd_arg2);
            break;
        case CMD_PAY_HK:
            prepare_mem_section_curr_block(&pay_hk_mem_section, current_cmd_arg2);
            break;
        case CMD_PAY_OPT:
            prepare_mem_section_curr_block(&pay_opt_mem_section, current_cmd_arg2);
            break;
        case CMD_PRIM_CMD_LOG:
            prepare_mem_section_curr_block(&prim_cmd_log_mem_section, current_cmd_arg2);
            break;
        case CMD_SEC_CMD_LOG:
            prepare_mem_section_curr_block(&sec_cmd_log_mem_section, current_cmd_arg2);
            break;
        default:
            add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
            finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
            return;
    }

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void get_mem_sec_addrs_fn(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);

        for (uint8_t i = 0; i < MEM_NUM_SECTIONS; i++) {
            append_to_trans_tx_resp((all_mem_sections[i]->start_addr >> 24) & 0xFF);
            append_to_trans_tx_resp((all_mem_sections[i]->start_addr >> 16) & 0xFF);
            append_to_trans_tx_resp((all_mem_sections[i]->start_addr >> 8) & 0xFF);
            append_to_trans_tx_resp((all_mem_sections[i]->start_addr >> 0) & 0xFF);
            append_to_trans_tx_resp((all_mem_sections[i]->end_addr >> 24) & 0xFF);
            append_to_trans_tx_resp((all_mem_sections[i]->end_addr >> 16) & 0xFF);
            append_to_trans_tx_resp((all_mem_sections[i]->end_addr >> 8) & 0xFF);
            append_to_trans_tx_resp((all_mem_sections[i]->end_addr >> 0) & 0xFF);
        }
        
        finish_trans_tx_resp();
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void set_mem_sec_start_addr_fn(void) {
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
            add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
            finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
            return;
    }

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void set_mem_sec_end_addr_fn(void) {
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
            add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
            finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
            return;
    }

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void get_auto_data_col_settings_fn(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);

        for (uint8_t i = 0; i < NUM_AUTO_DATA_COL_SECTIONS; i++) {
            append_to_trans_tx_resp((uint8_t) all_auto_data_cols[i]->enabled);
            append_to_trans_tx_resp((all_auto_data_cols[i]->period >> 24) & 0xFF);
            append_to_trans_tx_resp((all_auto_data_cols[i]->period >> 16) & 0xFF);
            append_to_trans_tx_resp((all_auto_data_cols[i]->period >> 8) & 0xFF);
            append_to_trans_tx_resp((all_auto_data_cols[i]->period >> 0) & 0xFF);
            append_to_trans_tx_resp((all_auto_data_cols[i]->count >> 24) & 0xFF);
            append_to_trans_tx_resp((all_auto_data_cols[i]->count >> 16) & 0xFF);
            append_to_trans_tx_resp((all_auto_data_cols[i]->count >> 8) & 0xFF);
            append_to_trans_tx_resp((all_auto_data_cols[i]->count >> 0) & 0xFF);
        }
        
        finish_trans_tx_resp();
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void set_auto_data_col_enable_fn(void) {
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            obc_hk_auto_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            write_eeprom(OBC_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR,  obc_hk_auto_data_col.enabled);
            break;
        case CMD_EPS_HK:
            eps_hk_auto_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            write_eeprom(EPS_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR,  eps_hk_auto_data_col.enabled);
            break;
        case CMD_PAY_HK:
            pay_hk_auto_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            write_eeprom(PAY_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR,  pay_hk_auto_data_col.enabled);
            break;
        case CMD_PAY_OPT:
            pay_opt_auto_data_col.enabled = current_cmd_arg2 ? 1 : 0;
            write_eeprom(PAY_OPT_AUTO_DATA_COL_ENABLED_EEPROM_ADDR, pay_opt_auto_data_col.enabled);
            break;
        default:
            add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
            finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
            return;
    }

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void set_auto_data_col_period_fn(void) {
    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            obc_hk_auto_data_col.period = current_cmd_arg2;
            write_eeprom(OBC_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR,  obc_hk_auto_data_col.period);
            break;
        case CMD_EPS_HK:
            eps_hk_auto_data_col.period = current_cmd_arg2;
            write_eeprom(EPS_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR,  eps_hk_auto_data_col.period);
            break;
        case CMD_PAY_HK:
            pay_hk_auto_data_col.period = current_cmd_arg2;
            write_eeprom(PAY_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR,  pay_hk_auto_data_col.period);
            break;
        case CMD_PAY_OPT:
            pay_opt_auto_data_col.period = current_cmd_arg2;
            write_eeprom(PAY_OPT_AUTO_DATA_COL_PERIOD_EEPROM_ADDR, pay_opt_auto_data_col.period);
            break;
        default:
            add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
            finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
            return;
    }

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void resync_auto_data_col_timers_fn(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        obc_hk_auto_data_col.count = 0;
        eps_hk_auto_data_col.count = 0;
        pay_hk_auto_data_col.count = 0;
        pay_opt_auto_data_col.count = 0;
    }

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}
