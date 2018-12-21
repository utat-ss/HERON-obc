/*
Program to test the functionality of the transceiver functions. Expected results
are written as comments.

IMPORTANT NOTES:
- See the transceiver handling guide before touching the transceiver: https://utat-ss.readthedocs.io/en/master/our-protocols/transceiver.html
- MAKE SURE an antenna is attached to the coax cable before powering the transceiver - it must have an antenna when transmitting or else this will cause damage
*/

#include <uart/uart.h>
#include <utilities/utilities.h>
#include "../../src/transceiver.h"


void test_all_gets(void);
void test_sets(void);
void test_reset(void);


int main(void){
    init_uart();
    print("\n\n"); //for avoiding programming gibberish

    print("Waiting 5 seconds...\n");
    _delay_ms(5000);

    init_trans();

    get_trans_scw(NULL, NULL);

    test_all_gets();
    // test_sets();
    // test_all_gets();
    // test_reset();
    // test_all_gets();


/*
    // // Turn off beacon mode
    // scw &= ~_BV(TRANS_BCN);
    // ret = set_trans_scw(scw);
    // print("set SCW: %u\n", ret);
    // ret = get_trans_scw(&rssi, &scw);
    // print("scw = %.4X\n", scw);

    // uint32_t freq;
    // get_trans_freq(&freq);
    // uint32_t dest_call_sign;
    // get_trans_dest_call_sign(&dest_call_sign);
    // uint32_t src_call_sign;
    // get_trans_src_call_sign(&src_call_sign);
    // uint32_t uptime;
    // get_trans_uptime(&uptime);

    //Go through functions
    set_trans_freq();

    uint8_t pipeline_timeout = 15; //in seconds
    set_trans_pipe_timeout(pipeline_timeout);

    char callsign[6] = "VA3ZZZ"; //Random callsign - get a legit one later
    set_trans_dest_call_sign(callsign);


    char source_callsign[6] = "VA3ZBR"; //Brytni's callsign
    set_trans_src_call_sign(source_callsign);


    //set_trans_pipeline();
    //delay(15500); //wait for timeout
    //Should return the same value as what was set - 15
    //get_trans_scw()

    //Turn on Beacon
    //turn_on_trans_beacon();
    //Turn off Beacon
    //turn_off_trans_beacon();
    */
}


// Calls all get functions
void test_all_gets(void) {
    uint8_t ret = 0;
    uint8_t rssi = 0;

    uint16_t scw = 0;
    ret = get_trans_scw(&rssi, &scw);
    print("get_trans_scw: ret = %u, rssi = %02X, scw = %04X\n", ret, rssi, scw);

    uint32_t freq = 0;
    ret = get_trans_freq(&rssi, &freq);
    print("get_trans_freq: ret = %u, rssi = %02X, freq = %08lX\n", ret, rssi, freq);

    uint8_t pipe_timeout = 0;
    ret = get_trans_pipe_timeout(&rssi, &pipe_timeout);
    print("get_trans_pipe_timeout: ret = %u, rssi = %02X, pipe_timeout = %02X\n", ret, rssi, pipe_timeout);

    uint16_t beacon_period = 0;
    ret = get_trans_beacon_period(&rssi, &beacon_period);
    print("get_trans_beacon_period: ret = %u, rssi = %02X, beacon_period = %04X\n", ret, rssi, beacon_period);

    char dest_call_sign[TRANS_CALL_SIGN_LEN] = { 0 };
    ret = get_trans_dest_call_sign(dest_call_sign);
    print("get_trans_dest_call_sign: ret = %u, dest_call_sign = ", ret);
    for (uint8_t i = 0; i < 6; i++) {
        print("%c", dest_call_sign[i]);
    }
    print("\n");

    char src_call_sign[TRANS_CALL_SIGN_LEN] = { 0 };
    ret = get_trans_src_call_sign(src_call_sign);
    print("get_trans_src_call_sign: ret = %u, src_call_sign = ", ret);
    for (uint8_t i = 0; i < 6; i++) {
        print("%c", src_call_sign[i]);
    }
    print("\n");

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


void test_sets(void) {
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

    ret = set_trans_freq(TRANS_DEF_FREQ);
    print("set_trans_freq: ret = %u\n", ret);

    ret = set_trans_pipe_timeout(5);
    print("set_trans_pipe_timeout: ret = %u\n", ret);

    ret = set_trans_beacon_period(2);
    print("set_trans_beacon_period: ret = %u\n", ret);

    char dest_call_sign[TRANS_CALL_SIGN_LEN] =
        {'A', 'B', 'C', 'D', 'E', 'F'};
    ret = set_trans_dest_call_sign(dest_call_sign);
    print("set_trans_dest_call_sign: ret = %u\n", ret);

    char src_call_sign[TRANS_CALL_SIGN_LEN] =
        {'F', 'E', 'D', 'C', 'B', 'A'};
    ret = set_trans_src_call_sign(src_call_sign);
    print("set_trans_src_call_sign: ret = %u\n", ret);
}


void test_reset(void) {
    print("Resetting transceiver...\n");
    reset_trans();
    print("Reset transceiver\n");
}
