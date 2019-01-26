/*
Transceiver library
EnduroSat UHF Transceiver Type II

Commands are sent through UART (by calling print())
Transceiver responses are handled in trans_uart_rx_cb

Note that all/most configuration functions will return 1 if configuration/read
successful and 0 if not successful

Each of the command functions has the structure of an "attempt" function,
which is repeated until the response is successfully received (with a maximum
number of attempts).

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

Unused features: Beacon contents, packets

UART RX Buffer:
There is a common buffer of characters received over UART from the transceiver.
These could either be a response to a command we send it, or a message from the
ground station. In either case, we save all received UART characters to the
buffer. Every time we receive a new character, scan the entire buffer to see if
the sequence of characters makes sense (i.e. detect it as either a command
response or a received message and set the appropriate boolean flag, which
functions in this library can check for and consume the data if desired). We
set a timeout where if we do not receive any characters for a certain number of
seconds, the entire buffer contents are discarded.

TODO - For most of the get commands, OBC misses the first 1 or 2 characters in
    the response for the first attempt, but the second attempt seems to always work

TODO - it was observed that after either sending data in pipe mode, the
    transceiver sent the UART message "+ESTTC<CR>" - figure out what this is

TODO - default status register and frequency?
TODO - macro for repeating command attempts?
TODO - if commands to the transceiver fail, change UART baud rate and switch
    the transceiver back to 9600 baud
TODO - clear trans_rx_buf buffer before sending a command
TODO - for ground station messages, use the first one or two bytes as the
    number of bytes in the message?

TODO - sort out possible race condition where we receive UART RX, which triggers
an action in some function, but before the function finished, the uptime timer
triggers and clears the buffer that the function was taking data from
    - currently assuming that the function acts quickly enough to use the data
      before it is cleared
*/

#include "transceiver.h"


/* Command response received back from transceiver */
// String - has '\0' termination
volatile uint8_t trans_rx_buf[TRANS_RX_BUF_MAX_SIZE] = {0x00};
// Length - number of characters (NOT including '\r' or '\0' termination)
volatile uint8_t trans_rx_buf_len = 0;

// Flag - gets set to true by the UART RX callback when a full command response
// (detected by \r termination) is available in `trans_rx_buf`
volatile bool trans_cmd_resp_avail = false;
// true if we received a message
volatile bool trans_rx_msg_avail = false;

// Default no-operation callback
void _trans_rx_msb_cb_nop(const uint8_t* msg, uint8_t len) {}
// RX message callback function
// It will give the function the message and length - when the callback returns,
// it will clear the RX buffer
trans_rx_msg_cb_t trans_rx_msg_cb = _trans_rx_msb_cb_nop;




/*
Initializes the transceiver for UART RX callbacks (does not change any settings).
*/
void init_trans(void) {
    set_uart_rx_cb(trans_uart_rx_cb);
}

/*
UART RX callback
buf - array of received characters
len - number of received characters
Returns - number of characters processed
*/
uint8_t trans_uart_rx_cb(const uint8_t* buf, uint8_t len) {
    // Add all received characters to the buffer, make sure not to overflow
    for (uint8_t i = 0;
        (i < len) && (trans_rx_buf_len < TRANS_RX_BUF_MAX_SIZE);
        i++) {

        trans_rx_buf[trans_rx_buf_len] = buf[i];
        trans_rx_buf_len++;
        print("added to buffer: 0x%.2x\n", buf[i]);
    }

    // Scan what we have in the buffer now
    scan_trans_cmd_resp_avail();
    scan_trans_rx_msg_avail();

    // If we got an RX message, call the callback function (it can do what it
    // wants with the message), then clear the buffer
    if (trans_rx_msg_avail) {
        trans_rx_msg_cb((uint8_t*) trans_rx_buf, trans_rx_buf_len);
        clear_trans_rx_buf();
    }

    // Processed all characters
    return len;
}


void set_trans_rx_msg_cb(trans_rx_msg_cb_t cb) {
    trans_rx_msg_cb = cb;
}

void clear_trans_rx_buf(void) {
    for (uint8_t i = 0; i < TRANS_RX_BUF_MAX_SIZE; i++) {
        trans_rx_buf[i] = 0;
    }
    trans_rx_buf_len = 0;
}


/*
Converts the ASCII representation of a hex number to an integer value.
c - character in ASCII format (should be between '0' to '9' or 'A' to 'F')
Returns - value between 0 to 15, or 0 if invalid character
*/
uint8_t char_to_hex(uint8_t c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    } else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    } else {
        return 0;
    }
}

/*
Extracts ASCII characters representing a hex number and returns the corresponding integer.
string - string containing ASCII characters of the number to process (volatile
         to prevent compiler warnings with the `trans_rx_buf` buffer)
offset - starting index in the string
count - number of characters (between 1 and 8)
Returns - 32-bit unsigned integer processed
*/
uint32_t scan_uint(volatile uint8_t* string, uint8_t offset, uint8_t count) {
    uint32_t value = 0;
    for (uint8_t i = offset; i < offset + count; i++) {
        value = value << 4;
        value = value + char_to_hex(string[i]);
    }
    return value;
}

/*
Compares strings to see if they are equal
first - first string to compare (volatile to prevent compiler warnings with the
        `trans_rx_buf` buffer)
second - second string to compare
len - number of characters to compare
Returns - 1 if strings are the same, returns 0 otherwise
*/
uint8_t string_cmp(volatile uint8_t* first, char* second, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        if (first[i] != second[i]) {
            return 0;
        }
    }
    return 1;
}


/*
Waits until `trans_cmd_resp_avail` is set to true by the UART RX callback,
then sets it back to false. Times out after 65,535 cycles if no response is available.
expected_len - expected length of the response, NOT INCLUDING the '\r'
Returns - 1 for success (response available of expected length), 0 for failure
    (timed out or not expected length)
*/
uint8_t wait_for_trans_cmd_resp(uint8_t expected_len) {
    // Wait for trans_cmd_resp_avail to become true, with a timeout
    uint16_t timeout = UINT16_MAX;
    while ((!trans_cmd_resp_avail) && (timeout > 0)) {
        timeout--;
    }
    // Set the flag back to false for later use
    trans_cmd_resp_avail = false;
    // Failed if the timeout went to 0
    if (timeout == 0) {
        return 0;
    }

    // Check if the string's length matched the expected number of characters
    // Add 1 to account for the '\r' character
    if (trans_rx_buf_len != expected_len + 1) {
        return 0;
    }

    // Succeeded
    // If none of the false conditions returned 0, the response is valid
    return 1;
}


// Scans the contents of trans_rx_buf for a command response, sets
// trans_cmd_resp_avail if appropriate
void scan_trans_cmd_resp_avail(void) {
    // TODO - is this really robust? what if an RX message has "OK" or "\r"?
    // Check if the string starts with "OK"
    // Minimum 3 characters
    // Starts with "OK"
    // Ends with "\r"
    // "ERR" (command was not sucessful) is considered invalid, but we don't
    // need to check for it explicitly if it is not equal to "OK"
    char* valid = "OK";
    if (trans_rx_buf_len >= 3 &&
        string_cmp(trans_rx_buf, valid, 2) == 1 &&
        trans_rx_buf[trans_rx_buf_len - 1] == '\r') {

        trans_cmd_resp_avail = true;

        print("Received trans cmd resp: %u chars: ", trans_rx_buf_len);
        for (uint8_t i = 0; i < trans_rx_buf_len; i++) {
            print("%c", trans_rx_buf[i]);
        }
        print("\n");
    }
}

// Scans the contents of trans_rx_buf for a received message, sets
// trans_rx_msg_avail if appropriate
void scan_trans_rx_msg_avail(void) {
    // TODO - is this really robust? what if an RX message has "OK" or "\r"?
    // Minimum 3 characters
    // Does not start with "OK"
    // First byte is 0
    // Second byte is the number of bytes remaining in the buffer
    char* ok = "OK";
    if (trans_rx_buf_len >= 3 &&
        string_cmp(trans_rx_buf, ok, 2) == 0 &&
        trans_rx_buf[0] == 0x00 &&
        trans_rx_buf[1] == trans_rx_buf_len - 2) {

        trans_rx_msg_avail = true;

        print("Received trans rx msg: %u chars: ", trans_rx_buf_len);
        for (uint8_t i = 0; i < trans_rx_buf_len; i++) {
            print("%c", trans_rx_buf[i]);
        }
        print("\n");
    }
}




uint8_t set_trans_scw_attempt(uint16_t scw) {
    // Send command
    print("\rES+W%02X00%04X\r", TRANS_ADDR, scw);

    // Wait for response
    uint8_t validity = wait_for_trans_cmd_resp(7);
    return validity;
}

/*
1. Write to status control register (p. 15-16)

scw - 16-bit data to write to register
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_scw(uint16_t scw) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = set_trans_scw_attempt(scw);
    }
    return ret;
}


uint8_t get_trans_scw_attempt(uint8_t* rssi, uint8_t* reset_count, uint16_t* scw) {
    // Send command
    print("\rES+R%02X00\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    // Extract values
    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_rx_buf, 3, 2);
    }
    if (reset_count != NULL) {
        *reset_count = (uint8_t) scan_uint(trans_rx_buf, 7, 2);
    }
    if (scw != NULL) {
        *scw = (uint16_t) scan_uint(trans_rx_buf, 9, 4);
    }

    return 1;
}

/*
1. Read status control register (p. 15-16)

rssi - this function will set it to the last RSSI
reset_count - this function will set it to the number of times the transceiver
    has been reset
scw - this function will set it to the 16-bit register data of register
Returns - 1 for success, 0 for failure

Example (different format from datasheet):
ES+R2200
OK+0022DD0303
00 - unknown, probably RSSI
22 - device address
DD - device reset counter (observed to increase by 1 every time the transceiver is reset or power cycled)
0303 - contents of status register
*/
uint8_t get_trans_scw(uint8_t* rssi, uint8_t* reset_count, uint16_t* scw) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = get_trans_scw_attempt(rssi, reset_count, scw);
    }
    return ret;
}


uint8_t set_trans_scw_bit_attempt(uint8_t bit_index, uint8_t value) {
    uint8_t ret;
    uint8_t rssi;
    uint8_t reset_count;
    uint16_t scw;

    ret = get_trans_scw(&rssi, &reset_count, &scw);
    if (ret == 0) {
        return 0;
    }

    if (value == 0) {
        scw &= ~_BV(bit_index);
    } else if (value == 1) {
        scw |= _BV(bit_index);
    }

    ret = set_trans_scw(scw);
    return ret;
}

/*
Sets the specified bit in the SCW register.
bit_index - index of the bit in the SCW register (MSB is 15, LSB is 0)
value - value to set the bit to (0 or 1)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_scw_bit(uint8_t bit_index, uint8_t value) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = set_trans_scw_bit_attempt(bit_index, value);
    }
    return ret;
}


/*
1. Resets the transceiver (using status register)
Returns - 1 for success, 0 for failure

From testing, it seems that the transceiver responds with OK+8787 when the reset
bit is set. It seems that it preserves the value of the status register across
resets.
*/
uint8_t reset_trans(void) {
    uint8_t ret = set_trans_scw_bit(TRANS_RESET, 1);
    // Delay to give the transceiver time to actually reset
    // (without a delay, operations done quickly after reset will always fail)
    _delay_ms(5000);
    return ret;
}


uint8_t set_trans_rf_mode_attempt(uint8_t mode) {
    uint8_t ret;
    uint8_t rssi;
    uint8_t reset_count;
    uint16_t scw;

    ret = get_trans_scw(&rssi, &reset_count, &scw);
    if (ret == 0) {
        return 0;
    }

    // Clear and set bits 10-8
    scw &= 0xF8FF;
    scw |= (mode << 8);

    ret = set_trans_scw(scw);
    return ret;
}

/*
Sets the RF Mode in the SCW register (p. 15).
mode - must be between 0 and 7 (see p. 15 table)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_rf_mode(uint8_t mode) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = set_trans_rf_mode_attempt(mode);
    }
    return ret;
}


/*
1. Turn on echo mode (using status register, p. 16)
Returns - 1 for success, 0 for failure
*/
uint8_t turn_on_trans_echo(void) {
    return set_trans_scw_bit(TRANS_ECHO, 1);
}


/*
1. Turn off echo mode (using status register, p. 16)
Returns - 1 for success, 0 for failure
*/
uint8_t turn_off_trans_echo(void) {
    return set_trans_scw_bit(TRANS_ECHO, 0);
}


/*
1. Turn on beacon mode (using status register)
Returns - 1 for success, 0 for failure
*/
uint8_t turn_on_trans_beacon(void) {
    return set_trans_scw_bit(TRANS_BCN, 1);
}


/*
1. Turn off beacon mode (using status register)
Returns - 1 for success, 0 for failure
*/
uint8_t turn_off_trans_beacon(void) {
    return set_trans_scw_bit(TRANS_BCN, 0);
}


/*
1. Turn on pipe (transparent) mode (using status control register) (p. 15-16).
The transceiver will turn off pipe mode on its own based on the pipe timeout.
Returns - 1 for success, 0 for failure
*/
uint8_t turn_on_trans_pipe(void) {
    return set_trans_scw_bit(TRANS_PIPE, 1);
}


uint8_t set_trans_freq_attempt(uint32_t freq) {
    print("\rES+W%02X01%08lX\r", TRANS_ADDR, freq);

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(2);
    return validity;
}

/*
2. Set transceiver frequency (p. 17-18)

freq - frequency to write, already in the converted 32-bit format that the
       transceiver expects (the output of the fwu conversion program)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_freq(uint32_t freq) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = set_trans_freq_attempt(freq);
    }
    return ret;
}


uint8_t get_trans_freq_attempt(uint8_t* rssi, uint32_t* freq) {
    print("\rES+R%02X01\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_rx_buf, 3, 2);
    }
    if (freq != NULL) {
        *freq = scan_uint(trans_rx_buf, 5, 8);
    }

    return 1;
}

/*
2. Get transceiver frequency (p. 17-18)
rssi -  is set by this function to the last RSSI value
freq - will be set to the read 32-bit value
Returns - 1 for sucess, 0 for failure

Answer format: OK+[RR][FFFFFF][NN]
*/
uint8_t get_trans_freq(uint8_t* rssi, uint32_t* freq) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = get_trans_freq_attempt(rssi, freq);
    }
    return ret;
}


uint8_t set_trans_pipe_timeout_attempt(uint8_t timeout) {
    print("\rES+W%02X06000000%02X\r", TRANS_ADDR, timeout);

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(2);
    return validity;
}

/*
4. Set transparent (pipe) mode timeout (p.18)
Sets timeout to turn off pipe mode if there are no UART messages
timeout - timeout (in seconds) to set
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_pipe_timeout(uint8_t timeout) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = set_trans_pipe_timeout_attempt(timeout);
    }
    return ret;
}


uint8_t get_trans_pipe_timeout_attempt(uint8_t* rssi, uint8_t* timeout) {
    print("\rES+R%02X06\r", TRANS_ADDR);

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    // Extract values
    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_rx_buf, 3, 2);
    }
    if (timeout != NULL) {
        *timeout = (uint8_t) scan_uint(trans_rx_buf, 11, 2);
    }

    return 1;
}

/*
4. Get transparent (pipe) mode timeout (p.18)
Gets timeout to turn off pipe mode if there are no UART messages
rssi -  is set by this function to the last RSSI value
timeout - is set by this function to the timeout (in seconds)
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_pipe_timeout(uint8_t* rssi, uint8_t* timeout) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = get_trans_pipe_timeout_attempt(rssi, timeout);
    }
    return ret;
}


uint8_t set_trans_beacon_period_attempt(uint16_t period) {
    print("\rES+W%02X070000%04X\r", TRANS_ADDR, period);

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(2);
    return validity;
}

/*
5. Set beacon transmission period - Default is 60s (p. 19)
period - desired period between beacon message transmissions (in seconds)
Max val = 0xFFFF = 65535s = 1092min = 18.2h
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_beacon_period(uint16_t period) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = set_trans_beacon_period_attempt(period);
    }
    return ret;
}


uint8_t get_trans_beacon_period_attempt(uint8_t* rssi, uint16_t* period) {
    print("\rES+R%02X07\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_rx_buf, 3, 2);
    }
    if (period != NULL) {
        *period = (uint16_t) scan_uint(trans_rx_buf, 9, 4);
    }

    return 1;
}

/*
5. Get beacon transmission period (p. 19)
rssi -  is set by this function to the last RSSI value
period - set to desired period between beacon message transmissions (in seconds)
Max val = 0xFFFF = 65535s = 1092min = 18.2h
Returns - 1 for success, 0 for failure

Answer: OK+[RR]0000[TTTT]<CR>
*/
uint8_t get_trans_beacon_period(uint8_t* rssi, uint16_t* period) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = get_trans_beacon_period_attempt(rssi, period);
    }
    return ret;
}


uint8_t set_trans_dest_call_sign_attempt(char* call_sign) {
    print("\rES+W%02XF5", TRANS_ADDR);
    for (uint8_t i = 0; i < TRANS_CALL_SIGN_LEN; i++) {
        print("%c", call_sign[i]);
    }
    print("\r");

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(2);
    return validity;
}

/*
7. Set destination call-sign (p.20)
call_sign - the call sign to set (6-byte array without termination or 7-byte array with termination)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_dest_call_sign(char* call_sign) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = set_trans_dest_call_sign_attempt(call_sign);
    }
    return ret;
}


uint8_t get_trans_dest_call_sign_attempt(char* call_sign) {
    print("\rES+R%02XF5\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(9);
    if (validity == 0) {
        return 0;
    }

    if (call_sign != NULL) {
        for (uint8_t i = 0; i < TRANS_CALL_SIGN_LEN; i++) {
            call_sign[i] = trans_rx_buf[3 + i];
        }
        call_sign[TRANS_CALL_SIGN_LEN] = '\0';
    }

    return 1;
}

/*
7. Get destination call-sign (p.20)
call_sign - a 7-char array that will be set by this function to the call sign read (6 chars plus '\0' termination)
Returns - 1 for success, 0 for failure

Answer: OK+DDDDDD<CR>
*/
uint8_t get_trans_dest_call_sign(char* call_sign) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = get_trans_dest_call_sign_attempt(call_sign);
    }
    return ret;
}


uint8_t set_trans_src_call_sign_attempt(char* call_sign) {
    print("\rES+W%02XF6", TRANS_ADDR);
    for (uint8_t i = 0; i < TRANS_CALL_SIGN_LEN; i++) {
        print("%c", call_sign[i]);
    }
    print("\r");

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(2);
    return validity;
}

/*
8. Set source call-sign (p.20)
call_sign - the call sign to set (6-byte array without termination or 7-byte array with termination)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_src_call_sign(char* call_sign) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = set_trans_src_call_sign_attempt(call_sign);
    }
    return ret;
}


uint8_t get_trans_src_call_sign_attempt(char* call_sign) {
    print("\rES+R%02XF6\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(9);
    if (validity == 0) {
        return 0;
    }

    if (call_sign != NULL) {
        for (uint8_t i = 0; i < TRANS_CALL_SIGN_LEN; i++) {
            call_sign[i] = trans_rx_buf[3 + i];
        }
        call_sign[TRANS_CALL_SIGN_LEN] = '\0';
    }

    return 1;
}

/*
8. Get source call-sign (p.20)
call_sign - a 7-char array that will be set by this function to the call sign read (6 chars plus '\0' termination)
Returns - 1 for success, 0 for failure

Answer: OK+DDDDDD<CR>
*/
uint8_t get_trans_src_call_sign(char* call_sign) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = get_trans_src_call_sign_attempt(call_sign);
    }
    return ret;
}


uint8_t get_trans_uptime_attempt(uint8_t* rssi, uint32_t* uptime) {
    print("\rES+R%02X02\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_rx_buf, 3, 2);
    }
    if (uptime != NULL) {
        *uptime = scan_uint(trans_rx_buf, 5, 8);
    }

    return 1;
}

/*
16. Get uptime (p. 23)
rssi - gets set to the last RSSI
uptime - gets set to the uptime
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_uptime(uint8_t* rssi, uint32_t* uptime) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = get_trans_uptime_attempt(rssi, uptime);
    }
    return ret;
}


uint8_t get_trans_num_tx_packets_attempt(uint8_t* rssi, uint32_t* num_tx_packets) {
    print("\rES+R%02X03\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_rx_buf, 3, 2);
    }
    if (num_tx_packets != NULL) {
        *num_tx_packets = scan_uint(trans_rx_buf, 5, 8);
    }

    return 1;
}

/*
17. Get number of transmitted packets (p.23)
rssi - gets set to the last RSSI
num_tx_packets - gets set to the number of transmitted packets
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_num_tx_packets(uint8_t* rssi, uint32_t* num_tx_packets) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = get_trans_num_tx_packets_attempt(rssi, num_tx_packets);
    }
    return ret;
}


uint8_t get_trans_num_rx_packets_attempt(uint8_t* rssi, uint32_t* num_rx_packets) {
    print("\rES+R%02X04\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_rx_buf, 3, 2);
    }
    if (num_rx_packets != NULL) {
        *num_rx_packets = scan_uint(trans_rx_buf, 5, 8);
    }

    return 1;
}

/*
18. Get number of received packets (p.23)
rssi - gets set to the last RSSI
num_rx_packets - gets set to the number of received packets
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_num_rx_packets(uint8_t* rssi, uint32_t* num_rx_packets) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = get_trans_num_rx_packets_attempt(rssi, num_rx_packets);
    }
    return ret;
}


uint8_t get_trans_num_rx_packets_crc_attempt(uint8_t* rssi, uint32_t* num_rx_packets_crc) {
    print("\rES+R%02X05\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_rx_buf, 3, 2);
    }
    if (num_rx_packets_crc != NULL) {
        *num_rx_packets_crc = scan_uint(trans_rx_buf, 5, 8);
    }

    return 1;
}

/*
19. Get number of received packets with CRC error (p.23)
rssi - gets set to the last RSSI
num_rx_packets_crc - gets set to the number of received packets with CRC error
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_num_rx_packets_crc(uint8_t* rssi, uint32_t* num_rx_packets_crc) {
    uint8_t ret = 0;
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        ret = get_trans_num_rx_packets_crc_attempt(rssi, num_rx_packets_crc);
    }
    return ret;
}
