#include "antenna.h"

// Number of seconds to wait before initializing comms
volatile uint32_t comms_delay_s = COMMS_DELAY_DEF_S;

// Delays 30 minutes before we can init comms
// Fetches previous value in EEPROM to see if we have already finished this
void run_comms_delay(void) {
    if (eeprom_read_dword(COMMS_DELAY_DONE_EEPROM_ADDR) == COMMS_DELAY_DONE_FLAG) {
        print("Already done comms delay\n");
        return;
    }

    print("Starting comms delay\n");

    // Use 100ms increments because that is tolerable for delay precision
    for (uint32_t seconds = 0; seconds < comms_delay_s; seconds++) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);
        for (uint8_t i = 0; i < 10; i++) {
            _delay_ms(100);
        }
    }

    eeprom_write_dword(COMMS_DELAY_DONE_EEPROM_ADDR, COMMS_DELAY_DONE_FLAG);

    print("Comms delay done\n");
}

// Currently a dummy I2C sequence
// TODO - implement real deployment commands
void deploy_antenna(void) {
    // TODO - read status of each door and deploy any that failed
    print("Deploying antenna...\n");

    for (uint8_t i = 0; i < 5; i++) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);
        print("Loop %u\n", i);

        uint8_t data[1] = {0x00};
        uint8_t status = 0;

        write_i2c(0x00, data, 1, &status);
        read_i2c(0x00, data, 1, &status);
        _delay_ms(5000);
    }
    
    print("Done deploying antenna\n");
}
