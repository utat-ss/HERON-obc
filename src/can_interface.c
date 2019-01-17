// TODO - define constants in lib-common for MOB numbers (0-5)

#include "can_interface.h"

void pay_cmd_tx_data_callback(uint8_t* data, uint8_t *len) {
    if (queue_empty(&pay_tx_msg_queue)) {
        *len = 0;
    }

    else {
        dequeue(&pay_tx_msg_queue, data);
        *len = 8;
    }
}

void eps_cmd_tx_data_callback(uint8_t* data, uint8_t *len) {
    if (!queue_empty(&eps_tx_msg_queue)) {
        dequeue(&eps_tx_msg_queue, data);
        print("%d\n", eps_tx_msg_queue.tail - eps_tx_msg_queue.head);
        *len = 8;

        print_bytes(data, *len);
        print("Sending EPS_HK #%u\n", data[2]);
    }

    else {
        *len = 0;
    }
}


void data_rx_callback(const uint8_t* data, uint8_t len) {
    // print("RX Callback\n");
    print_bytes((uint8_t *) data, len);

    if (len > 0) {
        // print("Enqueued to data_rx_msg_queue\n");
        enqueue(&data_rx_msg_queue, data);
    }
}


// TODO
// mob_t status_rx_mob;
// mob_t status_tx_mob;

// CAN mob for sending commands to PAY
mob_t pay_cmd_tx_mob = {
    .mob_num = 3,
    .mob_type = TX_MOB,
    .id_tag = OBC_PAY_CMD_TX_MOB_ID,
    .ctrl = default_tx_ctrl,
    .tx_data_cb = pay_cmd_tx_data_callback
};

// CAN mob for sending commands to EPS
mob_t eps_cmd_tx_mob = {
    .mob_num = 4,
    .mob_type = TX_MOB,
    .id_tag = OBC_EPS_CMD_TX_MOB_ID,
    .ctrl = default_tx_ctrl,
    .tx_data_cb = eps_cmd_tx_data_callback
};

// CAN mob for receiving data from any SSM
mob_t data_rx_mob = {
    .mob_num = 5,
    .mob_type = RX_MOB,
    .dlc = 8, // TODO - this might change
    .id_tag = OBC_DATA_RX_MOB_ID,
    .id_mask = CAN_RX_MASK_ID,
    .ctrl = default_rx_ctrl,
    .rx_cb = data_rx_callback
};
