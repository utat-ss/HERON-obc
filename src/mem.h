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
#include "rtc.h"

// Basic memory functions
void init_mem();
void init_curr_stack_ptrs();
void mem_erase();
void mem_sector_erase(uint8_t sector);
uint8_t mem_status_r();
void mem_status_w(uint8_t status);
uint8_t mem_command(uint8_t command, uint8_t data);
void mem_command_short(uint8_t command);
void mem_unlock(uint8_t sector);
void mem_lock(uint8_t sector);
void mem_read(uint32_t address, uint8_t * data, uint8_t data_len);
void mem_write_byte(uint32_t address, uint8_t data);
void mem_write_multibyte (uint32_t address, uint8_t * data, uint8_t data_len);

// Memory management
uint32_t* pointer(uint8_t type);
uint32_t block_size(uint8_t type);
uint32_t init_stack(uint8_t type);
void init_stacks();
uint8_t init_block(uint8_t type);
// void init_header(uint8_t *header, uint8_t type);
void read_from_flash (uint8_t type,uint8_t* data,uint8_t data_len);
void write_to_flash(uint8_t type, uint8_t field_num, uint8_t * data);


// Pins and Ports
#define MEM_CS      PB5
#define MEM_PORT    PORTB
#define MEM_DDR     DDRB

// Commands and Special registers
#define MEM_READ_STATUS         0x05
#define MEM_ERASE               0x60
#define MEM_SECTOR_ERASE        0x20
#define MEM_WRITE_STATUS        0x01
#define MEM_WR_ENABLE           0x06
#define MEM_WR_DISABLE          0x04
#define MEM_WR_STATUS_ENABLE    0x50
#define MEM_WR_BYTE             0x02
#define MEM_WR_AAI              0xAD
#define MEM_BUSY_ENABLE         0x70
#define MEM_BUSY_DISABLE        0x80
#define MEM_R_BYTE              0x03
#define MEM_FAST_READ           0x0B

// Status bits
#define BUSY    0
#define WEL     1
#define BP0     2
#define BP1     3
#define BP2     4
#define BP3     5
#define AAI     6
#define BPL     7

#define MEM_ALL_SECTORS        0x3C

//Memory organization

//Initial values of the stack pointers
#define SCI_INIT    0x0DB0 //TODO: FIX NUMBERS
#define PAY_INIT    0x100000
#define EPS_INIT    0x0DB0
//#define OBC_INIT    0x180000
//#define STATUS_INIT 0x1C0000

//address in EEPROM that stores current pointer in corresponding stack
//choose addresses 20-24
#define SCI_CURR_PTR_ADDR       0x20
#define PAY_HK_CURR_PTR_ADDR    0x24
#define EPS_HK_CURR_PTR_ADDR    0x28

//Field and block sizes
#define FIELD_SIZE        0x04
#define SCI_BLOCK_SIZE    (CAN_PAY_SCI_FIELD_COUNT)//0x250
#define PAY_BLOCK_SIZE    (CAN_PAY_HK_FIELD_COUNT)//0x100
#define EPS_BLOCK_SIZE    0x03//0x100
#define OBC_BLOCK_SIZE    0x100
#define STATUS_BLOCK_SIZE 0x100

//Block parameters
#define HEADER_SIZE       0x02 //number of fields per header
#define SCI_TYPE          0x00
#define PAY_HK_TYPE       0x01
#define EPS_HK_TYPE       0x02
#define OBC_HK_TYPE       0x03
#define STATUS_TYPE       0x04

#endif
