#ifndef COMMANDS_H
#define COMMANDS_H

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

// Period (time between timer callbacks) for automatic data collection (in
// minutes)
// TODO - change to seconds
#define AUT_DATA_COL_PERIOD  1

// TODO - volatile variable to point to currently executing command?

extern queue_t cmd_queue;

extern cmd_t req_eps_hk_cmd;
extern cmd_t req_pay_hk_cmd;
extern cmd_t req_pay_opt_cmd;
extern cmd_t pop_blister_packs_cmd;
extern cmd_t write_flash_cmd;
extern cmd_t read_flash_cmd;
extern cmd_t start_aut_data_col_cmd;

void enqueue_cmd(queue_t* queue, cmd_t* cmd);
void dequeue_cmd(queue_t* queue, cmd_t* cmd);

#endif
