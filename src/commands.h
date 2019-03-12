#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdbool.h>
#include <stdint.h>

#include <can/data_protocol.h>
#include <timer/timer.h>

#include "can_interface.h"
#include "mem.h"

// Callback function signature to run a command
typedef void(*cmd_fn_t)(void);

// Command
typedef struct {
    cmd_fn_t fn;
} cmd_t;

// Automatic data collection for one block type
typedef struct {
    // True if we are currently collecting this type of data
    bool enabled;
    // Seconds between collection
    uint32_t period;
    // Number of seconds counted (start at 0, go to `period`)
    uint32_t count;
} aut_data_col_t;

// Period (time between timer callbacks) for automatic data collection (in
// seconds)
#define AUT_DATA_COL_PERIOD  60

// Subsystems
#define CMD_SUBSYS_OBC  0
#define CMD_SUBSYS_EPS  1
#define CMD_SUBSYS_PAY  2

// Block types
#define CMD_BLOCK_EPS_HK    0
#define CMD_BLOCK_PAY_HK    1
#define CMD_BLOCK_PAY_OPT   2


extern bool sim_local_actions;

extern queue_t cmd_queue;
extern queue_t cmd_args_queue;

extern volatile cmd_t* volatile current_cmd;
extern volatile uint32_t current_cmd_arg1;
extern volatile uint32_t current_cmd_arg2;
extern volatile bool prev_cmd_succeeded;

extern cmd_t nop_cmd;
extern cmd_t col_block_cmd;
extern cmd_t pop_blister_packs_cmd;
extern cmd_t read_mem_block_cmd;

void finish_current_cmd(bool succeeded);

void aut_data_col_timer_cb(void);
void populate_header(mem_header_t* header, uint8_t block_num, uint8_t error);

void enqueue_cmd(cmd_t* cmd, uint32_t arg1, uint32_t arg2);
void dequeue_cmd(void);

#endif
