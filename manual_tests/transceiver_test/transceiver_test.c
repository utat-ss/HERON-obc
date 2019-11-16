/*
Program to test the functionality of the transceiver functions. Expected results
are written as comments.

IMPORTANT NOTES:
- See the transceiver handling guide before touching the transceiver: https://utat-ss.readthedocs.io/en/master/our-protocols/transceiver.html
- MAKE SURE an antenna is attached to the coax cable before powering the transceiver - it must have an antenna when transmitting or else this will cause damage

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
- Need to unplug SCK/RX from the PCB when uploading a new program, then plug it
    back in when the program is running
- Might need/want to press the reset button on the PCB to restart the program
*/

#include <uart/uart.h>
#include <utilities/utilities.h>
#include "../../src/transceiver.h"

void test_any(void);
void test_single_get(void);
void test_all_gets(void);
void test_all_sets(void);
void test_scw_bits(void);
void test_pipe(void);
void test_beacon(void);
void test_reset(void);


int main(void){
    init_uart();
    init_uptime();

    print("\n\n");

    // Time to change the switch from PGM to RUN after programming
    print("Waiting 5 seconds...\n");
    _delay_ms(5000);

    init_trans();

    // Test all functionality
    test_any();
    test_single_get();
    test_all_gets();
    test_all_sets();
    test_scw_bits();
    test_pipe();
    test_reset();
    test_beacon();

    // Get values again
    test_all_gets();

    print("\nDone transceiver test\n\n");
}

void test_any(void) {
    // Any command test
    get_trans_scw(NULL, NULL, NULL);
}

// Single command test
void test_single_get(void){
    uint8_t ret = 0;
    uint8_t rssi = 0;

    uint8_t reset_count = 0;
    uint16_t scw = 0;
    ret = get_trans_scw(&rssi, &reset_count, &scw);
    print("get_trans_scw: ret = %u, rssi = %02X, reset_count = %02X, scw = %04X\n", ret, rssi, reset_count, scw);
}

// Calls all get functions
void test_all_gets(void) {
    uint8_t ret = 0;
    uint8_t rssi = 0;

    uint8_t reset_count = 0;
    uint16_t scw = 0;
    ret = get_trans_scw(&rssi, &reset_count, &scw);
    print("get_trans_scw: ret = %u, rssi = %02X, reset_count = %02X, scw = %04X\n", ret, rssi, reset_count, scw);

    uint32_t freq = 0;
    ret = get_trans_freq(&rssi, &freq);
    print("get_trans_freq: ret = %u, rssi = %02X, freq = %08lX\n", ret, rssi, freq);

    uint8_t pipe_timeout = 0;
    ret = get_trans_pipe_timeout(&rssi, &pipe_timeout);
    print("get_trans_pipe_timeout: ret = %u, rssi = %02X, pipe_timeout = %02X\n", ret, rssi, pipe_timeout);

    uint16_t beacon_period = 0;
    ret = get_trans_beacon_period(&rssi, &beacon_period);
    print("get_trans_beacon_period: ret = %u, rssi = %02X, beacon_period = %04X\n", ret, rssi, beacon_period);

    char dest_call_sign[TRANS_CALL_SIGN_LEN + 1] = { 0 };
    ret = get_trans_dest_call_sign(dest_call_sign);
    print("get_trans_dest_call_sign: ret = %u, dest_call_sign = %s\n", ret, dest_call_sign);

    char src_call_sign[TRANS_CALL_SIGN_LEN + 1] = { 0 };
    ret = get_trans_src_call_sign(src_call_sign);
    print("get_trans_src_call_sign: ret = %u, src_call_sign = %s\n", ret, src_call_sign);

    uint32_t uptime = 0;
    ret = get_trans_uptime(&rssi, &uptime);
    print("get_trans_uptime: ret = %u, rssi = %02X, uptime = %08lX\n", ret, rssi, uptime);

    uint32_t num_tx_packets = 0;
    ret = get_trans_num_tx_packets(&rssi, &num_tx_packets);
    print("get_trans_num_tx_packets: ret = %u, rssi = %02X, num_tx_packets = %08lX\n", ret, rssi, num_tx_packets);

    uint32_t num_rx_packets = 0;
    ret = get_trans_num_rx_packets(&rssi, &num_rx_packets);
    print("get_trans_num_rx_packets: ret = %u, rssi = %02X, num_rx_packets = %08lX\n", ret, rssi, num_rx_packets);

    uint32_t num_rx_packets_crc = 0;
    ret = get_trans_num_rx_packets_crc(&rssi, &num_rx_packets_crc);
    print("get_trans_num_rx_packets_crc: ret = %u, rssi = %02X, num_rx_packets_crc = %08lX\n", ret, rssi, num_rx_packets_crc);
}


void test_all_sets(void) {
    uint8_t ret = 0;

    ret = set_trans_scw(0x0303);
    print("set_trans_scw: ret = %u\n", ret);

    ret = set_trans_freq(TRANS_DEF_FREQ);
    print("set_trans_freq: ret = %u\n", ret);

    ret = set_trans_pipe_timeout(5);
    print("set_trans_pipe_timeout: ret = %u\n", ret);

    ret = set_trans_beacon_period(2);
    print("set_trans_beacon_period: ret = %u\n", ret);

    char beacon_content[] = "Test beacon!";
    ret = set_trans_beacon_content(beacon_content);
    print("set_trans_beacon_content: ret = %u\n", ret);

    // Random call sign
    char dest_call_sign[TRANS_CALL_SIGN_LEN + 1] = "ABCDEF";
    ret = set_trans_dest_call_sign(dest_call_sign);
    print("set_trans_dest_call_sign: ret = %u\n", ret);

    // Brytni's call sign
    char src_call_sign[TRANS_CALL_SIGN_LEN + 1] = "VA3ZBR";
    ret = set_trans_src_call_sign(src_call_sign);
    print("set_trans_src_call_sign: ret = %u\n", ret);
}


void test_scw_bits(void) {
    uint8_t ret = 0;

    ret = set_trans_rf_mode(3);
    print("set_trans_rf_mode: ret = %u\n", ret);

    ret = turn_on_trans_echo();
    print("turn_on_trans_echo: ret = %u\n", ret);

    ret = turn_off_trans_echo();
    print("turn_off_trans_echo: ret = %u\n", ret);

    ret = turn_on_trans_beacon();
    print("turn_on_trans_beacon: ret = %u\n", ret);

    ret = turn_off_trans_beacon();
    print("turn_off_trans_beacon: ret = %u\n", ret);
}


void test_pipe(void) {
    uint8_t ret = 0;

    ret = set_trans_pipe_timeout(5);
    print("set_trans_pipe_timeout: ret = %u\n", ret);

    ret = turn_on_trans_pipe();
    print("turn_on_trans_pipe: ret = %u\n", ret);

    // This should send data (TX packet)
    // It should stay in pipe mode because we refresh the 5s timeout every time
    // we send data
    // TODO - is it sending 5 separate packets or 1 single packet?
    print("\n");
    for (uint8_t i = 0; i < 10; i++) {
        print("Sending data in pipe mode\n");
        _delay_ms(1000);
    }

    print("Stopped sending data in pipe mode\n");
    print("Waiting 10 seconds...\n");
    _delay_ms(10000);
    print("Pipe mode should be timed out\n");
    print("\n");
}


void test_beacon(void) {
    uint8_t ret = 0;

    ret = set_trans_beacon_content(TRANS_BEACON_DEF_MSG);
    print("set_trans_beacon_content: ret = %u\n", ret);

    // Get trans beacon content
    // This function is not implemented in the library due to variable content size
    // Can just view the response on the transceiver's TxD UART for testing
    clear_trans_cmd_resp();
    print("\rES+R%02XFB\r", TRANS_ADDR);
    clear_trans_cmd_resp();

    ret = turn_on_trans_beacon();
    print("turn_on_trans_beacon: ret = %u\n", ret);

    ret = turn_off_trans_beacon();
    print("turn_off_trans_beacon: ret = %u\n", ret);
}


void test_reset(void) {
    print("Resetting transceiver...\n");
    reset_trans();
    print("Done resetting transceiver\n");
}
