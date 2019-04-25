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

void print_string(uint8_t* data, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        put_uart_char(data[i]);
    }
    put_uart_char('\n');
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

bool msg1 = true;

// Alternate messages
void test_write_single(void) {
    uint8_t ret = 0;
    uint8_t status = 0;
    
    uint8_t write[3] = {0x00};
    if (msg1) {
        write[0] = 'A';
        write[1] = 'B';
        write[2] = 'C';
    } else {
        write[0] = 'D';
        write[1] = 'E';
        write[2] = 'F';
    };
    msg1 = !msg1;

    print("\n");
    print("Writing ");
    print_string(write, 3);
    ret = write_i2c(SLAVE_ADDR, write, 3, &status);
    print("ret = %u, status = %02x\n", ret, status);
}

void test_read_single(void) {
    uint8_t ret = 0;
    uint8_t status = 0;
    
    uint8_t read[5] = { 0x00 };
    print("\n");
    print("Reading 5 bytes\n");
    ret = read_i2c(SLAVE_ADDR, read, 5, &status);
    print("ret = %u, status = %02x\n", ret, status);
    print_bytes(read, 5);
    print_string(read, 5);
}

void test_write_read_inf(void) {
    while (1) {
        test_write_single();
        _delay_ms(1000);
        test_read_single();
        _delay_ms(1000);
    }
}

int main(void) {
    init_uart();
    init_spi();
    
    print("\n\n\nStarting I2C test\n\n");

    init_i2c();
    print_all_regs();

    test_power_down();
    test_write_read_inf();

    print("\nDone I2C test\n\n\n");
}
