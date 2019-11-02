// antenna microcontroller datasheet: http://www.ti.com/lit/ds/slas885a/slas885a.pdf

#include "antenna.h"

// #define ANTENNA_DEBUG

// Number of seconds to wait before initializing comms
volatile uint32_t comms_delay_s = COMMS_DELAY_DEF_S;

void init_ant(void) {
    // Initialize pins
    init_output_pin(ANT_REL_A, &DDR_ANT_REL, 0);
    init_output_pin(ANT_REL_B, &DDR_ANT_REL, 0);
    init_output_pin(ANT_DEP_WARN, &DDR_ANT_WARN, 0);
}

// Delays 30 minutes before we can init comms
// Fetches previous value in EEPROM to see if we have already finished this
void run_comms_delay(void) {
    if (read_eeprom(COMMS_DELAY_DONE_EEPROM_ADDR) == COMMS_DELAY_DONE_FLAG) {
        print("Already done comms delay\n");
        return;
    }

    print("Starting comms delay\n");

    // Use 100ms increments because that is tolerable for delay precision
    for (uint32_t seconds = comms_delay_s; seconds > 0; seconds--) {
        print("Time Remaining (seconds): %d\n", seconds);
        // reset watchdog timer
        WDT_ENABLE_SYS_RESET(WDTO_8S);
        // blink antenna warn light
        set_pin_high(ANT_DEP_WARN, &PORT_ANT_WARN);
        for (uint8_t hundred_ms = 0; hundred_ms < 5; hundred_ms++) {
            _delay_ms(100);
        }
        set_pin_low(ANT_DEP_WARN, &PORT_ANT_WARN);
        for (uint8_t hundred_ms = 0; hundred_ms < 5; hundred_ms++) {
            _delay_ms(100);
        }
    }

    write_eeprom(COMMS_DELAY_DONE_EEPROM_ADDR, COMMS_DELAY_DONE_FLAG);

    print("Comms delay done\n");
}

/*
NOTE: Must call init_spi() followed by init_i2c() before this function
*/
void deploy_antenna(void) {
    print("Antenna deploying now!\n");

    // 5 second delay before start deploying antenna
    for (uint32_t seconds = 0; seconds < 5; seconds++) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);
        for (uint8_t i = 0; i < 10; i++) {
            // blink antenna warn light
            set_pin_high(ANT_DEP_WARN, &PORT_ANT_WARN);
            _delay_ms(50);
            set_pin_low(ANT_DEP_WARN, &PORT_ANT_WARN);
            _delay_ms(50);
        }
    }

    // Set 369 kHz clock
    write_i2c_reg(I2C_CLOCK, 5);

    // Check Ant doors before the release algorithm FB1
    uint8_t door_positions[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t mode = 0x00;
    uint8_t main_heaters[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t backup_heaters[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t timer_s = 0x00;
    uint8_t i2c_status = 0x00;
    uint8_t timer = 0;

    write_antenna_clear(&i2c_status);
    _delay_ms(1000);

    read_antenna_data(door_positions, &mode, main_heaters, backup_heaters, &timer_s, &i2c_status);

    // Start algorithm 1 for all doors, each door takes a maximum of 15 seconds
    write_antenna_alg1(&i2c_status);
    read_antenna_data(door_positions, &mode, main_heaters, backup_heaters, &timer_s, &i2c_status);
    timer = 70; //Wait more than 15*4=60 seconds for algorithm to finish
    while (mode != 0 && timer > 0) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);
        _delay_ms(1000);
        timer -= 1;
        read_antenna_data(door_positions, &mode, main_heaters, backup_heaters, &timer_s, &i2c_status);
    }

    // Use algorithm 2 on doors not open
    read_antenna_data(door_positions, &mode, main_heaters, backup_heaters, &timer_s, &i2c_status);
    uint8_t doors_to_redeploy = 0x00;
    uint8_t num_doors = 0;
    for (uint8_t i = 0; i < 4; i ++) {
        // Door closed and heaters not on
        if (!door_positions[i]) {
            num_doors += 1;
            doors_to_redeploy = doors_to_redeploy | (0x01 << i);
        }
    }

    // Start Algorithm 2 for not opened doors
    uint8_t ret_status = write_antenna_alg2(doors_to_redeploy, &i2c_status);
    read_antenna_data(door_positions, &mode, main_heaters, backup_heaters, &timer_s, &i2c_status);
    // Wait ~35seconds per door for algorithm to finish
    timer = 35 * num_doors;
#ifdef ANTENNA_DEBUG
    print_bytes(&doors_to_redeploy, 1);
#endif
    while (mode != 0 && timer > 0) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);
        _delay_ms(1000);
        timer -= 1;
        read_antenna_data(door_positions, &mode, main_heaters, backup_heaters, &timer_s, &i2c_status);
    }

    // Check which doors are still open
    read_antenna_data(door_positions, &mode, main_heaters, backup_heaters, &timer_s, &i2c_status);
    num_doors = 0;
    for (uint8_t i = 0; i < 4; i ++) {
        // Door closed and heaters not on
        if (!door_positions[i]) {
            num_doors += 1;
        }
    }

    write_antenna_clear(&i2c_status);
    _delay_ms(1000);

    // I2C failed or doors are still open
    if (ret_status == 0 || num_doors > 0) {
        // Manual release for 5 seconds for each burning resistor
        set_pin_high(ANT_REL_A, &PORT_ANT_REL);
        for (uint8_t seconds = 0; seconds < 5; seconds += 1) {
            WDT_ENABLE_SYS_RESET(WDTO_8S);
            _delay_ms(1000);
        }
        set_pin_low(ANT_REL_A, &PORT_ANT_REL);
        set_pin_high(ANT_REL_B, &PORT_ANT_REL);
        for (uint8_t seconds = 0; seconds < 5; seconds += 1) {
            WDT_ENABLE_SYS_RESET(WDTO_8S);
            _delay_ms(1000);
        }
        set_pin_low(ANT_REL_B, &PORT_ANT_REL);
    }

#ifdef ANTENNA_DEBUG
    print("Done");
#endif
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
    uint8_t data[3] = {0x00};
    uint8_t status = 0;
    uint8_t ret = read_i2c(ANTENNA_I2C_ADDRESS, data, 3, &status);

    for (uint8_t i = 0; i < 4; i ++) {
        door_positions[i] = (data[0] >> (i + 4)) & 0x01;
    }
    *mode = data[0] & 0x03;
    for (uint8_t i = 0; i < 4; i ++) {
        main_heaters[i] = (data[1] >> (i + 4)) & 0x01;
        backup_heaters[i] = (data[1] >> i) & 0x01;
    }
    *timer_s = data[2];

    *i2c_status = status;

#ifdef ANTENNA_DEBUG
    print("\n");
    print("I2C read: ret = %u, status = 0x%.2x\n", ret, status);
    print("data = ");
    print_bytes(data, 3);
    print("door_positions = ");
    print_bytes(door_positions, 4);
    print("mode = %u\n", *mode);
    print("main_heaters = ");
    print_bytes(main_heaters, 4);
    print("backup_heaters = ");
    print_bytes(backup_heaters, 4);
    print("timer_s = %u\n", *timer_s);
    print("\n");
#endif

    return ret;
}

// Execute algorithm 1 for all antenna rods
uint8_t write_antenna_alg1(uint8_t* i2c_status) {
#ifdef ANTENNA_DEBUG
    print("Writing algorithm 1\n");
#endif
    uint8_t data[1] = {0x1F};
    uint8_t ret = write_i2c(ANTENNA_I2C_ADDRESS, data, 1, i2c_status);
    return ret;
}

// Execute algorithm 2 for specified antenna rod(s)
uint8_t write_antenna_alg2(uint8_t ant_num_in_bytes, uint8_t* i2c_status) {
#ifdef ANTENNA_DEBUG
    print("Writing algorithm 2\n");
#endif
    uint8_t data[1] = {0x20};
    data[0] = data[0] | ant_num_in_bytes;
    uint8_t ret = write_i2c(ANTENNA_I2C_ADDRESS, data, 1, i2c_status);
    return ret;
}

// Clear and interrupt all antenna commands
uint8_t write_antenna_clear(uint8_t* i2c_status) {
#ifdef ANTENNA_DEBUG
    print("Writing algorithm clear\n");
#endif
    uint8_t data[1] = {0x00};
    uint8_t ret = write_i2c(ANTENNA_I2C_ADDRESS, data, 1, i2c_status);
    return ret;
}
