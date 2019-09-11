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

// Length of `all_cmds_list` array
#define ALL_CMDS_LEN 24

extern cmd_t nop_cmd;
extern cmd_t ping_cmd;
extern cmd_t read_rec_status_info_cmd;
extern cmd_t get_rtc_cmd;
extern cmd_t set_rtc_cmd;
extern cmd_t read_mem_bytes_cmd;
extern cmd_t erase_mem_phy_sector_cmd;
extern cmd_t col_data_block_cmd;
extern cmd_t read_loc_block_cmd;
extern cmd_t read_mem_block_cmd;
extern cmd_t auto_data_col_enable_cmd;
extern cmd_t auto_data_col_period_cmd;
extern cmd_t auto_data_col_resync_cmd;
extern cmd_t act_pay_motors_cmd;
extern cmd_t reset_subsys_cmd;
extern cmd_t send_eps_can_msg_cmd;
extern cmd_t send_pay_can_msg_cmd;
extern cmd_t read_eeprom_cmd;
extern cmd_t get_cur_block_num_cmd;
extern cmd_t set_cur_block_num_cmd;
extern cmd_t set_mem_sec_start_addr_cmd;
extern cmd_t set_mem_sec_end_addr_cmd;
extern cmd_t erase_eeprom_cmd;
extern cmd_t erase_all_mem_cmd;
extern cmd_t erase_mem_phy_block_cmd;

extern cmd_t* all_cmds_list[];

#endif
