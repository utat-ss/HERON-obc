/*
This program tests OBC functioning as an I2C master, using an Arduino Due as the slave device (running the code in test-software/i2c_slave_arduino).

Connect SCL to Arduino pin 21, SDA to Arduino pin 20, and you might need to connect the grounds together.

Debugging help: https://www.avrfreaks.net/forum/i2c-problem-no-ack-slave-device
*/

#include "../../src/i2c.h"

#define SLAVE_ADDR 8

void print_all_regs(void) {
    print("Registers:\n");
    print("IO Config: %02x\n", read_i2c_reg(I2C_IO_CONFIG));
    print("IO State: %02x\n", read_i2c_reg(I2C_IO_STATE));
    print("Clock: %02x\n", read_i2c_reg(I2C_CLOCK));
    print("TO: %02x\n", read_i2c_reg(I2C_TO));
    print("STAT: %02x\n", read_i2c_reg(I2C_STAT));
    print("ADR: %02x\n", read_i2c_reg(I2C_ADR));
}

void test_power_down(void) {
    uint8_t write[3] = { 'A', 'B', 'C' };
    uint8_t ret = 0;
    uint8_t status = 0;

    print("\n");

    power_down_i2c();
    print("Powered down I2C\n");

    print("Writing 'ABC'\n");
    ret = write_i2c(SLAVE_ADDR, write, 3, &status);
    print("ret = %u, status = %02x\n", ret, status);

    power_up_i2c();
    print("Powered up I2C\n");

    print("Writing 'ABC'\n");
    ret = write_i2c(SLAVE_ADDR, write, 3, &status);
    print("ret = %u, status = %02x\n", ret, status);
}

void test_write_read(void) {
    uint8_t ret = 0;
    uint8_t status = 0;

    while (1) {
        print("\n");
        print("Writing 'ABC'\n");
        uint8_t write[3] = { 'A', 'B', 'C' };
        ret = write_i2c(SLAVE_ADDR, write, 3, &status);
        print("ret = %u, status = %02x\n", ret, status);

        print("\n");
        print("Reading 5 bytes\n");
        uint8_t read[5] = { 0x00 };
        ret = read_i2c(SLAVE_ADDR, read, 5, &status);
        print("ret = %u, status = %02x\n", ret, status);
        print_bytes(read, 5);

        _delay_ms(1000);
    }
}

int main(void) {
    init_uart();
    init_spi();
    init_i2c();

    print("\n\n\nStarting I2C test\n\n");

    print_all_regs();

    test_power_down();
    test_write_read();

    print("\nDone I2C test\n\n\n");
}
