/*
This program tests `run_phase2_delay()`. It writes to EEPROM and should restore it if it resets.

To test, you can run the command `make read-eeprom` while the program is
running. This will reset the MCU when you read the contents of the EEPROM from a
laptop. Check the address PHASE2_DELAY_DONE_EEPROM_ADDR for the
done flag. Also check that this value is restored properly when the program
restarts.
*/

#include <stdlib.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/antenna.h"
#include "../../src/general.h"

void reset_done(void) {
    write_eeprom(PHASE2_DELAY_DONE_EEPROM_ADDR, EEPROM_DEF_DWORD);
    print("Reset EEPROM done flag\n");
}

void write_done(void) {
    write_eeprom(PHASE2_DELAY_DONE_EEPROM_ADDR, PHASE2_DELAY_DONE_FLAG);
    print("Wrote EEPROM done flag\n");
}

int main(void) {
    init_uart();
    init_uptime();

    print("\n\n");
    print("Starting phase 2 delay test\n");

    phase2_delay.period_s = 15;
    print("Using 15 second period\n");

    init_phase2_delay();

    while (phase2_delay.in_progress) {
        run_phase2_delay();
    }

    // Optional
    // reset_done();
    // write_done();

    print("Done comms delay test\n");
}
