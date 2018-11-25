#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>

#include <can/data_protocol.h>

#include "can_interface.h"

// Callback function signature to run a command
typedef void(*cmd_fn_t)(void);

// Command
typedef struct {
    cmd_fn_t fn;
} cmd_t;

// TODO - volatile variable to point to currently executing command?

extern queue_t cmd_queue;

void enqueue_cmd(queue_t* queue, cmd_t* cmd);
void dequeue_cmd(queue_t* queue, cmd_t* cmd);

#endif
