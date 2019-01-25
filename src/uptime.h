#ifndef UPTIME_H
#define UPTIME_H

#include <stdint.h>

#include <avr/eeprom.h>

#include <timer/timer.h>
#include <uart/uart.h>

// Default double word (4-byte) value in EEPROM
// TODO - put in lib-common/utilities
#define EEPROM_DEF_DWORD 0xFFFFFFFF

// EEPROM address for storing number of resets
#define RESTART_COUNT_EEPROM_ADDR ((uint32_t*) 0x60)

extern volatile uint32_t restart_count;
extern volatile uint32_t uptime_s;

void init_restart_count(void);
void start_uptime_timer(void);

#endif
