#ifndef _can_handler_h
#define _can_handler_h

#include <avr/io.h>
#include <util/delay.h>
#include "../lib-common/can.h"


void can_handler ( uint16_t identifier, uint8_t* pt_data, uint8_t size);

#endif
