/*
Program to test the transceiver library for processing and distinguishing received UART.

NOTE: This does not use the actual transceiver, just a laptop with CoolTerm
sending UART to OBC.

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

int main(void){
    init_uart();
    print("\n\n");

    init_uptime();

    print("Starting test\n");

    // For simulated transceiver
    // init_trans_uart();

    // For actual transceiver - enable pipe mode from OBC
    init_trans();
    turn_on_trans_pipe();
    print("Pipe mode on\n");

    while (1) {
        if (trans_cmd_resp_avail) {
            // Wait a bit or else the first couple of characaters are dropped
            _delay_ms(10);
            print("\nReceived trans cmd resp: %u chars: ", trans_cmd_resp_len);
            for (uint8_t i = 0; i < trans_cmd_resp_len; i++) {
                put_uart_char(trans_cmd_resp[i]);
            }
            print("\n");

            trans_cmd_resp_avail = false;
        }

        // Make sure we detect the encoded message before it gets decoded
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            if (trans_rx_enc_msg_avail) {
                _delay_ms(10);
                print("\nReceived trans encoded RX msg: %u bytes: ", trans_rx_enc_msg_len);
                print_bytes((uint8_t*) trans_rx_enc_msg, trans_rx_enc_msg_len);
                // Don't clear the encoded message (the decode function should do it)
            }

            decode_trans_rx_msg();

            if (trans_rx_dec_msg_avail) {
                _delay_ms(10);
                print("\nReceived trans decoded RX msg: %u bytes: ", trans_rx_dec_msg_len);
                print_bytes((uint8_t*) trans_rx_dec_msg, trans_rx_dec_msg_len);
                trans_rx_dec_msg_avail = false;
            }
        }
    }
}
