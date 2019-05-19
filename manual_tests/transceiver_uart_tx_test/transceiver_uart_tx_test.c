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

void add_message(char* string) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Don't add the terminating '\0' character
        trans_tx_dec_msg_len = strlen(string);
        for (uint8_t i = 0; i < trans_tx_dec_msg_len; i++) {
            trans_tx_dec_msg[i] = string[i];
        }
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

void test_message(char* string) {
    add_message(string);
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

    test_message("hello");
    test_message("world!");
    test_message("UTAT");

    print("Done test\n");
}
