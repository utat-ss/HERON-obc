#include "cmd_science.h"

void req_sci_timer_callback(){
  CANQ_enqueue(&sci_tx_queue, 0);
  print("Enqueued Science TX Message\n");
}

void receive_science(uint8_t field_num, uint8_t* data){
  write_to_flash(SCI_TYPE, field_num, data);
  if (field_num < SCI_BLOCK_SIZE){
    CANQ_enqueue(&sci_tx_queue, field_num+1);
    print("Enqueued Science TX with field_num:%d\n", (field_num+1));
  } else {
    print("field num equals SCI_BLOCK_SIZE");
  }
}
