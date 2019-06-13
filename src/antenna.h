#ifndef ANTENNA_H
#define ANTENNA_H

#include <avr/eeprom.h>

#include <uart/uart.h>
#include <utilities/utilities.h>
#include <watchdog/watchdog.h>

#include "i2c.h"

// EEPROM address to store whether comms delay has finished
#define COMMS_DELAY_DONE_EEPROM_ADDR ((uint32_t*) 0x34)
// Write 1 when it has already been done
#define COMMS_DELAY_DONE_FLAG   1

// Default number of seconds to wait before initializing comms (in seconds)
// Currently 30 minutes
#define COMMS_DELAY_DEF_S (30 * 60)

extern volatile uint32_t comms_delay_s;

void run_comms_delay(void);
void deploy_antenna(void);

#endif
