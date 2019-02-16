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

For interrupts (e.g. when read and write operations complete), we can just poll because we are blocking in a loop until the operation is finished anyways.

- Not implementing read after write or write after write
- Default I2CClk register is 0x19 (p. 5) -> 73.728 kHz (p. 9)
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

    // TODO - remove interrupt
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
        print("Timed out\n");
        return 0;
    }

    return 1;
}

/*
Executes a write I2C command (write data to device, p. 12).
addr - slave address
data - array of data bytes to write (`len` bytes long)
len - number of data bytes to send
status - will be set by this function to the status register value
Returns - 1 for success, 0 for failure
*/
uint8_t write_i2c(uint8_t addr, uint8_t* data, uint8_t len, uint8_t* status) {

    print("before write Waiting until not busy\n");
    while (1) {
        uint8_t test_status = read_i2c_reg(I2C_STAT);
        print("test_status = %02x\n", test_status);
        if (test_status != I2C_BUSY) {
            break;
        }
    }

    start_i2c_spi();
    send_spi(I2C_WRITE);
    send_spi(len);
    send_spi(addr);
    for (uint8_t i = 0; i < len; i++) {
        send_spi(data[i]);
    }
    end_i2c_spi();

    // Wait for interrupt
    uint8_t ret = wait_for_i2c_int();
    if (ret == 0) {
        return 0;
    }

    print("Waiting until not busy\n");
    while (1) {
        uint8_t test_status = read_i2c_reg(I2C_STAT);
        print("test_status = %02x\n", test_status);
        if (test_status != I2C_BUSY) {
            break;
        }
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
addr - slave address
data - array of data bytes read (`len` bytes long, must be already allocated,
       will be populated by this function)
len - number of data bytes to read
status - will be set by this function to the status register value
Returns - 1 for success, 0 for failure
*/
uint8_t read_i2c(uint8_t addr, uint8_t* data, uint8_t len, uint8_t* status) {

    print("before read Waiting until not busy\n");
    while (1) {
        uint8_t test_status = read_i2c_reg(I2C_STAT);
        print("test_status = %02x\n", test_status);
        if (test_status != I2C_BUSY) {
            break;
        }
    }

    start_i2c_spi();
    send_spi(I2C_READ);
    send_spi(len);
    send_spi(addr);
    end_i2c_spi();

    // Wait for interrupt
    uint8_t ret = wait_for_i2c_int();
    if (ret == 0) {
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

// Interrupt service routine for pin change interrupt (INT pin)
// Can just use for testing, but don't need in the final version
// since we block until the interrupt is asserted
ISR(PCINT2_vect) {
    // print("PCINT2 ISR: I2C INT pin = %u\n",
    //     get_pin_val(i2c_int.pin, i2c_int.port));
}
