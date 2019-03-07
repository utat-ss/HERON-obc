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
#include "../../src/transceiver.h"
#include "../../src/uptime.h"

int main(void){
    init_uart();
    print("\n\n");

    rtc_date_t date;
    rtc_time_t time;
    init_uptime(date, time);

    print("Starting test\n");

    init_trans_uart();

    while (1) {
        if (trans_cmd_resp_avail) {
            print("Received trans cmd resp: %u chars: ", trans_cmd_resp_len);
            for (uint8_t i = 0; i < trans_cmd_resp_len; i++) {
                put_uart_char(trans_cmd_resp[i]);
            }
            print("\n");

            trans_cmd_resp_avail = false;
        }

        if (trans_encoded_rx_msg_avail) {
            print("Received trans encoded RX msg: %u bytes: ", trans_encoded_rx_msg_len);
            print_bytes((uint8_t*) trans_encoded_rx_msg, trans_encoded_rx_msg_len);

            trans_encoded_rx_msg_avail = false;
        }
    }
}
