#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include <can/can.h>
#include <can/data_protocol.h>
#include <can/ids.h>
#include <queue/queue.h>
#include <uart/uart.h>

#include "can_commands.h"

extern mob_t pay_cmd_tx_mob;
extern mob_t eps_cmd_tx_mob;
extern mob_t data_rx_mob;

#endif
