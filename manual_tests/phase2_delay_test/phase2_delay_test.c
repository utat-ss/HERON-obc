/*
This program tests `run_phase2_delay()`. It writes to EEPROM and should restore it if it resets.

To test, you can run the command `make read-eeprom` while the program is
running. This will reset the MCU when you read the contents of the EEPROM from a
laptop. Check the address COMMS_TIME_EEPROM_ADDR (should be 0x34) for the
done flag. Also check that this value is restored properly when the program
restarts.
*/

#include <stdlib.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/antenna.h"
#include "../../src/general.h"

void reset_eeprom(void) {
    write_eeprom(PHASE2_DELAY_DONE_EEPROM_ADDR, EEPROM_DEF_DWORD);
    print("Reset EEPROM dword\n");
}

int main(void) {
    init_uart();
    init_ant();

    print("\n\n");
    print("Starting comms delay test\n");

    phase2_delay.period_s = 15;
    print("Using 15 seconds\n");

    run_phase2_delay();

    // Optional
    // reset_eeprom();

    print("Done comms delay test\n");
}
