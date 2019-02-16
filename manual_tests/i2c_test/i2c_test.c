/*
This program tests OBC functioning as an I2C master, using an Arduino Due as the slave device (running the code in test-software/i2c_slave_arduino).
*/

#include "../../src/i2c.h"

// Defined in the Arduino slave program
#define SLAVE_ADDR 16

void print_all_regs(void) {
    print("Registers:\n");
    print("IO Config: %02x\n", read_i2c_reg(I2C_IO_CONFIG));
    print("IO State: %02x\n", read_i2c_reg(I2C_IO_STATE));
    print("Clock: %02x\n", read_i2c_reg(I2C_CLOCK));
    print("TO: %02x\n", read_i2c_reg(I2C_TO));
    print("STAT: %02x\n", read_i2c_reg(I2C_STAT));
    print("ADR: %02x\n", read_i2c_reg(I2C_ADR));
}

int main(void) {
    init_uart();
    init_spi();
    init_i2c();

    print("\n\n\nStarting I2C test\n");

    print_all_regs();

    // print("Waiting until not busy\n");
    // while (1) {
    //     uint8_t status = read_i2c_reg(I2C_STAT);
    //     print("status = %02x\n", status);
    //     if (status != I2C_BUSY) {
    //         break;
    //     }
    // }

    uint8_t status = 0;

    print("Writing 'ABC'\n");
    uint8_t write[3] = { 'A', 'B', 'C' };
    write_i2c(SLAVE_ADDR, write, 3, &status);
    print("status = %02x\n", status);

    print("Waiting until not busy\n");
    while (1) {
        uint8_t status = read_i2c_reg(I2C_STAT);
        print("status = %02x\n", status);
        if (status != I2C_BUSY) {
            break;
        }
    }

    print_all_regs();

    print("Reading 5 bytes\n");
    uint8_t read[5] = { 0x00 };
    read_i2c(SLAVE_ADDR, read, 5, &status);
    print("status = %02X\n", status);

    print("\nDone I2C test\n\n\n");
}
