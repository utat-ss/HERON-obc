// standard C ;ibraries
#include  	<stdbool.h>
#include  	<stdint.h>

// avr-libc includes
#include 	<avr/io.h>
#include 	<util/delay.h>

// lib-common includes
#include 	<spi/spi.h>
#include 	<uart/uart.h>
#include 	<uart/log.h>
#include 	<can/can.h>

// project file includes

#include 	"../../src/mem.h"
#include 	"../../src/rtc.c"

int main(void) {
    init_uart();
    init_spi();
    init_mem();
    init_rtc();
    init_curr_stack_ptrs();

    _delay_ms (5000);

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

    int j;

    for (j=0;j<4;j++) {
        uint8_t read_test [10] = {0};
        uint8_t test [4]= {0x08, 0x07, 0x06, 0x08};
      /*  for(i=0; i<4; i++) {
          print ("%x\n", test[i]);
        }*/
        write_to_flash(SCI_TYPE, j, test);
        print ("\n***********Read back from expected address: %x******\n", 0x250+8+SCI_INIT+(4*j));
        mem_read(0x250+8+(SCI_INIT+(4*j)), &read_test, 10);
        for(i=0; i<10; i++) {
          print ("%x\n", read_test[i]);
        }

  }

    print ("*** End of test******\n");

}
