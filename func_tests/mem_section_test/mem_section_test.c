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

    print("\n\n\nStarting test\n");

    //erase all three memory chips
    for(uint8_t i = 0; i < MEM_NUM_CHIPS; i++) {
        erase_mem(i);
    }

    print("\nErased memory\n\n");

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
    1. Initializes all sections
    2. Write field value: 0x070914 into PAY SCI section
    3. Read back the same values from PAY SCI
    */

    print ("PAY HK section address: %.6lx\n", pay_hk_mem_section.start_addr);
    print ("PAY SCI section address: %.6lx\n", pay_sci_mem_section.start_addr);

    // Example field data to write
    uint32_t write_test = 0x07091B;
    // Field data read back, should be the same
    uint32_t read_test = 0;

    // Check to see if values had been initialized (mem_erase)
    read_test = read_mem_field(
        &pay_sci_mem_section, pay_sci_mem_section.curr_block, 0x03);
    print ("Initial value in PAY SCI address: %.6lx\n", read_test);

    write_mem_header(
        &pay_sci_mem_section, pay_sci_mem_section.curr_block);
    write_mem_field(
        &pay_sci_mem_section, pay_sci_mem_section.curr_block, 0x03, write_test);

    read_test = read_mem_field(
        &pay_sci_mem_section, pay_sci_mem_section.curr_block, 0x03);
    print ("Final value in PAY SCI address: %.6lx\n", read_test);

    print ("*** End of test ***\n");
}
