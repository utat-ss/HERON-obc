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
#include "../../src/transceiver.h"
#include "../../src/uptime.h"

void add_message(char* string) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Don't add the terminating '\0' character
        trans_decoded_tx_msg_len = strlen(string);
        for (uint8_t i = 0; i < trans_decoded_tx_msg_len; i++) {
            trans_decoded_tx_msg[i] = string[i];
        }
        trans_decoded_tx_msg_avail = true;
    }
}

int main(void) {
    init_uart();
    print("\n\n");

    rtc_date_t date;
    rtc_time_t time;
    init_uptime(date, time);

    print("Starting test\n");

    init_trans_uart();

    add_message("hello");
    encode_trans_tx_msg();
    send_trans_encoded_tx_msg();
    print("\n\n");

    add_message("world");
    encode_trans_tx_msg();
    send_trans_encoded_tx_msg();
    print("\n\n");

    add_message("UTAT");
    encode_trans_tx_msg();
    send_trans_encoded_tx_msg();
    print("\n\n");

    print("Done test\n");
}
