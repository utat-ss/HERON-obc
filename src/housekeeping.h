#ifndef HOUSEKEEPING_H
#define HOUSEKEEPING_H

#include "global_header.h"

#define EPS_HK 2
#define PAY_HK 1

struct HK_packet {

  uint8_t         packet_type;       //     \use SSM_RX macros defined in can.h
  uint8_t         packet_id;        //      increment once every time
  uint8_t         data_length;      //      number of 8 bits in the packet
  uint8_t   *     data;             //      data

};

void decode_HK_message (uint16_t sender_id, uint16_t message_id, uint8_t* pt_data, uint8_t size);
void request_HK (uint8_t target);
void hk_req_all();

#endif
