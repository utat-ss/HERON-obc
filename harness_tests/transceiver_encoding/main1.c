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

// Example messages from https://utat-ss.readthedocs.io/en/master/our-protocols/ground-to-satellite.html

// 1
void decode_trans_rx_msg_test(void){
    // Encoded message: [00:1b:00:02:02:c1:6c:bf:c9:0f:01:01:71:97:00]
    uint8_t enc_msg_len = 15;
    // Can't use the `enc_msg_len` variable as the array size, even if it is const
    uint8_t enc_msg[15] = { 0x00, 0x1b, 0x00, 0x02, 0x02, 0xc1, 0x6c, 0xbf, 0xc9, 0x0f, 0x01, 0x01, 0x71, 0x97, 0x00 };

    // Decoded message: [f3:ff:34:9e:1e:28:9a:6e:b7]
    uint8_t dec_msg_len = 9;
    uint8_t dec_msg[9] = { 0xf3, 0xff, 0x34, 0x9e, 0x1e, 0x28, 0x9a, 0x6e, 0xb7 };

    // Set rx encode buffer
    trans_rx_enc_len = enc_msg_len;
    for (uint8_t i = 0; i < trans_rx_enc_len; i++) {
        trans_rx_enc_msg[i] = enc_msg[i];
    }
    trans_rx_enc_avail = true;

    // decode the message
    decode_trans_rx_msg();

    // check length of message and contents 
    ASSERT_EQ(trans_rx_dec_len, dec_msg_len);
    for (uint8_t i = 0; i < trans_rx_dec_len; i++) {
        ASSERT_EQ(trans_rx_dec_msg[i], dec_msg[i]);
    }
    ASSERT_EQ(trans_rx_enc_avail, false);
    ASSERT_EQ(trans_rx_dec_avail, true);
    
    // set dec msg avail flag back to false
    trans_rx_dec_avail = false;
}

// 2
void encode_trans_tx_msg_test(void){
    // Decoded message: [f3:ff:34:9e:1e:28:9a:6e:b7]
    uint8_t dec_msg_len = 9;
    uint8_t dec_msg[9] = { 0xf3, 0xff, 0x34, 0x9e, 0x1e, 0x28, 0x9a, 0x6e, 0xb7 };

    // Encoded message: [00:1b:00:02:02:c1:6c:bf:c9:0f:01:01:71:97:00]
    uint8_t enc_msg_len = 15;
    // Can't use the `enc_msg_len` variable as the array size, even if it is const
    uint8_t enc_msg[15] = { 0x00, 0x1b, 0x00, 0x02, 0x02, 0xc1, 0x6c, 0xbf, 0xc9, 0x0f, 0x01, 0x01, 0x71, 0x97, 0x00 };

    // Set up tx decode message buffer
    trans_tx_dec_len = dec_msg_len;
    for (uint8_t i = 0; i < trans_tx_dec_len; i++) {
        trans_tx_dec_msg[i] = dec_msg[i];
    }
    trans_tx_dec_avail = true;

    // encode message
    encode_trans_tx_msg();

    // check length of message as well as contents
    ASSERT_EQ(trans_tx_enc_len, enc_msg_len);
    for(uint8_t i=0; i<enc_msg_len; i++) {
        ASSERT_EQ(trans_tx_enc_msg[i], enc_msg[i]);
    }
    ASSERT_EQ(trans_tx_dec_avail, false);
    ASSERT_EQ(trans_tx_enc_avail, true);

    // set the enc msg avail back to false
    trans_tx_enc_avail = false;
}

test_t t1 = {.name = "decode_trans_rx_msg_test", .fn = decode_trans_rx_msg_test};
test_t t2 = {.name = "encode_trans_tx_msg_test", .fn = encode_trans_tx_msg_test};

test_t* suite[] = { &t1, &t2 };

int main(void) {
    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
