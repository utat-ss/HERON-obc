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

#include "obc.h"
#include "callbacks.h"
#include "cmd_queue.h"
#include "can_queue.h"
#include "housekeeping.h"
#include "cmd_science.h"

#endif
