#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "global_header.h"

uint8_t GLOBAL_SCI_FIELD_NUM,
        GLOBAL_PAY_HK_FIELD_NUM,
        GLOBAL_EPS_HK_FIELD_NUM;

void init_callbacks();
void PAY_CMD_Tx_data_callback(uint_t* data, uint_t len);
void EPS_CMD_Tx_data_callback(uint_t* data, uint_t len);
void data_rx_mob_callback(uint8_t* data, uint8_t len);

#endif
