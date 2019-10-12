/*
This program tests writing to and reading from specific sections and fields
in memory for the layout scheme we have defined.
*/

// standard C libraries
#include    <stdbool.h>
#include    <stdint.h>
#include    <stdlib.h>

// avr-libc includes
#include    <avr/io.h>

// lib-common includes
#include    <spi/spi.h>
#include    <uart/uart.h>
#include    <can/can.h>

// project file includes
#include     "../../src/mem.h"
#include     "../../src/rtc.c"

void print_section(char* name, mem_section_t* section) {
    print("%s: start_addr = 0x%.8lx, ",
        name, section->start_addr);
    print("curr_block = %lu, fields_per_block = %u\n",
        section->curr_block, section->fields_per_block);
}

void print_sections(void) {
    print_section("EPS_HK", &eps_hk_mem_section);
    print_section("PAY_HK", &pay_hk_mem_section);
    print_section("PAY_OPT", &pay_opt_mem_section);
}

void print_header(mem_header_t* header) {
    print("block_num = %lu, status = %u, ",
        header->block_num, header->status);
    print("date = %u:%u:%u, time = %u:%u:%u\n",
        header->date.yy, header->date.mm, header->date.dd,
        header->time.hh, header->time.mm, header->time.ss);
}

void set_rtc(void) {
    rtc_time_t curr_time;
    curr_time.ss = 00;
    curr_time.mm = 00;
    curr_time.hh = 15;

    rtc_date_t curr_date;
    curr_date.dd = 06;
    curr_date.mm = 10;
    curr_date.yy = 18;

    set_rtc_time(curr_time);
    set_rtc_date(curr_date);
    print("Set RTC date and time\n");
}

void test_eeprom(void) {
    print_sections();
    print("\n");

    read_all_mem_sections_eeprom();
    print("Loaded section data from EEPROM\n");
    print_sections();
    print("\n");

    set_mem_section_curr_block(&eps_hk_mem_section, eps_hk_mem_section.curr_block + 1);
    set_mem_section_curr_block(&pay_hk_mem_section, pay_hk_mem_section.curr_block + 1);
    set_mem_section_curr_block(&pay_opt_mem_section, pay_opt_mem_section.curr_block + 1);
    print("Incremented all current blocks, wrote to EEPROM\n");
}


void test_header(char* name, mem_section_t* section) {
    print("\nTesting %s section - header\n", name);

    mem_header_t write = {
        .block_num = section->curr_block,
        .date = read_rtc_date(),
        .time = read_rtc_time(),
        .status = 0x00,
    };
    mem_header_t read;

    read_mem_header(section, section->curr_block, &read);
    print("Initial value in memory: ");
    print_header(&read);

    write_mem_header(section, section->curr_block, &write);
    print("Wrote header to memory: ");
    print_header(&write);

    read_mem_header(section, section->curr_block, &read);
    print("Final value in memory: ");
    print_header(&read);
}


void test_field(char* name, mem_section_t* section) {
    print("\nTesting %s section - field\n", name);

    uint8_t field_num = random() % section->fields_per_block;
    print("field_num = %u\n", field_num);

    // Example field data to write
    uint32_t write = random() & 0xFFFFFF;
    // Field data read back, should be the same
    uint32_t read = 0;

    // Check to see initial value
    read = read_mem_field(section, section->curr_block, field_num);
    print("Initial value in memory: 0x%.6lx\n", read);

    write_mem_field(section, section->curr_block, field_num, write);
    print("Wrote field to memory: 0x%.6lx\n", write);

    read = read_mem_field(section, section->curr_block, field_num);
    print("Final value in memory: 0x%.6lx\n", read);
}


// Need to change this manually
#define RANDOM_SEED 0x48BCDE14

int main(void) {
    init_uart();
    init_spi();
    init_mem();
    init_rtc();

    print("\n\n\n**Starting test**\n\n");

    srandom(RANDOM_SEED);
    print("Random seed = 0x%.8lx\n", RANDOM_SEED);

    set_rtc();
    test_eeprom();

    /*
    What should happen in this test:
    1. Initializes all sections
    2. Write field value: 0x070914 into PAY OPT section
    3. Read back the same values from PAY OPT
    */

    test_header("EPS_HK", &eps_hk_mem_section);
    test_header("PAY_HK", &pay_hk_mem_section);
    test_header("PAY_OPT", &pay_opt_mem_section);

    test_field("EPS_HK", &eps_hk_mem_section);
    test_field("PAY_HK", &pay_hk_mem_section);
    test_field("PAY_OPT", &pay_opt_mem_section);

    print ("\n*** End of test ***\n");
}
