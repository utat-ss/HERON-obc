#ifndef can_handler_h
#define can_handler_h

#include <avr/io.h>
#include <util/delay.h>
#include "../lib-common/*.h"


void can_handler ( uint16_t identifier, uint8_t* pt_data, uint8_t size);

#endif
