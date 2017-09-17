#ifndef OBC_H
#define OBC_H

#include <avr/io.h>
#include <util/delay.h>
#include <time.h>
#include <
#include <spi/spi.h>
#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>

#include "queue.h"
// Global variable, sorry, data buffer used in files can_handler and obc

#include "data_buffer.h"

extern struct data_buffer * data_buffer_pointer;
extern struct housekeeping_data_buffer * housekeeping_buffer;

extern uint8_t hk_flag_pay;
extern uint8_t hk_flag_comms;
extern uint8_t hk_flag_eps;
extern uint8_t hk_all_received;
extern uint8_t hk_all_requested;

#endif
