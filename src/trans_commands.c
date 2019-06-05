/*
Code for processing received transceiver messages and generating commands.
Based on ground station to satellite protocol: https://utat-ss.readthedocs.io/en/master/our-protocols/ground-to-satellite.html
*/

#include "trans_commands.h"

/*
If there is a message in trans_rx_dec_msg, processes its components and enqueues the appropriate command and arguments.
*/
void handle_trans_rx_dec_msg(void) {
    // Need to put everything in an atomic block because the message is in a global array
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!trans_rx_dec_msg_avail) {
            return;
        }
        // Only accept 9 byte messages
        if (trans_rx_dec_msg_len != TRANS_RX_DEC_MSG_MAX_SIZE) {
            trans_rx_dec_msg_avail = false;
            return;
        }

        // Use shorter names for now
        uint8_t* msg = (uint8_t*) trans_rx_dec_msg;
        uint8_t msg_type = msg[0];
        uint32_t arg1 =
            (((uint32_t) msg[1]) << 24) |
            (((uint32_t) msg[2]) << 16) |
            (((uint32_t) msg[3]) << 8) |
            (((uint32_t) msg[4]));
        uint32_t arg2 =
            (((uint32_t) msg[5]) << 24) |
            (((uint32_t) msg[6]) << 16) |
            (((uint32_t) msg[7]) << 8) |
            (((uint32_t) msg[8]));

        trans_rx_dec_msg_avail = false;

        cmd_t* cmd = trans_msg_type_to_cmd(msg_type);
        if (cmd == NULL) {
            return;
        }
        enqueue_cmd(cmd, arg1, arg2);
    }
}

// NOTE: these three functions should be used within the same atomic block

void start_trans_tx_dec_msg(void) {
    trans_tx_dec_msg[0] = trans_cmd_to_msg_type((cmd_t*) current_cmd);
    trans_tx_dec_msg[1] = (current_cmd_arg1 >> 24) & 0xFF;
    trans_tx_dec_msg[2] = (current_cmd_arg1 >> 16) & 0xFF;
    trans_tx_dec_msg[3] = (current_cmd_arg1 >> 8) & 0xFF;
    trans_tx_dec_msg[4] = current_cmd_arg1 & 0xFF;
    trans_tx_dec_msg[5] = (current_cmd_arg2 >> 24) & 0xFF;
    trans_tx_dec_msg[6] = (current_cmd_arg2 >> 16) & 0xFF;
    trans_tx_dec_msg[7] = (current_cmd_arg2 >> 8) & 0xFF;
    trans_tx_dec_msg[8] = current_cmd_arg2 & 0xFF;

    trans_tx_dec_msg_len = 9;
}

void append_to_trans_tx_dec_msg(uint8_t byte) {
    if (trans_tx_dec_msg_len < TRANS_TX_DEC_MSG_MAX_SIZE) {
        trans_tx_dec_msg[trans_tx_dec_msg_len] = byte;
        trans_tx_dec_msg_len++;
    }
}

void finish_trans_tx_dec_msg(void) {
    trans_tx_dec_msg_avail = true;
}
