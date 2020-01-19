#ifndef ANTENNA_H
#define ANTENNA_H

#include <avr/eeprom.h>

#include <uart/uart.h>
#include <utilities/utilities.h>
#include <watchdog/watchdog.h>

#include "i2c.h"

// Antenna I2C address according to the datasheet
#define ANTENNA_I2C_ADDRESS ((uint8_t) 0x33)

#define ANT_REL_A       PC6
#define ANT_REL_B       PC7
#define PORT_ANT_REL    PORTC
#define DDR_ANT_REL     DDRC
#define ANT_DEP_WARN    PD0
#define PORT_ANT_WARN   PORTD
#define DDR_ANT_WARN    DDRD


void init_ant(void);
void deploy_antenna(void);
uint8_t read_antenna_data(uint8_t* door_positions, uint8_t* mode,
        uint8_t* main_heaters, uint8_t* backup_heaters, uint8_t* timer_s,
        uint8_t* i2c_status);
uint8_t write_antenna_alg1(uint8_t* i2c_status);
uint8_t write_antenna_alg2(uint8_t ant_num_in_bytes, uint8_t* i2c_status);
uint8_t write_antenna_clear(uint8_t* i2c_status);

#endif
