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
extern cmd_t ping_cmd;
extern cmd_t get_restart_uptime_cmd;
extern cmd_t get_rtc_cmd;
extern cmd_t set_rtc_cmd;
extern cmd_t read_mem_bytes_cmd;
extern cmd_t erase_mem_sector_cmd;
extern cmd_t col_block_cmd;
extern cmd_t read_loc_block_cmd;
extern cmd_t read_mem_block_cmd;
extern cmd_t auto_data_col_enable_cmd;
extern cmd_t auto_data_col_period_cmd;
extern cmd_t auto_data_col_resync_cmd;
extern cmd_t pay_act_motors_cmd;
extern cmd_t reset_subsys_cmd;
extern cmd_t eps_can_cmd;
extern cmd_t pay_can_cmd;
extern cmd_t read_eeprom_cmd;
extern cmd_t get_cur_block_num_cmd;
extern cmd_t set_cur_block_num_cmd;
extern cmd_t set_mem_sec_start_addr_cmd;
extern cmd_t set_mem_sec_end_addr_cmd;
extern cmd_t erase_eeprom_cmd;
extern cmd_t erase_all_mem_cmd;
extern cmd_t erase_mem_phy_block_cmd;

#endif
