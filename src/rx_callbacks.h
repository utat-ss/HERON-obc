#ifndef RX_CALLBACKS_H
#define RX_CALLBACKS_H

#include <stdint.h>
#include <can/can_data_protocol.h>
#include <utilities/utilities.h>
#include "main.h"

void data_rx_callback(const uint8_t* data, uint8_t len);
void receive_pay_hk(const uint8_t* data, uint8_t len);
void receive_pay_sci(const uint8_t* data, uint8_t len);
void receive_eps_hk(const uint8_t* data, uint8_t len);
void receive_pay_motor(const uint8_t* data, uint8_t len);

extern uint32_t pay_hk_data[];
extern uint32_t pay_sci_data[];
extern uint32_t eps_hk_data[];

#endif
