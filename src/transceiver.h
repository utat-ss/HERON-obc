// AVR Library Includes
#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>


// lib-common includes
#include <uart/uart.h>

#define FREQ 0x9DD80942UL //default 437 MHz freq

// Status control register bits
#define TRANS_UARTBAUD 12 //requires bits 12-13
#define TRANS_RESET 11
#define TRANS_RFMODE 8 //requires bits 8-10
#define TRANS_ECHO 7
#define TRANS_BCN 6
#define TRANS_PIPE 5
#define TRANS_BOOT 4
#define TRANS_CTS 3
#define TRANS_SEC 2
#define TRANS_FRAM 1
#define TRANS_RFTS 0

void init_trans(void);

//Helper Functions to process responses
uint8_t trans_cb(const uint8_t* buf, uint8_t len);
uint8_t char_hex_to_dec(uint8_t c);
uint32_t scan_uint(uint8_t* string, uint8_t offset, uint8_t count);
uint8_t valid_cmd(uint8_t* string);
uint8_t string_cmp(uint8_t*string, uint8_t*string2, uint8_t len);

uint8_t set_trans_scw(uint16_t reg);
uint16_t read_trans_scw();
uint8_t set_trans_freq();
uint32_t read_trans_freq();
uint8_t set_trans_addr(uint8_t addr);
void set_trans_pipe_timeout(uint8_t timeout);
void set_pipeline ();

//Beacon
void set_beacon ();
void off_pipeline ();
uint8_t set_beacon_period (uint16_t period);
uint16_t read_beacon_period ();
uint8_t set_beacon_content(uint32_t content, uint8_t len);
uint32_t read_beacon_content();

uint8_t set_destination_callsign(char* callsign);
uint32_t read_destination_callsign();
uint8_t set_source_callsign(char* callsign);
uint32_t read_source_callsign();

uint32_t get_trans_uptime();
uint32_t get_transmitted_num_of_packets();
uint32_t get_received_num_of_packets();
uint32_t get_received_num_of_packets_CRC();
