// antenna microcontroller datasheet: http://www.ti.com/lit/ds/slas885a/slas885a.pdf

#include "antenna.h"

// #define ANTENNA_DEBUG


void init_ant(void) {
    // Initialize pins
    init_output_pin(ANT_REL_A, &DDR_ANT_REL, 0);
    init_output_pin(ANT_REL_B, &DDR_ANT_REL, 0);
    init_output_pin(ANT_DEP_WARN, &DDR_ANT_WARN, 0);
}

/*
NOTE: Must call init_spi() followed by init_i2c() before this function
*/
void deploy_antenna(void) {
    // 10 second delay before start deploying antenna
    for (uint32_t seconds = 0; seconds < 10; seconds++) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);
        print("Antenna would normally deploy now! (no dep version)\n");

        for (uint8_t i = 0; i < 10; i++) {
            // blink antenna warn light
            set_pin_high(ANT_DEP_WARN, &PORT_ANT_WARN);
            _delay_ms(50);
            set_pin_low(ANT_DEP_WARN, &PORT_ANT_WARN);
            _delay_ms(50);
        }
    }

    print("Skipped deployment! (no dep version)\n");
}

/*
Send a read command via I2C to the antenna
door_positions - 4-byte array ([0] is D1, etc.)
mode - single byte
main_heaters - 4-byte array ([0] is A1, etc.)
backup_heaters - 4-byte array ([0] is B1, etc.)
returns - 1 for I2C success, 0 for failure
*/
uint8_t read_antenna_data(uint8_t* door_positions, uint8_t* mode,
        uint8_t* main_heaters, uint8_t* backup_heaters, uint8_t* timer_s,
        uint8_t* i2c_status) {
    return 0;
}

// Execute algorithm 1 for all antenna rods
uint8_t write_antenna_alg1(uint8_t* i2c_status) {
    return 0;
}

// Execute algorithm 2 for specified antenna rod(s)
uint8_t write_antenna_alg2(uint8_t ant_num_in_bytes, uint8_t* i2c_status) {
    return 0;
}

// Clear and interrupt all antenna commands
uint8_t write_antenna_clear(uint8_t* i2c_status) {
    return 0;
}
