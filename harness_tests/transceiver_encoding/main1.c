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
    // Can't use the `enc_msg_len` variable as the array size, even if it is const

    // Encoded message
    // Checksum calculated from https://crccalc.com
    uint8_t enc_msg_len = 24;
    uint8_t enc_msg[24] = {
        0x55,
        0x0f,
        0x55,
        0x13, 0xa4, 0x12, 0xf3, 0xff, 0x34, 0x9e, 0x1e, 0x28, 0x9a, 0x6e, 0xa5,
        0x49, 0x7e, 0xf2,
        0x55, 
        0x16, 0xA4, 0xF6, 0x92,
        0x55
    };

    // Decoded message
    uint8_t dec_msg_len = 15;
    uint8_t dec_msg[15] = {
        0x13, 0xa4, 0x12, 0xf3, 0xff, 0x34, 0x9e, 0x1e, 0x28, 0x9a, 0x6e, 0xa5,
        0x49, 0x7e, 0xf2
    };

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
    // Decoded message
    uint8_t dec_msg_len = 15;
    uint8_t dec_msg[15] = {
        0x13, 0xa4, 0x12, 0xf3, 0xff, 0x34, 0x9e, 0x1e, 0x28, 0x9a, 0x6e, 0xa5,
        0x49, 0x7e, 0xf2
    };

    // Encoded message
    // Checksum calculated from https://crccalc.com
    uint8_t enc_msg_len = 24;
    uint8_t enc_msg[24] = {
        0x55,
        0x0f,
        0x55,
        0x13, 0xa4, 0x12, 0xf3, 0xff, 0x34, 0x9e, 0x1e, 0x28, 0x9a, 0x6e, 0xa5,
        0x49, 0x7e, 0xf2,
        0x55, 
        0x16, 0xA4, 0xF6, 0x92,
        0x55
    };

    // Set tx decode buffer
    trans_tx_dec_len = dec_msg_len;
    for (uint8_t i = 0; i < trans_tx_dec_len; i++) {
        trans_tx_dec_msg[i] = dec_msg[i];
    }
    trans_tx_dec_avail = true;

    // encode message
    encode_trans_tx_msg();

    // check length of message as well as contents
    ASSERT_EQ(trans_tx_enc_len, enc_msg_len);
    for (uint8_t i=0; i<enc_msg_len; i++) {
        ASSERT_EQ(trans_tx_enc_msg[i], enc_msg[i]);
    }

    ASSERT_EQ(trans_tx_dec_avail, false);
    ASSERT_EQ(trans_tx_enc_avail, true);

    trans_tx_enc_avail = false;
}

// Verifies that a randomly-generated decoded message is the same after an encode-decode sequence
void random_encode_decode_test(void){
    // Decoded -> encoded ------------------------------------------------------

    // Generate random message of arbitrary length (max length 15, must be at least 1)
    uint8_t dec_msg_len = (rand() % 15) + 1;
    uint8_t dec_msg[15] = {0};
    for (uint8_t i = 0; i < dec_msg_len; i++){
        dec_msg[i] = (uint8_t)(rand() % 256);
    }
    // Fill rest of buffer with 0s just in case
    for (uint8_t i = dec_msg_len; i < 15; i++) {
        dec_msg[i] = 0;
    }

    // Set up decoded message buffer and encode message
    trans_tx_dec_len = dec_msg_len;
    for (uint8_t i = 0; i < trans_tx_dec_len; i++) {
        trans_tx_dec_msg[i] = dec_msg[i];
    }
    trans_tx_dec_avail = true;
    encode_trans_tx_msg();

    // Check available flags
    ASSERT_FALSE(trans_tx_dec_avail);
    ASSERT_TRUE(trans_tx_enc_avail);

    // Check minimum length
    // Encoded length = decoded length + 9 always
    ASSERT_EQ(trans_tx_enc_len, dec_msg_len + 9);
    
    // Assert that encoded message is terminated correctly with 0x55 bytes in the
    // proper places
    ASSERT_EQ(trans_tx_enc_msg[0], 0x55);
    ASSERT_EQ(trans_tx_enc_msg[2], 0x55);
    ASSERT_EQ(trans_tx_enc_msg[trans_tx_enc_len-6], 0x55);
    ASSERT_EQ(trans_tx_enc_msg[trans_tx_enc_len-1], 0x55);

    // Check that the length byte is correct
    ASSERT_EQ(trans_tx_enc_msg[1], dec_msg_len);

    // Encoded -> decoded ------------------------------------------------------

    // Set up rx encoded message buffer and decode message
    trans_rx_enc_len = trans_tx_enc_len;
    for (uint8_t i = 0; i < trans_rx_enc_len; i++) {
        trans_rx_enc_msg[i] = trans_tx_enc_msg[i];
    }
    trans_rx_enc_avail = true;
    decode_trans_rx_msg();

    // Check available flags
    ASSERT_FALSE(trans_rx_enc_avail);
    ASSERT_TRUE(trans_rx_dec_avail);

    // Check length is same as length of originally generated message
    ASSERT_EQ(trans_rx_dec_len, dec_msg_len);

    // Assert that decoded values are same as initial values
    for (uint8_t i = 0; i < dec_msg_len; i++){
        ASSERT_EQ(trans_rx_dec_msg[i], dec_msg[i]);
    }
}

/* Ensure crc function generates correct checksum for messages
 CRC of bytes calculated using http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
 */
void checksum_test(void){
    uint8_t msg1_len = 9;
    uint8_t msg1[9] = { 0xf3, 0x00, 0x00, 0x12, 0x8d, 0x9a, 0x4f, 0x5e, 0xc5 };
    uint32_t msg1_check = crc32(msg1, msg1_len);
    ASSERT_EQ(msg1_check, 0x2817B19A);

    // For this case it unclear what the correct value is. The sunshine2k
    // website says 0x10ABCC35, while https://crccalc.com and
    // https://www.scadacore.com/tools/programming-calculators/online-checksum-calculator/
    // say 0xD843216C (matches what our code produces), so treat that as correct.
    uint8_t msg2_len = 23;
    uint8_t msg2[23] = {
        0xeb, 0x4d, 0xa0, 0x99, 0x4f, 0xfd, 0x69, 0x20, 0x25, 0x90, 0x2b, 0x59,
        0xb7, 0x9b, 0xf7,0x13, 0xe2, 0x57, 0x5e, 0x1c, 0x35, 0x49, 0x78 };
    uint32_t msg2_check = crc32(msg2, msg2_len);
    ASSERT_EQ(msg2_check, 0xD843216C);

    uint8_t msg3_len = 1;
    uint8_t msg3[1] = { 0x00 };
    uint32_t msg3_check = crc32(msg3, msg3_len);
    ASSERT_EQ(msg3_check, 0xD202EF8D);
}


test_t t1 = {.name = "decode_trans_rx_msg_test", .fn = decode_trans_rx_msg_test};
test_t t2 = {.name = "encode_trans_tx_msg_test", .fn = encode_trans_tx_msg_test};
test_t t3 = {.name = "random_encode_decode_test", .fn = random_encode_decode_test};
test_t t4 = {.name = "checksum_test", .fn = checksum_test};


test_t* suite[] = { &t1, &t2, &t3, &t4};

int main(void) {
    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
