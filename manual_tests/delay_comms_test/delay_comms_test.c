/*
This program tests `delay_comms()` with the timer. It writes to EEPROM and should restore it if reset.

To test, you can run the command `make read-eeprom` while the program is
running. This will reset the MCU when you read the contents of the EEPROM from a
laptop. Check the address COMMS_TIME_EEPROM_ADDR (should be 0x34) for the
correct number of seconds written. Also check that this value is restored
properly when the program restarts.
*/

#include <stdlib.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/general.h"

int main(void) {
    init_uart();

    print("\n\n\nStarting delay comms test\n\n");

    init_obc_core();
    print("restart_count = %lu\n", restart_count);
    print("comms_time_s = %lu\n", comms_time_s);

    // comms_time_s = 0;
    // print("set comms_time_s = %lu\n", comms_time_s);
    
    comms_time_threshold_s = 10 * 60;
    print("set comms_time_threshold_s = %lu\n", comms_time_threshold_s);
    delay_comms();

    print("\nDone delay comms test\n\n\n");

    while (1) {}
}
