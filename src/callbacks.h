#ifndef CALLBACKS_H
#define CALLBACKS_H

// #include "global_header.h"
#include  <stdint.h>

#include "housekeeping.h"
#include "cmd_science.h"

#define HK_REQ 0x1 // use packets.h after merge
#define SCI_REQ 0x2 // use packets.h after merge

uint8_t GLOBAL_SCI_FIELD_NUM,
        GLOBAL_PAY_HK_FIELD_NUM,
        GLOBAL_EPS_HK_FIELD_NUM;

// mob_t obc_pay_cmd_tx, obc_eps_cmd_tx, obc_data_rx;

void init_callbacks();
void PAY_CMD_Tx_data_callback(uint8_t* data, uint8_t *len);
void EPS_CMD_Tx_data_callback(uint8_t* data, uint8_t *len);
void data_rx_mob_callback(const uint8_t* data, uint8_t len);

#endif
