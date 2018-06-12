#include "tx_callbacks.h"

void pay_cmd_tx_data_callback(uint8_t* data, uint8_t *len) {
    *len = 0;

    if(send_next_pay_hk_field_num && next_pay_hk_field_num < CAN_PAY_HK_FIELD_COUNT){
        data[0] = 0;    // TODO
        data[1] = CAN_PAY_HK;
        data[2] = next_pay_hk_field_num;
        *len = 8;

        print("Sending PAY_HK Request\n");
        print_hex_bytes(data, *len);

        send_next_pay_hk_field_num = false;
    }

    else if(send_next_pay_sci_field_num && next_pay_sci_field_num < CAN_PAY_SCI_FIELD_COUNT){
        data[0] = 0;    // TODO
        data[1] = CAN_PAY_SCI;
        data[2] = next_pay_sci_field_num;
        *len = 8;

        print("Sending PAY_SCI Request\n");
        print_hex_bytes(data, *len);

        send_next_pay_sci_field_num = false;
    }
}

void eps_cmd_tx_data_callback(uint8_t* data, uint8_t *len) {
    *len = 0;

    if(send_next_eps_hk_field_num && next_eps_hk_field_num < CAN_EPS_HK_FIELD_COUNT){
        data[0] = 0;    // TODO
        data[1] = CAN_EPS_HK;
        data[2] = next_eps_hk_field_num;
        *len = 8;

        print("Sending EPS_HK Request\n");
        print_hex_bytes(data, *len);

        send_next_eps_hk_field_num = false;
    }
}
