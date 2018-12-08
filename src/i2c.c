/*
I2C library
SC18IS600
Datasheet: https://www.nxp.com/docs/en/data-sheet/SC18IS600.pdf

MCU datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-8209-8-bit%20AVR%20ATmega16M1-32M1-64M1_Datasheet.pdf

I2C Info:
http://i2c.info
https://learn.sparkfun.com/tutorials/i2c/all

This library controls the SPI-I2C bridge, i.e. uses SPI to communicate with the
bridge (integrated circuit), which will then use I2C to communicate with another
component(s).

Each device on the I2C bus has a 7-bit address.
Each operation is either a read or a write operation.

TODO - should we implement read after write or write after write?
TODO - look at power-down mode and WAKEUP pin (p.5)
TODO - look at timeout and clock rate
*/

#include "i2c.h"

// CS pin connection
pin_info_t i2c_cs = {
    .port = &I2C_CS_PORT,
    .ddr = &I2C_CS_DDR,
    .pin = I2C_CS_PIN
};

// RESETn pin connection
pin_info_t i2c_reset = {
    .port = &I2C_RESET_PORT,
    .ddr = &I2C_RESET_DDR,
    .pin = I2C_RESET_PIN
};

// INT pin connection
pin_info_t i2c_int = {
    .port = &I2C_INT_PORT,
    .ddr = &I2C_INT_DDR,
    .pin = I2C_INT_PIN
};

// Gets set to true by the interrupt handler when the I2C bridge activates the
// INTn interrupt pin (p.4)
// i.e. active low is true, high is false
// Must be volatile because the interrupt handler changes its value
volatile bool i2c_int_active = false;


/*
Initializes the microcontroller's output pin for the I2C bridge.
*/
void init_i2c(void) {
    // Initialize the CS output pin
    init_cs(i2c_cs.pin, i2c_cs.ddr);

    // Initialize reset pin (default high - not active)
    init_output_pin(i2c_reset.pin, i2c_reset.ddr, 1);

    // Initialize interrupt pin
    init_input_pin(i2c_int.pin, i2c_int.ddr);

    // Enable PCIE3 bit (pin change interrupt 3, pins 31-24) (p. 88)
    PCICR |= _BV(PCIE2);

    // Enable PCINT22 -  pin change interrupts for pin 22 (p. 90)
    PCMSK2 |= _BV(PCINT22);

    // Enable all (global) interrupts
    sei();

    // Reset the I2C bridge
    reset_i2c();
}

/*
Resets the I2C bridge.
*/
void reset_i2c(void) {
    // Toggle RESETn low then high
    set_pin_low(i2c_reset.pin, i2c_reset.port);
    // Minimum 125ns (p. 18-19)
    _delay_us(1);
    set_pin_high(i2c_reset.pin, i2c_reset.port);
}

/*
Sets SPI to mode 3 (p. 1,5) and sets chip select low.
TODO - 1.2Mbit SPI? (p. 5)
*/
void start_i2c_spi(void) {
    set_spi_mode(3);
    set_cs_low(i2c_cs.pin, i2c_cs.port);
}

/*
Resets the SPI mode and sets chip select high.
*/
void end_i2c_spi(void) {
    set_cs_high(i2c_cs.pin, i2c_cs.port);
    reset_spi_mode();
}

/*
Writes a byte of data to an internal register in the I2C bridge (p. 15).
addr - register address
data - byte of data to write
*/
void write_i2c_reg(uint8_t addr, uint8_t data) {
    start_i2c_spi();
    send_spi(I2C_WRITE_REG);
    send_spi(addr);
    send_spi(data);
    end_i2c_spi();
}

/*
Reads a byte of data from an internal register in the I2C bridge (p. 15).
addr - register address
Returns - byte of data read
TODO - should it have error checking and pass the return value by pointer?
*/
uint8_t read_i2c_reg(uint8_t addr) {
    start_i2c_spi();
    send_spi(I2C_READ_REG);
    send_spi(addr);
    uint8_t data = send_spi(0x00);
    end_i2c_spi();

    return data;
}

/*
Reads a series of bytes of data from the read buffer in the I2C bridge (p. 13).
data - array of data bytes read (`len` bytes long, must be already allocated,
       will be populated by this function)
len - number of data bytes to read
*/
void read_i2c_buf(uint8_t* data, uint8_t len) {
    start_i2c_spi();
    send_spi(I2C_READ_BUF);
    for (uint8_t i = 0; i < len; i++) {
        data[i] = send_spi(0x00);
    }
    end_i2c_spi();
}

/*
Executes a write I2C command (write data to device, p. 12).
addr - slave address
data - array of data bytes to write (`len` bytes long)
len - number of data bytes to send
Returns - 1 for success, 0 for failure
TODO - error codes? (p. 10)
*/
uint8_t write_i2c(uint8_t addr, uint8_t* data, uint8_t len) {
    start_i2c_spi();
    send_spi(I2C_WRITE);
    send_spi(len);
    send_spi(addr);
    for (uint8_t i = 0; i < len; i++) {
        send_spi(data[i]);
    }
    end_i2c_spi();

    // Wait for INT pin interrupt
    // TODO timeout
    while (!i2c_int_active) {}
    i2c_int_active = false;

    // TODO check if successful
    return 1;
}

/*
Executes a read I2C command (read data from device, p. 13).
addr - slave address
data - array of data bytes read (`len` bytes long, must be already allocated,
       will be populated by this function)
len - number of data bytes to read
Returns - 1 for success, 0 for failure
TODO - error codes? (p. 10)
*/
uint8_t read_i2c(uint8_t addr, uint8_t* data, uint8_t len) {
    start_i2c_spi();
    send_spi(I2C_READ);
    send_spi(len);
    send_spi(addr);
    end_i2c_spi();

    // Wait for INT pin interrupt
    // TODO timeout
    while (!i2c_int_active) {}
    i2c_int_active = false;

    read_i2c_buf(data, len);

    // TODO check if successful
    return 1;
}

// Interrupt service routine for pin change interrupt (INT pin)
ISR(PCINT2_vect) {
    print("PCINT2 ISR\n");

    // Check the value of the I2C INT pin
    if (get_pin_val(i2c_int.pin, i2c_int.port) == 0) {
        i2c_int_active = true;
        print("I2C int active\n");
    } else {
        i2c_int_active = false;
    }
}
