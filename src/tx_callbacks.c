#include "tx_callbacks.h"

void init_callbacks(){
    GLOBAL_SCI_FIELD_NUM = CAN_PAY_SCI_FIELD_COUNT;
    GLOBAL_PAY_HK_FIELD_NUM = CAN_PAY_HK_FIELD_COUNT;
    GLOBAL_EPS_HK_FIELD_NUM = CAN_EPS_HK_FIELD_COUNT;
    // resume data_rx_mob?
}

void PAY_CMD_Tx_data_callback(uint8_t* data, uint8_t *len) {
    *len = 0;
    if(GLOBAL_SCI_FIELD_NUM < CAN_PAY_SCI_FIELD_COUNT){
        data[0] = CAN_PAY_SCI;
        data[1] = GLOBAL_SCI_FIELD_NUM;
        *len = 8;
        print("Sending Science Request\n");
        print("Transmitting Message to PAY:\n");
        print_bytes(data, *len);
        return;
    }
    if(GLOBAL_PAY_HK_FIELD_NUM < CAN_PAY_HK_FIELD_COUNT){
        data[0] = CAN_PAY_HK;
        data[1] = GLOBAL_PAY_HK_FIELD_NUM;
        *len = 8;
        print("Sending Housekeeping Request\n");
        print("Transmitting Message to PAY:\n");
        print_bytes(data, *len);
        return;
    }
}

void EPS_CMD_Tx_data_callback(uint8_t* data, uint8_t *len) {
    // if(GLOBAL_EPS_HK_FIELD_NUM < CAN_EPS_HK_FIELD_COUNT){\
    //   data[0] = HK_REQ;
    //   data[1] = GLOBAL_EPS_HK_FIELD_NUM;
    //   *len = 2;
    //   print("Sending Housekeeping Request\n");
    //   print("Transmitting Message to EPS:\n");
    //   print_bytes(data, *len);
    //   return;
    // }
}
