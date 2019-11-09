#include "can_interface.h"

void pay_cmd_tx_callback(uint8_t* data, uint8_t *len) {
    if (queue_empty(&pay_tx_msg_queue)) {
        *len = 0;
        return;
    }

    dequeue(&pay_tx_msg_queue, data);
    *len = 8;
}

void eps_cmd_tx_callback(uint8_t* data, uint8_t *len) {
    if (queue_empty(&eps_tx_msg_queue)) {
        *len = 0;
        return;
    }

    dequeue(&eps_tx_msg_queue, data);
    *len = 8;
}

void cmd_rx_callback(const uint8_t* data, uint8_t len) {
    if (len == 0) {
        return;
    }

    enqueue(&data_rx_msg_queue, data);
}




// CAN mob for sending commands to PAY
mob_t pay_cmd_tx_mob = {
    .mob_num = PAY_CMD_MOB_NUM,
    .mob_type = TX_MOB,
    .id_tag = { OBC_PAY_CMD_TX_MOB_ID },
    .ctrl = default_tx_ctrl,
    .tx_data_cb = pay_cmd_tx_callback
};

// CAN mob for sending commands to EPS
mob_t eps_cmd_tx_mob = {
    .mob_num = EPS_CMD_MOB_NUM,
    .mob_type = TX_MOB,
    .id_tag = { OBC_EPS_CMD_TX_MOB_ID },
    .ctrl = default_tx_ctrl,
    .tx_data_cb = eps_cmd_tx_callback
};

// CAN mob for receiving data from any SSM
mob_t cmd_rx_mob = {
    .mob_num = OBC_CMD_MOB_NUM,
    .mob_type = RX_MOB,
    .dlc = 8,
    .id_tag = { OBC_OBC_CMD_MOB_ID },
    .id_mask = { CAN_RX_MASK_ID },
    .ctrl = default_rx_ctrl,
    .rx_cb = cmd_rx_callback
};
