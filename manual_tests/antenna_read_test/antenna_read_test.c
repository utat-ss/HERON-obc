/*
THIS PROGRAM TESTS ONLY AN I2C READ, NOT A WRITE.

This is used to verify the real antenna can be communicated with over I2C
without actually deploying it.

MAKE SURE TO SET THE CURRENT LIMIT ON THE POWER SUPPLY TO JUST ABOVE THE NOMINAL
CURRENT DRAW. THIS MITIGATES RISK JUST IN CASE THE ANTENNA IS ACCIDENTALLY
DEPLOYED.
*/

#include "../../src/antenna.h"

int main(void) {
    init_uart();
    init_spi();
    init_i2c();
    init_ant();

    print("\n\n\nStarting antenna read test\n\n");

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        write_i2c_reg(I2C_CLOCK, 5);
    }

    // Check Ant doors before the release algorithm FB1
    uint8_t door_positions[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t mode = 0x00;
    uint8_t main_heaters[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t backup_heaters[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t timer_s = 0x00;
    uint8_t i2c_status = 0x00;

    print("Reading...\n");
    uint8_t ret = read_antenna_data(door_positions, &mode, main_heaters, backup_heaters, &timer_s, &i2c_status);
    print("Returned %u\n", ret);

    print("Done\n");
}
