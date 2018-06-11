#include "tx_callbacks.h"

void init_callbacks(){
  GLOBAL_SCI_FIELD_NUM = SCI_BLOCK_SIZE;
  GLOBAL_PAY_HK_FIELD_NUM = PAY_HK_BLOCK_SIZE;
  GLOBAL_EPS_HK_FIELD_NUM = EPS_HK_BLOCK_SIZE;
  // resume data_rx_mob?
}

void PAY_CMD_Tx_data_callback(uint8_t* data, uint8_t *len) {
  *len = 0;
  if(GLOBAL_SCI_FIELD_NUM < SCI_BLOCK_SIZE){
    data[0] = SCI_TYPE;
    data[1] = GLOBAL_SCI_FIELD_NUM;
    *len = 8;
    print("Sending Science Request\n");
    print("Transmitting Message to PAY:\n");
    print_bytes(data, *len);
    return;
  }
  if(GLOBAL_PAY_HK_FIELD_NUM < PAY_HK_BLOCK_SIZE){
    data[0] = PAY_HK_TYPE;
    data[1] = GLOBAL_PAY_HK_FIELD_NUM;
    *len = 8;
    print("Sending Housekeeping Request\n");
    print("Transmitting Message to PAY:\n");
    print_bytes(data, *len);
    return;
  }
}

void EPS_CMD_Tx_data_callback(uint8_t* data, uint8_t *len) {
  // if(GLOBAL_EPS_HK_FIELD_NUM < EPS_HK_BLOCK_SIZE){\
  //   data[0] = HK_REQ;
  //   data[1] = GLOBAL_EPS_HK_FIELD_NUM;
  //   *len = 2;
  //   print("Sending Housekeeping Request\n");
  //   print("Transmitting Message to EPS:\n");
  //   print_bytes(data, *len);
  //   return;
  // }
}
