#include "obc.h"

// CAN mob for sending commands to PAY
mob_t obc_pay_cmd_tx = {
  .mob_num = PAY_CMD_TX_MOB,
  .mob_type = TX_MOB,
  .id_tag = OBC_PAY_CMD_TX_MOB_ID,
  .ctrl = default_tx_ctrl,
  .tx_data_cb = PAY_CMD_Tx_data_callback
};

// CAN mob for sending commands to EPS
mob_t obc_eps_cmd_tx = {
  .mob_num = EPS_CMD_TX_MOB,
  .mob_type = TX_MOB,
  .id_tag = OBC_EPS_CMD_TX_MOB_ID,
  .ctrl = default_tx_ctrl,
  .tx_data_cb = EPS_CMD_Tx_data_callback
};

// CAN mob for receiving data from any SSM
mob_t obc_data_rx = {
    .mob_num = DATA_RX_MOB,
    .mob_type = RX_MOB,
    .dlc = 7, // this might change
    .id_tag = OBC_DATA_RX_MOB_ID,
    .id_mask = { 0x0000 },
    .ctrl = default_rx_ctrl,
    .rx_cb = data_rx_mob_callback
};

void print_bytes(uint8_t *data, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        print("0x%02x ", data[i]);
    }
    print("\n");
}

int main(void) {

    // Initialize lib-common libraries
    init_uart();
    print("starting main\n");
    init_can();

    /* Steps to send commands to other SSMs */
    // 1. Initialize the callback function to preprocess the command packet as needed
    // 2. Initialize the CAN mob to send the command (or data) with the callback
    // 3. Initialize queue for command if needed
    // 4. Initalize timer with callback to enqueue Command to queue
    // 5. Check if any enqueued Commands in queue in loop below

    // Initialize CAN mobs to send commands
    init_tx_mob(&obc_pay_cmd_tx);
    init_tx_mob(&obc_eps_cmd_tx);

    // Initialize CAN mobs to recieve data
    init_rx_mob(&obc_data_rx);

    // Initialize block sizes for flash purposes
    // Define CAN callbacks via function declarations
    init_callbacks();

    // Timed commands are housekeeping (PAY and EPS) and science data
    // TODO: Change these times
    // TODO: Implement two timers
    //init_timer_8bit(1,req_hk_timer_callback);
  //  init_timer_16bit(1,req_sci_timer_callback);

    OBC_Command command;
    uint8_t field_num;

    print("starting main\n");

    // Generating queues

    // Misc. queue
    obc_queue = initCmdQueue();
    // Request science data queue
    sci_tx_queue = initCanQueue();
    // Request PAY housekeeping queue
    pay_hk_tx_queue = initCanQueue();
    // Request EPS housekeeping queue
    eps_hk_tx_queue = initCanQueue();

    req_hk_timer_callback();
    req_sci_timer_callback();

    // Loop to check if queues have a command to be dequeued
    while (1) {
      if (!CANQ_isEmpty(&sci_tx_queue)) {
          CANQ_dequeue(&sci_tx_queue, &field_num);
          GLOBAL_SCI_FIELD_NUM = field_num;
          resume_mob(&obc_pay_cmd_tx);
      }
      if (!CANQ_isEmpty(&pay_hk_tx_queue)) {
          CANQ_dequeue(&pay_hk_tx_queue, &field_num);
          GLOBAL_PAY_HK_FIELD_NUM = field_num;
          resume_mob(&obc_pay_cmd_tx);
      }
      if (!CANQ_isEmpty(&eps_hk_tx_queue)) {
          CANQ_dequeue(&eps_hk_tx_queue, &field_num);
          GLOBAL_EPS_HK_FIELD_NUM = field_num;
          resume_mob(&obc_eps_cmd_tx);
      }
      if (!CMDQ_isEmpty(&obc_queue)) {
          CMDQ_dequeue(&obc_queue, &command);
          // handle here
      }
    }
}
