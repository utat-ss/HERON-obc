#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <stdint.h>

#include <can/data_protocol.h>
#include <timer/timer.h>
#include <uptime/uptime.h>

#include "can_interface.h"
#include "command_utilities.h"
#include "mem.h"

extern cmd_t nop_cmd;

extern cmd_t ping_obc_cmd;
extern cmd_t get_rtc_cmd;
extern cmd_t set_rtc_cmd;
extern cmd_t read_obc_eeprom_cmd;
extern cmd_t erase_obc_eeprom_cmd;
extern cmd_t read_obc_ram_byte_cmd;
extern cmd_t set_indef_beacon_enable_cmd;
extern cmd_t send_eps_can_msg_cmd;
extern cmd_t send_pay_can_msg_cmd;
extern cmd_t reset_subsys_cmd;

extern cmd_t read_rec_status_info_cmd;
extern cmd_t read_data_block_cmd;
extern cmd_t read_rec_loc_data_block_cmd;
extern cmd_t read_prim_cmd_blocks_cmd;
extern cmd_t read_sec_cmd_blocks_cmd;
extern cmd_t read_raw_mem_bytes_cmd;
extern cmd_t erase_mem_phy_sector_cmd;
extern cmd_t erase_mem_phy_block_cmd;
extern cmd_t erase_all_mem_cmd;

extern cmd_t col_data_block_cmd;
extern cmd_t get_cur_block_nums_cmd;
extern cmd_t set_cur_block_num_cmd;
extern cmd_t get_mem_sec_addrs_cmd;
extern cmd_t set_mem_sec_start_addr_cmd;
extern cmd_t set_mem_sec_end_addr_cmd;
extern cmd_t get_auto_data_col_settings_cmd;
extern cmd_t set_auto_data_col_enable_cmd;
extern cmd_t set_auto_data_col_period_cmd;
extern cmd_t resync_auto_data_col_timers_cmd;

extern cmd_t* all_cmds_list[];
extern const uint8_t all_cmds_list_len;

#endif
