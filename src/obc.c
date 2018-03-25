#include "obc.h"

mob_t obc_pay_cmd_tx = {
  .mob_num = PAY_CMD_TX_MOB,
  .mob_type = TX_MOB,
  .id_tag = { 0x0000 },
  .ctrl = default_tx_ctrl,
  .tx_data_cb = PAY_CMD_Tx_data_callback
};
mob_t obc_eps_cmd_tx = {
  .mob_num = EPS_CMD_TX_MOB,
  .mob_type = TX_MOB,
  .id_tag = { 0x0000 },
  .ctrl = default_tx_ctrl,
  .tx_data_cb = EPS_CMD_Tx_data_callback
};
mob_t obc_data_rx = {
    .mob_num = DATA_RX_MOB,
    .mob_type = RX_MOB,
    .dlc = 7, // this might change
    .id_tag = { 0x0000 },
    .id_mask = { 0x0000 },
    .ctrl = default_rx_ctrl,
    .rx_cb = data_rx_mob_callback
};

int main(void) {
    init_uart();
    init_can();
    init_tx_mob(&obc_pay_cmd_tx);
    init_tx_mob(&obc_eps_cmd_tx);
    init_rx_mob(&obc_data_rx);
    init_callbacks();

    // change these times
    init_timer(1,req_hk_timer_callback);
    init_timer(1,req_sci_timer_callback);

    OBC_Command command;
    uint8_t field_num;

    print("starting main\n");

    // Generating queues
    obc_queue = initCmdQueue();
    sci_tx_queue = initCanQueue();
    pay_hk_tx_queue = initCanQueue();
    eps_hk_tx_queue = initCanQueue();

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
