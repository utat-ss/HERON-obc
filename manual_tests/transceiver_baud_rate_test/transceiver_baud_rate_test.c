/*
Program to test correction of the transceiver's baud rate function

IMPORTANT NOTES:
- See the transceiver handling guide before touching the transceiver: https://utat-ss.readthedocs.io/en/master/our-protocols/transceiver.html

Hardware Setup:
    - Need equipment:
        - Transceiver
        - Transceiver evaluation protoboard
        - Transceiver antenna
        - 2 programmers
        - one of our PCBs with UART (should be any of them)
    - Transceiver connected to evaluation protoboard with PC104 header
    - Antenna connected to transceiver!! (VERY IMPORTANT - SEE ABOVE)
    - Transceiver 5V -> power supply 5V
    - Transceiver GND -> power supply GND
    - PCB MOSI -> Transceiver RxD
    - PCB SCK/RX -> Transceiver TxD
    - Programmer 1 6-pin programming header -> PCB programming header
    - Programmer 1 RX -> transceiver RxD
    - Programmer 1 GND -> transceiver GND
    - Programmer 2 RX -> transceiver TxD
    - Programmer 2 GND -> transceiver GND or power supply GND

Other Notes:
- Need to unplug SCK/RX from the PCB when uploading a new program, then plug it back in when the program is running
- Might need/want to press the reset button on the PCB to restart the program
*/

#include <uart/uart.h>
#include <utilities/utilities.h>
#include "../../src/transceiver.h"

void test_tranceiver_baud_rate(void);

int main(void){
    init_uart();
    print("\n\n");
    print(" 1200: %u, 9600: %u, 19200: %u, 115200: %u\n", UART_BAUD_1200, UART_BAUD_9600, UART_BAUD_19200, UART_BAUD_115200);

    // Time to connect the SCK/RX pin after programming
    print("Waiting 5 seconds...\n");
    _delay_ms(5000);

    init_trans();

    test_tranceiver_baud_rate();

    print("\nDone testing transceiver baud rate correction test\n\n");
}

void test_tranceiver_baud_rate(void) {
    uint8_t ret = 0;
    uint8_t rssi = 0;

    uint8_t reset_count = 0;
    uint16_t scw = 0;
    ret = get_trans_scw(&rssi, &reset_count, &scw);
    print("get_trans_scw: ret = %u, scw = %04X\n", ret, scw);

    uart_baud_rate_t baud_rate = 0;
    uint8_t corrected = correct_transceiver_baud_rate(&baud_rate);

    // Set the tranceiver's baud rate to 1200, 01
    scw = (scw & ~_BV(13)) | _BV(12);
    set_trans_scw(scw);
    print("Set transceiver's baudrate to 1200");
    corrected = correct_transceiver_baud_rate(&baud_rate);
    ret = get_trans_scw(&rssi, &reset_count, &scw);
    print("get_trans_scw: ret = %u, corrected = %u, baudrate = %u, scw = %04X\n", ret, corrected, baud_rate, scw);

    // Set the tranceiver's baudrate to 19200, 10
    scw = (scw | _BV(13)) & ~_BV(12);
    set_trans_scw(scw);
    print("Set transceiver's baudrate to 19200");
    corrected = correct_transceiver_baud_rate(&baud_rate);
    ret = get_trans_scw(&rssi, &reset_count, &scw);
    print("get_trans_scw: ret = %u, corrected = %u, baudrate = %u, scw = %04X\n", ret, corrected, baud_rate, scw);

    // Set the transceiver's baudrate to 115200
    scw = (scw | _BV(13)) | _BV(12);
    set_trans_scw(scw);
    print("Set transceiver's baudrate to 115200");
    corrected = correct_transceiver_baud_rate(&baud_rate);
    ret = get_trans_scw(&rssi, &reset_count, &scw);
    print("get_trans_scw: ret = %u, corrected = %u, baudrate = %u, scw = %04X\n", ret, corrected, baud_rate, scw);

    // Confirm 9600
    set_uart_baud_rate(UART_BAUD_9600);
    ret = get_trans_scw(&rssi, &reset_count, &scw);
    print("get_trans_scw: ret = %u, corrected = %u, baudrate = 0, scw = %04X\n", ret, corrected, scw);

}
