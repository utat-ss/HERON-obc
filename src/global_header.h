#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#include <avr/io.h>
#include <util/delay.h>
#include  <stdbool.h>
#include  <stdint.h>

#include <spi/spi.h>
#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>

#include "cmd_queue.h"
#include "cmd_comms.h"
#include "data_buffer.h"
#include "obc.h"
#include "can_handler.h"
#include "housekeeping.h"
#include "cmd_science.h"

// Global variables, sorry, data buffer and housekeeping_data_buffer used in files can_handler and obc
extern struct data_buffer * data_buffer_pointer;
extern struct housekeeping_data_buffer * housekeeping_buffer;

// Global variables used in files can_handler and obc
extern uint8_t hk_flag_pay;
extern uint8_t hk_flag_comms;
extern uint8_t hk_flag_eps;
extern uint8_t hk_all_received;
extern uint8_t hk_all_requested;

#endif
