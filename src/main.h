#ifndef MAIN_H
#define MAIN_H

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <stdint.h>
#include <util/delay.h>

#include <uart/uart.h>
#include <timer/timer.h>
#include <can/can.h>
#include <can/can_ids.h>

#include "can_queue.h"
#include "cmd_queue.h"
#include "tx_callbacks.h"
#include "sci_hk.h"
#include "timer_callbacks.h"

#define PAY_CMD_TX_MOB 3
#define EPS_CMD_TX_MOB 4
#define DATA_RX_MOB 5

#define PAY_HK_TYPE 0x00 // use packets.h after merge
#define SCI_TYPE 0x01 // use packets.h after merge

#define EPS_HK_BLOCK_SIZE 2
#define SCI_BLOCK_SIZE 2
#define PAY_HK_BLOCK_SIZE 2

uint8_t GLOBAL_SCI_FIELD_NUM,
        GLOBAL_PAY_HK_FIELD_NUM,
        GLOBAL_EPS_HK_FIELD_NUM;

void print_bytes(uint8_t *data, uint8_t len);

Can_queue sci_tx_queue, pay_hk_tx_queue, eps_hk_tx_queue;
Cmd_queue obc_queue;

#endif
