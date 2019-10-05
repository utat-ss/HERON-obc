#ifndef ANTENNA_H
#define ANTENNA_H

#include <avr/eeprom.h>

#include <uart/uart.h>
#include <utilities/utilities.h>
#include <watchdog/watchdog.h>

#include "i2c.h"

// EEPROM address to store whether comms delay has finished
#define COMMS_DELAY_DONE_EEPROM_ADDR    0x120
// Write 1 when it has already been done
#define COMMS_DELAY_DONE_FLAG   1
// Antenna I2C addresses according to the datasheet
// They said
#define ANTENNA_I2C_ADDRESS ((uint8_t) 0x33)

// Default number of seconds to wait before initializing comms (in seconds)
// Currently 30 minutes
#define COMMS_DELAY_DEF_S (30 * 60)

#define ANT_REL_A       6
#define ANT_REL_B       7
#define PORT_ANT_REL    PORTC
#define DDR_ANT_REL     DDRC
#define ANT_DEP_WARN    0
#define PORT_ANT_WARN   PORTD
#define DDR_ANT_WARN    DDRD

extern volatile uint32_t comms_delay_s;

void init_ant(void);
void run_comms_delay(void);
void deploy_antenna(void);
uint8_t read_antenna_data(uint8_t* door_positions, uint8_t* mode,
        uint8_t* main_heaters, uint8_t* backup_heaters, uint8_t* timer_s,
        uint8_t* i2c_status);
uint8_t write_antenna_alg1(uint8_t* i2c_status);
uint8_t write_antenna_alg2(uint8_t ant_num_in_bytes, uint8_t* i2c_status);
uint8_t write_antenna_clear(uint8_t* i2c_status);

#endif
