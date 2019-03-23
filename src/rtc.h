#ifndef RTC_H
#define RTC_H

// AVR Library Includes
#include <avr/io.h>
#include <avr/interrupt.h>

// lib-common includes
#include <spi/spi.h>
#include <uart/uart.h>


// Type definitions
typedef struct {
    uint8_t ss; //seconds
    uint8_t mm; //minutes
    uint8_t hh; //hours, 24 hour clock!
} rtc_time_t;

typedef struct {
    uint8_t dd; //date of the month (NOT "day", which means day of the week)
    uint8_t mm; //month
    uint8_t yy; // 00 = 2000; 99 = 2099; century always 0.
} rtc_date_t;

// Alarms 1 and 2
typedef enum {
    RTC_ALARM_1,
    RTC_ALARM_2
} rtc_alarm_t;

// Callback function signatures for alarms
typedef void (*alarm_fn_t)(void);

// Basic Functions
void init_rtc(void);

// Time and Date
void set_rtc_time(rtc_time_t time);
rtc_time_t read_rtc_time(void);
rtc_date_t read_rtc_date(void);
void set_rtc_date(rtc_date_t date);

//Alarm functions
uint8_t set_rtc_alarm(rtc_time_t time, rtc_date_t date,
    rtc_alarm_t alarm_number, alarm_fn_t cmd);
uint8_t disable_rtc_alarm(rtc_alarm_t alarm_number);

// Read/write registers
uint8_t rtc_read(uint8_t reg_address);
void rtc_write(uint8_t reg_address, uint8_t data);

// SPI
void start_rtc_spi(void);
void end_rtc_spi(void);

// Helper functions
uint8_t rtc_bcd_to_dec(uint8_t bcd);
uint8_t rtc_dec_to_bcd(uint8_t dec);


// Pins and Ports
#define RTC_CS      PB5
#define RTC_PORT    PORTB
#define RTC_DDR     DDRB

// R/W Control
#define RTC_R   0x00
#define RTC_W   0x80

// Special Register Addresses
#define RTC_CTRL_R      0x0E
#define RTC_STATUS_R    0x0F

// Date/Time Register Addresses
#define RTC_SEC_R       0x00
#define RTC_MIN_R       0x01
#define RTC_HOUR_R      0x02
#define RTC_WKDAY_R     0x03
#define RTC_DAY_R       0x04
#define RTC_MONTH_R     0x05
#define RTC_YEAR_R      0x06


// Control register bits
#define RTC_EOSC_N  7
#define RTC_BBSQW   6
#define RTC_CONV    5
#define RTC_RS2     4
#define RTC_RS1     3
#define RTC_INTCN   2
#define RTC_A2IE    1
#define RTC_A1IE    0

// Status register bits
#define RTC_OSF     7
#define RTC_BB32KHZ 6
#define RTC_CRATE1  5
#define RTC_CRATE0  4
#define RTC_EN32KHZ 3
#define RTC_BSY     2
#define RTC_A2F     1
#define RTC_A1F     0

// Default Register Values
#define RTC_CTRL_DEF    _BV(RTC_INTCN)
#define RTC_STATUS_DEF  0x00

// Alarm Mask Bits
#define RTC_ALARM_1_SEC_R   0X07
#define RTC_ALARM_1_MIN_R   0X08
#define RTC_ALARM_1_HOUR_R  0X09
#define RTC_ALARM_1_DAY_R   0X0A
#define RTC_ALARM_2_MIN_R   0X0B
#define RTC_ALARM_2_HOUR_R  0X0C
#define RTC_ALARM_2_DAY_R   0X0D

#endif
