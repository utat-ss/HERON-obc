#ifndef HOUSEKEEPING_H
#define HOUSEKEEPING_H

#include  <stdint.h>

#include "obc.h"


#define PAY_HK_TYPE 0 // kamran has defined these elsewhere
#define EPS_HK_TYPE 1
#define EPS_HK_BLOCK_SIZE 100
#define PAY_HK_BLOCK_SIZE 2

#define SCI_BLOCK_SIZE 218
#define SCI_TYPE 0 // kamran will define in his code, delete this then

// CAN callbacks
void req_hk_timer_callback();
void data_rx_mob_callback(const uint8_t* data, uint8_t len);
void receive_science(uint8_t field_num, uint8_t* data);
void receive_hk(uint8_t board_num, uint8_t field_num, uint8_t* payload);
void write_to_flash(uint8_t board_num, uint8_t field_num, uint8_t* data);

#endif
