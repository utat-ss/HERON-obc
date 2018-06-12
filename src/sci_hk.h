#ifndef HOUSEKEEPING_H
#define HOUSEKEEPING_H

#include <stdint.h>
#include <can/can_data_protocol.h>
#include "main.h"

// CAN callbacks
void req_hk_timer_callback();
void data_rx_mob_callback(const uint8_t* data, uint8_t len);
void receive_science(uint8_t field_num, uint8_t* data);
void receive_hk(uint8_t board_num, uint8_t field_num, uint8_t* payload);
void write_to_flash(uint8_t board_num, uint8_t field_num, uint8_t* data);

#endif
