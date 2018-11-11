#ifndef MEM_H
#define MEM_H

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

// AVR Library Includes
#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>

// lib-common includes
#include <spi/spi.h>
#include <uart/uart.h>
#include <can/data_protocol.h>
#include "rtc.h"

// Sections in memory
typedef struct {
    // Start address of section in memory
    uint32_t start_addr;
    // Current block number being written to in this section of memory (starting from 0, increasing by 1)
    uint32_t curr_block;
    // Address in EEPROM that stores the current block number
    uint32_t* curr_block_eeprom_addr;
    // Number of fields in one block (NOT including the header/RTC data)
    uint16_t fields_per_block;
} mem_section_t;

extern mem_section_t eps_hk_mem_section;
extern mem_section_t pay_hk_mem_section;
extern mem_section_t pay_sci_mem_section;

// Chips are numbered 0-2
#define MEM_NUM_CHIPS 3

// Pins and Ports
// TODO - change
#define MEM_CS_PORT         PORTB
#define MEM_CS_DDR          DDRB
#define MEM_CHIP0_CS_PIN    PB2
#define MEM_CHIP1_CS_PIN    PB3
#define MEM_CHIP2_CS_PIN    PB4

// Commands and Special registers
#define MEM_READ_STATUS         0x05
#define MEM_ERASE               0xC7
#define MEM_RST                 0x99
#define MEM_RSTEN               0x66
#define MEM_SECTOR_ERASE        0x20
#define MEM_WRITE_STATUS        0x01
#define MEM_WR_ENABLE           0x06
#define MEM_WR_DISABLE          0x04
#define MEM_PG_PRG              0x02
#define MEM_R_BYTE              0x03
#define MEM_FAST_READ           0x0B
#define MEM_ALL_SECTORS         0x3C

// Status bits
#define BUSY    0
#define WEL     1
#define BP0     2
#define BP1     3
#define BP2     4
#define BP3     5
#define AAI     6
#define BPL     7

// Number of bytes in a header
#define BYTES_PER_HEADER 8
// Number of bytes in one field (one measurement)
#define BYTES_PER_FIELD 3




// Initialization
void init_mem(void);

// EEPROM
void write_curr_block_to_eeprom(mem_section_t* section);
void read_curr_block_from_eeprom(mem_section_t* section);
void increment_curr_block(mem_section_t* section);

// Headers and fields
void write_mem_header(mem_section_t* section, uint32_t block_num);
void read_mem_header(mem_section_t* section, uint32_t block_num, uint8_t* data);
void write_mem_field(mem_section_t* section, uint32_t block_num, uint8_t field_num, uint32_t data);
uint32_t read_mem_field(mem_section_t* section, uint32_t block_num, uint8_t field_num);

// Low-level operations
void write_mem_bytes(uint32_t address, uint8_t* data, uint8_t data_len);
void read_mem_bytes(uint32_t address, uint8_t* data, uint8_t data_len);
void erase_mem(uint8_t chip);
void erase_mem_sector(uint8_t sector, uint8_t chip);
void unlock_mem(void);

// Status
uint8_t read_mem_status(uint8_t chip);
void write_mem_status(uint8_t status, uint8_t chip);

// Commands
uint8_t send_mem_command(uint8_t command, uint8_t data, uint8_t chip);
void send_short_mem_command(uint8_t command, uint8_t chip);

#endif
