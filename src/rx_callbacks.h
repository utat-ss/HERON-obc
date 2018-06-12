#ifndef RX_CALLBACKS_H
#define RX_CALLBACKS_H

#include <stdint.h>
#include <can/can_data_protocol.h>
#include "main.h"

void data_rx_callback(const uint8_t* data, uint8_t len);
void receive_pay_hk(uint8_t field_num);
void receive_pay_sci(uint8_t field_num);
void receive_eps_hk(uint8_t field_num);

#endif
