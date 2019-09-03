/*
Separate this into a separate harness test from `transceiver` because the main
transceiver test cannot use the standard test harness (the output must be
manually observed). This test can be automated with the standard test harness
since the  transceiver itself is not used.
*/

#include <stdlib.h>
#include <test/test.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/transceiver.h"//also includes uptime.h by extension


// 1
void decode_trans_rx_msg_test(void){
    // Encoded message: [00:0c:02:0e:81:98:fd:dc:f8:8c:01:06:d3]
    uint8_t enc_msg_len = 13;
    uint8_t enc_msg[enc_msg_len] = { 0x00, 0x0c, 0x02, 0x0e, 0x81, 0x98, 0xfd, 0xdc, 0xf8, 0x8c, 0x01, 0x06, 0xd3 };
    // Decoded message: [ff:34:9e:1d:fe:38:6e:05:be]
    uint8_t dec_msg_len = 9;
    uint8_t dec_msg[dec_msg_len] = { 0xff, 0x34, 0x9e, 0x1d, 0xfe, 0x38, 0x6e, 0x05, 0xbe };

    // Set rx encode buffer
    trans_rx_enc_len = enc_msg_len;
    for (uint8_t i = 0; i < trans_rx_enc_len; i++) {
        trans_rx_enc_msg[i] = enc_msg[i];
    }
    trans_rx_enc_msg_avail = true;

    // decode the message
    decode_trans_rx_msg();

    // check length of message and contents 
    ASSERT_EQ(trans_tx_dec_msg_len, dec_msg_len);
    for (uint8_t i = 0; i < trans_rx_dec_msg_len; i++) {
        trans_rx_dec_msg[i] = dec_msg[i];
    }
    ASSERT_EQ(trans_rx_enc_msg_avail, false);
    ASSERT_EQ(trans_rx_dec_msg_avail, true);
    
    // set dec msg avail flag back to false
    trans_rx_dec_msg_avail = false;
}

// 2
void encode_trans_tx_msg_test(void){
    // Decoded message: [ff:34:9e:1d:fe:38:6e:05:be]
    uint8_t dec_msg_len = 9;
    uint8_t dec_msg[dec_msg_len] = { 0xff, 0x34, 0x9e, 0x1d, 0xfe, 0x38, 0x6e, 0x05, 0xbe };
    // Encoded message: [00:0c:02:0e:81:98:fd:dc:f8:8c:01:06:d3]
    uint8_t enc_msg_len = 13;
    uint8_t enc_msg[enc_msg_len] = { 0x00, 0x0c, 0x02, 0x0e, 0x81, 0x98, 0xfd, 0xdc, 0xf8, 0x8c, 0x01, 0x06, 0xd3 };

    // Set up tx decode message buffer
    trans_tx_dec_msg_len = dec_msg_len;
    for (uint8_t i = 0; i < trans_tx_dec_msg_len; i++) {
        trans_tx_dec_msg[i] = dec_msg[i];
    }
    trans_tx_dec_msg_avail = true;

    // encode message
    encode_trans_tx_msg();

    // check length of message as well as contents
    ASSERT_EQ(trans_tx_enc_len, enc_msg_len);
    for(uint8_t i=0; i<enc_msg_len; i++) {
        ASSERT_EQ(trans_tx_enc_msg[i], enc_msg[i]);
    }
    ASSERT_EQ(trans_tx_dec_msg_avail, false);
    ASSERT_EQ(trans_tx_enc_msg_avail, true);

    // set the enc msg avail back to false
    trans_tx_enc_msg_avail = false;
}

test_t t1 = {.name = "decode_trans_rx_msg_test", .fn = decode_trans_rx_msg_test};
test_t t2 = {.name = "encode_trans_tx_msg_test", .fn = encode_trans_tx_msg_test};

test_t* suite[] = { &t1, &t2 };

int main(void) {
    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
