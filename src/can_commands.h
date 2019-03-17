#ifndef CAN_COMMANDS_H
#define CAN_COMMANDS_H

#include <stdint.h>

#include <can/data_protocol.h>
#include <queue/queue.h>
#include <uart/uart.h>

#include "can_interface.h"
#include "commands.h"
#include "mem.h"
#include "rtc.h"

extern queue_t eps_tx_msg_queue;
extern queue_t pay_tx_msg_queue;
extern queue_t data_rx_msg_queue;

extern mem_header_t eps_hk_header;
extern uint32_t eps_hk_fields[];
extern mem_header_t pay_hk_header;
extern uint32_t pay_hk_fields[];
extern mem_header_t pay_opt_header;
extern uint32_t pay_opt_fields[];

void handle_rx_msg(void);

void enqueue_tx_msg(queue_t* queue, uint8_t msg_type, uint8_t field_num, uint32_t data);
void enqueue_eps_hk_tx_msg(uint8_t field_num);
void enqueue_eps_ctrl_tx_msg(uint8_t field_num, uint32_t data);
void enqueue_pay_hk_tx_msg(uint8_t field_num);
void enqueue_pay_opt_tx_msg(uint8_t field_num);
void enqueue_pay_ctrl_tx_msg(uint8_t field_num, uint32_t data);

#endif
