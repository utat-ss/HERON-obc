#include "housekeeping.h"

void req_hk_timer_callback(){
  CANQ_enqueue(&pay_hk_tx_queue, 0);
  CANQ_enqueue(&eps_hk_tx_queue, 0);
  print("Enqueued Housekeeping TX Messages\n");
}

void receive_hk(uint8_t board_num, uint8_t field_num, uint8_t* payload){
  write_to_flash(board_num, field_num, payload);
  switch (board_num) {
    case PAY_HK_TYPE:
      if(field_num<PAY_HK_BLOCK_SIZE) {
        CANQ_enqueue(&pay_hk_tx_queue, field_num+1);
        print("Enqueued TX PAY HK Message, field_num:%d\n",(field_num+1));
      } else {
        print("field num equals PAY_HK_BLOCK_SIZE\n\n");
      }
      break;
    case EPS_HK_TYPE:
      if(field_num<EPS_HK_BLOCK_SIZE) {
        CANQ_enqueue(&eps_hk_tx_queue, field_num+1);
        print("Enqueued TX EPS HK Message,field_num:%d\n",(field_num+1));
      } else {
        print("field num equals EPS_HK_BLOCK_SIZE\n\n");
      }
      break;
    default:
      break;
  }
}

void write_to_flash(uint8_t board_num, uint8_t field_num, uint8_t* data){
  return;
}
