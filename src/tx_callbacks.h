#ifndef CALLBACKS_H
#define CALLBACKS_H

// #include "global_header.h"
#include <stdint.h>
#include <can/can_data_protocol.h>
#include "main.h"

// mob_t obc_pay_cmd_tx, obc_eps_cmd_tx, obc_data_rx;

void init_callbacks();
void PAY_CMD_Tx_data_callback(uint8_t* data, uint8_t *len);
void EPS_CMD_Tx_data_callback(uint8_t* data, uint8_t *len);

#endif
