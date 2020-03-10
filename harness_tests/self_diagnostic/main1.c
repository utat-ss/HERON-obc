/*
 * Pre-flight diagnostic for OBC
 * Performs a brief test of the functionality of the RTC and EEPROM Flash Memory to
 * verify that components are responding as expected */

#include <stdlib.h>
#include <test/test.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include <util/atomic.h>

#include "../../src/general.h"
#include "../../src/mem.h"
#include "../../src/rtc.h"

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

#define ASSERT_BYTES_EQ(bytes1, bytes2, count)  \
    for (uint8_t __i = 0; __i < (count); __i++) { \
        ASSERT_EQ((bytes1)[__i], (bytes2)[__i]);    \
    }

void populate_bytes(uint8_t* bytes, uint8_t value, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        bytes[i] = value;
    }
}


void restart_info_test(void){
    ASSERT_GREATER(uptime_s, 1); /* 1 second */
    ASSERT_LESS(uptime_s, 60); /* 60 seconds */

    ASSERT_EQ(restart_count, 1);

    ASSERT_EQ(restart_reason, UPTIME_RESTART_REASON_EXTRF);

    ASSERT_GREATER(restart_date.yy + 1, 0);
    ASSERT_LESS(restart_date.yy, 99 + 1);
    ASSERT_GREATER(restart_date.mm + 1, 1);
    ASSERT_LESS(restart_date.mm, 12 + 1);
    ASSERT_GREATER(restart_date.dd + 1, 1);
    ASSERT_LESS(restart_date.dd, 31 + 1);

    ASSERT_GREATER(restart_time.hh + 1, 0);
    ASSERT_LESS(restart_time.hh, 23 + 1);
    ASSERT_GREATER(restart_time.mm + 1, 0);
    ASSERT_LESS(restart_time.mm, 59 + 1);
    ASSERT_GREATER(restart_time.ss + 1, 0);
    ASSERT_LESS(restart_time.ss, 59 + 1);
}

/* Write and read from each chip in flash memory to verify
  that all are working as expected */
void read_write_erase_mem_test(void){
    srandom(RANDOM_SEED);

    uint8_t zeros[DATA_LENGTH];
    populate_bytes(zeros, 0x00, DATA_LENGTH);

    uint32_t addr_per_chip = MEM_NUM_ADDRESSES/3;
    uint8_t data_array[DATA_LENGTH] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    uint8_t read[DATA_LENGTH];
    populate_bytes(read, 0x00, DATA_LENGTH);
    ASSERT_BYTES_EQ(read, zeros, DATA_LENGTH);

    /* Write to one address per chip */
    erase_mem();
    for (uint8_t chip = 0; chip < MEM_NUM_CHIPS; chip++){
        populate_bytes(read, 0x00, DATA_LENGTH);
        ASSERT_BYTES_EQ(read, zeros, DATA_LENGTH);

        uint32_t addr = (addr_per_chip*chip) + (random() % addr_per_chip);
        ASSERT_NEQ(addr, 0);

        write_mem_bytes(addr, data_array, DATA_LENGTH);
        read_mem_bytes(addr, read, DATA_LENGTH);
        ASSERT_BYTES_EQ(read, data_array, DATA_LENGTH);
    }

    /* Check that 3 addresses per chip are erased correctly */
    erase_mem();
    for (uint8_t chip = 0; chip < MEM_NUM_CHIPS; chip++){
        for (uint8_t i = 0; i < ERASE_ADDR_COUNT; i++){
            uint32_t addr_erase = (addr_per_chip*chip) + (random() % addr_per_chip);
            ASSERT_NEQ(addr_erase, 0);

            read_mem_bytes(addr_erase, read, 1);
            ASSERT_EQ(read[0], 0xFF);
        }
    }
}

/* Load all data from eeprom and verify that data is as expected */
void test_eeprom(void){
    read_all_mem_sections_eeprom();

    uint32_t eps_hk_block_prev = read_eeprom(eps_hk_mem_section.curr_block_eeprom_addr);
    uint32_t pay_hk_block_prev = read_eeprom(pay_hk_mem_section.curr_block_eeprom_addr);
    uint32_t pay_opt_block_prev = read_eeprom(pay_opt_mem_section.curr_block_eeprom_addr);

    set_mem_section_curr_block(&eps_hk_mem_section, eps_hk_mem_section.curr_block + 1);
    set_mem_section_curr_block(&pay_hk_mem_section, pay_hk_mem_section.curr_block + 1);
    set_mem_section_curr_block(&pay_opt_mem_section, pay_opt_mem_section.curr_block + 1);

    ASSERT_EQ(eps_hk_block_prev + 1, read_eeprom(eps_hk_mem_section.curr_block_eeprom_addr));
    ASSERT_EQ(pay_hk_block_prev + 1, read_eeprom(pay_hk_mem_section.curr_block_eeprom_addr));
    ASSERT_EQ(pay_opt_block_prev + 1, read_eeprom(pay_opt_mem_section.curr_block_eeprom_addr));
}

/* Test setting and reading a time on the RTC */
void rtc_date_time_test(void){
    rtc_time_t time_set = {.ss = 4, .mm = 8, .hh = 16};
    rtc_date_t date_set = {.dd = 5, .mm = 10, .yy = 20};
    set_rtc_time(time_set);
    set_rtc_date(date_set);

    rtc_time_t time_read;
    rtc_date_t date_read;

    for (uint8_t i = 0;i < 3; i++){
        time_read = read_rtc_time();
        date_read = read_rtc_date();
        ASSERT_EQ_TIME(time_set, time_read);
        ASSERT_EQ_DATE(date_set, date_read);
        _delay_ms(2000);
        time_set.ss +=2;
    }
}

// ONLY READ, NOT WRITE
void antenna_read_test(void) {
    print("Reading antenna data in 5 seconds...\n");
    _delay_ms(5000);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        write_i2c_reg(I2C_CLOCK, 5);
    }

    // Check Ant doors before the release algorithm FB1
    uint8_t door_positions[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t mode = 0x00;
    uint8_t main_heaters[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t backup_heaters[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t timer_s = 0x00;
    uint8_t i2c_status = 0x00;

    print("Reading now...\n");
    uint8_t ret = read_antenna_data(door_positions, &mode, main_heaters, backup_heaters, &timer_s, &i2c_status);

    ASSERT_TRUE(ret);
    ASSERT_EQ(mode, 0);
    ASSERT_EQ(timer_s, 0);
    ASSERT_EQ(i2c_status, I2C_SUCCESS);
    for (uint8_t i = 0; i < 4; i++) {
        ASSERT_EQ(door_positions[i], 0);
        ASSERT_EQ(main_heaters[i], 0);
        ASSERT_EQ(backup_heaters[i], 0);
    }

    print("Done reading\n");
}


test_t t1 = { .name = "restart info test", .fn = restart_info_test };
test_t t2 = { .name = "read/write mem test", .fn = read_write_erase_mem_test };
test_t t3 = { .name = "eeprom test", .fn = test_eeprom };
test_t t4 = { .name = "rtc date/time test", .fn = rtc_date_time_test };
test_t t5 = { .name = "antenna read test", .fn = antenna_read_test };

test_t* suite[] = { &t1, &t2, &t3, &t4, &t5 };

int main(void) {
    WDT_OFF();
    init_obc_phase1_core();
    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
