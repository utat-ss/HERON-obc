#ifndef OBC_H
#define OBC_H

#include <avr/io.h>
#include <util/delay.h>
#include <time.h>

#include <spi/spi.h>
#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>

#include "queue.h"
#include "data_buffer.h"

// Global variable, sorry, data buffer used in files can_handler and obc

extern struct data_buffer * data_buffer_pointer;

#endif
