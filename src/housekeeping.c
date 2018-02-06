#include "housekeeping.h"

/*
  Take in a housekeeping CAN message from can_hanadler, place it into a
  housekeeping struct, place into the memory
*/

// Make sure a header is added for HK
// Write in order of HEADER - OBC - EPS - COMMS - PAY - END
// Write to data buffer for now
// Decide when to write to mem, when data buffer is full? does the data buffer act like cache?

// 2 bytes of data * 2 sensors * 2 boards = 8 bytes needed (4 per board)
// This means it should only require one hk message from each board
void decode_HK_message (uint16_t sender_id, uint16_t message_id, uint8_t* pt_data, uint8_t size){
  /*
    // returns address to where 4 bytes were written
    int addr = write_hk_data_to_flash(sensor1a, sensor1b, sensor2a, sensor2b);

    // OPTION A: ENQUEUE TM REQUEST
    // xxx = PAY or EPS
    enqueue(&queue, read_and_telemtry_xxx_hk, 4, addr);

    // OPTION B: WAIT FOR CONNECTION
    enqueue(&tm_queue, read_and_telemtry_xxx_hk, 4, addr);

  */
}


/*
  Sends CAN message requesting housekeepign from all subsystems
*/
void request_HK ( uint8_t target ){
  if(target & PAY_HK){
      // MOB 3: Use a defined code to represent a hk request
  }
  if(target & EPS_HK){
      // MOB 4: Use a defined code to represent a hk request
  }
}

void hk_req_all(){
  print("about to request all\n");
  request_HK(PAY_HK | EPS_HK);
}
