/*
This program tests initialization of the restart count (read EEPROM, increment,
write EEPROM) and continuous updating of the OBC uptime.
*/

#include "../../src/uptime.h"

int main(void) {
    init_uart();

    print("\n\n\nStarting uptime test\n\n");

    init_restart_count();
    print("restart_count = %lu\n", restart_count);
    print("uptime_s = %lu\n", uptime_s);

    start_uptime_timer();
    print("Started uptime timer\n");

    // Make sure the program doesn't end here
    while (1) {}
}
