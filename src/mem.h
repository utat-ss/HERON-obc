#ifndef MEM_H
#define MEM_H

// AVR Library Includes
#include 	<avr/io.h>
#include 	<util/delay.h>

// lib-common includes
#include 	<spi/spi.h>
#include 	<uart/uart.h>
#include 	<uart/log.h>


void 	init_mem			    ();
void mem_erase          ();
uint8_t	mem_status_r		();
void	mem_status_w		  (uint8_t status);
// uint8_t mem_send 		(uint8_t data);
uint8_t mem_command			(uint8_t command, uint8_t data);
void 	mem_command_short	(uint8_t command);
void 	mem_unlock			  (uint8_t sector);
void 	mem_lock			    (uint8_t sector);
void	mem_read		    (uint32_t address, uint8_t * data, uint8_t data_len);
uint8_t mem_read_byte(uint32_t address);
void mem_write_multibyte  (uint32_t address, uint8_t * data, uint8_t data_len);
void mem_multibyte_demo();

// Pins and Ports
#define MEM_CS 	      PB5
#define MEM_PORT      PORTB
#define MEM_DDR       DDRB


// Commands and Special registers
#define MEM_READ_STATUS			0x05
#define MEM_ERASE           0x60
#define MEM_WRITE_STATUS 		0x01
#define MEM_WR_ENABLE			0x06
#define	MEM_WR_DISABLE			0x04
#define	MEM_WR_STATUS_ENABLE	0x50
#define MEM_WR_BYTE				0x02
#define MEM_WR_AAI         0xAD
#define MEM_BUSY_ENABLE   0x70
#define MEM_BUSY_DISABLE  0x80
#define MEM_R_BYTE				0x03
#define MEM_FAST_READ			0x0B

// Status bits
#define BUSY	0
#define WEL		1
#define BP0		2
#define BP1		3
#define BP2		4
#define BP3		5
#define AAI		6
#define BPL		7

#define MEM_ALL_SECTORS		0x3C


#endif
