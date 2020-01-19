/*
This program writes the flag for the phase 2 delay done to EEPROM.
You can run this before running the main code to make it skip the delay.
*/

#include <uart/uart.h>
#include "../../src/general.h"

int main(void) {
    init_uart();

    print("\n\n");
    write_eeprom(PHASE2_DELAY_DONE_EEPROM_ADDR, PHASE2_DELAY_DONE_FLAG);
    print("Wrote phase 2 delay done to EEPROM\n");
}
