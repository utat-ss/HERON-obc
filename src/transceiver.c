/*
Transceiver library
EnduroSat UHF Transceiver Type II

See datasheet in Google Drive folder "Design/Comms/New Endurosat Documentation"

Commands are sent through UART (by calling print())
Transceiver responses are handled in trans_uart_rx_cb

Note that all/most configuration functions will return 1 if configuration/read
successful and 0 if not successful.

Each of the command functions calls a send_trans_command function which 
attempts to send a message to the transceiver until it receives a 
valid response (with a maximum number of attempts)

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

Formerly we copied all characters from the standard UART buffer to a separate
buffer in this library, but that was unnecessary.

Note that when pipe mode times out, the transceiver automatically sends the
following 16 bytes of UART to OBC:
"+ESTTC [CCCCCCCC]<CR>"
where [CCCCCCCC] is the 8-byte checksum.
In a raw hexdump of bytes, this is:
2b:45:53:54:54:43:20:43:46:42:35:32:44:33:35:0d
*/

#include "transceiver.h"

// Uncomment for extra debugging prints
#define TRANSCEIVER_DEBUG

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
volatile uint8_t    trans_rx_enc_len = 0;
volatile bool       trans_rx_enc_avail = false;

// Decoded RX message (from ground station)
volatile uint8_t    trans_rx_dec_msg[TRANS_RX_DEC_MSG_MAX_SIZE] = {0x00};
volatile uint8_t    trans_rx_dec_len = 0;
volatile bool       trans_rx_dec_avail = false;

// ACK/NACK to send to ground
volatile uint8_t    trans_tx_ack_opcode = 0;
volatile uint32_t   trans_tx_ack_arg1 = 0;
volatile uint32_t   trans_tx_ack_arg2 = 0;
volatile uint8_t    trans_tx_ack_status = 0xFF;
volatile bool       trans_tx_ack_avail = false;

// Decoded TX message (to ground station)
volatile uint8_t    trans_tx_dec_msg[TRANS_TX_DEC_MSG_MAX_SIZE] = {0x00};
volatile uint8_t    trans_tx_dec_len = 0;
volatile bool       trans_tx_dec_avail = false;

// Encoded TX message (to ground station)
volatile uint8_t    trans_tx_enc_msg[TRANS_TX_ENC_MSG_MAX_SIZE] = {0x00};
volatile uint8_t    trans_tx_enc_len = 0;
volatile bool       trans_tx_enc_avail = false;

// Last time we have received a UART character
volatile uint32_t trans_rx_prev_uptime_s = 0;

// Set to true to print transceiver messages
bool print_trans_msgs = false;



// TODO: Clean up -> make lib-common PRINT_BUF_SIZE visible to outside
// UART print buff used ot send commands
#ifndef PRINT_BUF_SIZE
#define PRINT_BUF_SIZE 80
#endif
extern uint8_t print_buf[PRINT_BUF_SIZE];
#define COMMAND_BUF_SIZE 80
static uint8_t command_buf[COMMAND_BUF_SIZE];


/*
Initializes the transceiver for UART RX callbacks (does not change any settings).
*/
void init_trans(void) {
    init_trans_uart();
}

void init_trans_uart(void) {
    set_uart_rx_cb(trans_uart_rx_cb);
    add_uptime_callback(trans_uptime_cb);
}

void trans_uptime_cb(void) {
    // Check for a timeout in receiving characters to clear the buffer
    if (uptime_s > trans_rx_prev_uptime_s &&
        uptime_s - trans_rx_prev_uptime_s >= TRANS_RX_BUF_TIMEOUT &&
        get_uart_rx_count() > 0) {

#ifdef TRANSCEIVER_DEBUG
        print("UART RX buf (%u bytes): ", get_uart_rx_count());
        print_bytes((uint8_t*) get_uart_rx_buf(), get_uart_rx_count());
        print("\nTimed out, clearing UART RX buf\n");
#endif

        // Only send an ACK if we received more than 1 byte from a packet
        // i.e. ignore 1-byte ground station packets that are used to improve
        // transmission reliability
        // TODO - what threshold?
        if (get_uart_rx_count() > 1) {
            add_trans_tx_ack(CMD_OPCODE_UNKNOWN, CMD_ARG_UNKNOWN, CMD_ARG_UNKNOWN, CMD_ACK_STATUS_INVALID_PKT);
        }

        // Clearing buffer must happen after checking length
        clear_uart_rx_buf();
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
    if (trans_rx_enc_avail) {
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

// Scans the contents of the UART RX buffer for a received message, sets
// trans_rx_msg_avail if appropriate
// This should be called within an ISR so it is atomic
void scan_trans_rx_enc_msg(const uint8_t* buf, uint8_t len) {
    // Check conditions
    // Check the most likely to fail conditions first (want to process as quickly as possible if still receiving characters)
    // This callback will only check for the 0x00 bytes to be fast,
    // and to NACK in a different place if the length doesn't match the data
    if (len >= 5 &&
        buf[0] == 0x00 &&
        buf[2] == 0x00 &&
        buf[len - 1] == 0x00 &&
        len <= TRANS_RX_ENC_MSG_MAX_SIZE) {

        // Copy all characters
        for (uint8_t i = 0; i < len; i++) {
            trans_rx_enc_msg[i] = buf[i];
        }
        trans_rx_enc_len = len;
        trans_rx_enc_avail = true;
    }
}

void add_trans_tx_ack(uint8_t opcode, uint32_t arg1, uint32_t arg2, uint8_t status) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        trans_tx_ack_opcode = opcode;
        trans_tx_ack_arg1 = arg1;
        trans_tx_ack_arg2 = arg2;
        trans_tx_ack_status = status;
        trans_tx_ack_avail = true;
    }
}

void print_uint64(uint64_t num) {
    print("0x%.8lx%.8lx",
        (uint32_t)((num >> 32) & 0xFFFFFFFF),
        (uint32_t)(num & 0xFFFFFFFF));
}

// trans_rx_enc_msg -> trans_rx_dec_msg
void decode_trans_rx_msg(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Check encoded message available
        if (!trans_rx_enc_avail) {
            return;
        }
        trans_rx_enc_avail = false;

        if (print_trans_msgs) {
            print("\n");
            print("Trans RX (Encoded): %u bytes: ", trans_rx_enc_len);
            print_bytes((uint8_t*) trans_rx_enc_msg, trans_rx_enc_len);
        }

        // Check invalid length
        if (!(trans_rx_enc_msg[1] > 0x10 &&
            trans_rx_enc_msg[1] - 0x10 == trans_rx_enc_len - 4)) {
            // NACK for invalid matching of length byte to length
            add_trans_tx_ack(CMD_OPCODE_UNKNOWN, CMD_ARG_UNKNOWN, CMD_ARG_UNKNOWN, CMD_ACK_STATUS_INVALID_PKT);
            return;
        }

        // length of base-254 encoded message can be extracted from the first field, the mapping is undone
        uint8_t enc_len = trans_rx_enc_msg[1] - 0x10;
        // 64 bit integer that will hold the 56 bit values from the byte group
        uint64_t base_conversion_buff = 0;
        // concatenate the message into 8 byte groups and leftovers, then calculate length
        uint8_t num_byte_groups = enc_len / 8;
        uint8_t num_remainder_bytes = enc_len % 8;

        uint8_t dec_len;
        // TODO - what if 1 remainder byte?
        if (num_remainder_bytes == 0) {
            dec_len = num_byte_groups * 7;
        } else {
            dec_len = (num_byte_groups * 7) + (num_remainder_bytes - 1);
        }

        // print("enc_len = %u\n", enc_len);
        // print("dec_len = %u\n", dec_len);
        // print("num_byte_groups = %u\n", num_byte_groups);
        // print("num_remainder_bytes = %u\n", num_remainder_bytes);

        // unmap the values in the buffer
        for(uint8_t i = 0; i < enc_len; i++) {
            if(trans_rx_enc_msg[3 + i] >= 1 && trans_rx_enc_msg[3 + i] <= 12)
                trans_rx_enc_msg[3 + i] -= 1;
            else if(trans_rx_enc_msg[3 + i] >= 14 && trans_rx_enc_msg[3 + i] <= 255)
                trans_rx_enc_msg[3 + i] -= 2;
        }

        // print("trans_rx_enc_msg = ");
        // print_bytes(trans_rx_enc_msg, trans_rx_enc_len);

        // Set up array of powers of 254
        // ULL is unsigned long long (unsigned 64-bit on AVR)
        // [0] = 254^0, [7] = 254^7
        uint64_t pow_254[8] = {0x00};
        pow_254[0] = 1;
        for (uint8_t i = 1; i < 8; i++) {
            pow_254[i] = pow_254[i - 1] * 254ULL;
        }

        // print("pow_254 =");
        // for (uint8_t i = 0; i < 8; i++) {
        //     print(" ");
        //     print_uint64(pow_254[i]);
        // }
        // print("\n");

        for (uint8_t i_group = 0; i_group < num_byte_groups; i_group++) {
            base_conversion_buff = 0;

            for (uint8_t i_byte = 0; i_byte < 8; i_byte++) {
                base_conversion_buff += trans_rx_enc_msg[ 3 + (8 * i_group) + i_byte ] * pow_254[7 - i_byte];
                // print("base_conversion_buff = ");
                // print_uint64(base_conversion_buff);
                // print("\n");
            }

            for (uint8_t i_byte = 0; i_byte < 7; i_byte++) {
                trans_rx_dec_msg[ (7 * i_group) + i_byte ] = (base_conversion_buff / (1ULL << ((6 - i_byte) * 8))) % 256;
                // print("trans_rx_dec_msg = ");
                // print_bytes(trans_rx_dec_msg, dec_len);
            }
        }

        if (num_remainder_bytes > 1) {
            base_conversion_buff = 0;
            for (uint8_t i_byte = 0; i_byte < num_remainder_bytes; i_byte++) 
                base_conversion_buff += trans_rx_enc_msg[ 3 + (num_byte_groups * 8) + i_byte ] * pow_254[num_remainder_bytes - 1 - i_byte];
            // print("base_conversion_buff = ");
            // print_uint64(base_conversion_buff);
            // print("\n");

            for (uint8_t i_byte = 0; i_byte < (num_remainder_bytes - 1); i_byte++)
                trans_rx_dec_msg[ (num_byte_groups * 7) + i_byte ] = (base_conversion_buff / (1ULL << ((num_remainder_bytes - 2 - i_byte) * 8))) % 256;
            // print("trans_rx_dec_msg = ");
            // print_bytes(trans_rx_dec_msg, dec_len);
        }

        // print("base_conversion_buff = ");
        // print_uint64(base_conversion_buff);
        // print("\n");

        trans_rx_dec_len = dec_len;
        trans_rx_dec_avail = true;

        // print("trans_rx_dec_len = %u\n", trans_rx_dec_len);
        // print("trans_rx_dec_avail = %u\n", trans_rx_dec_avail);
        // print("trans_rx_dec_msg = ");
        // print_bytes(trans_rx_dec_msg, trans_rx_dec_len);
    }
}

// trans_tx_dec_msg -> trans_tx_enc_msg
void encode_trans_tx_msg(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!trans_tx_dec_avail) {
            return;
        }
        trans_tx_dec_avail = false;

        if (print_trans_msgs) {
            print("Trans TX (Decoded): %u bytes: ", trans_tx_dec_len);
            print_bytes((uint8_t*) trans_tx_dec_msg, trans_tx_dec_len);
        }

        if (trans_tx_dec_len == 0 || trans_tx_dec_len > TRANS_TX_DEC_MSG_MAX_SIZE) {
            return;
        }

        // 64 bit integer that will hold the 56 bit values from the byte groups
        uint64_t base_conversion_buff = 0;
        // Number of 7 byte groups in the decoded message
        uint8_t num_byte_groups = trans_tx_dec_len / 7;
        // Number of bytes leftover
        uint8_t num_remainder_bytes = trans_tx_dec_len % 7;
        // Encoded length
        uint8_t enc_len = (num_remainder_bytes > 0) ? (num_byte_groups * 8 + num_remainder_bytes + 1) : (num_byte_groups * 8);

        // print("enc_len = %u\n", enc_len);
        // print("num_byte_groups = %u\n", num_byte_groups);
        // print("num_remainder_bytes = %u\n", num_remainder_bytes);

        // All encoded messages start with 0x00
        trans_tx_enc_msg[0] = 0x00;
        // Next field is the length. This value will later be mapped similar to the other bytes.
        trans_tx_enc_msg[1] = enc_len + 0x10;
        trans_tx_enc_msg[2] = 0x00;

        // print("trans_tx_enc_msg = ");
        // print_bytes(trans_tx_enc_msg, trans_tx_enc_len);

        // Set up array of powers of 254
        // ULL is unsigned long long (unsigned 64-bit on AVR)
        // [0] = 254^0, [7] = 254^7
        uint64_t pow_254[8] = {0x00};
        pow_254[0] = 1;
        for (uint8_t i = 1; i < 8; i++) {
            pow_254[i] = pow_254[i - 1] * 254ULL;
        }

        // print("pow_254 =");
        // for (uint8_t i = 0; i < 8; i++) {
        //     print(" ");
        //     print_uint64(pow_254[i]);
        // }
        // print("\n");

        // Convert each of the 7 base-256 digits to 8 base-254 digits
        for (uint8_t i_group = 0; i_group < num_byte_groups; i_group++) {
            base_conversion_buff = 0;

            for (uint8_t i_byte = 0; i_byte < 7; i_byte++) {
                base_conversion_buff += trans_tx_dec_msg[ (7 * i_group) + i_byte] * (1ULL << ((6 - i_byte) * 8));  
                // print("base_conversion_buff = ");
                // print_uint64(base_conversion_buff);
                // print("\n");
            }
            
            for (uint8_t i_byte = 0; i_byte < 8; i_byte++) {
                trans_tx_enc_msg[3 + (8 * i_group) + i_byte] = (base_conversion_buff / pow_254[7 - i_byte]) % 254;
                // print("trans_tx_enc_msg = ");
                // print_bytes(trans_tx_enc_msg, enc_len + 4);
            }
        }

        // encode the remainining bytes
        if(num_remainder_bytes > 0) {
            base_conversion_buff = 0;
            for (uint8_t i_byte = 0; i_byte < num_remainder_bytes; i_byte++) {
                base_conversion_buff += trans_tx_dec_msg[num_byte_groups*7 + i_byte] * (1ULL << ((num_remainder_bytes - 1 - i_byte) * 8));
                // print("base_conversion_buff = ");
                // print_uint64(base_conversion_buff);
                // print("\n");
            }
            for (uint8_t i_byte = 0; i_byte < (num_remainder_bytes + 1); i_byte++) {
                trans_tx_enc_msg[3 + (num_byte_groups * 8) + i_byte] = (base_conversion_buff / pow_254[num_remainder_bytes - i_byte]) % 254;
                // print("trans_tx_enc_msg = ");
                // print_bytes(trans_tx_enc_msg, enc_len + 4);
            }  
        }

        // Perform the mapping to avoid values 0 and 13. 0-11 -> 1-12, 12-253 -> 14-255
        // Note that the length of the message is not put through this mapping. The other digit that doesn't get mapped is the 0x00.
        for (uint8_t i = 3; i < 3 + enc_len; i++) {
            if( trans_tx_enc_msg[i] >= 0 && trans_tx_enc_msg[i] <= 11 ) {
                trans_tx_enc_msg[i] += 1;
            }
            else if( trans_tx_enc_msg[i] >= 12 && trans_tx_enc_msg[i] <= 253 ) {
                trans_tx_enc_msg[i] += 2;
            }
        }

        trans_tx_enc_msg[3 + enc_len] = 0x00;
        trans_tx_enc_len = 4 + enc_len;
        trans_tx_enc_avail = true;

        // print("trans_tx_enc_len = %u\n", trans_tx_enc_len);
        // print("trans_tx_enc_avail = %u\n", trans_tx_enc_avail);
        // print("trans_tx_enc_msg = ");
        // print_bytes(trans_tx_enc_msg, trans_tx_enc_len);
    }
}

void send_trans_tx_enc_msg(void) {
    // Assume the transceiver is already in pipe mode (should only be a few
    // seconds since when we received a packet)

    // Make sure all the bytes are sent atomically over UART
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!trans_tx_enc_avail) {
            return;
        }

        if (print_trans_msgs) {
            print("Trans TX (Encoded): %u bytes: ", trans_tx_enc_len);
            print_bytes((uint8_t*) trans_tx_enc_msg, trans_tx_enc_len);
        }

        // We only need to supply the message, not any additional packet
        // information from Transceiver Packet Protocol document

        // Send a CR termination to terminate any packet accidentally sent from
        // print statements we previously sent over UART
        put_uart_char('\r');
        for (uint8_t i = 0; i < trans_tx_enc_len; i++) {
            put_uart_char(trans_tx_enc_msg[i]);
        }
        // Need to terminate the packet to send it
        put_uart_char('\r');

        trans_tx_enc_avail = false;
    }
}

/*
 * Calculates the checksum for the string message
 */
uint32_t crc32(unsigned char *message) {
   int8_t i, j;
   uint32_t byte;
   uint32_t mask;
   uint32_t crc = 0xFFFFFFFF;

   i = 0;
   while (message[i] != 0) {
      byte = message[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);    // 0xEDB88320 is 0x04C11DB7 reversed
      }
      i = i + 1;
   }
   return ~crc;
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
    if (trans_cmd_resp_len != expected_len + 9) {
        return 0;
    }

    // Succeeded
    // If none of the false conditions returned 0, the response is valid
    return 1;
}

bool send_trans_cmd(uint8_t expected_len, char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf((char *) command_buf, PRINT_BUF_SIZE, fmt, args);
    va_end(args);
    
    // Generate check sum
    uint32_t check_sum = crc32(command_buf);
    
    uint8_t ret = 0;

    // Attempt to send some commands and wait for response with timeout
    for (uint8_t i = 0; (i < TRANS_MAX_CMD_ATTEMPTS) && (ret == 0); i++) {
        // Regenerate the print buffer in case you get interrupted
        // va_start(args, fmt);
        // vsnprintf((char *) print_buf, PRINT_BUF_SIZE, fmt, args);
        // va_end(args);

        // Send command
        clear_trans_cmd_resp();
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print("\r%s %.8lX\r", command_buf, check_sum);
            // send_uart(print_buf, strlen((char*) print_buf)); // Command
        }

        // Wait for response
        ret = wait_for_trans_cmd_resp(expected_len);
    }

    return ret;
}


/*
1. Write to status control register (p. 15-16)

scw - 16-bit data to write to register
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_scw(uint16_t scw) {
    uint8_t ret = send_trans_cmd(7, "ES+W%02X00%04X", TRANS_ADDR, scw);
    clear_trans_cmd_resp();
    return ret;
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
    uint8_t ret = send_trans_cmd(13, "ES+R%02X00", TRANS_ADDR);
    if (ret) {
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
    }
    return ret;
}


/*
Sets the specified bit in the SCW register.
bit_index - index of the bit in the SCW register (MSB is 15, LSB is 0)
value - value to set the bit to (0 or 1)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_scw_bit(uint8_t bit_index, uint8_t value) {
    uint8_t rssi;
    uint8_t reset_count;
    uint16_t scw;

    uint8_t ret = get_trans_scw(&rssi, &reset_count, &scw);
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


/*
Sets the RF Mode in the SCW register (p. 15).
mode - must be between 0 and 7 (see p. 15 table)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_rf_mode(uint8_t mode) {
    uint8_t rssi;
    uint8_t reset_count;
    uint16_t scw;

    uint8_t ret = get_trans_scw(&rssi, &reset_count, &scw);
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


/*
2. Set transceiver frequency (p. 17-18)

freq - frequency to write, already in the converted 32-bit format that the
       transceiver expects (the output of the fwu conversion program)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_freq(uint32_t freq) {
    uint8_t ret = send_trans_cmd(2, "ES+W%02X01%08lX", TRANS_ADDR, freq);
    clear_trans_cmd_resp();
    return ret;
}


/*
2. Get transceiver frequency (p. 17-18)
rssi -  is set by this function to the last RSSI value
freq - will be set to the read 32-bit value
Returns - 1 for sucess, 0 for failure

Answer format: OK+[RR][FFFFFF][NN]
*/
uint8_t get_trans_freq(uint8_t* rssi, uint32_t* freq) {
    uint8_t ret = send_trans_cmd(13, "ES+R%02X01", TRANS_ADDR);
    if (ret == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (freq != NULL) {
        *freq = scan_uint(trans_cmd_resp, 5, 8);
    }

    clear_trans_cmd_resp();
    return ret;
}


/*
4. Set transparent (pipe) mode timeout (p.18)
Sets timeout to turn off pipe mode if there are no UART messages
timeout - timeout (in seconds) to set
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_pipe_timeout(uint8_t timeout) {
    uint8_t ret = send_trans_cmd(2, "ES+W%02X06000000%02X", TRANS_ADDR, timeout);
    clear_trans_cmd_resp();
    return ret;
}


/*
4. Get transparent (pipe) mode timeout (p.18)
Gets timeout to turn off pipe mode if there are no UART messages
rssi -  is set by this function to the last RSSI value
timeout - is set by this function to the timeout (in seconds)
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_pipe_timeout(uint8_t* rssi, uint8_t* timeout) {
    uint8_t ret = send_trans_cmd(13, "ES+R%02X06", TRANS_ADDR);
    if (ret == 0) {
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
    return ret;
}


/*
5. Set beacon transmission period - Default is 60s (p. 19)
period - desired period between beacon message transmissions (in seconds)
Max val = 0xFFFF = 65535s = 1092min = 18.2h
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_beacon_period(uint16_t period) {
    uint8_t ret = send_trans_cmd(2, "ES+W%02X070000%04X", TRANS_ADDR, period);
    clear_trans_cmd_resp();
    return ret;
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
    uint8_t ret = send_trans_cmd(13, "ES+R%02X07", TRANS_ADDR);
    if (ret == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (period != NULL) {
        *period = (uint16_t) scan_uint(trans_cmd_resp, 9, 4);
    }

    clear_trans_cmd_resp();
    return ret;
}

/*
2.2.6. Set beacon message content configuration - default is "Hello, world!" (p.6)
content - string, must be zero-terminated, but '\0' will not be included in the message
    - it must NOT include the <CR> (13 character), see datasheet
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_beacon_content(char* content) {
    uint8_t ret = send_trans_cmd(2, "ES+W%02XFB%02X%s", TRANS_ADDR, strlen(content), content);
    if (ret == 0) {
        return 0;
    }
    clear_trans_cmd_resp();
    return ret;
}

// Don't need to implement a get content function

/*
7. Set destination call-sign (p.20)
call_sign - the call sign to set (6-byte array without termination or 7-byte array with termination)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_dest_call_sign(char* call_sign) {
    uint8_t ret = send_trans_cmd(2, "ES+W%02XF5%s", TRANS_ADDR, call_sign);
    clear_trans_cmd_resp();    
    return ret;
}


/*
7. Get destination call-sign (p.20)
call_sign - a 7-char array that will be set by this function to the call sign read (6 chars plus '\0' termination)
Returns - 1 for success, 0 for failure

Answer: OK+DDDDDD<CR>
*/
uint8_t get_trans_dest_call_sign(char* call_sign) {
    uint8_t ret = send_trans_cmd(9, "ES+R%02XF5", TRANS_ADDR);
    if (ret == 0) {
        return 0;
    }

    if (call_sign != NULL) {
        for (uint8_t i = 0; i < TRANS_CALL_SIGN_LEN; i++) {
            call_sign[i] = trans_cmd_resp[3 + i];
        }
        call_sign[TRANS_CALL_SIGN_LEN] = '\0';
    }

    clear_trans_cmd_resp();
    return ret;
}


/*
8. Set source call-sign (p.20)
call_sign - the call sign to set (6-byte array without termination or 7-byte array with termination)
Returns - 1 for success, 0 for failure
*/
uint8_t set_trans_src_call_sign(char* call_sign) {
    uint8_t ret = send_trans_cmd(2, "ES+W%02XF6%s", TRANS_ADDR, call_sign);
    clear_trans_cmd_resp();    
    return ret;
}


/*
8. Get source call-sign (p.20)
call_sign - a 7-char array that will be set by this function to the call sign read (6 chars plus '\0' termination)
Returns - 1 for success, 0 for failure

Answer: OK+DDDDDD<CR>
*/
uint8_t get_trans_src_call_sign(char* call_sign) {
    uint8_t ret = send_trans_cmd(9, "ES+R%02XF6", TRANS_ADDR);
    if (ret == 0) {
        return 0;
    }

    if (call_sign != NULL) {
        for (uint8_t i = 0; i < TRANS_CALL_SIGN_LEN; i++) {
            call_sign[i] = trans_cmd_resp[3 + i];
        }
        call_sign[TRANS_CALL_SIGN_LEN] = '\0';
    }

    clear_trans_cmd_resp();
    return ret;
}


/*
16. Get uptime (p. 23)
rssi - gets set to the last RSSI
uptime - gets set to the uptime
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_uptime(uint8_t* rssi, uint32_t* uptime) {
    uint8_t ret = send_trans_cmd(13, "ES+R%02X02", TRANS_ADDR);
    if (ret == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (uptime != NULL) {
        *uptime = scan_uint(trans_cmd_resp, 5, 8);
    }

    clear_trans_cmd_resp();
    return ret;
}


/*
17. Get number of transmitted packets (p.23)
rssi - gets set to the last RSSI
num_tx_packets - gets set to the number of transmitted packets
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_num_tx_packets(uint8_t* rssi, uint32_t* num_tx_packets) {
    uint8_t ret = send_trans_cmd(13, "ES+R%02X03", TRANS_ADDR);
    if (ret == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (num_tx_packets != NULL) {
        *num_tx_packets = scan_uint(trans_cmd_resp, 5, 8);
    }

    clear_trans_cmd_resp();
    return ret;
}


/*
18. Get number of received packets (p.23)
rssi - gets set to the last RSSI
num_rx_packets - gets set to the number of received packets
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_num_rx_packets(uint8_t* rssi, uint32_t* num_rx_packets) {
    uint8_t ret = send_trans_cmd(13, "ES+R%02X04", TRANS_ADDR);
    if (ret == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (num_rx_packets != NULL) {
        *num_rx_packets = scan_uint(trans_cmd_resp, 5, 8);
    }

    clear_trans_cmd_resp();
    return ret;
}


/*
19. Get number of received packets with CRC error (p.23)
rssi - gets set to the last RSSI
num_rx_packets_crc - gets set to the number of received packets with CRC error
Returns - 1 for success, 0 for failure
*/
uint8_t get_trans_num_rx_packets_crc(uint8_t* rssi, uint32_t* num_rx_packets_crc) {
    uint8_t ret = send_trans_cmd(13, "ES+R%02X05", TRANS_ADDR);
    if (ret == 0) {
        return 0;
    }

    if (rssi != NULL) {
        *rssi = (uint8_t) scan_uint(trans_cmd_resp, 3, 2);
    }
    if (num_rx_packets_crc != NULL) {
        *num_rx_packets_crc = scan_uint(trans_cmd_resp, 5, 8);
    }

    clear_trans_cmd_resp();
    return ret;
}

/*
Checks that the transceiver baud rate is 9600. If not, then attempts to read transceiver's baud rate and then set it back to 9600 by changing MCU baud rate.
Assumes the UART's baud rate is already set to 9600

previous - pointer to transceiver's previous baud_rate
Returns 1 if success, 0 if failed
*/
uint8_t correct_transceiver_baud_rate(const uart_baud_rate_t new_rate, uart_baud_rate_t* previous) {
    uint8_t rssi = 0, reset_count = 0;
    uint16_t scw = 0;

    set_uart_baud_rate(new_rate);
    // Check if the transciever is already at the baud rate that we want, if it is we don't have to do anything
    uint8_t received = get_trans_scw(&rssi, &reset_count, &scw);
    if (received == 1) {
        *previous = new_rate;
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

    // set bits 12 and 13 of the scw to correspond to the new baud rate
    uint16_t scw_new;
    switch (new_rate) {
        case UART_BAUD_1200:
            // 01
            scw_new = (scw & ~_BV(13)) | _BV(12);
            break;
        case UART_BAUD_9600:
            // 00
            scw_new = (scw & ~_BV(13)) & ~_BV(12);
            break;
        case UART_BAUD_19200:
            // 10
            scw_new = (scw | _BV(13)) & ~_BV(12);
            break;
        case UART_BAUD_115200:
            // 11
            scw_new = (scw | _BV(13)) | _BV(12);
            break;
        default: // Default baud rate will be 9600
            scw_new = (scw & ~_BV(13)) & ~_BV(12);
            break;
    }

    set_trans_scw(scw_new);
    // Set the UART baud rate to new rate
    set_uart_baud_rate(new_rate);

    // Make sure it got set
    received = get_trans_scw(&rssi, &reset_count, &scw);
    if (received == 1) {
        *previous = baud_rate;
        return 1;
    }
    else {
        return 0;
    }
}
