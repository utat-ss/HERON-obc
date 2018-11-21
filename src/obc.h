#ifndef OBC_H
#define OBC_H

#include <stdint.h>

#include <can/can.h>
#include <queue/queue.h>
#include <spi/spi.h>

#include "can_interface.h"
#include "commands.h"
#include "mem.h"
#include "rtc.h"

void init_obc_core(void);
void execute_next_cmd(void);

#endif
