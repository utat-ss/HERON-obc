#include "housekeeping.h"

/*
  Take in a housekeeping CAN message from can_hanadler, place it into a
  housekeeping struct, place into the memory
*/

// Make sure a header is added for HK
// Write in order of HEADER - OBC - EPS - COMMS - PAY - END
// Write to data buffer for now
// Decide when to write to mem, when data buffer is full? does the data buffer act like cache?

void decode_HK_message (uint16_t sender_id, uint16_t message_id, uint8_t* pt_data, uint8_t size){


}


/*
  Sends CAN message requesting housekeepign from all subsystems
*/
void request_HK ( uint16_t target ){
  // if(target & ALL_RX)
    print("hk_req_all success\n");

}

void hk_req_all(){
  print("about to request all\n");
  //request_HK(ALL_RX);
}
