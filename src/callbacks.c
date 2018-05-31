#include "callbacks.h"

void init_callbacks(){
  GLOBAL_SCI_FIELD_NUM = SCI_BLOCK_SIZE;
  GLOBAL_PAY_HK_FIELD_NUM = PAY_HK_BLOCK_SIZE;
  GLOBAL_EPS_HK_FIELD_NUM = EPS_HK_BLOCK_SIZE;
  // resume data_rx_mob?
}

void PAY_CMD_Tx_data_callback(uint8_t* data, uint8_t *len) {
  *len = 0;
  if(GLOBAL_SCI_FIELD_NUM < SCI_BLOCK_SIZE){
    data[0] = SCI_REQ;
    data[1] = GLOBAL_SCI_FIELD_NUM;
    *len = 2;
    return;
  }
  if(GLOBAL_PAY_HK_FIELD_NUM < PAY_HK_BLOCK_SIZE){
    data[0] = HK_REQ;
    data[1] = GLOBAL_PAY_HK_FIELD_NUM;
    *len = 2;
    return;
  }
}

void EPS_CMD_Tx_data_callback(uint8_t* data, uint8_t *len) {
  if(GLOBAL_EPS_HK_FIELD_NUM < EPS_HK_BLOCK_SIZE){
    data[0] = HK_REQ;
    data[1] = GLOBAL_EPS_HK_FIELD_NUM;
    *len = 2;
    return;
  }
}

/*
  Assumed data format:
  1 byte: indicate board number
  1 byte: indicates science/hk value is being received (stripped)
  1 byte: indicates field number (uint8_t)
  4 bytes: data
*/

void data_rx_mob_callback(const uint8_t* data, uint8_t len) {
/*
    uint8_t board_num = data[0];
    uint8_t data_type = data[1];
    uint8_t field = data[2];
    uint8_t* payload = &(data[3]);
*/
  uint8_t data_type = data[0];
  uint8_t field = data[1];
  uint8_t* payload = (uint8_t *) (&(data[2]));

    switch (data_type) {
        case SCI_REQ:
            receive_science(field, payload);
            break;
        case HK_REQ:
            receive_hk(PAY_HK_TYPE, field, payload);
            break;
        default:
            break;
    }
}
