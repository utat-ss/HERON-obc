#ifndef HOUSEKEEPING_H
#define HOUSEKEEPING_H

#include "global_header.h"

#define HK_REQ 0 // can set later
#define PAY_HK_TYPE 1 // kamran has defined these elsewhere
#define EPS_HK_TYPE 2
#define HK_BLOCK_SIZE 100

struct HK_packet {

  uint8_t         packet_type;       //     \use SSM_RX macros defined in can.h
  uint8_t         packet_id;        //      increment once every time
  uint8_t         data_length;      //      number of 8 bits in the packet
  uint8_t   *     data;             //      data

};

void decode_HK_message (uint16_t sender_id, uint16_t message_id, uint8_t* pt_data, uint8_t size);

// CAN callbacks
void req_hk_timer_callback();
void receive_hk(uint8_t board_num, uint8_t field_num, uint8_t* payload);

#endif
