#include "sci_hk.h"
/*
  Assumed data format:
  1 byte: indicate board number
  1 byte: indicates science/hk value is being received (stripped)
  1 byte: indicates field number (uint8_t)
  4 bytes: data
*/

void data_rx_mob_callback(const uint8_t* data, uint8_t len) {
/*
    uint8_t board_num = data[0];
    uint8_t data_type = data[1];
    uint8_t field = data[2];
    uint8_t* payload = &(data[3]);
*/
  uint8_t data_type = data[0];
  uint8_t field = data[1];
  uint8_t* payload = (uint8_t *) (&(data[2]));

  print("RX Callback\n");
  print("Received Message:\n");
  print_bytes((uint8_t *) data, len);

    switch (data_type) {
        case CAN_PAY_SCI:
            receive_science(field, payload);
            break;
        case CAN_PAY_HK:
            receive_hk(CAN_PAY_HK, field, payload);
            break;
        default:
            print("Invalid received message\n");
            break;
  }
}

void receive_science(uint8_t field_num, uint8_t* data){
  if (field_num + 1 < CAN_PAY_SCI_FIELD_COUNT){
    CANQ_enqueue(&sci_tx_queue, field_num+1);
    print("Enqueued Science TX with field_num:%d\n", (field_num+1));
  } else {
    print("field num equals SCI_BLOCK_SIZE");
  }
}

void receive_hk(uint8_t board_num, uint8_t field_num, uint8_t* payload){
  switch (board_num) {
    case CAN_PAY_HK:
      if(field_num + 1 < CAN_PAY_HK_FIELD_COUNT) {
        CANQ_enqueue(&pay_hk_tx_queue, field_num+1);
        print("Enqueued TX PAY HK Message, field_num:%d\n",(field_num+1));
      } else {
        print("field num equals PAY_HK_BLOCK_SIZE\n\n");
      }
      break;

    // case EPS_HK_TYPE:
    //   if(field_num<EPS_HK_BLOCK_SIZE) {
    //     CANQ_enqueue(&eps_hk_tx_queue, field_num+1);
    //     print("Enqueued TX EPS HK Message,field_num:%d\n",(field_num+1));
    //   } else {
    //     print("field num equals EPS_HK_BLOCK_SIZE\n\n");
    //   }
    //   break;

    default:
      break;
  }
}
