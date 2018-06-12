#include "rx_callbacks.h"

void data_rx_callback(const uint8_t* data, uint8_t len) {
    uint8_t boards = data[0];
    uint8_t message_type = data[1];
    uint8_t field_num = data[2];

    print("RX Callback\n");
    print("Received Message:\n");
    print_bytes((uint8_t *) data, len);

    switch (message_type) {
        case CAN_PAY_HK:
            receive_pay_hk(field_num);
            break;
        case CAN_EPS_HK:
            receive_eps_hk(field_num);
            break;
        case CAN_PAY_SCI:
            receive_pay_sci(field_num);
            break;
        default:
            print("Invalid received message\n");
            break;
    }
}

void receive_pay_hk(uint8_t field_num){
    if(field_num + 1 < CAN_PAY_HK_FIELD_COUNT) {
        next_pay_hk_field_num = field_num + 1;
        send_next_pay_hk_field_num = true;
        print("Enqueued PAY_HK, field_num: %d\n", field_num + 1);
    } else {
        print("PAY_HK done\n");
    }
}

void receive_eps_hk(uint8_t field_num){
    if (field_num + 1 < CAN_EPS_HK_FIELD_COUNT) {
        next_eps_hk_field_num = field_num + 1;
        send_next_eps_hk_field_num = true;
        print("Enqueued EPS_HK, field_num: %d\n", field_num + 1);
    } else {
        print("EPS_HK done\n");
    }
}

void receive_pay_sci(uint8_t field_num){
    if (field_num + 1 < CAN_PAY_SCI_FIELD_COUNT){
        next_pay_sci_field_num = field_num + 1;
        send_next_pay_sci_field_num = true;
        print("Enqueued PAY_SCI, field_num: %d\n", field_num + 1);
    } else {
        print("PAY_SCI done\n");
    }
}
