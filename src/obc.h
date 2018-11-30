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
void init_obc_comms(void);

void execute_next_cmd(void);

void process_next_rx_msg(void);
void send_next_eps_tx_msg(void);
void send_next_pay_tx_msg(void);

#endif
