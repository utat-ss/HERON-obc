/*
A program to configure OBC before launch (e.g. erase memory, check EEPROM erased,
set RTC to date/time 0).
*/

#include "../../src/antenna.h"

void config_comms_delay_eeprom(void) {
    write_eeprom(PHASE2_DELAY_DONE_EEPROM_ADDR, EEPROM_DEF_DWORD);
}

// Comment/uncomment functionality as desired
int main(void){
    init_uart();

    print("\n\n");
    print("Starting pre launch configuration\n");

    config_comms_delay_eeprom();
   
    print("Done pre launch configuration\n");

    return 0;
}
