#ifndef CAN_COMMANDS_H
#define CAN_COMMANDS_H

#include <stdint.h>

#include <can/data_protocol.h>
#include <queue/queue.h>
#include <uart/uart.h>

#include "can_interface.h"
#include "commands.h"

void handle_rx_msg(void);

#endif
