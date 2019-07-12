/*
 * Pre-flight diagnostic for OBC
 * Performs a brief test of the functionality of the RTC and EEPROM Flash Memory to
 * verify that components are responding as expected */

#include <stdlib.h>

#include <test/test.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/mem.h"
#include "../../src/rtc.h"

 #define TEST_SIZE        4
 #define DATA_LENGTH      5
 #define RANDOM_SEED      0x5729AB7D
 #define ERASE_SEED       0x162FAF13
 #define ERASE_ADDR_COUNT 3

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

void rtc1_test(void){

}

void rtc2_test(void){

}

test_t t1 = { .name = "read/write mem test", .fn = read_write_erase_mem_test };
test_t t2 = { .name = "eeprom test", .fn = test_eeprom };
test_t t3 = { .name = "rtc1 test", .fn = rtc1_test };
test_t t4 = { .name = "rtc2 test", .fn = rtc2_test };

test_t* suite[] = { &t1, &t2, &t3, &t4 };

int main(void) {
    init_uart();
    init_spi();
    init_mem();

    run_tests(suite, TEST_SIZE);
    return 0;
}
