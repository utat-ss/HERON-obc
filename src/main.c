#include "main.h"

// CAN mob for sending commands to PAY
mob_t pay_cmd_tx = {
    .mob_num = PAY_CMD_TX_MOB,
    .mob_type = TX_MOB,
    .id_tag = OBC_PAY_CMD_TX_MOB_ID,
    .ctrl = default_tx_ctrl,
    .tx_data_cb = PAY_CMD_Tx_data_callback
};

// CAN mob for sending commands to EPS
mob_t eps_cmd_tx = {
    .mob_num = EPS_CMD_TX_MOB,
    .mob_type = TX_MOB,
    .id_tag = OBC_EPS_CMD_TX_MOB_ID,
    .ctrl = default_tx_ctrl,
    .tx_data_cb = EPS_CMD_Tx_data_callback
};

// CAN mob for receiving data from any SSM
mob_t data_rx = {
    .mob_num = DATA_RX_MOB,
    .mob_type = RX_MOB,
    .dlc = 8, // this might change
    .id_tag = OBC_DATA_RX_MOB_ID,
    .id_mask = { 0x0000 },
    .ctrl = default_rx_ctrl,
    .rx_cb = data_rx_mob_callback
};


uint8_t next_pay_hk_field_num = 0;
bool send_next_pay_hk_field_num = false;

uint8_t next_pay_sci_field_num = 0;
bool send_next_pay_sci_field_num = false;

uint8_t next_eps_hk_field_num = 0;
bool send_next_eps_hk_field_num = false;


void print_bytes(uint8_t *data, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        print("0x%02x ", data[i]);
    }
    print("\n");
}

int main(void) {
    // Initialize lib-common libraries
    init_uart();
    print("UART Initialized\n");

    /* Steps to send commands to other SSMs */
    // 1. Initialize the callback function to preprocess the command packet as needed
    // 2. Initialize the CAN mob to send the command (or data) with the callback
    // 3. Initialize queue for command if needed
    // 4. Initalize timer with callback to enqueue Command to queue
    // 5. Check if any enqueued Commands in queue in loop below

    // Initialize CAN mobs to send commands
    init_can();
    init_tx_mob(&pay_cmd_tx);
    init_tx_mob(&eps_cmd_tx);

    // Initialize CAN mobs to recieve data
    init_rx_mob(&data_rx);

    // Timed commands are housekeeping (PAY and EPS) and science data
    // TODO: Change these times
    // TODO: Implement two timers
    init_timer_8bit(2, req_hk_timer_callback);
    init_timer_16bit(1, req_sci_timer_callback);
    print("Initialized timers\n");

    //req_hk_timer_callback();
    //req_sci_timer_callback();

    print("Starting main loop\n");
    // Loop to check if we should send a CAN message
    while (1) {
        if (send_next_pay_hk_field_num) {
            resume_mob(&pay_cmd_tx);
        }
        else if (send_next_pay_sci_field_num) {
            resume_mob(&pay_cmd_tx);
        }
        else if (send_next_eps_hk_field_num) {
            resume_mob(&eps_cmd_tx);
        }

        _delay_ms(1000);
    }
}
