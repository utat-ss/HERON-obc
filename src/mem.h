#ifndef MEM_H
#define MEM_H

// AVR Library Includes
#include <avr/io.h>
#include <avr/eeprom.h>

// lib-common includes
#include <spi/spi.h>
#include <uart/uart.h>
#include <can/data_protocol.h>

#include "rtc.h"


// Pins and Ports
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
#define MEM_UNLOCK              0x98

// Status bits
#define MEM_BUSY    0
#define MEM_WEL     1
#define MEM_BP0     2
#define MEM_BP1     3
#define MEM_BP2     4
#define MEM_BP3     5
#define MEM_AAI     6
#define MEM_BPL     7


// Chips are numbered 0-2
#define MEM_NUM_CHIPS 3

// The number of bits used to address all bytes in one chip
// Because one chip is 2MB and each adddress is for one byte
#define MEM_CHIP_ADDR_WIDTH 21

// Number of sections in memory layout
#define MEM_NUM_SECTIONS 3
// Number of bytes in a header
#define MEM_BYTES_PER_HEADER 8
// Number of bytes in one field (one measurement)
#define MEM_BYTES_PER_FIELD 3

#define MEM_EPS_HK_START_ADDR   0x000000UL
#define MEM_PAY_HK_START_ADDR   0x200000UL
#define MEM_PAY_OPT_START_ADDR  0x300000UL

#define MEM_EPS_HK_CURR_BLOCK_EEPROM_ADDR   ((uint32_t*) 0x20)
#define MEM_PAY_HK_CURR_BLOCK_EEPROM_ADDR   ((uint32_t*) 0x24)
#define MEM_PAY_OPT_CURR_BLOCK_EEPROM_ADDR  ((uint32_t*) 0x28)


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

// A collection of the data for one header in memory
typedef struct {
    // Block number within section
    // TODO - should it be 32-bit?
    uint8_t block_num;
    // Error data - TODO
    uint8_t error;
    // RTC data
    rtc_date_t date;
    // RTC time
    rtc_time_t time;
} mem_header_t;

// Make the memory sections visible to other files so they can write data to sections
extern mem_section_t eps_hk_mem_section;
extern mem_section_t pay_hk_mem_section;
extern mem_section_t pay_opt_mem_section;
extern mem_section_t* all_mem_sections[];


// Initialization
void init_mem(void);
void clear_mem_header(mem_header_t* header);

// EEPROM
void write_mem_section_eeprom(mem_section_t* section);
void read_mem_section_eeprom(mem_section_t* section);
void read_all_mem_sections_eeprom(void);
void inc_mem_section_curr_block(mem_section_t* section);

// High-level operations - blocks
void write_mem_block(mem_section_t* section, uint8_t block_num,
    mem_header_t* header, uint32_t* fields);
void read_mem_block(mem_section_t* section, uint8_t block_num,
    mem_header_t* header, uint32_t* fields);

// High-level operations - headers and fields
void write_mem_header(mem_section_t* section, uint8_t block_num,
    mem_header_t* header);
void read_mem_header(mem_section_t* section, uint8_t block_num,
    mem_header_t* header);
void write_mem_field(mem_section_t* section, uint32_t block_num,
    uint8_t field_num, uint32_t data);
uint32_t read_mem_field(mem_section_t* section, uint32_t block_num,
    uint8_t field_num);

// Address calculations
uint32_t mem_block_addr(mem_section_t* section, uint32_t block_num);
uint32_t mem_field_addr(mem_section_t* section, uint32_t block_num,
    uint32_t field_num);
void process_mem_addr(uint32_t address, uint8_t* chip_num, uint8_t* addr1,
    uint8_t* addr2, uint8_t* addr3);

// Low-level operations - bytes
void write_mem_bytes(uint32_t address, uint8_t* data, uint8_t data_len);
void read_mem_bytes(uint32_t address, uint8_t* data, uint8_t data_len);
void erase_mem(void);
void erase_mem_chip(uint8_t chip);
void unlock_mem(void);

// Status
void wait_for_mem_not_busy(uint8_t chip_num);
uint8_t read_mem_status(uint8_t chip);
void write_mem_status(uint8_t status, uint8_t chip);

// Commands
uint8_t send_mem_command(uint8_t command, uint8_t data, uint8_t chip);
void send_short_mem_command(uint8_t command, uint8_t chip);

#endif
