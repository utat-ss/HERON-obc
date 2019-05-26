#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <stdint.h>

#include <can/data_protocol.h>
#include <timer/timer.h>
#include <uptime/uptime.h>

#include "can_interface.h"
#include "mem.h"
#include "trans_commands.h"



// Automatic data collection for one block type
typedef struct {
    // True if we are currently collecting this type of data
    bool enabled;
    // Seconds between collection
    uint32_t period;
    // Number of seconds counted (start at 0, go to `period`)
    uint32_t count;
} auto_data_col_t;

// Callback function signature to run a command
typedef void(*cmd_fn_t)(void);

// Command
typedef struct {
    cmd_fn_t fn;
} cmd_t;


// Default period for automatic data collection for each block type
// (time between timer callbacks, in seconds)
#define EPS_HK_AUTO_DATA_COL_PERIOD     60
#define PAY_HK_AUTO_DATA_COL_PERIOD     120
#define PAY_OPT_AUTO_DATA_COL_PERIOD    300

// Subsystems
#define CMD_SUBSYS_OBC  0
#define CMD_SUBSYS_EPS  1
#define CMD_SUBSYS_PAY  2

// Block types
#define CMD_BLOCK_EPS_HK    0
#define CMD_BLOCK_PAY_HK    1
#define CMD_BLOCK_PAY_OPT   2


// TODO
extern rtc_date_t restart_date;		
extern rtc_time_t restart_time;

extern bool sim_local_actions;

extern queue_t cmd_queue;
extern queue_t cmd_args_queue;

extern volatile cmd_t* volatile current_cmd;
extern volatile uint32_t current_cmd_arg1;
extern volatile uint32_t current_cmd_arg2;
extern volatile bool prev_cmd_succeeded;

extern cmd_t nop_cmd;
extern cmd_t ping_cmd;
extern cmd_t get_restart_uptime_cmd;
extern cmd_t get_rtc_cmd;
extern cmd_t set_rtc_cmd;
extern cmd_t read_mem_cmd;
extern cmd_t erase_mem_cmd;
extern cmd_t collect_block_cmd;
extern cmd_t read_local_block_cmd;
extern cmd_t read_mem_block_cmd;
extern cmd_t set_auto_data_col_enable_cmd;
extern cmd_t set_auto_data_col_period_cmd;
extern cmd_t resync_auto_data_col_cmd;
extern cmd_t set_eps_heater_sp_cmd;
extern cmd_t set_pay_heater_sp_cmd;
extern cmd_t actuate_pay_motors_cmd;
extern cmd_t reset_cmd;
extern cmd_t send_eps_can_cmd;
extern cmd_t send_pay_can_cmd;
extern cmd_t read_eeprom_cmd;
extern cmd_t get_curr_block_num_cmd;

void finish_current_cmd(bool succeeded);

void auto_data_col_timer_cb(void);
void populate_header(mem_header_t* header, uint32_t block_num, uint8_t error);

void append_header_to_tx_msg(mem_header_t* header);
void append_fields_to_tx_msg(uint32_t* fields, uint8_t num_fields);

void enqueue_cmd(cmd_t* cmd, uint32_t arg1, uint32_t arg2);
void dequeue_cmd(void);

cmd_t* trans_msg_type_to_cmd(uint8_t msg_type);
uint8_t trans_cmd_to_msg_type(cmd_t* cmd);

#endif
