/*
Program to test the transceiver library for encoding and sending messages over
    UART.
NOTE: This does not use the actual transceiver, just a laptop with CoolTerm
    and UART from OBC.

Other Notes:
- Need to unplug SCK/RX from the PCB when uploading a new program, then plug it
    back in when the program is running
- Might need/want to press the reset button on the PCB to restart the program
*/

#include <uart/uart.h>
#include <utilities/utilities.h>
#include <uptime/uptime.h>

#include "../../src/rtc.h"
#include "../../src/transceiver.h"

void add_standard_message(uint8_t msg_type, uint32_t arg1, uint32_t arg2) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        trans_tx_dec_msg[0] = msg_type;
        trans_tx_dec_msg[1] = (arg1 >> 24) & 0xFF;
        trans_tx_dec_msg[2] = (arg1 >> 16) & 0xFF;
        trans_tx_dec_msg[3] = (arg1 >> 8) & 0xFF;
        trans_tx_dec_msg[4] = arg1 & 0xFF;
        trans_tx_dec_msg[5] = (arg2 >> 24) & 0xFF;
        trans_tx_dec_msg[6] = (arg2 >> 16) & 0xFF;
        trans_tx_dec_msg[7] = (arg2 >> 8) & 0xFF;
        trans_tx_dec_msg[8] = arg2 & 0xFF;

        trans_tx_dec_msg_len = 9;
        trans_tx_dec_msg_avail = true;
    }
}

void add_string_message(char* string) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Don't add the terminating '\0' character
        uint8_t len = strlen(string);
        for (uint8_t i = 0; i < len; i++) {
            trans_tx_dec_msg[i] = string[i];
        }
        trans_tx_dec_msg_len = len;
        trans_tx_dec_msg_avail = true;
    }
}

void print_decoded(void) {
    if (!trans_tx_dec_msg_avail) {
        print("No decoded message available\n");
        return;
    }
    print("Decoded: ");
    print_bytes((uint8_t*) trans_tx_dec_msg, trans_tx_dec_msg_len);
}

void print_encoded(void) {
    if (!trans_tx_enc_msg_avail) {
        print("No encoded message available\n");
        return;
    }
    print("Encoded: ");
    print_bytes((uint8_t*) trans_tx_enc_msg, trans_tx_enc_msg_len);
}

void test_standard_message(uint8_t msg_type, uint32_t arg1, uint32_t arg2) {
    add_standard_message(msg_type, arg1, arg2);
    print_decoded();
    encode_trans_tx_msg();
    print_encoded();
    send_trans_tx_enc_msg();
    print("\n");
    print_decoded();
    print_encoded();
    print("\n\n");
}

void test_string_message(char* string) {
    add_string_message(string);
    print_decoded();
    encode_trans_tx_msg();
    print_encoded();
    send_trans_tx_enc_msg();
    print("\n");
    print_decoded();
    print_encoded();
    print("\n\n");
}

int main(void) {
    init_uart();
    print("\n\n");
    init_uptime();

    print("Starting test\n\n");

    init_trans_uart();

    test_string_message("hello");
    test_string_message("world!");
    test_string_message("UTAT");

    test_standard_message(0, 0, 0);
    test_standard_message(12, UINT32_MAX, 5);
    test_standard_message(UINT8_MAX, 17, UINT32_MAX);

    print("Done test\n");
}
