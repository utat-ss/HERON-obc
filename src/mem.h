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
#define MEM_BLOCK_ERASE         0xD8

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
#define MEM_NUM_CHIPS               3
// The number of bits used to address all bytes in one chip
// Because one chip is 2MB and each address is for one byte
#define MEM_CHIP_ADDR_WIDTH         21
// Number of sections in memory layout
#define MEM_NUM_SECTIONS            6
// Number of bytes in a header
#define MEM_BYTES_PER_HEADER        10
// Location of status byte in header
#define MEM_STATUS_HEADER_OFFSET    (MEM_BYTES_PER_HEADER - 1)
// Number of bytes in one field (one measurement)
#define MEM_BYTES_PER_FIELD         3
// Number of bytes in one command log
#define MEM_BYTES_PER_CMD           11
// Number of bytes per memory sector
#define MEM_BYTES_PER_SECTOR        4096

#define MEM_OBC_HK_START_ADDR       0x000000UL
#define MEM_OBC_HK_END_ADDR         0x07FFFFUL
#define MEM_EPS_HK_START_ADDR       0x080000UL
#define MEM_EPS_HK_END_ADDR         0x27FFFFUL
#define MEM_PAY_HK_START_ADDR       0x280000UL
#define MEM_PAY_HK_END_ADDR         0x3FFFFFUL
#define MEM_PAY_OPT_START_ADDR      0x400000UL
#define MEM_PAY_OPT_END_ADDR        0x57FFFFUL
#define MEM_PRIM_CMD_LOG_START_ADDR 0x580000UL
#define MEM_PRIM_CMD_LOG_END_ADDR   0x5BFFFFUL
#define MEM_SEC_CMD_LOG_START_ADDR  0x5C0000UL
#define MEM_SEC_CMD_LOG_END_ADDR    0x5FFFFFUL

#define MEM_NUM_ADDRESSES           0x600000UL

#define MEM_OBC_HK_CURR_BLOCK_EEPROM_ADDR       0x120
#define MEM_OBC_HK_START_ADDR_EEPROM_ADDR       0x124
#define MEM_OBC_HK_END_ADDR_EEPROM_ADDR         0x128
#define MEM_EPS_HK_CURR_BLOCK_EEPROM_ADDR       0x140
#define MEM_EPS_HK_START_ADDR_EEPROM_ADDR       0x144
#define MEM_EPS_HK_END_ADDR_EEPROM_ADDR         0x148
#define MEM_PAY_HK_CURR_BLOCK_EEPROM_ADDR       0x160
#define MEM_PAY_HK_START_ADDR_EEPROM_ADDR       0x164
#define MEM_PAY_HK_END_ADDR_EEPROM_ADDR         0x168
#define MEM_PAY_OPT_CURR_BLOCK_EEPROM_ADDR      0x180
#define MEM_PAY_OPT_START_ADDR_EEPROM_ADDR      0x184
#define MEM_PAY_OPT_END_ADDR_EEPROM_ADDR        0x188
#define MEM_PRIM_CMD_LOG_CURR_BLOCK_EEPROM_ADDR 0x1A0
#define MEM_PRIM_CMD_LOG_START_ADDR_EEPROM_ADDR 0x1A4
#define MEM_PRIM_CMD_LOG_END_ADDR_EEPROM_ADDR   0x1A8
#define MEM_SEC_CMD_LOG_CURR_BLOCK_EEPROM_ADDR  0x1C0
#define MEM_SEC_CMD_LOG_START_ADDR_EEPROM_ADDR  0x1C4
#define MEM_SEC_CMD_LOG_END_ADDR_EEPROM_ADDR    0x1C8


// Sections in memory
typedef struct {
    // Start address of section in memory
    uint32_t start_addr;
    uint16_t start_addr_eeprom_addr;
    // End address of section in memory
    uint32_t end_addr;
    uint16_t end_addr_eeprom_addr;
    // Current block number being written to in this section of memory (starting from 0, increasing by 1)
    uint32_t curr_block;
    // Address in EEPROM that stores the current block number
    uint16_t curr_block_eeprom_addr;
    // Number of fields in one block (NOT including the header)
    // This only matters for the data block sections, not the command log block sections
    uint8_t fields_per_block;
} mem_section_t;

// A collection of the data for one header in memory
typedef struct {
    // Block number within section
    uint32_t block_num;
    // RTC data
    rtc_date_t date;
    // RTC time
    rtc_time_t time;
    // Status - result of command (pass/fail and reason)
    uint8_t status;
} mem_header_t;

// Make the memory sections visible to other files so they can write data to sections
extern mem_section_t obc_hk_mem_section;
extern mem_section_t eps_hk_mem_section;
extern mem_section_t pay_hk_mem_section;
extern mem_section_t pay_opt_mem_section;
extern mem_section_t prim_cmd_log_mem_section;
extern mem_section_t sec_cmd_log_mem_section;
extern mem_section_t* all_mem_sections[];


// Initialization
void init_mem(void);
void clear_mem_header(mem_header_t* header);

// EEPROM
void write_mem_section_eeprom(mem_section_t* section);
void read_mem_section_eeprom(mem_section_t* section);
void read_all_mem_sections_eeprom(void);
void set_mem_section_curr_block(mem_section_t* section, uint32_t curr_block);
void set_mem_section_start_addr(mem_section_t* section, uint32_t start_addr);
void set_mem_section_end_addr(mem_section_t* section, uint32_t end_addr);

// High-level operations - blocks
void read_mem_data_block(mem_section_t* section, uint32_t block_num,
    mem_header_t* header, uint32_t* fields);

uint8_t write_mem_cmd_block(mem_section_t* section, uint32_t block_num,
    mem_header_t* header, uint16_t cmd_id, uint8_t opcode, uint32_t arg1,
    uint32_t arg2);
void read_mem_cmd_block(mem_section_t* section, uint32_t block_num,
    mem_header_t* header, uint16_t* cmd_id,
    uint8_t* opcode, uint32_t* arg1, uint32_t* arg2);

// High-level operations - headers and fields
void write_mem_header_main(mem_section_t* section, uint32_t block_num,
    mem_header_t* header);
void write_mem_header_status(mem_section_t* section, uint32_t block_num,
    uint8_t status);
void read_mem_header(mem_section_t* section, uint32_t block_num,
    mem_header_t* header);
void write_mem_field(mem_section_t* section, uint32_t block_num,
    uint8_t field_num, uint32_t data);
uint32_t read_mem_field(mem_section_t* section, uint32_t block_num,
    uint8_t field_num);

// Address calculations
uint32_t mem_sector_for_addr(uint32_t address);
uint32_t mem_addr_for_sector(uint32_t sector);
uint32_t mem_block_size(mem_section_t* section);
uint32_t mem_block_section_addr(mem_section_t* section, uint32_t block_num);
uint32_t mem_block_addr(mem_section_t* section, uint32_t block_num);
uint32_t mem_block_end_section_addr(mem_section_t* section, uint32_t block_num);
uint32_t mem_block_end_addr(mem_section_t* section, uint32_t block_num);
uint32_t mem_field_section_addr(mem_section_t* section, uint32_t block_num,
    uint32_t field_num);
uint32_t mem_cmd_section_addr(mem_section_t* section, uint32_t block_num);
void process_mem_addr(uint32_t address, uint8_t* chip_num, uint8_t* addr1,
    uint8_t* addr2, uint8_t* addr3);

// Section operations
uint8_t write_mem_section_bytes(mem_section_t *section, uint32_t address, uint8_t* data, uint8_t data_len);
void read_mem_section_bytes(mem_section_t *section, uint32_t address, uint8_t* data, uint8_t data_len);

// Low-level operations - raw bytes
void write_mem_bytes(uint32_t address, uint8_t* data, uint32_t data_len);
void read_mem_bytes(uint32_t address, uint8_t* data, uint32_t data_len);
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

// Block and sector erase
void erase_mem_block(uint32_t address);
void erase_mem_sector(uint32_t address);

#endif
