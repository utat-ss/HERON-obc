#ifndef CAN_COMMANDS_H
#define CAN_COMMANDS_H

#include <stdint.h>

#include <can/data_protocol.h>
#include <queue/queue.h>
#include <uart/uart.h>

#include "can_interface.h"
#include "command_utilities.h"
#include "commands.h"
#include "mem.h"
#include "rtc.h"

extern queue_t eps_tx_msg_queue;
extern queue_t pay_tx_msg_queue;
extern queue_t data_rx_msg_queue;

extern bool print_can_msgs;


void handle_rx_msg(void);

void process_next_rx_msg(void);
void send_next_eps_tx_msg(void);
void send_next_pay_tx_msg(void);

void enqueue_tx_msg_bytes(queue_t* queue, uint32_t data1, uint32_t data2);
void enqueue_tx_msg(queue_t* queue, uint8_t opcode, uint8_t field_num, uint32_t data);

#endif
