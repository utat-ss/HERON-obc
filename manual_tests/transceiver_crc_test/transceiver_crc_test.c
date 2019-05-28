/*
Program to test the CRC calculation algorithm.
*/

#include <uart/uart.h>
#include <utilities/utilities.h>
#include <uptime/uptime.h>

#include "../../src/rtc.h"
#include "../../src/transceiver.h"

void add_message(uint8_t msg_type, uint32_t arg1, uint32_t arg2) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        trans_tx_dec_msg[0] = msg_type;
        trans_tx_dec_msg[1] = (arg1 >> 24) & 0xFF;
        trans_tx_dec_msg[2] = (arg1 >> 16) & 0xFF;
        trans_tx_dec_msg[3] = (arg1 >> 8) & 0xFF;
        trans_tx_dec_msg[4] = arg1 & 0xFF;
        trans_tx_dec_msg[5] = (arg2 >> 24) & 0xFF;
        trans_tx_dec_msg[6] = (arg2 >> 16) & 0xFF;
        trans_tx_dec_msg[7] = (arg2 >> 8) & 0xFF;
        trans_tx_dec_msg[8] = arg2 & 0xFF;

        trans_tx_dec_msg_len = 9;
        trans_tx_dec_msg_avail = true;
    }
}

void print_decoded(void) {
    if (!trans_tx_dec_msg_avail) {
        print("No decoded message available\n");
        return;
    }
    print("Decoded: ");
    print_bytes((uint8_t*) trans_tx_dec_msg, trans_tx_dec_msg_len);
}

void print_encoded(void) {
    if (!trans_tx_enc_msg_avail) {
        print("No encoded message available\n");
        return;
    }
    print("Encoded: ");
    print_bytes((uint8_t*) trans_tx_enc_msg, trans_tx_enc_msg_len);
}

void test_message(uint8_t msg_type, uint32_t arg1, uint32_t arg2) {
    add_message(msg_type, arg1, arg2);
    print_decoded();
    encode_trans_tx_msg();
    print_encoded();
    uint16_t crc = calc_trans_crc();
    print("crc = %u = 0x%.4x\n", crc, crc);
    send_trans_tx_enc_msg();
    print("\n");
    print_decoded();
    print_encoded();
    print("\n\n");
}


int main(void) {
    init_uart();
    print("\n\n");
    init_uptime();

    print("Starting transceiver CRC test\n\n");

    init_trans_uart();

    test_message(0, 0, 0);
    test_message(12, UINT32_MAX, 5);
    test_message(UINT8_MAX, 17, UINT32_MAX);

    print("Done test\n");
}
