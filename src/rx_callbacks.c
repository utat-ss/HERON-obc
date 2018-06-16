#include "rx_callbacks.h"

uint32_t pay_hk_data[CAN_PAY_HK_FIELD_COUNT];
uint32_t pay_sci_data[CAN_PAY_SCI_FIELD_COUNT];
uint32_t eps_hk_data[CAN_EPS_HK_FIELD_COUNT];

void print_can_data(void) {
    // TODO - print in readable format
    // TODO - convert data

    for (uint8_t i = 0; i < CAN_PAY_HK_FIELD_COUNT; ++i) {
        print("pay_hk_data[%u] = %06x\n", i, pay_hk_data[i]);
    }
    print("\n");

    for (uint8_t i = 0; i < CAN_PAY_SCI_FIELD_COUNT; ++i) {
        print("pay_sci_data[%u] = %06x\n", i, pay_sci_data[i]);
    }
    print("\n");

    for (uint8_t i = 0; i < CAN_EPS_HK_FIELD_COUNT; ++i) {
        print("eps_hk_data[%u] = %06x\n", i, eps_hk_data[i]);
    }
    print("\n");
}

void data_rx_callback(const uint8_t* data, uint8_t len) {
    print("RX Callback\n");
    print("Received Message:\n");
    print_hex_bytes((uint8_t *) data, len);

    //uint8_t boards = data[0];
    uint8_t message_type = data[1];

    switch (message_type) {
        case CAN_PAY_HK:
            receive_pay_hk(data, len);
            break;
        case CAN_PAY_SCI:
            receive_pay_sci(data, len);
            break;
        case CAN_EPS_HK:
            receive_eps_hk(data, len);
            break;
        default:
            print("Invalid received message\n");
            break;
    }
}

void receive_pay_hk(const uint8_t* data, uint8_t len){
    uint8_t field_num = data[2];

    if (field_num < CAN_PAY_HK_FIELD_COUNT) {
        // Save data
        pay_hk_data[field_num] =
                ( ((uint32_t) data[3]) << 16 ) |
                ( ((uint32_t) data[4]) << 8  ) |
                ( ((uint32_t) data[5]) << 0  );

        if(field_num + 1 < CAN_PAY_HK_FIELD_COUNT) {
            next_pay_hk_field_num = field_num + 1;
            send_next_pay_hk_field_num = true;
            print("Enqueued PAY_HK, field_num: %d\n", field_num + 1);
        } else {
            print("PAY_HK done\n");
            next_pay_sci_field_num = 0;
            send_next_pay_sci_field_num = true;
        }
    }

    else {
        print("Unexpected field number\n");
    }
}

void receive_pay_sci(const uint8_t* data, uint8_t len){
    uint8_t field_num = data[2];

    if (field_num < CAN_PAY_SCI_FIELD_COUNT) {
        // Save data
        pay_sci_data[field_num] =
                ( ((uint32_t) data[3]) << 16 ) |
                ( ((uint32_t) data[4]) << 8  ) |
                ( ((uint32_t) data[5]) << 0  );

        if (field_num + 1 < CAN_PAY_SCI_FIELD_COUNT){
            next_pay_sci_field_num = field_num + 1;
            send_next_pay_sci_field_num = true;
            print("Enqueued PAY_SCI, field_num: %d\n", field_num + 1);
        } else {
            print("PAY_SCI done\n");
            next_eps_hk_field_num = 0;
            send_next_eps_hk_field_num = true;
        }
    }
}

void receive_eps_hk(const uint8_t* data, uint8_t len){
    uint8_t field_num = data[2];

    if (field_num < CAN_EPS_HK_FIELD_COUNT) {
        // Save data
        eps_hk_data[field_num] =
                ( ((uint32_t) data[3]) << 16 ) |
                ( ((uint32_t) data[4]) << 8  ) |
                ( ((uint32_t) data[5]) << 0  );

        if (field_num + 1 < CAN_EPS_HK_FIELD_COUNT) {
            next_eps_hk_field_num = field_num + 1;
            send_next_eps_hk_field_num = true;
            print("Enqueued EPS_HK, field_num: %d\n", field_num + 1);
        } else {
            print("EPS_HK done\n");
            print_can_data();
            uart_cmd_busy = false;
        }
    }
}
