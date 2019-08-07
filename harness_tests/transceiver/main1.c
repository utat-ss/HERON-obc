/*
 * Harness based test for the transceiver.
 * PS TRANSCEIVER NOT TRANSCIEVER.
 *
 */

#include <stdlib.h>
#include <test/test.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/transceiver.h"//also includes uptime.h by extension


// 1
void trans_uart_rx_cb_test(void){
    uint8_t len = 5;
    //uint8_t prev_buffer_len = trans_cmd_resp_len;
    //will compare update time before and after calling trans_uart_rx_cb
    uint32_t prev_uptime = uptime_s;
    trans_cmd_resp_avail = true;

    //buffer needs be remade...
    //note an issue: the globally defined buffer is used in the function.
    //--resolve later.

    //cast globally defined buffer trans_cmd_resp as buf for fcn to accept the input.
    uint8_t* buf = (uint8_t*)trans_cmd_resp;
    uint8_t test = trans_uart_rx_cb(buf, len);

    //return value should be same as input length (length processed)
    ASSERT_EQ(len,test);

    //new uptime must be greater than previous uptime .
    ASSERT_GREATER(uptime_s,prev_uptime);
}

// 2
void clear_trans_cmd_resp_test(void){
    trans_cmd_resp[0] = 'A';
    trans_cmd_resp[1] = 'B';
    trans_cmd_resp[2] = 'C';
    trans_cmd_resp[3] = 'D';
    trans_cmd_resp[4] = 'E';
    trans_cmd_resp[5] = '\r';
    trans_cmd_resp_len = 5;
    clear_trans_cmd_resp();
    //for (uint8_t i = 0; i < TRANS_CMD_RESP_MAX_SIZE; i++) {
    //    ASSERT_EQ(trans_cmd_resp[i],0);
    //}
    ASSERT_EQ(trans_cmd_resp_len,0);
    ASSERT_EQ(trans_cmd_resp_avail,false);
}

// 3
void wait_for_trans_cmd_resp_test(void){
    trans_cmd_resp_len = 5;
    trans_cmd_resp_avail = true;
    uint8_t expected_len = trans_cmd_resp_len;
    uint8_t test = wait_for_trans_cmd_resp(expected_len);
    ASSERT_EQ(test,1);
    //TODO- how to check the timeout?
}

// 4
// test scan_trans_cmd_resp_avail itself, and then through wait_for_trans_cmd_resp
void scan_trans_cmd_resp_avail_test(void){
    trans_cmd_resp[0] = 'O';
    trans_cmd_resp[1] = 'K';
    trans_cmd_resp[2] = 1;
    trans_cmd_resp[3] = '\r';
    trans_cmd_resp_len = 4; //this includes the '\r'
    uint8_t expected_len = trans_cmd_resp_len - 1;
    //make temp buf from casting trans_cmd_resp to make fcn happy with the input
    uint8_t* buf = (uint8_t*)trans_cmd_resp;
    scan_trans_cmd_resp(buf, trans_cmd_resp_len);
    //trans_cmd_resp_avail should be true
    ASSERT_EQ(trans_cmd_resp_avail,true);
    //check through the wait_for_trans_cmd_resp fcn as well
    uint8_t test = wait_for_trans_cmd_resp(expected_len);
    ASSERT_EQ(test,1);
}

// 5
void trans_scw_test(void){
    uint16_t scw = 0;
    //check set_trans_scw succeeded
    uint8_t test1 = set_trans_scw(scw);
    ASSERT_EQ(test1,1);
    //buffer should've been cleared too
    ASSERT_EQ(trans_cmd_resp_len,0);
    ASSERT_EQ(trans_cmd_resp_avail,false);
    ASSERT_EQ(trans_cmd_resp_avail,false);

    uint8_t rssi = 0;
    uint8_t reset_count = 0;
    scw = 0;
    uint8_t test2 = get_trans_scw(&rssi, &reset_count, &scw);
    //check get_trans_scw_attempt
    ASSERT_EQ(test2,1);
    //buffer cleared
    ASSERT_EQ(trans_cmd_resp_len,0);
    ASSERT_EQ(trans_cmd_resp_avail,false);
    ASSERT_EQ(trans_cmd_resp_avail,false);
    ASSERT_NEQ(reset_count,0); //this ok?

    //now i have a scw value i can use to test set_trans_scw_bit...
    printf("--scw:%x--",scw);
    uint8_t scw_initial = scw; //preserve old value for comparison
    uint8_t test3 = 0;
    uint8_t value = 0;
    for (uint8_t bit_index=0; bit_index<11 && bit_index!=5; bit_index++){
        value = 0;
        scw = scw_initial;
        test3 = set_trans_scw_bit(bit_index,value);
        ASSERT_EQ(test3,1);

        value = 1;
        scw = scw_initial;
        test3 = set_trans_scw_bit(bit_index,value);
        ASSERT_EQ(test3,1);
    }
}

// 6
void reset_test(void){
    uint8_t test = reset_trans();
    ASSERT_EQ(test,1);
}

// 7
void trans_freq_test(void){
    uint8_t test = set_trans_freq(TRANS_DEF_FREQ);
    ASSERT_EQ(test,1);
    uint8_t rssi = 0;
    uint32_t freq = 0;
    test = get_trans_freq(&rssi, &freq);
    ASSERT_EQ(test,1);
}

// 8
void trans_pipe_timeout_test(void){
    uint8_t timeout = 5;
    uint8_t test = set_trans_pipe_timeout(timeout);
    ASSERT_EQ(test,1);
    uint8_t rssi = 0;
    test = get_trans_pipe_timeout(&rssi, &timeout);
    ASSERT_EQ(test,1);
}

// 9
void trans_beacon_period_test(void){
    uint16_t period = 1;
    uint8_t test = turn_on_trans_beacon();
    ASSERT_EQ(test,1);
    test = set_trans_beacon_period(period);
    ASSERT_EQ(test,1);
    uint8_t rssi = 0;
    period = 0;
    test = get_trans_beacon_period(&rssi,&period);
    ASSERT_EQ(test,1);
    test = turn_off_trans_beacon();
    ASSERT_EQ(test,1);
}

// 10
void trans_call_sign_test(void){
    //gets
    char dest_call_sign[TRANS_CALL_SIGN_LEN + 1] = { 0 };
    _delay_ms(5000);
    uint8_t test = get_trans_dest_call_sign(dest_call_sign);
    ASSERT_EQ(test,1);

    char src_call_sign[TRANS_CALL_SIGN_LEN + 1] = { 0 };
    test = get_trans_src_call_sign(src_call_sign);
    ASSERT_EQ(test,1);

    //sets
    dest_call_sign[0] = 'A';
    dest_call_sign[1] = 'B';
    dest_call_sign[2] = 'C';
    dest_call_sign[3] = 'D';
    dest_call_sign[4] = 'E';
    dest_call_sign[5] = 'F';
    test = set_trans_dest_call_sign(dest_call_sign);
    ASSERT_EQ(test,1);
    src_call_sign[0] = 'V';
    src_call_sign[1] = 'A';
    src_call_sign[2] = '3';
    src_call_sign[3] = 'Z';
    src_call_sign[4] = 'B';
    src_call_sign[5] = 'R';
    test = set_trans_src_call_sign(src_call_sign);
    ASSERT_EQ(test,1);
}

// 11
void get_uptime_test(void){
    uint8_t rssi = 0;
    uptime_s = 0;
    volatile uint32_t* uptime_s_pointer = &uptime_s;
    uint8_t test = get_trans_uptime(&rssi, (uint32_t *) uptime_s_pointer);
    ASSERT_EQ(test,1);
    ASSERT_NEQ(uptime_s,0);
}

// 12
void get_trans_num_packets_test(void){
    uint8_t rssi = 0;
    uint32_t num_tx_packets = 0;
    uint8_t test = get_trans_num_tx_packets(&rssi, &num_tx_packets);
    ASSERT_EQ(test,1);
    rssi = 0;
    uint32_t num_rx_packets = 0;
    test = get_trans_num_rx_packets(&rssi, &num_rx_packets);
    ASSERT_EQ(test,1);
    rssi = 0;
    uint32_t num_rx_packets_crc = 0;
    test = get_trans_num_rx_packets_crc(&rssi, &num_rx_packets_crc);
    ASSERT_EQ(test,1);
}

// 13
void swc_bits_test(void){
    uint8_t mode = 3;// mode 3 default
    uint8_t test = set_trans_rf_mode(mode);
    ASSERT_EQ(test,1);
    test = turn_on_trans_echo();
    ASSERT_EQ(test,1);
    test = turn_off_trans_echo();
    ASSERT_EQ(test,1);
    test = turn_on_trans_beacon();
    ASSERT_EQ(test,1);
    test = turn_off_trans_beacon();
    ASSERT_EQ(test,1);
    test = turn_on_trans_pipe();
    ASSERT_EQ(test,1);
}

// 14
void decode_trans_rx_msg_test(void){
    // Encoded message: [0x02, 0x04, 0xC4, oxED, 0x97, 0x48, 0x74, 0x0F, 0x09, 0x00]
    // Decoded message: 


}

// 15
void encode_trans_tx_msg_test(void){
    // Decoded message:
    // Encoded message: 



}

test_t t1 = {.name = "trans_uart_rx_cb_test", .fn = trans_uart_rx_cb_test };
test_t t2 = {.name = "clear_trans_cmd_resp_test", .fn =  clear_trans_cmd_resp_test };
test_t t3 = {.name = "wait_for_trans_cmd_resp_test", .fn = wait_for_trans_cmd_resp_test };
test_t t4 = {.name = "scan_trans_cmd_resp_avail_test", .fn = scan_trans_cmd_resp_avail_test };
test_t t5 = {.name = "trans_scw_test", .fn = trans_scw_test };
test_t t6 = {.name = "reset_test", .fn = reset_test };
test_t t7 = {.name = "trans_freq_test", .fn = trans_freq_test };
test_t t8 = {.name = "trans_pipe_timeout_test", .fn = trans_pipe_timeout_test };
test_t t9 = {.name = "trans_beacon_period_test", .fn = trans_beacon_period_test };
test_t t10 = {.name = "trans_call_sign_test", .fn = trans_call_sign_test };
test_t t11 = {.name = "get_uptime_test", .fn = get_uptime_test };
test_t t12 = {.name = "get_trans_num_packets_test", .fn = get_trans_num_packets_test };
test_t t13 = {.name = "swc_bits_test", .fn = swc_bits_test };
test_t t14 = {.name = "decode_trans_rx_msg_test", .fn = decode_trans_rx_msg_test};
test_t t15 = {.name = "encode_trans_tx_msg_test", .fn = encode_trans_tx_msg_test};


test_t* suite[] = { &t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9, &t10, &t11, &t12, &t13, &t14, &t15 };

void run_test(test_t*);

//test harness needs be bypassed as the UART of the transceiver and OBC happen at
//the same time and thats not ok. ...
//test harness output needs be observed and evaluated manually as a result :D
int main(void) {
    init_uart();

    print("delay 5s\n");
    _delay_ms(5000);

    init_trans();
    for (int i=0;i<13;i++){
        run_test(suite[i]);
    }
    return 0;
}
