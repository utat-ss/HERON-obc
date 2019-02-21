/*
I2C library
SC18IS600
Datasheet: https://www.nxp.com/docs/en/data-sheet/SC18IS600.pdf

MCU datasheet: https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-8209-8-bit%20AVR%20ATmega16M1-32M1-64M1_Datasheet.pdf

I2C Info:
http://www.ti.com/lit/an/slva704/slva704.pdf
http://i2c.info
https://learn.sparkfun.com/tutorials/i2c/all

This library controls the SPI-I2C bridge, i.e. uses SPI to communicate with the
bridge (integrated circuit), which will then use I2C to communicate with another
component(s).

Addresses:
Each device on the I2C bus has a 7-bit address. Each operation is either a read
    (1) or a write (0) operation. The address byte has the read/write bit as the
    least significant bit.

NOTE: There are inconsistencies in representing I2C addresses. Some use the
    7-bit address, while others use the 8-bit address (ignoring the least
    significant bit). The address bytes in the datasheet use the 8-bit version
    - "The SC18IS600 will ignore the least significant bit" (p. 12, 13). All
    functions in this library take 7-bit addresses as input and shift it left by
    1 bit within the function.

For interrupts (e.g. when read and write operations complete), we can just poll
    because we are blocking in a loop until the operation is finished anyways.

- Not implementing read after write or write after write
- Default I2CClk register is 0x19 (p. 5) -> 73.728 kHz (p. 9)

TODO - test I2C at 5V with level translator
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

pin_info_t i2c_wakeup = {
    .port = &I2C_WAKEUP_PORT,
    .ddr = &I2C_WAKEUP_DDR,
    .pin = I2C_WAKEUP_PIN
};


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
    // Initialize wakeup pin
    init_output_pin(i2c_wakeup.pin, i2c_wakeup.ddr, 1);

    // Reset the I2C bridge
    reset_i2c();

    // Need to delay before writing any registers or else it doesn't work
    // 1ms just from experimentation
    _delay_ms(1);

    // Use the lowest clock frequency by default (7.2 kHz, p. 9)
    write_i2c_reg(I2C_CLOCK, 255);
    // Set the timeout value and enable the timeout function
    // (0xFF would be 65,535 cycles of a 57.6 kHz clock, so about 1 second, p. 9)
    // 0x3F register -> 0x3FFF counter -> 2^14 / 57,600 = about 278 ms
    write_i2c_reg(I2C_TO, 0x3F);
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
Puts the I2C bridge in the power-down state (p. 16)
*/
void power_down_i2c(void) {
    // Must set WAKEUPn high before power-down mode
    set_pin_high(i2c_wakeup.pin, i2c_wakeup.port);

    start_i2c_spi();
    send_spi(I2C_POWER_DOWN);
    send_spi(I2C_POWER_DOWN_1);
    send_spi(I2C_POWER_DOWN_2);
    end_i2c_spi();
}

/*
Takes the I2C bridge out of power-down state (p. 16)
*/
void power_up_i2c(void) {
    // Set WAKEUPn low
    set_pin_low(i2c_wakeup.pin, i2c_wakeup.port);
}

/*
Sets SPI to mode 3 (p. 1,5) and sets chip select low.
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
Waits until the INT pin goes low.
Returns - 1 for success, 0 for failure (timed out)
*/
uint8_t wait_for_i2c_int(void) {
    uint16_t timeout = UINT16_MAX;
    while (get_pin_val(i2c_int.pin, i2c_int.port) && (timeout > 0)) {
        timeout--;
    }

    // Check for timeout
    if (timeout == 0) {
        return 0;
    }

    return 1;
}

/*
Waits until the status register is not busy (0xF3).
Returns - 1 for success, 0 for failure (still busy)
*/
uint8_t wait_for_i2c_not_busy(void) {
    uint16_t timeout = UINT16_MAX;
    while ((read_i2c_reg(I2C_STAT) == I2C_BUSY) && (timeout > 0)) {
        timeout--;
    }
    if (timeout == 0) {
        return 0;
    }

    return 1;
}

/*
Executes a write I2C command (write data to device, p. 12).
addr - 7-bit slave address
data - array of data bytes to write (`len` bytes long)
len - number of data bytes to send
status - will be set by this function to the status register value
Returns - 1 for success, 0 for failure
*/
uint8_t write_i2c(uint8_t addr, uint8_t* data, uint8_t len, uint8_t* status) {
    if (!wait_for_i2c_not_busy()) {
        return 0;
    }

    start_i2c_spi();
    send_spi(I2C_WRITE);
    send_spi(len);
    send_spi(addr << 1);
    for (uint8_t i = 0; i < len; i++) {
        send_spi(data[i]);
    }
    end_i2c_spi();

    // Wait for interrupt
    if (!wait_for_i2c_int()) {
        return 0;
    }
    if (!wait_for_i2c_not_busy()) {
        return 0;
    }

    // Check status register (could be successful)
    uint8_t read_status = read_i2c_reg(I2C_STAT);
    if (status != NULL) {
        *status = read_status;
    }
    if (read_status != I2C_SUCCESS) {
        return 0;
    }

    // Successful otherwise
    return 1;
}

/*
Executes a read I2C command (read data from device, p. 13).
addr - 7-bit slave address
data - array of data bytes read (`len` bytes long, must be already allocated,
       will be populated by this function)
len - number of data bytes to read
status - will be set by this function to the status register value
Returns - 1 for success, 0 for failure
*/
uint8_t read_i2c(uint8_t addr, uint8_t* data, uint8_t len, uint8_t* status) {
    if (!wait_for_i2c_not_busy()) {
        return 0;
    }

    start_i2c_spi();
    send_spi(I2C_READ);
    send_spi(len);
    send_spi(addr << 1);
    end_i2c_spi();

    // Wait for interrupt
    if (!wait_for_i2c_int()) {
        return 0;
    }
    if (!wait_for_i2c_not_busy()) {
        return 0;
    }

    // Check status register (could be successful)
    uint8_t read_status = read_i2c_reg(I2C_STAT);
    if (status != NULL) {
        *status = read_status;
    }
    if (read_status != I2C_SUCCESS) {
        return 0;
    }

    // Copy read buffer contents
    read_i2c_buf(data, len);

    // Successful otherwise
    return 1;
}


// Can uncomment and use the following code to test the INT pin with an
// interrupt vector
// Can use for testing, but this is not needed in the final version since we
// just block until the interrupt is asserted (INT goes low)

// void init_i2c_int(void) {
//     // Enable PCIE3 bit (pin change interrupt 3, pins 31-24) (p. 88)
//     PCICR |= _BV(PCIE2);
//     // Enable PCINT22 -  pin change interrupts for pin 22 (p. 90)
//     PCMSK2 |= _BV(PCINT22);
//     // Enable all (global) interrupts
//     sei();
// }
//
// // Interrupt service routine for pin change interrupt (INT pin)
// ISR(PCINT2_vect) {
//     print("PCINT2 ISR: I2C INT pin = %u\n",
//         get_pin_val(i2c_int.pin, i2c_int.port));
// }
