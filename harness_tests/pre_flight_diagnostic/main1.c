/*
 * Pre-flight diagnostic for OBC
 * Performs a brief test of the functionality of the RTC and EEPROM Flash Memory to
 * verify that components are responding as expected */

#include <stdlib.h>
#include <test/test.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include <util/atomic.h>

#include "../../src/mem.h"
#include "../../src/rtc.h"

#define TEST_SIZE        4
#define DATA_LENGTH      5
#define RANDOM_SEED      0x5729AB7D
#define ERASE_SEED       0x162FAF13
#define ERASE_ADDR_COUNT 3

/* Assertion macros to test equality of date and time structs */
#define ASSERT_EQ_DATE(date1, date2) \
    ASSERT_EQ(date1.yy, date2.yy); \
    ASSERT_EQ(date1.mm, date2.mm); \
    ASSERT_EQ(date1.dd, date2.dd);

#define ASSERT_EQ_TIME(time1, time2) \
    ASSERT_EQ(time1.hh, time2.hh); \
    ASSERT_EQ(time1.mm, time2.mm); \
    ASSERT_EQ(time1.ss, time2.ss);

volatile uint8_t alarm_byte = 0x00;

void rtc_alarm(void){
    alarm_byte = 0xFF;
}
/* Write and read from each chip in flash memory to verify
  that all are working as expected */
void read_write_erase_mem_test(void){
    srandom(RANDOM_SEED);
    uint32_t addr_per_chip = MEM_NUM_ADDRESSES/3;
    uint8_t data_array[DATA_LENGTH] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    uint8_t read[1] = {0};

    /* Write to one address per chip */
    erase_mem();
    for (uint8_t chip = 0; chip < MEM_NUM_CHIPS; chip++){
    uint32_t addr = (addr_per_chip*chip) + (random() % addr_per_chip);
    write_mem_bytes(addr, data_array, DATA_LENGTH);
        for (uint32_t i = addr; i < (addr + DATA_LENGTH); i++){
            read_mem_bytes(i, read, 1);
            ASSERT_EQ(read[0], data_array[i-addr]);
        }
    }

    /* Check that 3 addresses per chip are erased correctly */
    erase_mem();
    for (uint8_t chip = 0; chip < MEM_NUM_CHIPS; chip++){
        for (uint8_t i = 0; i < ERASE_ADDR_COUNT; i++){
            uint32_t addr_erase = (addr_per_chip*chip) + (random() % addr_per_chip);
            read_mem_bytes(addr_erase, read, 1);
            ASSERT_EQ(read[0], 0xFF);
        }
    }
}

/* Load all data from eeprom and verify that data is as expected */
void test_eeprom(void){
    read_all_mem_sections_eeprom();

    uint32_t eps_hk_block_prev = eeprom_read_dword(eps_hk_mem_section.curr_block_eeprom_addr);
    uint32_t pay_hk_block_prev = eeprom_read_dword(pay_hk_mem_section.curr_block_eeprom_addr);
    uint32_t pay_opt_block_prev = eeprom_read_dword(pay_opt_mem_section.curr_block_eeprom_addr);

    inc_mem_section_curr_block(&eps_hk_mem_section);
    inc_mem_section_curr_block(&pay_hk_mem_section);
    inc_mem_section_curr_block(&pay_opt_mem_section);

    ASSERT_EQ(eps_hk_block_prev + 1, eeprom_read_dword(eps_hk_mem_section.curr_block_eeprom_addr));
    ASSERT_EQ(pay_hk_block_prev + 1, eeprom_read_dword(pay_hk_mem_section.curr_block_eeprom_addr));
    ASSERT_EQ(pay_opt_block_prev + 1, eeprom_read_dword(pay_opt_mem_section.curr_block_eeprom_addr));
}

/* Test setting and reading a time on the RTC */
void rtc_date_time_test(void){
    rtc_time_t time_set = {.ss = 4, .mm = 8, .hh = 16};
    rtc_date_t date_set = {.dd = 5, .mm = 10, .yy = 20};

    rtc_time_t time_read;
    rtc_date_t date_read;

    set_rtc_time(time_set);
    set_rtc_date(date_set);

    for (uint8_t i = 0;i < 3; i++){
        time_read = read_rtc_time();
        date_read = read_rtc_date();
        ASSERT_EQ_TIME(time_set, time_read);
        ASSERT_EQ_DATE(date_set, date_read);
        _delay_ms(2000);
        time_set.ss +=2;
    }
}

/* Set one alarm and verify that it changes alarm_byte when expected */
void rtc_alarm_test(void){
    rtc_time_t time_set = {.ss = 4, .mm = 9, .hh = 16};
    rtc_date_t date_set = {.dd = 4, .mm = 10, .yy = 20};
    rtc_time_t alarm_time_set = {.ss = 9, .mm = 9, .hh = 16};

    set_rtc_time(time_set);
    set_rtc_date(date_set);
    set_rtc_alarm(alarm_time_set, date_set, RTC_ALARM_1, rtc_alarm);

    while(!alarm_byte){
        /* wait until alarm_byte is triggered by alarm */
    }

    rtc_time_t time_read = read_rtc_time();
    rtc_date_t date_read = read_rtc_date();
    ASSERT_EQ_TIME(alarm_time_set, time_read);
    ASSERT_EQ_DATE(date_set, date_read);
}

test_t t1 = { .name = "read/write mem test", .fn = read_write_erase_mem_test };
test_t t2 = { .name = "eeprom test", .fn = test_eeprom };
test_t t3 = { .name = "rtc date/time test", .fn = rtc_date_time_test };
test_t t4 = { .name = "rtc alarm test", .fn = rtc_alarm_test };

test_t* suite[] = { &t1, &t2, &t3, &t4 };

int main(void) {
    init_uart();
    init_spi();
    init_mem();
    init_rtc();

    run_tests(suite, TEST_SIZE);
    return 0;
}
