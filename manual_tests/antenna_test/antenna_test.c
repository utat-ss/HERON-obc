#include "../../src/antenna.h"

// Test different lengths
void test_i2c_read(void) {
    print("%s\n", __FUNCTION__);

    for (uint8_t len = 1; len <= 20; len++) {
        uint8_t read[20];
        // Clear buffer
        for (uint8_t i = 0; i < 20; i++) {
            read[i] = 0x00;
        }
        uint8_t status = 0x00;

        uint8_t result = read_i2c(ANTENNA_I2C_ADDRESS, read, len, &status);
        print("len = %u: result = %u, status = 0x%.2x, read = ", len, result, status);
        print_bytes(read, len);
    }
}

// Test different lengths
void test_i2c_write(void) {
    print("%s\n", __FUNCTION__);

    for (uint8_t len = 1; len <= 20; len++) {
        uint8_t write[20];
        // Populate buffer
        for (uint8_t i = 0; i < 20; i++) {
            write[i] = i + 2;
        }
        uint8_t status = 0x00;

        uint8_t result = write_i2c(ANTENNA_I2C_ADDRESS, write, len, &status);
        print("len = %u: result = %u, status = 0x%.2x, write = ", len, result, status);
        print_bytes(write, len);
    }
}


int main(void) {
    init_uart();
    init_spi();
    init_i2c();
    WDT_OFF();
    WDT_ENABLE_SYS_RESET(WDTO_8S);

    print("\n\n\nStarting antenna test\n\n");

    // Test which lengths of I2C commands do something
    // Set 369 kHz clock
    // write_i2c_reg(I2C_CLOCK, 5);
    // test_i2c_read();
    // test_i2c_write();

    deploy_antenna();
    WDT_OFF();
}
