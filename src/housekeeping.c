#include "housekeeping.h"

/*
  Take in a housekeeping CAN message from can_hanadler, place it into a
  housekeeping struct, place into the memory
*/
void decode_HK_message (uint16_t sender_id, uint16_t message_id, uint8_t* pt_data, uint8_t size){


}


/*
  Sends CAN message requesting housekeepign from all subsystems
*/
void request_HK ( uint8_t target ){


}
