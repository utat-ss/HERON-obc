#include "housekeeping.h"

/*
  Take in a housekeeping CAN message from can_hanadler, place it into a
  housekeeping struct, place into the memory
*/

void req_hk_timer_callback(){
  enqueue(&pay_hk_tx_queue, 0);
  enqueue(&eps_hk_tx_queue, 0);
}

void receive_hk(uint8_t board_num, uint8_t field_num, uint8_t* payload){
  write_to_flash(board_num, field_num, payload);
  if(field<HK_BLOCK_SIZE){
    switch (board_num) {
      case PAY_HK_TYPE:
        enqueue(&pay_hk_tx_queue, field_num+1);
        break;
      case EPS_HK_TYPE:
        enqueue(&eps_hk_tx_queue, field_num+1)
        break;
      default:
        break;
    }
  }
}
