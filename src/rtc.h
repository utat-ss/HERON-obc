#ifndef RTC_H
#define RTC_H

// AVR Library Includes
#include 	<avr/io.h>
#include 	<util/delay.h>

// lib-common includes
#include 	<spi/spi.h>
#include 	<uart/uart.h>
#include 	<uart/log.h>


// Type definitions
typedef struct Time time_t;
struct Time {
	uint8_t ss;
	uint8_t mm;
	uint8_t hh; //24 hour clock!
};

typedef struct Date date_t;
struct Date{
	uint8_t dd;
	uint8_t mm;
	uint8_t yy; // 00 = 2000; 99 = 2099; century always 0.
};

// Basic Functions
void 	init_rtc	();
void 	rtc_write	(uint8_t reg_address, uint8_t data);
uint8_t rtc_read 	(uint8_t reg_address);

//T Time and Date
void 	set_time	(time_t time);
time_t 	read_time	();
void 	set_date	(date_t date);
date_t 	read_date	();

// Helper functions
uint8_t bcd_to_dec	(uint8_t bcd);
uint8_t dec_to_bcd	(uint8_t dec);

// testing functions
void 	rtc_time_demo	();


// Pins and Ports
#define RTC_CS 	      PB6
#define RTC_PORT      PORTB
#define RTC_DDR       DDRB

#define LED_CS		  PD0
#define LED_PORT	  PORTD
#define LED_DDR		  DDRD

// R/W Control
#define		RTC_R			0x00
#define		RTC_W			0x80

// Special Register Addresses
#define		RTC_CTRL_R		0x0E
#define		RTC_STATUS_R	0x0F

// Date/Time Register Addresses
#define 	RTC_SEC_R		0x00
#define		RTC_MIN_R		0x01
#define		RTC_HOUR_R		0x02
#define		RTC_WKDAY_R		0x03
#define		RTC_DAY_R		0x04
#define		RTC_MONTH_R		0x05
#define		RTC_YEAR_R		0x06


// Control register bits
#define 	EOSC_N			7
#define		BBSQW			6
#define 	CONV			5
#define 	RS2				4
#define 	RS1				3
#define 	INTCN			2
#define		A2IE			1
#define		A1IE			0

// Status register bits
#define		OSF				7
#define 	BB32kHz			6
#define		CRATE1			5
#define 	CRATE0			4
#define 	EN32kHz			3
#define		BSY				2
#define 	A2F				1
#define		A1F				0

// Default Register Values
#define		RTC_CTRL_DEF	_BV(INTCN)
#define 	RTC_STATUS_DEF	0x00



#endif