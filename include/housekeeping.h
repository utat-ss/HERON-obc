#ifndef HOUSEKEEPING_H
#define HOUSEKEEPING_H

// #include "global_header.h"
#include  <stdint.h>

#include "can_queue.h"
#include "obc.h"

#define PAY_HK_TYPE 0 // kamran has defined these elsewhere
#define EPS_HK_TYPE 1
#define EPS_HK_BLOCK_SIZE 100
#define PAY_HK_BLOCK_SIZE 6

// CAN callbacks
void req_hk_timer_callback();
void receive_hk(uint8_t board_num, uint8_t field_num, uint8_t* payload);
void write_to_flash(uint8_t board_num, uint8_t field_num, uint8_t* data);

#endif
