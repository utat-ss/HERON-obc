/*
Transceiver library
EnduroSat UHF Transceiver Type II

UART commands are sent through UART (print statements)
Transceiver responses are handled in trans_cb

Note that all/most configuration functions will return 1 if configuration/read successful
and 0 if not successful

Status Control Register Bits:
15-14: Reserved
13-12: Baudrate = 0 for 9600
11: Reset
10-8: RfMode Choose 0
7: UART Echo Enable (1 = ON 0 = OFF)
6: Beacon Message Enable (1 = ON 0 = OFF)
5: Pipeline Communication Enable
All bits below this point are read only (no writing)
4: Bootloader mode: 1, Application Mode: 0
3/2: CTS/SEC (Reserved)
1: Correct initialization of FRAM (0 = Error)
0: Correct initialization of Radio Transceiver (0 = Error)

TODO: default status control word
RF Mode chosen to be default 3 for now
From LSB to MSB Default: 0303
baud_rate = 0b00;
reset = 0b0;
rF_mode = 0b011;
echo = 0b0;
beacon = 0b0;
pipeline = 0b0;

Unneded: Beacon contents, packets

TODO - check each response for the proper number of characters
TODO - set timeouts for waiting for command responses
*/

#include "transceiver.h"

//Default Address - do not change
#define TRANS_ADDR  0x22

// Has '\0' termination
volatile uint8_t received_cmd[20];
// Gets set to true by the UART RX callback when a full command
// (detected by \r termination) is available in `received_cmd`
volatile bool received_cmd_available = false;



/*
Initializes the transceiver for UART RX callbacks (does not change any settings).
*/
void init_trans(void) {
    set_uart_rx_cb(trans_cb);
}

/*
UART RX callback
*/
uint8_t trans_cb(const uint8_t* buf, uint8_t len) {
    // If we found the termination, copy the command
    if (buf[len - 1] == '\r') {
        // Copy each character
        for (uint8_t i = 0; i < len - 1; ++i) {
            received_cmd[i] = buf[i];
        }

        // Add termination character
        received_cmd[len - 1] = '\0';
        // Set command available flag
        received_cmd_available = true;

        print("received command: %u chars: %s\n", len - 1, received_cmd);

        // Processed all characters
        return len;
    }

    // If we haven't found the termination, don't process anything yet
    return 0;
}

// Converts the ASCII representation of a hex number to the integer
uint8_t char_hex_to_dec(uint8_t c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    } else {
        return 0;
    }
}

/*
Extracts ASCII characters representing a hex number and returns the corresponding integer.
offset - starting index in the string
count - number of characters (between 1 and 8)
*/
uint32_t scan_uint(volatile uint8_t* string, uint8_t offset, uint8_t count) {
    uint32_t value = 0;
    for (uint8_t i = offset; i < offset + count; i++) {
        value = value << 4;
        value = value + char_hex_to_dec(string[i]);
    }
    return value;
}

/*
Check if Command was successful by looking at first 2 or 3 characters in the
received command
Values: 'OK' means that command was successful. Return 1
Values: 'ERR' means that Command was not sucessful. Return 0
Other values are unexpected. Return 0
*/
uint8_t valid_cmd(volatile uint8_t* string) {
    uint8_t valid[] = {'O', 'K'}; //"OK"
    uint8_t invalid[] = {'E', 'R', 'R'}; //"ERR"

    if (string_cmp(string, valid, 2) == 1) {
        return 1;
    }
    else if (string_cmp(string, invalid, 3) == 1) {
        return 0;
    }
    return 0;
}

//Compares strings, returns 1 if strings are the same, returns 0 otherwise
uint8_t string_cmp(volatile uint8_t* first, uint8_t* second, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        if (first[i] != second[i]) {
            return 0;
        }
    }
    return 1;
}




/*
1. Write to status control register (p. 15-16)

scw - 16-bit data to write to register
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_scw(uint16_t scw) {
    print("\rES+W%02X00%04X\r", TRANS_ADDR, scw);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //Check validity
    uint8_t validity = valid_cmd(received_cmd);
    received_cmd_available = false;
    return validity;
}

/*
1. Read status control register (p. 15-16)

scw - this function will set it to the 16-bit register data of register
Returns - 1 for success, 0 for failure

Example (different format from datasheet):
ES+R2200
OK+0022DD0303

00 - unknown, probably RSSI
22 - device address
DD - device reset counter (increases by 1 every time the transceiver is reset or power cycled)
0303 - contents of status register
*/
uint8_t get_trans_scw(uint8_t* rssi, uint16_t* scw) {
    print("\rES+R%02X00\r", TRANS_ADDR);

    //Wait for response
    while (!received_cmd_available) {}

    //check validity
    uint8_t validity = valid_cmd(received_cmd); //command valid

    if (validity == 1) {
        uint8_t offset = 3;
        uint8_t count = 2;
        *rssi = (uint8_t) scan_uint(received_cmd, offset, count);

        offset = 9;
        count = 4;
        *scw = (uint16_t) scan_uint(received_cmd, offset, count);
    }

    received_cmd_available = false; //reset flag
    return validity;
}

/*
1. Turn on beacon mode (using status register)
Returns - 1 for success, 0 for failure
*/
uint8_t turn_on_trans_beacon(void) {
    uint8_t ret;
    uint8_t rssi;
    uint16_t scw;

    ret = get_trans_scw(&rssi, &scw);
    if (ret == 0) {
        return 0;
    }

    scw |= _BV(TRANS_BCN); //set bit 6 = 1
    ret = set_trans_scw(scw);
    return ret;
}

/*
1. Turn off beacon mode (using status register)
Returns - 1 for success, 0 for failure
*/
uint8_t turn_off_trans_beacon(void) {
    uint8_t ret;
    uint8_t rssi;
    uint16_t scw;

    ret = get_trans_scw(&rssi, &scw);
    if (ret == 0) {
        return 0;
    }

    scw &= ~_BV(TRANS_BCN); //set bit 6 = 0
    ret = set_trans_scw(scw);
    return ret;
}

/*
1. Turn on pipe (transparent) mode (using status control register) (p. 15-16).
It will turn off pipe mode on its own based on the pipe timeout.
*/
uint8_t turn_on_trans_pipe(void) {
    uint8_t ret;
    uint8_t rssi;
    uint16_t scw;

    ret = get_trans_scw(&rssi, &scw);
    if (ret == 0) {
        return 0;
    }

    scw |= _BV(TRANS_PIPE); //set bit 5 = 1
    ret = set_trans_scw(scw);
    return ret;
}


/*
2. Set transceiver frequency (p. 17-18)

freq - frequency to write, already in the converted 32-bit format that the
       transceiver expects (the output of the conversion program)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_freq(uint32_t freq) {
    print("\rES+W%02X01%08X\r", TRANS_ADDR, freq);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //Check validity
    uint8_t validity = valid_cmd(received_cmd);
    received_cmd_available = false;
    return validity;
}

/*
2. Get transceiver frequency (p. 17-18)
freq - will be set to the read 32-bit value
Returns - 1 for sucess, 0 for failure

Answer format: OK+[RR][FFFFFF][NN]
*/
uint8_t get_trans_freq(uint8_t* rssi, uint32_t* freq) {
    print("\rES+R%02X01\r", TRANS_ADDR);

    while (!received_cmd_available) {}

    //check validity
    uint8_t validity = valid_cmd(received_cmd); //command valid

    if (validity == 1) {
        uint8_t offset = 3;
        uint8_t count = 2;
        *rssi = (uint8_t) scan_uint(received_cmd, offset, count);

        offset = 5;
        count = 4;
        *freq = scan_uint(received_cmd, offset, count);
    }

    received_cmd_available = false; //reset flag
    return validity;
}


/*
4. Set transparent (pipe) mode timeout (p.18)
Sets timeout to turn off pipe mode if there are no UART messages
timeout - timeout (in seconds) to set
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_pipe_timeout(uint8_t timeout) {
    print("\rES+W%02X06000000%02X\r", TRANS_ADDR, timeout);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //Check validity
    uint8_t validity = valid_cmd(received_cmd);
    received_cmd_available = false;
    return validity;
}


/*
5. Set beacon transmission period - Default is 60s (p. 19)
period - desired period between beacon message transmissions (in seconds)
Max val = 0xFFFF = 65535s = 1092min = 18.2h
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_beacon_period(uint16_t period) {
    print("\rES+W%02X070000%04X\r", TRANS_ADDR, period);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //Check validity
    uint8_t validity = valid_cmd(received_cmd);
    received_cmd_available = false;
    return validity;
}

/*
5. Get beacon transmission period (p. 19)
period - set to desired period between beacon message transmissions (in seconds)
Max val = 0xFFFF = 65535s = 1092min = 18.2h
Returns - 1 for success, 0 for failure

Answer: OK+[RR]0000[TTTT]<CR>
*/
uint8_t get_trans_beacon_period(uint8_t* rssi, uint16_t* period) {
    print("\rES+R%02X07%04X\r", TRANS_ADDR);

    while (!received_cmd_available) {}

    //check validity
    uint8_t validity = valid_cmd(received_cmd); //command valid

    if (validity == 1) {
        uint8_t offset = 3;
        uint8_t count = 2;
        *rssi = (uint8_t) scan_uint(received_cmd, offset, count);

        offset = 9;
        count = 4;
        *period = (uint16_t) scan_uint(received_cmd, offset, count);
    }

    received_cmd_available = false;
    return validity;
}


/*
7. Set destination call-sign (p.20)
call_sign - the call sign to set (6 bytes array)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_dest_call_sign(char* call_sign) {
    print("\rES+W%02XF5", TRANS_ADDR);
    for (uint8_t i = 0; i < 6; i++) {
        print("%c", call_sign[i]);
    }
    print("\r");

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //Check validity
    uint8_t validity = valid_cmd(received_cmd);
    received_cmd_available = false;
    return validity;
}

/*
7. Get destination call-sign (p.20)
call_sign - a pre-allocated 6-byte array that gets set to the call sign read
Returns - 1 for success, 0 for failure

Answer: OK+DDDDDD<CR>
*/
uint8_t get_trans_dest_call_sign(char* call_sign) {
    print("\rES+R%02XF5\r", TRANS_ADDR);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //check validity
    uint8_t validity = valid_cmd(received_cmd); //command valid

    if (validity == 1) {
        for (uint8_t i = 0; i < 6; i++) {
            call_sign[i] = received_cmd[3 + i];
        }
    }

    received_cmd_available = false;
    return validity;
}


/*
8. Set source call-sign (p.20)
call_sign - the call sign to set (6 bytes array)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_src_call_sign(char* call_sign) {
    print("\rES+W%02XF6", TRANS_ADDR);
    for (uint8_t i = 0; i < 6; i++) {
        print("%c", call_sign[i]);
    }
    print("\r");

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //Check validity
    uint8_t validity = valid_cmd(received_cmd);
    received_cmd_available = false;
    return validity;
}

/*
8. Get source call-sign (p.20)
call_sign - a pre-allocated 6-byte array that gets set to the call sign read
Returns - 1 for success, 0 for failure

Answer: OK+DDDDDD<CR>
*/
uint8_t get_trans_src_call_sign(char* call_sign) {
    print("\rES+R%02XF6\r", TRANS_ADDR);

    while (!received_cmd_available) {}

    //check validity
    uint8_t validity = valid_cmd(received_cmd); //command valid

    if (validity == 1) {
        for (uint8_t i = 0; i < 6; i++) {
            call_sign[i] = received_cmd[3 + i];
        }
    }

    received_cmd_available = false;
    return validity;
}


/*
16. Get uptime (p. 23)
rssi - gets set to the last RSSI
uptime - gets set to the uptime
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_uptime(uint8_t* rssi, uint32_t* uptime) {
    print("\rES+R%02X02\r", TRANS_ADDR);

    while (!received_cmd_available) {}

    //check validity
    uint8_t validity = valid_cmd(received_cmd); //command valid

    if (validity == 1) {
        uint8_t offset = 3;
        uint8_t count = 2;
        *rssi = (uint8_t) scan_uint(received_cmd, offset, count);

        offset = 5;
        count = 8;
        *uptime = scan_uint(received_cmd, offset, count);
    }

    received_cmd_available = false;
    return validity;
}


/*
17. Get number of transmitted packets (p.23)
rssi - gets set to the last RSSI
num_packets - gets set to the number of transmitted packets
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_num_tx_packets(uint8_t* rssi, uint32_t* num_packets) {
    print("\rES+R%02X03\r", TRANS_ADDR);

    while (!received_cmd_available) {}

    //check validity
    uint8_t validity = valid_cmd(received_cmd); //command valid

    if (validity == 1) {
        uint8_t offset = 3;
        uint8_t count = 2;
        *rssi = (uint8_t) scan_uint(received_cmd, offset, count);

        offset = 5;
        count = 8;
        *num_packets = scan_uint(received_cmd, offset, count);
    }

    received_cmd_available = false;
    return validity;
}


/*
18. Get number of received packets (p.23)
rssi - gets set to the last RSSI
num_packets - gets set to the number of received packets
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_num_rx_packets(uint8_t* rssi, uint32_t* num_packets) {
    print("\rES+R%02X04", TRANS_ADDR);

    while (!received_cmd_available) {}

    //check validity
    uint8_t validity = valid_cmd(received_cmd); //command valid

    if (validity == 1) {
        uint8_t offset = 3;
        uint8_t count = 2;
        *rssi = (uint8_t) scan_uint(received_cmd, offset, count);

        offset = 5;
        count = 8;
        *num_packets = scan_uint(received_cmd, offset, count);
    }

    received_cmd_available = false;
    return validity;
}


/*
19. Get number of received packets with CRC error (p.23)
rssi - gets set to the last RSSI
num_packets_crc - gets set to the number of received packets with CRC error
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_num_rx_packets_crc(uint8_t* rssi, uint32_t* num_packets_crc) {
    print("\rES+R%02X05", TRANS_ADDR);

    while (!received_cmd_available) {}

    //check validity
    uint8_t validity = valid_cmd(received_cmd); //command valid

    if (validity == 1) {
        uint8_t offset = 3;
        uint8_t count = 2;
        *rssi = (uint8_t) scan_uint(received_cmd, offset, count);

        offset = 5;
        count = 8;
        *num_packets_crc = scan_uint(received_cmd, offset, count);
    }

    received_cmd_available = false;
    return validity;
}
