// standard C libraries
#include    <stdbool.h>
#include    <stdint.h>

// avr-libc includes
#include    <avr/io.h>
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
    init_curr_stack_ptrs();
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
    uint8_t i;

    //  while (1) {
    print ("******EEPROM addresses are:\n");
    print ("Sci stack address curr ptr: %x\n", eeprom_read_dword(SCI_CURR_PTR_ADDR));
    print ("Pay housekeeping stack address curr ptr: %x\n", eeprom_read_dword(PAY_HK_CURR_PTR_ADDR));


    uint8_t read_test [12] = {1};
    uint8_t test [4]= {0x08, 0x07, 0x09, 0x14};

    write_to_flash(SCI_TYPE, 0x00, test);
    print ("\n***********Read back from expected address: %x******\n", init_stack(SCI_TYPE)+block_size(SCI_TYPE));
    read_from_flash(SCI_TYPE, read_test, 12, 0x1, 0x00);
    for(i=0; i<12; i++)
    {
        print ("%x\n", read_test[i]);
    }

    print ("*** End of test******\n");

}
