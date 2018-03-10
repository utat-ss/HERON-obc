#include "cmd_science.h"

void req_sci_timer_callback(){
  enqueue(&sci_tx_queue, 0);
}

void receive_science(uint8_t field_num, uint8_t* data){
  write_to_flash(SCI_TYPE, field_num, data);
  if (field_num < SCI_BLOCK_SIZE)
    enqueue(&sci_tx_queue, field_num+1);
}
