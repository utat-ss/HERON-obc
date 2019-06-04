/*
Transceiver library
EnduroSat UHF Transceiver Type II

See datasheet in Google Drive folder "Design/Comms/New Endurosat Documentation"

Commands are sent through UART (by calling print())
Transceiver responses are handled in trans_uart_rx_cb

Note that all/most configuration functions will return 1 if configuration/read
successful and 0 if not successful.

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
There is a common buffer of characters received over UART from the transceiver in the UART library.
These could either be a response to a command we send it, or a message from the
ground station. In either case, we leave all received UART characters in the
buffer. Every time we receive a new character, scan the entire buffer to see if
the sequence of characters makes sense (i.e. detect it as either a command
response or a received message and set the appropriate boolean flag, which
functions in this library can check for and consume the data if desired). We
set a timeout where if we do not receive any characters for a certain number of
seconds, the entire buffer contents are discarded.

Formerly, we copied all characters from the standard UART buffer to a separate
buffer in this library, but that was unnecessary.

TODO - For most of the get commands, OBC misses the first 1 or 2 characters in
    the response for the first attempt, but the second attempt seems to always work
TODO - it was observed that after either sending data in pipe mode, the
    transceiver sent the UART message "+ESTTC<CR>" - figure out what this is

TODO - perhaps refactor sending a transceiver command and receiving UART back
uint8_t send_trans_cmd(uint8_t resp_len, char* fmt, ...) {
    clear_uart_rx_buf();
    // var args stuff, vsnprintf...
    send_uart();
    // check response
    // if failed, try again
    // return 1 for success, 0 for failure
}
*/

#include "transceiver.h"

/*
All buffers have the following format:
[...] - characters in buffer (NO '\0' OR '\r' TERMINATION)
[...]_len - number of valid characters in buffer (NOT INCLUDING TERMINATION)
[...]_avail - true if the buffer is valid (received something valid)
*/

/* Command response received back from transceiver (detected by \r termination) */
volatile uint8_t    trans_cmd_resp[TRANS_CMD_RESP_MAX_SIZE] = {0x00};
volatile uint8_t    trans_cmd_resp_len = 0;
volatile bool       trans_cmd_resp_avail = false;

// Encoded RX message (from ground station)
volatile uint8_t    trans_rx_enc_msg[TRANS_RX_ENC_MSG_MAX_SIZE] = {0x00};
volatile uint8_t    trans_rx_enc_msg_len = 0;
volatile bool       trans_rx_enc_msg_avail = false;

// Decoded RX message (from ground station)
volatile uint8_t    trans_rx_dec_msg[TRANS_RX_DEC_MSG_MAX_SIZE] = {0x00};
volatile uint8_t    trans_rx_dec_msg_len = 0;
volatile bool       trans_rx_dec_msg_avail = false;

// Decoded TX message (to ground station)
volatile uint8_t    trans_tx_dec_msg[TRANS_TX_DEC_MSG_MAX_SIZE] = {0x00};
volatile uint8_t    trans_tx_dec_msg_len = 0;
volatile bool       trans_tx_dec_msg_avail = false;

// Encoded TX message (to ground station)
volatile uint8_t    trans_tx_enc_msg[TRANS_TX_ENC_MSG_MAX_SIZE] = {0x00};
volatile uint8_t    trans_tx_enc_msg_len = 0;
volatile bool       trans_tx_enc_msg_avail = false;

// Last time we have received a UART character
volatile uint32_t trans_rx_prev_uptime_s = 0;




/*
Initializes the transceiver for UART RX callbacks (does not change any settings).
*/
void init_trans(void) {
    init_trans_uart();
    uart_baud_rate_t previous_baud = UART_BAUD_9600;
    correct_transceiver_baud_rate(&previous_baud);
}

void init_trans_uart(void) {
    set_uart_rx_cb(trans_uart_rx_cb);
    add_uptime_callback(trans_uptime_cb);
}

extern volatile uint8_t uart_rx_buf[];


void trans_uptime_cb(void) {
    // Check for a timeout in receiving characters to clear the buffer
    if (uptime_s > trans_rx_prev_uptime_s &&
        uptime_s - trans_rx_prev_uptime_s >= TRANS_RX_BUF_TIMEOUT &&
        get_uart_rx_buf_count() > 0) {
        
        print("UART RX buf (%u bytes): ", get_uart_rx_buf_count());
        print_bytes((uint8_t*) uart_rx_buf, get_uart_rx_buf_count());
        clear_uart_rx_buf();
        print("\nTimed out, cleared UART RX buf\n");
    }
}

/*
UART RX callback
buf - array of received characters
len - number of received characters
Returns - number of characters processed
*/
uint8_t trans_uart_rx_cb(const uint8_t* buf, uint8_t len) {
    // Save the new time we have received a character
    trans_rx_prev_uptime_s = uptime_s;

    // Output new character
    // put_uart_char(buf[len - 1]);

    // Scan what we have in the buffer now
    // If we found something, clear it from the main UART buffer because the
    // other function has already copied it to a dedicated buffer

    // Command response
    scan_trans_cmd_resp(buf, len);
    if (trans_cmd_resp_avail) {
        print("cmd resp: %u chars: ", trans_cmd_resp_len);
        for (uint8_t i = 0; i < trans_cmd_resp_len; i++) {
            put_uart_char(trans_cmd_resp[i]);
        }
        put_uart_char('\n');
        return len;
    }

    // RX encoded message
    scan_trans_rx_enc_msg(buf, len);
    if (trans_rx_enc_msg_avail) {
        return len;
    }

    // By default, we haven't processed any characters
    return 0;
}


// Scans the contents of trans_cmd_resp for a command response, sets
// trans_cmd_resp_avail if appropriate
void scan_trans_cmd_resp(const uint8_t* buf, uint8_t len) {
    // This should be a safe distinction
    // An RX encoded message should always start with 0x00 and some number
    // It should not start with "OK" and should not end with "\r"

    // Check conditions:
    // - Minimum 3 characters
    // - Not too many characters to overflow the buffer
    //   (ignoring the '\r' termination)
    // - Starts with "OK"
    // - Ends with "\r"

    // "ERR" (command was not sucessful) is considered invalid, but we don't
    // need to check for it explicitly if it is not equal to "OK"

    if (len >= 3 &&
        len - 1 <= TRANS_CMD_RESP_MAX_SIZE &&
        string_cmp(buf, "OK", 2) == 1 &&
        buf[len - 1] == '\r') {

        // Copy all characters except '\r'
        for (uint8_t i = 0; i < len - 1; i++) {
            trans_cmd_resp[i] = buf[i];
        }
        trans_cmd_resp_len = len - 1;
        trans_cmd_resp_avail = true;
    }
}

// Scans the contents of trans_cmd_resp for a received message, sets
// trans_rx_msg_avail if appropriate
// This should be called within an ISR so it is atomic
void scan_trans_rx_enc_msg(const uint8_t* buf, uint8_t len) {
    // Check conditions:
    // - 20 characters, fits in buffer (all RX messages should be the same length)
    // - First byte is 0
    // - Second byte is not 0
    // - Second byte is the number of bytes remaining in the buffer
    if (len == TRANS_RX_ENC_MSG_MAX_SIZE &&
        buf[0] == 0x00 &&
        buf[1] != 0x00 &&
        buf[1] == len - 2) {

        // Copy all characters
        for (uint8_t i = 0; i < len; i++) {
            trans_rx_enc_msg[i] = buf[i];
        }
        trans_rx_enc_msg_len = len;
        trans_rx_enc_msg_avail = true;
    }
}

// trans_rx_enc_msg -> trans_rx_dec_msg
void decode_trans_rx_msg(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!trans_rx_enc_msg_avail) {
            return;
        }
        if (trans_rx_enc_msg_len != TRANS_RX_ENC_MSG_MAX_SIZE) {
            trans_rx_enc_msg_avail = false;
            return;
        }

        uint8_t dec_len = (trans_rx_enc_msg_len - 2) / 2;

        // Decode two ASCII hex byte to one byte
        for (uint8_t i = 0; i < dec_len; i++) {
            trans_rx_dec_msg[i] = scan_uint(trans_rx_enc_msg, 2 + (i * 2), 2);
        }
        trans_rx_dec_msg_len = dec_len;
        trans_rx_dec_msg_avail = true;

        trans_rx_enc_msg_avail = false;
    }
}

// trans_tx_dec_msg -> trans_tx_enc_msg
void encode_trans_tx_msg(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!trans_tx_dec_msg_avail) {
            return;
        }
        if (trans_tx_dec_msg_len == 0 || trans_tx_dec_msg_len > TRANS_TX_DEC_MSG_MAX_SIZE) {
            trans_tx_dec_msg_avail = false;
            return;
        }

        trans_tx_enc_msg[0] = 0x00;
        trans_tx_enc_msg[1] = trans_tx_dec_msg_len * 2;
        // Encode one byte to two ASCII hex bytes
        for (uint8_t i = 0; i < trans_tx_dec_msg_len; i++) {
            trans_tx_enc_msg[2 + (i * 2) + 0] = hex_to_char((trans_tx_dec_msg[i] >> 4) & 0x0F);
            trans_tx_enc_msg[2 + (i * 2) + 1] = hex_to_char(trans_tx_dec_msg[i] & 0x0F);
        }
        trans_tx_enc_msg_len = 2 + (trans_tx_dec_msg_len * 2);
        trans_tx_enc_msg_avail = true;

        trans_tx_dec_msg_avail = false;
    }
}

void send_trans_tx_enc_msg(void) {
    // Assume the transceiver is already in pipe mode (should only be a few
    // seconds since when we received a packet)

    // Make sure all the bytes are sent atomically over UART
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!trans_tx_enc_msg_avail) {
            return;
        }

        uint16_t crc = calc_trans_crc();

        // Send message using the additional packet information
        // From Transceiver Packet Protocol document on Google Drive
        for (uint8_t i = 0; i <= 4; i++) {
            put_uart_char(0x55);
        }
        put_uart_char(0x7E);
        put_uart_char(trans_tx_enc_msg_len);
        for (uint8_t i = 0; i < trans_tx_enc_msg_len; i++) {
            put_uart_char(trans_tx_enc_msg[i]);
        }
        put_uart_char((crc >> 8) & 0xFF);
        put_uart_char(crc & 0xFF);
        // Need to terminate the packet to send it
        put_uart_char('\r');

        trans_tx_enc_msg_avail = false;
    }
}

// Calculates and returns the 16-bit CRC on trans_tx_enc_msg of length
// trans_tx_enc_msg_len
// Based on update_bad_crc() from
// https://github.com/HeronMkII/ground-station/blob/master/crc16.c
uint16_t calc_trans_crc(void) {
    /* based on code found at
    http://www.programmingparadise.com/utility/crc.html
    */
    uint16_t crc = 0xffff;

    // TODO - is it supposed to include the size byte in the calculation?
    update_trans_crc(&crc, trans_tx_enc_msg_len);
    for (uint8_t i = 0; i < trans_tx_enc_msg_len; i++) {
        update_trans_crc(&crc, trans_tx_enc_msg[i]);
    }

    return crc;
}

void update_trans_crc(uint16_t* crc, uint8_t byte) {
    /*
    Why are they shifting this byte left by 8 bits??
    How do the low bits of the poly ever see it?
    */
    uint16_t ch = ((uint16_t) byte) << 8;

    for (uint8_t i = 0; i < 8; i++) {
        uint16_t xor_flag;
        if ((*crc ^ ch) & 0x8000) {
            xor_flag = 1;
        }
        else {
            xor_flag = 0;
        }
        *crc = *crc << 1;
        if (xor_flag) {
            *crc = *crc ^ TRANS_CRC_POLY;
        }
        ch = ch << 1;
    }
}

/*
Converts a number between 0 and 15 to the ASCII representation of it in hex
(uses capital letters).
*/
uint8_t hex_to_char(uint8_t num) {
    if (0x0 <= num && num <= 0x9) {
        return '0' + num;
    } else if (0xA <= num && num <= 0xF) {
        return 'A' + (num - 0xA);
    } else {
        return 0;
    }
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
         to prevent compiler warnings with the `trans_cmd_resp` buffer)
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
        `trans_cmd_resp` buffer)
second - second string to compare
len - number of characters to compare
Returns - 1 if strings are the same, returns 0 otherwise
*/
uint8_t string_cmp(const uint8_t* first, const char* second, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        if (first[i] != second[i]) {
            return 0;
        }
    }
    return 1;
}

void clear_trans_cmd_resp(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        clear_uart_rx_buf();
        trans_cmd_resp_len = 0;
        trans_cmd_resp_avail = false;
    }
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
        _delay_us(2);
        timeout--;
    }
    // Failed if the timeout went to 0
    if (timeout == 0) {
        return 0;
    }

    // Check if the string's length matched the expected number of characters
    // Add 1 to account for the '\r' character
    if (trans_cmd_resp_len != expected_len) {
        return 0;
    }

    // Succeeded
    // If none of the false conditions returned 0, the response is valid
    return 1;
}




uint8_t set_trans_scw_attempt(uint16_t scw) {
    // Send command
    clear_trans_cmd_resp();
    print("\rES+W%02X00%04X\r", TRANS_ADDR, scw);

    // Wait for response
    uint8_t validity = wait_for_trans_cmd_resp(7);
    clear_trans_cmd_resp();
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
    clear_trans_cmd_resp();
    print("\rES+R%02X00\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    // Extract values
    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (reset_count != NULL) {
        *reset_count = (uint8_t) scan_uint(trans_cmd_resp, 7, 2);
    }
    if (scw != NULL) {
        *scw = (uint16_t) scan_uint(trans_cmd_resp, 9, 4);
    }

    clear_trans_cmd_resp();

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
    clear_trans_cmd_resp();
    print("\rES+W%02X01%08lX\r", TRANS_ADDR, freq);

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(2);
    clear_trans_cmd_resp();
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
    clear_trans_cmd_resp();
    print("\rES+R%02X01\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (freq != NULL) {
        *freq = scan_uint(trans_cmd_resp, 5, 8);
    }

    clear_trans_cmd_resp();

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
    clear_trans_cmd_resp();
    print("\rES+W%02X06000000%02X\r", TRANS_ADDR, timeout);

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(2);
    clear_trans_cmd_resp();
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
    clear_trans_cmd_resp();
    print("\rES+R%02X06\r", TRANS_ADDR);

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    // Extract values
    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (timeout != NULL) {
        *timeout = (uint8_t) scan_uint(trans_cmd_resp, 11, 2);
    }

    clear_trans_cmd_resp();

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
    clear_trans_cmd_resp();
    print("\rES+W%02X070000%04X\r", TRANS_ADDR, period);

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(2);
    clear_trans_cmd_resp();
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
    clear_trans_cmd_resp();
    print("\rES+R%02X07\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (period != NULL) {
        *period = (uint16_t) scan_uint(trans_cmd_resp, 9, 4);
    }

    clear_trans_cmd_resp();

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
    clear_trans_cmd_resp();
    print("\rES+W%02XF5", TRANS_ADDR);
    for (uint8_t i = 0; i < TRANS_CALL_SIGN_LEN; i++) {
        print("%c", call_sign[i]);
    }
    print("\r");

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(2);
    clear_trans_cmd_resp();
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
    clear_trans_cmd_resp();
    print("\rES+R%02XF5\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(9);
    if (validity == 0) {
        return 0;
    }

    if (call_sign != NULL) {
        for (uint8_t i = 0; i < TRANS_CALL_SIGN_LEN; i++) {
            call_sign[i] = trans_cmd_resp[3 + i];
        }
        call_sign[TRANS_CALL_SIGN_LEN] = '\0';
    }

    clear_trans_cmd_resp();

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
    clear_trans_cmd_resp();
    print("\rES+W%02XF6", TRANS_ADDR);
    for (uint8_t i = 0; i < TRANS_CALL_SIGN_LEN; i++) {
        print("%c", call_sign[i]);
    }
    print("\r");

    // Wait for response
    //Check validity
    uint8_t validity = wait_for_trans_cmd_resp(2);
    clear_trans_cmd_resp();
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
    clear_trans_cmd_resp();
    print("\rES+R%02XF6\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(9);
    if (validity == 0) {
        return 0;
    }

    if (call_sign != NULL) {
        for (uint8_t i = 0; i < TRANS_CALL_SIGN_LEN; i++) {
            call_sign[i] = trans_cmd_resp[3 + i];
        }
        call_sign[TRANS_CALL_SIGN_LEN] = '\0';
    }

    clear_trans_cmd_resp();

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
    clear_trans_cmd_resp();
    print("\rES+R%02X02\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (uptime != NULL) {
        *uptime = scan_uint(trans_cmd_resp, 5, 8);
    }

    clear_trans_cmd_resp();

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
    clear_trans_cmd_resp();
    print("\rES+R%02X03\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (num_tx_packets != NULL) {
        *num_tx_packets = scan_uint(trans_cmd_resp, 5, 8);
    }

    clear_trans_cmd_resp();

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
    clear_trans_cmd_resp();
    print("\rES+R%02X04\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (num_rx_packets != NULL) {
        *num_rx_packets = scan_uint(trans_cmd_resp, 5, 8);
    }

    clear_trans_cmd_resp();

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
    clear_trans_cmd_resp();
    print("\rES+R%02X05\r", TRANS_ADDR);

    //Wait for response
    //check validity
    uint8_t validity = wait_for_trans_cmd_resp(13);
    if (validity == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (num_rx_packets_crc != NULL) {
        *num_rx_packets_crc = scan_uint(trans_cmd_resp, 5, 8);
    }

    clear_trans_cmd_resp();

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

/*
Checks that the transceiver baud rate is 9600. If not, then attempts to read transceiver's baud rate and then set it back to 9600 by changing MCU baud rate.
Assumes the UART's baud rate is already set to 9600

previous - pointer to transceiver's previous baud_rate
Returns 1 if success, 0 if failed
*/
uint8_t correct_transceiver_baud_rate(uart_baud_rate_t* previous) {
    uint8_t rssi = 0, reset_count = 0;
    uint16_t scw = 0;

    // Check if the transciever is already at 9600, if if is we don't have to do anything
    uint8_t received = get_trans_scw(&rssi, &reset_count, &scw);
    if (received == 1) {
        *previous = UART_BAUD_9600;
        return 1;
    }

    uint8_t baud_rate = UART_BAUD_1200;
    // Iterate through the baud rates and see which one works
    for ( ;baud_rate <= UART_BAUD_115200; baud_rate++) {
        // Set the MCU baud rate
        set_uart_baud_rate(baud_rate);
        received = get_trans_scw(&rssi, &reset_count, &scw);
        // Break out of the loop if we found the baudrate
        if (received == 1) {
            break;
        }
    }

    // set bits 12 and 13 of the scw to 00 which sets baud rate to 9600
    uint16_t scw_new = (scw & ~_BV(12)) & ~_BV(13);

    set_trans_scw(scw_new);
    // Set the UART baud rate back to 9600
    set_uart_baud_rate(UART_BAUD_9600);

    // Make sure it got set
    received = get_trans_scw(&rssi, &reset_count, &scw);
    if (received == 1 && scw == scw_new) {
        *previous = baud_rate;
        return 1;
    }
    else {
        return 0;
    }
}
