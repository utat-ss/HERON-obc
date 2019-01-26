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

void rx_msg_cb(const uint8_t* data, uint8_t len) {
    print("Received RX msg: ");
    for (uint8_t i = 0; i < len; i++) {
        print("%c");
    }
    print("\n");
}

int main(void){
    init_uart();
    print("\n\n");

    print("Starting test\n");

    init_trans();
    set_trans_rx_msg_cb(rx_msg_cb);
    start_uptime_timer();

    while (1) {}
}
