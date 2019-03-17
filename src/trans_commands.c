/*
Code for processing received transceiver messages and generating commands.
Based on ground station to satellite protocol: https://utat-ss.readthedocs.io/en/master/our-protocols/ground-to-satellite.html
*/

#include "trans_commands.h"

/*
If there is a message in trans_decoded_rx_msg, processes its components and enqueues the appropriate command and arguments.
*/
void handle_trans_decoded_rx_msg(void) {
    // Need to put everything in an atomic block because the message is in a global array
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!trans_decoded_rx_msg_avail) {
            return;
        }
        // Only accept 9 byte messages
        // TODO
        if (trans_decoded_rx_msg_len != 9) {
            trans_decoded_rx_msg_avail = false;
            return;
        }

        // Use shorter names for now
        uint8_t* msg = (uint8_t*) trans_decoded_rx_msg;

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

        cmd_t* cmd = NULL;

        switch (msg_type) {
            case TRANS_CMD_PING:
                cmd = &ping_cmd;
                break;
            case TRANS_CMD_GET_RESTART_UPTIME:
                cmd = &get_restart_uptime_cmd;
                break;
            case TRANS_CMD_GET_RTC:
                cmd = &get_rtc_cmd;
                break;
            case TRANS_CMD_SET_RTC:
                cmd = &set_rtc_cmd;
                break;
            case TRANS_CMD_READ_MEM:
                cmd = &read_mem_cmd;
                break;
            case TRANS_CMD_ERASE_MEM:
                cmd = &erase_mem_cmd;
                break;
            case TRANS_CMD_COL_BLOCK:
                cmd = &collect_block_cmd;
                break;
            case TRANS_CMD_READ_LOC_BLOCK:
                cmd = &read_local_block_cmd;
                break;
            case TRANS_CMD_READ_MEM_BLOCK:
                cmd = &read_mem_block_cmd;
                break;
            case TRANS_CMD_AUTO_DATA_COL_ENABLE:
                cmd = &set_auto_data_col_enable_cmd;
                break;
            case TRANS_CMD_AUTO_DATA_COL_PERIOD:
                cmd = &set_auto_data_col_period_cmd;
                break;
            case TRANS_CMD_AUTO_DATA_COL_RESYNC:
                cmd = &resync_auto_data_col_cmd;
                break;
            case TRANS_CMD_EPS_HEAT_SP:
                cmd = &set_eps_heater_sp_cmd;
                break;
            case TRANS_CMD_PAY_HEAT_SP:
                cmd = &set_pay_heater_sp_cmd;
                break;
            case TRANS_CMD_PAY_ACT_MOTORS:
                cmd = &actuate_pay_motors_cmd;
                break;
            default:
                break;
        }

        trans_decoded_rx_msg_avail = false;

        if (cmd == NULL) {
            return;
        }
        enqueue_cmd(cmd, arg1, arg2);
    }
}
