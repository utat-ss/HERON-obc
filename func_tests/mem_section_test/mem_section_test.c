/*
This program tests writing to and reading from specific sections and fields
in memory for the layout scheme we have defined.
*/

// standard C libraries
#include    <stdbool.h>
#include    <stdint.h>

// avr-libc includes
#include    <avr/io.h>
#define F_CPU 8000000UL
#include    <util/delay.h>

// lib-common includes
#include    <spi/spi.h>
#include    <uart/uart.h>
#include    <can/can.h>

// project file includes

#include     "../../src/mem.h"
#include     "../../src/rtc.c"

int main(void) {
    init_uart();
    init_spi();
    init_mem();
    init_rtc();

    time_t curr_time;
    curr_time.ss = 00;
    curr_time.mm = 00;
    curr_time.hh = 15;

    date_t curr_date;
    curr_date.dd = 06;
    curr_date.mm = 10;
    curr_date.yy = 18;

    set_time(curr_time);
    set_date(curr_date);

    /*
    What should happen in this test:
    1. Initializes all stacks- can be checked with those EEPROM addresses
    print statements
    2. Write values: 0x08, 0x07, 0x06, 0x08 into SCI STACK
    3. Read back the same values from SCI STACK- there should be a header
    FFF and then these 4 values.
    */

    print ("******EEPROM addresses are:\n");
    print ("Pay sci section address curr ptr: %lx\n", pay_sci_mem_section.start_addr);
    print ("Pay housekeeping section address curr ptr: %x\n", pay_hk_mem_section.start_addr);

    // Example field data to write
    uint32_t write_test = 0x070914;
    // Field data read back, should be the same
    uint32_t read_test = 0;

    // Check to see if values had been initialized (mem_erase)
    read_test = read_mem_field(&pay_sci_mem_section, pay_sci_mem_section.curr_block, 0x00);
    print ("Initial values in Pay sci address:\n");
    print ("%lx\n", read_test);

    write_mem_header(&pay_sci_mem_section, pay_sci_mem_section.curr_block);
    write_mem_field(&pay_sci_mem_section, pay_sci_mem_section.curr_block, 0x00, write_test);

    // print ("\n***********Read back from expected address: %x******\n", pay_sci_mem_section.start_addr+block_size(SCI_TYPE));
    read_test = read_mem_field(&pay_sci_mem_section, pay_sci_mem_section.curr_block, 0x00);
    print ("%lx\n", read_test);

    // Test chip rollover
    uint32_t address[] = {0x00FFFFFD, 0x01FFFFFD, 0x02FFFFFD};
    // Chip number: (address >> 24) & 0x03

    // Testing all 3 chip rollover conditions
    for (uint8_t i = 0; i < 3; i ++) {
        write_mem_header((long) address[i], pay_sci_mem_section.curr_block);
        write_mem_field((long) address[i], pay_sci_mem_section.curr_block, 0x00, write_test);
        read_test = read_mem_field((long) address[i], pay_sci_mem_section.curr_block, 0x00);

        print ("Testing chip rollover for address %lx\n", address[i]);
        print ("Values in Pay sci address after writing 0x070914:\n");
        print ("%lx\n", read_test);
    }

    print ("*** End of test******\n");
}
