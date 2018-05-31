#ifndef OBC_H
#define OBC_H

#include <stdint.h>

#include <uart/uart.h>
#include <uart/log.h>
#include <timer/timer.h>
#include <can/can.h>

#include "can_queue.h"
#include "cmd_queue.h"
#include "callbacks.h"

#define PAY_CMD_TX_MOB 3
#define EPS_CMD_TX_MOB 4
#define DATA_RX_MOB 5

Can_queue sci_tx_queue, pay_hk_tx_queue, eps_hk_tx_queue;
Cmd_queue obc_queue;

#endif
