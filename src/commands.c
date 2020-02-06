#include "commands.h"

// Uncomment for extra debugging prints
// #define COMMANDS_DEBUG
// #define COMMANDS_VERBOSE


void nop_fn(void);

void ping_obc_fn(void);
void get_rtc_fn(void);
void set_rtc_fn(void);
void read_obc_eeprom_fn(void);
void erase_obc_eeprom_fn(void);
void read_obc_ram_byte_fn(void);
void set_indef_beacon_enable_fn(void);
void send_eps_can_msg_fn(void);
void send_pay_can_msg_fn(void);
void reset_subsys_fn(void);

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
cmd_t set_indef_beacon_enable_cmd = {
    .fn = set_indef_beacon_enable_fn,
    .opcode = CMD_SET_INDEF_BEACON_ENABLE,
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
cmd_t reset_subsys_cmd = {
    .fn = reset_subsys_fn,
    .opcode = CMD_RESET_SUBSYS,
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
    &set_indef_beacon_enable_cmd,
    &send_eps_can_msg_cmd,
    &send_pay_can_msg_cmd,
    &reset_subsys_cmd,
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
    uint32_t data = read_eeprom((uint16_t) current_cmd_arg1);

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

void set_indef_beacon_enable_fn(void) {
    if (current_cmd_arg1 == 1) {
        write_eeprom(BEACON_ENABLE_1_EEPROM_ADDR, current_cmd_arg2);
    } else if (current_cmd_arg1 == 2) {
        write_eeprom(BEACON_ENABLE_2_EEPROM_ADDR, current_cmd_arg2);
    } else {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
        return;
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
        enqueue_tx_msg(&eps_tx_msg_queue, CAN_EPS_CTRL, CAN_EPS_CTRL_RESET, 0);

        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
        finish_current_cmd(CMD_RESP_STATUS_OK);
    }
    else if (current_cmd_arg1 == CMD_PAY) {
        enqueue_tx_msg(&pay_tx_msg_queue, CAN_PAY_CTRL, CAN_PAY_CTRL_RESET_SSM, 0);

        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
        finish_current_cmd(CMD_RESP_STATUS_OK);
    }
    else {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
    }
}

void read_rec_status_info_fn(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);

        for (uint8_t i = 0; i < NUM_DATA_COL_SECTIONS; i++) {
            data_col_t* data_col = all_data_cols[i];

            // Exclude PAY_OPT
            if (data_col != &pay_opt_data_col) {
                uint32_t block = data_col->mem_section->curr_block;
                if (block > 0) {
                    block -= 1;
                }

                read_mem_data_block(data_col->mem_section, block,
                    &data_col->header, data_col->fields);

                uint8_t start_field = 0;
                uint8_t end_field = 0;

                if (data_col == &obc_hk_data_col) {
                    // 5 fields
                    start_field = CAN_OBC_HK_UPTIME;
                    end_field = CAN_OBC_HK_RESTART_TIME;
                } else if (data_col == &eps_hk_data_col) {
                    // 3 fields
                    start_field = CAN_EPS_HK_UPTIME;
                    end_field = CAN_EPS_HK_RESTART_REASON;
                } else if (data_col == &pay_hk_data_col) {
                    // 3 fields
                    start_field = CAN_PAY_HK_UPTIME;
                    end_field = CAN_PAY_HK_RESTART_REASON;
                } else {
                    continue;
                }

                for (uint8_t field = start_field; field <= end_field; field++) {
                    append_to_trans_tx_resp((data_col->fields[field] >> 16) & 0xFF);
                    append_to_trans_tx_resp((data_col->fields[field] >> 8) & 0xFF);
                    append_to_trans_tx_resp((data_col->fields[field] >> 0) & 0xFF);
                }
            }
        }

        finish_trans_tx_resp();
        finish_current_cmd(CMD_RESP_STATUS_OK);
    }
}

// fields and num_fields are different for PAY_OPT_1 and PAY_OPT_2
void read_data_block_impl(data_col_t* data_col, bool read_mem, uint32_t* fields,
        uint8_t num_fields) {
    if (read_mem) {
#ifdef COMMANDS_DEBUG
        print("Reading data block from mem\n");
#endif
        // Read data block from flash memory into variables for header
        // and fields
        read_mem_data_block(data_col->mem_section, current_cmd_arg2,
            &data_col->header, data_col->fields);
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);
        append_header_to_tx_msg(&data_col->header);
        append_fields_to_tx_msg(fields, num_fields);
        finish_trans_tx_resp();
    }
}

void read_data_block_common(bool read_mem) {
    if (current_cmd_arg1 == CMD_OBC_HK ||
            current_cmd_arg1 == CMD_EPS_HK ||
            current_cmd_arg1 == CMD_PAY_HK) {
        for (uint8_t i = 0; i < NUM_DATA_COL_SECTIONS; i++) {
            data_col_t* data_col = all_data_cols[i];

            if (current_cmd_arg1 == data_col->cmd_arg1) {
                read_data_block_impl(data_col, read_mem, data_col->fields,
                    data_col->mem_section->fields_per_block);

                finish_current_cmd(CMD_RESP_STATUS_OK);
                return;
            }
        }
    }

    else if (current_cmd_arg1 == CMD_PAY_OPT_OD) {
        read_data_block_impl(&pay_opt_data_col, read_mem,
            &pay_opt_data_col.fields[0],
            CAN_PAY_OPT_OD_FIELD_COUNT);

        finish_current_cmd(CMD_RESP_STATUS_OK);
        return;
    }
    else if (current_cmd_arg1 == CMD_PAY_OPT_FL) {
        read_data_block_impl(&pay_opt_data_col, read_mem,
            &pay_opt_data_col.fields[CAN_PAY_OPT_OD_FIELD_COUNT],
            CAN_PAY_OPT_FL_FIELD_COUNT);

        finish_current_cmd(CMD_RESP_STATUS_OK);
        return;
    }

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
    finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
    return;
}

void read_data_block_fn(void) {
    read_data_block_common(true);
}

void read_rec_loc_data_block_fn(void) {
    read_data_block_common(false);    
}

// Common functionality for primary and secondary blocks
void read_cmd_blocks(mem_section_t* section) {
    if (current_cmd_arg2 > CMD_READ_CMD_BLOCKS_MAX_COUNT) {
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
    // Enforce starting address
    if (current_cmd_arg1 >= MEM_NUM_ADDRESSES) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
        return;
    }

    // Enforce max number of bytes
    if (current_cmd_arg2 > CMD_READ_MEM_MAX_COUNT) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
        return;
    }

    // Enforce not rolling over addresses past the third chip
    if (current_cmd_arg1 + current_cmd_arg2 - 1 >= MEM_NUM_ADDRESSES) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
        return;
    }

    uint8_t data[CMD_READ_MEM_MAX_COUNT] = { 0x00 };
    read_mem_bytes(current_cmd_arg1, data, current_cmd_arg2);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        start_trans_tx_resp(CMD_RESP_STATUS_OK);
        for (uint32_t i = 0; i < current_cmd_arg2; i++) {
            append_to_trans_tx_resp(data[i]);
        }
        finish_trans_tx_resp();
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void erase_mem_phy_sector_fn(void) {
    if (current_cmd_arg1 >= MEM_NUM_ADDRESSES) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
        return;
    }

    erase_mem_sector(current_cmd_arg1);

    // Only send a transceiver packet if the erase was initiated by the ground
    // station
    if (current_cmd_id != CMD_CMD_ID_AUTO_ENQUEUED) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    }
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void erase_mem_phy_block_fn(void) {
    if (current_cmd_arg1 >= MEM_NUM_ADDRESSES) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
        return;
    }

    erase_mem_block(current_cmd_arg1);

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void erase_all_mem_fn(void) {
    erase_mem();

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void col_data_block_obc_hk(void) {
#ifdef COMMANDS_VERBOSE
    print("Start %s\n", obc_hk_data_col.name);
#endif

    data_col_t* data_col = &obc_hk_data_col;

    // Populate header
    populate_header(&data_col->header,
        data_col->mem_section->curr_block,
        CMD_RESP_STATUS_UNKNOWN);
    // Write header (except status) to memory
    write_mem_header_main(data_col->mem_section,
        data_col->mem_section->curr_block,
        &data_col->header);
    // Increment the block number
    inc_and_prepare_mem_section_curr_block(data_col->mem_section);

    // Populate fields
    data_col->fields[CAN_OBC_HK_UPTIME] = uptime_s;
    data_col->fields[CAN_OBC_HK_RESTART_COUNT] = restart_count;
    data_col->fields[CAN_OBC_HK_RESTART_REASON] = restart_reason;
    data_col->fields[CAN_OBC_HK_RESTART_DATE] =
        ((uint32_t) restart_date.yy << 16) |
        ((uint32_t) restart_date.mm << 8) |
        ((uint32_t) restart_date.dd << 0);
    data_col->fields[CAN_OBC_HK_RESTART_TIME] =
        ((uint32_t) restart_time.hh << 16) |
        ((uint32_t) restart_time.mm << 8) |
        ((uint32_t) restart_time.ss << 0);

    // Write fields to memory
    for (uint8_t i = 0; i < data_col->mem_section->fields_per_block; i++) {
        write_mem_field(data_col->mem_section,
            data_col->mem_section->curr_block,
            i, data_col->fields[i]);
        // i is field number; fields[i] corresponds to associated field data
    }

#ifdef COMMANDS_VERBOSE
    print("Done %s\n", data_col->name);
#endif

    // Only send back a transceiver packet if the command was sent from
    // ground (not auto)
    if (current_cmd_id != CMD_CMD_ID_AUTO_ENQUEUED) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            start_trans_tx_resp(CMD_RESP_STATUS_OK);
            // Need to use the block number from the header because the block
            // number for the memory section has already been incremented
            // past the block just collected
            append_to_trans_tx_resp((data_col->header.block_num >> 24) & 0xFF);
            append_to_trans_tx_resp((data_col->header.block_num >> 16) & 0xFF);
            append_to_trans_tx_resp((data_col->header.block_num >> 8) & 0xFF);
            append_to_trans_tx_resp((data_col->header.block_num >> 0) & 0xFF);
            finish_trans_tx_resp();
        }
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
    return;
}

// Field 0
void col_data_block_other_start(data_col_t* data_col) {
#ifdef COMMANDS_VERBOSE
    print("Start data col\n", data_col->name, cmd_field);
#endif

    populate_header(&data_col->header,
        data_col->mem_section->curr_block,
        CMD_RESP_STATUS_UNKNOWN);
    write_mem_header_main(data_col->mem_section,
        data_col->mem_section->curr_block,
        &data_col->header);
    
    // Reset all field data so we don't leave garbage data from last collection
    for (uint8_t i = 0; i < data_col->mem_section->fields_per_block; i++) {
        data_col->fields[i] = 0;
    }

    // This increment invalidates the current block number for the
    // memory section struct for the current command, so the command
    // will need to fetch the block number from the header
    inc_and_prepare_mem_section_curr_block(
        data_col->mem_section);

    enqueue_tx_msg(data_col->can_tx_queue,
        data_col->can_opcode, 0, 0);

    // Enqueue command (check for response 0 then send field 1)
    enqueue_cmd(current_cmd_id, &col_data_block_cmd, current_cmd_arg1, 1);

    // Store the current uptime before receiving first field
    data_col->prev_field_col_uptime_s = uptime_s;

    finish_current_cmd(CMD_RESP_STATUS_DATA_COL_IN_PROGRESS);
    return;
}

// Fields 1...N
void col_data_block_other_check(data_col_t* data_col) {
    // Check if we have timed out before receiving a field response
    if (uptime_s >= data_col->prev_field_col_uptime_s +
            CMD_COL_DATA_BLOCK_FIELD_TIMEOUT_S) {
#ifdef COMMANDS_DEBUG
        print("COL DATA TIMEOUT\n");
#endif
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_TIMED_OUT);
        finish_current_cmd(CMD_RESP_STATUS_TIMED_OUT);
        return;
    }

    uint8_t msg[8] = {0x00};

    uint8_t opcode = 0;
    uint8_t field_num = 0;
    uint32_t data = 0;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // If there are no messages in the CAN RX queue, re-enqueue
        // this command to the command queue and stop
        if (queue_empty(&data_rx_msg_queue)) {
            enqueue_cmd(current_cmd_id, &col_data_block_cmd,
                current_cmd_arg1, current_cmd_arg2);
            finish_current_cmd(CMD_RESP_STATUS_DATA_COL_IN_PROGRESS);
            return;
        }
        // Dequeue the message, but we might re-enqueue it after
        dequeue(&data_rx_msg_queue, msg);
    }

    if (print_can_msgs) {
        // Extra spaces to align with CAN TX messages
        print("CAN RX (DC): ");
        print_bytes(msg, 8);
    }

    // Break down the message into components
    opcode = msg[0];
    field_num = msg[1];
    // Don't need to check status
    data =
        ((uint32_t) msg[4] << 24) |
        ((uint32_t) msg[5] << 16) |
        ((uint32_t) msg[6] << 8) |
        ((uint32_t) msg[7]);

    // If the opcode does not match this block type, put it back in the queue
    if (opcode != data_col->can_opcode) {
        enqueue_front(&data_rx_msg_queue, msg);
        if (print_can_msgs) {
            print("Re-enqueued\n");
        }

        // Re-enqueue the same command to check for this field
        enqueue_cmd(current_cmd_id, &col_data_block_cmd,
            current_cmd_arg1, current_cmd_arg2);
        finish_current_cmd(CMD_RESP_STATUS_DATA_COL_IN_PROGRESS);
        return;
    }

    // If the CAN opcode in the message matches the opcode for
    // this block type
    // Continue with processing the message

    // If the field number received in the CAN message is what we are expecting
    if (field_num != current_cmd_arg2 - 1) {
        // Re-enqueue the same command to check for this field
        enqueue_cmd(current_cmd_id, &col_data_block_cmd,
            current_cmd_arg1, current_cmd_arg2);
        finish_current_cmd(CMD_RESP_STATUS_DATA_COL_IN_PROGRESS);
        return;
    }

#ifdef COMMANDS_VERBOSE
    print("Received field %u\n", field_num);
#endif

    // Update the current uptime for receiving this field
    data_col->prev_field_col_uptime_s = uptime_s;

    // Save received field data
    // Note that data_col->mem_section.curr_block has already been incremented,
    // so we need to use the block number from the header that was populated
    // at the start of this command
    data_col->fields[field_num] = data;
    write_mem_field(data_col->mem_section,
        data_col->header.block_num,
        field_num, data);

    // Send request for next field if there are more fields
    uint8_t next_field_num = field_num + 1;
    if (next_field_num < data_col->mem_section->fields_per_block) {
        // Enqueue CAN message
        enqueue_tx_msg(data_col->can_tx_queue,
            data_col->can_opcode, next_field_num, 0);
        // Enqueue command to check for the response
        // (note command argument 2 must be 1 greater
        // than the field number in the CAN message)
        enqueue_cmd(current_cmd_id,
            &col_data_block_cmd, current_cmd_arg1,
            next_field_num + 1);

#ifdef COMMANDS_VERBOSE
        print("Req field %u\n", next_field_num);
#endif

        finish_current_cmd(CMD_RESP_STATUS_DATA_COL_IN_PROGRESS);
        return;
    }

    // If we have received all the fields
    else {
        // Only send back a transceiver packet if the
        // command was sent from ground
        if (current_cmd_id != CMD_CMD_ID_AUTO_ENQUEUED) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                start_trans_tx_resp(CMD_RESP_STATUS_OK);
                append_to_trans_tx_resp((data_col->header.block_num >> 24) & 0xFF);
                append_to_trans_tx_resp((data_col->header.block_num >> 16) & 0xFF);
                append_to_trans_tx_resp((data_col->header.block_num >> 8) & 0xFF);
                append_to_trans_tx_resp((data_col->header.block_num >> 0) & 0xFF);
                finish_trans_tx_resp();
            }
        }

#ifdef COMMANDS_VERBOSE
        print("Done %s\n", data_col->name);
#endif
        finish_current_cmd(CMD_RESP_STATUS_OK);
        return;
    }
}

void col_data_block_other(void) {
    for (uint8_t i = 0; i < NUM_DATA_COL_SECTIONS; i++) {
        data_col_t* data_col = all_data_cols[i];

        if (data_col->cmd_arg1 == current_cmd_arg1) {
#ifdef COMMANDS_VERBOSE
            print("%s: field %lu\n", data_col->name, current_cmd_arg2);
            print("prev field uptime = %lu\n", data_col->prev_field_col_uptime_s);
#endif

            if (current_cmd_arg2 == 0) {
                col_data_block_other_start(data_col);
                return;
            }

            // Checking for a CAN response
            // If not receiving the last field, send the next request
            else if (1 <= current_cmd_arg2 &&
                    current_cmd_arg2 <= data_col->mem_section->fields_per_block) {
                col_data_block_other_check(data_col);
                return;
            }
        }
    }

    if (current_cmd_id != CMD_CMD_ID_AUTO_ENQUEUED) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
    }
    finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
    return;
}


void col_data_block_fn(void) {
    if (current_cmd_arg1 == CMD_OBC_HK) {
        col_data_block_obc_hk();
        return;
    }

    // Not OBC_HK
    else if (current_cmd_arg1 == CMD_EPS_HK ||
            current_cmd_arg1 == CMD_PAY_HK ||
            current_cmd_arg1 == CMD_PAY_OPT) {
        col_data_block_other();
        return;
    }

    // If it doesn't match any of the data col types
    if (current_cmd_id != CMD_CMD_ID_AUTO_ENQUEUED) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
    }
    finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
    return;
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
    if (current_cmd_arg2 >= MEM_NUM_ADDRESSES) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
        return;
    }

    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            set_mem_section_start_addr(
                &obc_hk_mem_section, current_cmd_arg2);
            break;
        case CMD_EPS_HK:
            set_mem_section_start_addr(
                &eps_hk_mem_section, current_cmd_arg2);
            break;
        case CMD_PAY_HK:
            set_mem_section_start_addr(
                &pay_hk_mem_section, current_cmd_arg2);
            break;
        case CMD_PAY_OPT:
            set_mem_section_start_addr(
                &pay_opt_mem_section, current_cmd_arg2);
            break;
        case CMD_PRIM_CMD_LOG:
            set_mem_section_start_addr(
                &prim_cmd_log_mem_section, current_cmd_arg2);
            break;
        case CMD_SEC_CMD_LOG:
            set_mem_section_start_addr(
                &sec_cmd_log_mem_section, current_cmd_arg2);
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
    if (current_cmd_arg2 >= MEM_NUM_ADDRESSES) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
        return;
    }

    switch (current_cmd_arg1) {
        case CMD_OBC_HK:
            set_mem_section_end_addr(
                &obc_hk_mem_section, current_cmd_arg2);
            break;
        case CMD_EPS_HK:
            set_mem_section_end_addr(
                &eps_hk_mem_section, current_cmd_arg2);
            break;
        case CMD_PAY_HK:
            set_mem_section_end_addr(
                &pay_hk_mem_section, current_cmd_arg2);
            break;
        case CMD_PAY_OPT:
            set_mem_section_end_addr(
                &pay_opt_mem_section, current_cmd_arg2);
            break;
        case CMD_PRIM_CMD_LOG:
            set_mem_section_end_addr(
                &prim_cmd_log_mem_section, current_cmd_arg2);
            break;
        case CMD_SEC_CMD_LOG:
            set_mem_section_end_addr(
                &sec_cmd_log_mem_section, current_cmd_arg2);
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

        // Current system uptime
        append_to_trans_tx_resp((uptime_s >> 24) & 0xFF);
        append_to_trans_tx_resp((uptime_s >> 16) & 0xFF);
        append_to_trans_tx_resp((uptime_s >> 8) & 0xFF);
        append_to_trans_tx_resp((uptime_s >> 0) & 0xFF);

        for (uint8_t i = 0; i < NUM_DATA_COL_SECTIONS; i++) {
            append_to_trans_tx_resp((uint8_t) all_data_cols[i]->auto_enabled);
            append_to_trans_tx_resp((all_data_cols[i]->auto_period >> 24) & 0xFF);
            append_to_trans_tx_resp((all_data_cols[i]->auto_period >> 16) & 0xFF);
            append_to_trans_tx_resp((all_data_cols[i]->auto_period >> 8) & 0xFF);
            append_to_trans_tx_resp((all_data_cols[i]->auto_period >> 0) & 0xFF);
            append_to_trans_tx_resp((all_data_cols[i]->prev_auto_col_uptime_s >> 24) & 0xFF);
            append_to_trans_tx_resp((all_data_cols[i]->prev_auto_col_uptime_s >> 16) & 0xFF);
            append_to_trans_tx_resp((all_data_cols[i]->prev_auto_col_uptime_s >> 8) & 0xFF);
            append_to_trans_tx_resp((all_data_cols[i]->prev_auto_col_uptime_s >> 0) & 0xFF);
        }
        
        finish_trans_tx_resp();
    }

    finish_current_cmd(CMD_RESP_STATUS_OK);
}

void set_auto_data_col_enable_fn(void) {
    for (uint8_t i = 0; i < NUM_DATA_COL_SECTIONS; i++) {
        data_col_t* data_col = all_data_cols[i];

        if (current_cmd_arg1 == data_col->cmd_arg1) {
            if (current_cmd_arg2 > 1) {
                add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
                finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
                return;
            }

            data_col->auto_enabled = current_cmd_arg2 ? true : false;
            write_eeprom(data_col->auto_enabled_eeprom_addr, (uint32_t) data_col->auto_enabled);

            // In case we just enabled data collection, set the uptime of last
            // data collection to now
            // e.g. say we never enabled OBC_HK auto, so last auto time is 0,
            // say it is uptime 100s and period is 60s, it would trigger
            // immediately
            data_col->prev_auto_col_uptime_s = uptime_s;

            add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
            finish_current_cmd(CMD_RESP_STATUS_OK);
            return;
        }
    }

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
    finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
    return;
}

void set_auto_data_col_period_fn(void) {
    // Enforce minimum period to not collect too quickly
    if (current_cmd_arg2 < CMD_AUTO_DATA_COL_MIN_PERIOD) {
        add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
        finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
        return;
    }

    for (uint8_t i = 0; i < NUM_DATA_COL_SECTIONS; i++) {
        data_col_t* data_col = all_data_cols[i];

        if (current_cmd_arg1 == data_col->cmd_arg1) {
            data_col->auto_period = current_cmd_arg2;
            write_eeprom(data_col->auto_period_eeprom_addr, (uint32_t) data_col->auto_period);

            add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
            finish_current_cmd(CMD_RESP_STATUS_OK);
            return;
        }
    }

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_INVALID_ARGS);
    finish_current_cmd(CMD_RESP_STATUS_INVALID_ARGS);
    return;
}

void resync_auto_data_col_timers_fn(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        for (uint8_t i = 0; i < NUM_DATA_COL_SECTIONS; i++) {
            all_data_cols[i]->prev_auto_col_uptime_s = uptime_s;
        }
    }

    add_def_trans_tx_dec_msg(CMD_RESP_STATUS_OK);
    finish_current_cmd(CMD_RESP_STATUS_OK);
}
