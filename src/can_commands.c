#include "can_commands.h"



uint32_t pay_hk_data[CAN_PAY_HK_FIELD_COUNT] = { 0 };
uint32_t pay_sci_data[CAN_PAY_SCI_FIELD_COUNT] = { 0 };
uint32_t eps_hk_data[CAN_EPS_HK_FIELD_COUNT] = { 0 };


void handle_pay_hk(const uint8_t* data);
void handle_pay_sci(const uint8_t* data);
void handle_eps_hk(const uint8_t* data);
void handle_pay_motor(const uint8_t* data);

void handle_rx_msg(void) {
    // TODO

    if (queue_empty(&data_rx_msg_queue)) {
        return;
    }

    else {
        uint8_t data[8];
        dequeue(&data_rx_msg_queue, data);

        uint8_t boards = data[0];
        uint8_t message_type = data[1];

        switch (message_type) {
            case CAN_PAY_HK:
                handle_pay_hk(data);
                break;
            case CAN_PAY_SCI:
                handle_pay_sci(data);
                break;
            case CAN_EPS_HK:
                handle_eps_hk(data);
                break;
            case CAN_PAY_MOTOR:
                handle_pay_motor(data);
                break;
            default:
                print("Invalid RX\n");
                break;
        }
    }
}

void handle_pay_hk(const uint8_t* data){
    // print("%s\n", __FUNCTION__);
    uint8_t field_num = data[2];

    if (field_num < CAN_PAY_HK_FIELD_COUNT) {
        // Save data
        pay_hk_data[field_num] =
                ( ((uint32_t) data[3]) << 16 ) |
                ( ((uint32_t) data[4]) << 8  ) |
                ( ((uint32_t) data[5]) << 0  );

        if (field_num + 1 < CAN_PAY_HK_FIELD_COUNT) {
            uint8_t d[8] = { 0 };
            d[0] = 0;   // TODO
            d[1] = CAN_PAY_HK;
            d[2] = field_num + 1;
            enqueue(&pay_tx_msg_queue, d);
            enqueue(&cmd_queue, (uint8_t*) &resume_mob_cmd);
            enqueue(&cmd_arg_queue, (uint8_t*) &pay_cmd_tx_mob);
            // print("Enqueued PAY_HK #%d\n", field_num + 1);
        } else {
            print("PAY_HK done\n");

            uint8_t d[8] = { 0 };
            d[0] = 0;   // TODO
            d[1] = CAN_PAY_SCI;
            d[2] = 0;
            enqueue(&pay_tx_msg_queue, d);
            enqueue(&cmd_queue, (uint8_t *) &resume_mob_cmd);
            enqueue(&cmd_arg_queue, (uint8_t *) &pay_cmd_tx_mob);
        }
    }

    else {
        print("Unexpected\n");
    }
}

void handle_pay_sci(const uint8_t* data){
    uint8_t field_num = data[2];

    if (field_num < CAN_PAY_SCI_FIELD_COUNT) {
        // Save data
        pay_sci_data[field_num] =
                ( ((uint32_t) data[3]) << 16 ) |
                ( ((uint32_t) data[4]) << 8  ) |
                ( ((uint32_t) data[5]) << 0  );

        if (field_num + 1 < CAN_PAY_SCI_FIELD_COUNT){
            uint8_t d[8] = { 0 };
            d[0] = 0;   // TODO
            d[1] = CAN_PAY_SCI;
            d[2] = field_num + 1;
            enqueue(&pay_tx_msg_queue, d);
            enqueue(&cmd_queue, (uint8_t *) &resume_mob_cmd);
            enqueue(&cmd_arg_queue, (uint8_t *) &pay_cmd_tx_mob);
        } else {
          //  print("PAY_SCI done\n");

            uint8_t d[8] = { 0 };
            d[0] = 0;   // TODO
            d[1] = CAN_EPS_HK;
            d[2] = 0;
            enqueue(&eps_tx_msg_queue, d);
            enqueue(&cmd_queue, (uint8_t*) &resume_mob_cmd);
            enqueue(&cmd_arg_queue, (uint8_t*) &eps_cmd_tx_mob);
        }
    }
}

void handle_eps_hk(const uint8_t* data){
    uint8_t field_num = data[2];

    if (field_num < CAN_EPS_HK_FIELD_COUNT) {
        // TODO: Save to flash, instead of to this array
        eps_hk_data[field_num] =
                ( ((uint32_t) data[3]) << 16 ) |
                ( ((uint32_t) data[4]) << 8  ) |
                ( ((uint32_t) data[5]) << 0  );

        if (field_num + 1 < CAN_EPS_HK_FIELD_COUNT) {
            // enqueue the next HK req
            uint8_t d[8] = { 0 };
            d[0] = 0;   // TODO
            d[1] = CAN_EPS_HK;
            d[2] = field_num + 1;
            enqueue(&eps_tx_msg_queue, d);
            enqueue(&cmd_queue, (uint8_t*) &resume_mob_cmd);
            enqueue(&cmd_arg_queue, (uint8_t*) &eps_cmd_tx_mob);
            print("Enqueued\n");
        } else {
            print("EPS_HK done\n");

            print("\nDone CAN data\n\n");
            // print_raw_can_data();
            // print_can_data();
        }
    }
}

void handle_pay_motor(const uint8_t* data){
    uint8_t field_num = data[2];
    if (field_num == CAN_PAY_MOTOR_ACTUATE) {
        print("PAY_MOTOR done\n");
    }
}
