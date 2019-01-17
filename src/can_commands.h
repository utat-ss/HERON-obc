#ifndef CAN_COMMANDS_H
#define CAN_COMMANDS_H

#include <stdint.h>

#include <can/data_protocol.h>
#include <queue/queue.h>
#include <uart/uart.h>

#include "can_interface.h"
#include "commands.h"

extern queue_t eps_tx_msg_queue;
extern queue_t pay_tx_msg_queue;
extern queue_t data_rx_msg_queue;

extern uint32_t eps_hk_data[];
extern uint32_t pay_hk_data[];
extern uint32_t pay_opt_data[];

void handle_rx_msg(void);

void enqueue_tx_msg(queue_t* queue, uint8_t msg_type, uint8_t field_num);
void enqueue_eps_hk_tx_msg(uint8_t field_num);
void enqueue_pay_hk_tx_msg(uint8_t field_num);
void enqueue_pay_opt_tx_msg(uint8_t field_num);
void enqueue_pay_exp_tx_msg(uint8_t field_num);

#endif
