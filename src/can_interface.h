#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include <can/can.h>
#include <can/data_protocol.h>
#include <can/ids.h>
#include <queue/queue.h>
#include <uart/uart.h>

extern queue_t pay_tx_msg_queue;
extern queue_t eps_tx_msg_queue;
extern queue_t data_rx_msg_queue;

extern mob_t pay_cmd_tx_mob;
extern mob_t eps_cmd_tx_mob;
extern mob_t data_rx_mob;

#endif
