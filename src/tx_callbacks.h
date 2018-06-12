#ifndef TX_CALLBACKS_H
#define TX_CALLBACKS_H

#include <stdint.h>
#include <can/can_data_protocol.h>
#include <utilities/utilities.h>
#include "main.h"

void init_callbacks();
void pay_cmd_tx_data_callback(uint8_t* data, uint8_t *len);
void eps_cmd_tx_data_callback(uint8_t* data, uint8_t *len);

#endif
