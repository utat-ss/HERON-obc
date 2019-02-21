#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stdint.h>

#include <avr/interrupt.h>

#include <spi/spi.h>
#include <uart/uart.h>
#include <utilities/utilities.h>

// CS output pin
#define I2C_CS_PIN  PD1
#define I2C_CS_PORT PORTD
#define I2C_CS_DDR  DDRD

// RESET output pin
#define I2C_RESET_PIN  PD5
#define I2C_RESET_PORT PORTD
#define I2C_RESET_DDR  DDRD

// INT interrupt pin
// PCINT22 (p. 12)
#define I2C_INT_PIN  PD6
#define I2C_INT_PORT PORTD
#define I2C_INT_DDR  DDRD

// WAKEUPn pin
#define I2C_WAKEUP_PIN  PD7
#define I2C_WAKEUP_PORT PORTD
#define I2C_WAKEUP_DDR  DDRD

// Commands
#define I2C_WRITE           0x00  // p. 12
#define I2C_READ            0x01  // p. 13
#define I2C_READ_BUF        0x06  // p. 13
#define I2C_WRITE_REG       0x20  // p. 15
#define I2C_READ_REG        0x21  // p. 15
#define I2C_POWER_DOWN      0x30  // p. 16
#define I2C_POWER_DOWN_1    0x5A
#define I2C_POWER_DOWN_2    0xA5

// Internal register addresses (p. 5)
#define I2C_IO_CONFIG   0x00
#define I2C_IO_STATE    0x01
#define I2C_CLOCK       0x02
#define I2C_TO          0x03
#define I2C_STAT        0x04
#define I2C_ADR         0x05

// I2C status register values (p. 10)
#define I2C_SUCCESS         0xF0
#define I2C_ADDR_NACK       0xF1
#define I2C_BYTE_NACK       0xF2
#define I2C_BUSY            0xF3
#define I2C_TIME_OUT        0xF8
#define I2C_INVALID_COUNT   0xF9


void init_i2c(void);
void reset_i2c(void);

void power_down_i2c(void);
void power_up_i2c(void);

void start_i2c_spi(void);
void end_i2c_spi(void);

void write_i2c_reg(uint8_t addr, uint8_t data);
uint8_t read_i2c_reg(uint8_t addr);
void read_i2c_buf(uint8_t* data, uint8_t len);

uint8_t wait_for_i2c_int(void);
uint8_t wait_for_i2c_not_busy(void);

uint8_t write_i2c(uint8_t addr, uint8_t* data, uint8_t len, uint8_t* status);
uint8_t read_i2c(uint8_t addr, uint8_t* data, uint8_t len, uint8_t* status);

#endif
