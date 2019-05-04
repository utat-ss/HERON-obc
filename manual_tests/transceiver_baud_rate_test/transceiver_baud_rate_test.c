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
void test_transceiver_baud_rate_1200(uint16_t* scw, uart_baud_rate_t* current_rate);
void test_transceiver_baud_rate_9600(uint16_t* scw, uart_baud_rate_t* current_rate);
void test_transceiver_baud_rate_19200(uint16_t* scw, uart_baud_rate_t* current_rate);
void test_transceiver_baud_rate_115200(uint16_t* scw, uart_baud_rate_t* current_rate);


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
    set_uart_baud_rate(UART_BAUD_9600);
    _delay_ms(100);

    ret = get_trans_scw(&rssi, &reset_count, &scw);
    print("get_trans_scw: ret = %u, scw = %04X\n", ret, scw);

    uart_baud_rate_t baud_rate = 0;
    uint8_t corrected;

    // Try changing transceiver to different baud rates and then fixing it using the function
    test_transceiver_baud_rate_1200(&scw, &baud_rate);
    test_transceiver_baud_rate_19200(&scw, &baud_rate);
    test_transceiver_baud_rate_115200(&scw, &baud_rate);

    // Confirm 9600
    set_uart_baud_rate(UART_BAUD_9600);
    _delay_ms(100);

    ret = get_trans_scw(&rssi, &reset_count, &scw);
    print("Final check for 9600 \n");
    print("get_trans_scw: retrieved = %u, baudrate = 9600, previous = %u, scw = %04X\n", ret, baud_rate, scw);
}

// Simple baud rate test fix for 1200
void test_transceiver_baud_rate_1200(uint16_t* scw, uart_baud_rate_t* current_rate) {
    uint8_t ret = 0;
    uint8_t rssi = 0;
    uint8_t reset_count = 0;

    // Set the tranceiver's baud rate to 1200, 01
    *scw = (*scw & ~_BV(13)) | _BV(12);
    set_trans_scw(*scw);
    print("Set transceiver's baudrate to 1200");
    uint8_t corrected = correct_transceiver_baud_rate(current_rate);
    ret = get_trans_scw(&rssi, &reset_count, scw);
    print("get_trans_scw: ret = %u, corrected = %u, baudrate = %u, scw = %04X\n", ret, corrected, *current_rate, scw);
}

// Simple test for baud rate of 9600
void test_transceiver_baud_rate_9600(uint16_t* scw, uart_baud_rate_t* current_rate) {
    correct_transceiver_baud_rate(current_rate);
}

// Simple test for baud rate at 19200
void test_transceiver_baud_rate_19200(uint16_t* scw, uart_baud_rate_t* current_rate) {
    uint8_t ret = 0;
    uint8_t rssi = 0;
    uint8_t reset_count = 0;

    // Set the tranceiver's baudrate to 19200, 10
    *scw = (*scw | _BV(13)) & ~_BV(12);
    set_trans_scw(*scw);
    print("Set transceiver's baudrate to 19200");
    uint8_t corrected = correct_transceiver_baud_rate(current_rate);
    ret = get_trans_scw(&rssi, &reset_count, scw);
    print("get_trans_scw: ret = %u, corrected = %u, baudrate = %u, scw = %04X\n", ret, corrected, *current_rate, scw);
}

// Simple test for baud rate at 115200
void test_transceiver_baud_rate_115200(uint16_t* scw, uart_baud_rate_t* current_rate) {
    uint8_t ret = 0;
    uint8_t rssi = 0;
    uint8_t reset_count = 0;

    *scw = (*scw | _BV(13)) | _BV(12);
    set_trans_scw(*scw);
    print("Set transceiver's baudrate to 115200");
    uint8_t corrected = correct_transceiver_baud_rate(current_rate);
    ret = get_trans_scw(&rssi, &reset_count, scw);
    print("get_trans_scw: ret = %u, corrected = %u, baudrate = %u, scw = %04X\n", ret, corrected, *current_rate, scw);
}
