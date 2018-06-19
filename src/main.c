#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>

#include <uart/uart.h>
#include <timer/timer.h>
#include <can/can.h>
#include <queue/queue.h>
#include <can/can_ids.h>

#define PAY_CMD_TX_MOB 3
#define EPS_CMD_TX_MOB 4
#define DATA_RX_MOB 5

queue_t eps_tx_queue;
queue_t pay_sci_tx_queue;
queue_t pay_hk_tx_queue;
queue_t pay_motor_tx_queue;

#include <can/can_data_protocol.h>
#include <utilities/utilities.h>

uint32_t pay_hk_data[CAN_PAY_HK_FIELD_COUNT];
uint32_t pay_sci_data[CAN_PAY_SCI_FIELD_COUNT];
uint32_t eps_hk_data[CAN_EPS_HK_FIELD_COUNT];

void data_rx_callback(const uint8_t* data, uint8_t len);
void receive_pay_hk(const uint8_t* data, uint8_t len);
void receive_pay_sci(const uint8_t* data, uint8_t len);
void receive_eps_hk(const uint8_t* data, uint8_t len);
void receive_pay_motor(const uint8_t* data, uint8_t len);

void init_callbacks();
void pay_cmd_tx_data_callback(uint8_t* data, uint8_t *len);
void eps_cmd_tx_data_callback(uint8_t* data, uint8_t *len);

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
    //print_hex_bytes((uint8_t *) data, len);

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

        if (field_num + 1 < CAN_PAY_HK_FIELD_COUNT) {
            uint8_t d[8] = { 0 };
            d[0] = field_num + 1;
            enqueue(&pay_hk_tx_queue, d);
            print("Enqueued PAY_HK, field_num: %d\n", field_num + 1);
        } else {
            print("PAY_HK done\n");
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
            uint8_t d[8] = { 0 };
            d[0] = field_num + 1;
            enqueue(&pay_sci_tx_queue, d);
            print("Enqueued PAY_SCI, field_num: %d\n", field_num + 1);
        } else {
            print("PAY_SCI done\n");
        }
    }
}

void receive_eps_hk(const uint8_t* data, uint8_t len){
    uint8_t field_num = data[2];

    if (field_num < CAN_EPS_HK_FIELD_COUNT) {
        // TODO: Save to flash, instead of to this array
        // TODO: This is strange; should use a multi-dim array
        eps_hk_data[field_num] =
                ( ((uint32_t) data[3]) << 16 ) |
                ( ((uint32_t) data[4]) << 8  ) |
                ( ((uint32_t) data[5]) << 0  );

        if (field_num + 1 < CAN_EPS_HK_FIELD_COUNT) {
            // enqueue the next HK req
            // TODO: Fix me, very hacky
            uint8_t d[8] = { 0 };
            d[0] = field_num + 1;
            enqueue(&eps_tx_queue, d);
            print("Enqueued EPS_HK, field_num: %d\n", field_num + 1);
        } else {
            print("EPS_HK done\n");
            //print_can_data();
        }
    }
}

void receive_pay_motor(const uint8_t* data, uint8_t len){
    uint8_t field_num = data[2];
    if (field_num == CAN_PAY_MOTOR_ACTUATE) {
        print("PAY_MOTOR done\n");
    }
}

void pay_cmd_tx_data_callback(uint8_t* data, uint8_t *len) {
    *len = 0;

    if (!is_empty(&pay_hk_tx_queue)) {
        uint8_t next_field_num_packet[8] = { 0 };
        dequeue(&pay_hk_tx_queue, next_field_num_packet);

        data[0] = 0;    // TODO
        data[1] = CAN_PAY_HK;
        data[2] = next_field_num_packet[0];
        *len = 8;

        print("Sending PAY_HK Request\n");
        //print_hex_bytes(data, *len);
    } else if (!is_empty(&pay_sci_tx_queue)) {
        uint8_t next_field_num_packet[8] = { 0 };
        dequeue(&pay_sci_tx_queue, next_field_num_packet);

        data[0] = 0;    // TODO
        data[1] = CAN_PAY_SCI;
        data[2] = next_field_num_packet[0];
        *len = 8;

        print("Sending PAY_SCI Request\n");
        //print_hex_bytes(data, *len);

    } else if (!is_empty(&pay_motor_tx_queue)) {
        data[0] = 0;    // TODO
        data[1] = CAN_PAY_MOTOR;
        data[2] = CAN_PAY_MOTOR_ACTUATE;
        *len = 8;

        print("Sending PAY_MOTOR Request\n");
        //print_hex_bytes(data, *len);
    }
}

void eps_cmd_tx_data_callback(uint8_t* data, uint8_t *len) {
    *len = 0;
    // eps_tx_queue contains numbers of the field_nums to send
    if (!is_empty(&eps_tx_queue)) {
        uint8_t next_field_num_packet[8] = { 0 };
        dequeue(&eps_tx_queue, next_field_num_packet);

        data[0] = 0;
        data[1] = CAN_EPS_HK;
        data[2] = next_field_num_packet[0];
        *len = 8;

        print("Sending EPS_HK Request\n");
        //print_hex_bytes(data, *len);
    }
}

// CAN mob for sending commands to PAY
mob_t pay_cmd_tx = {
    .mob_num = PAY_CMD_TX_MOB,
    .mob_type = TX_MOB,
    .id_tag = OBC_PAY_CMD_TX_MOB_ID,
    .ctrl = default_tx_ctrl,
    .tx_data_cb = pay_cmd_tx_data_callback
};

// CAN mob for sending commands to EPS
mob_t eps_cmd_tx = {
    .mob_num = EPS_CMD_TX_MOB,
    .mob_type = TX_MOB,
    .id_tag = OBC_EPS_CMD_TX_MOB_ID,
    .ctrl = default_tx_ctrl,
    .tx_data_cb = eps_cmd_tx_data_callback
};

// CAN mob for receiving data from any SSM
mob_t data_rx = {
    .mob_num = DATA_RX_MOB,
    .mob_type = RX_MOB,
    .dlc = 8, // this might change
    .id_tag = OBC_DATA_RX_MOB_ID,
    .id_mask = CAN_RX_MASK_ID,
    .ctrl = default_rx_ctrl,
    .rx_cb = data_rx_callback
};

queue_t uart_cmd_queue;

typedef void(*uart_cmd_fn_t)(void);

typedef struct {
    char* cmd;
    uart_cmd_fn_t fn;
} cmd_t;

// Available UART commands

void handle_pay_eps_req();
cmd_t pay_eps_req = {
    .cmd = "PAY EPS REQUEST\r\n",
    .fn = handle_pay_eps_req
};

void handle_read_flash();
cmd_t read_flash = {
    .cmd = "READ FLASH\r\n",
    .fn = handle_read_flash
};

void handle_actuate_motor();
cmd_t actuate_motor = {
    .cmd = "ACTUATE MOTOR\r\n",
    .fn = handle_actuate_motor
};

// TODO - do we even need variable input arguments?
// To accept variable input, e.g. a command like "LAST MSG 4" which
// returns the last 4 CAN msgs, set the .cmd member to be the prefix
// of the command, without the variable; in this case, "LAST MSG "

#define CMD_LIST_LEN 3
cmd_t* cmd_list[CMD_LIST_LEN] = {
    &pay_eps_req,
    &read_flash,
    &actuate_motor
};

uint8_t handle_uart_cmd(const uint8_t* data, uint8_t len) {
    print("\n%s\n", __FUNCTION__);
    print("data = ");
    print_hex_bytes((uint8_t *) data, len);
    print("len = %u\n", len);
    for (uint8_t i = 0; i < len; i++) {
        print("%c", data[i]);
    }
    print("\n");

    if (data[len - 1] == '\n') {
        for (uint8_t i = 0; i < CMD_LIST_LEN; i++) {
            cmd_t* cmd = cmd_list[i];
            uint8_t cmd_len = strlen(cmd->cmd);

            if (len < cmd_len) continue;

            // FIXME: might be len - 1 below
            uint8_t match = 1;
            for (uint8_t i = 0; i < cmd_len; i++) {
                if (data[i] != (cmd->cmd)[i]) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                // WOW: just enqueue the command directly into the queue!

                print("%d\n", cmd);
                enqueue(&uart_cmd_queue, (uint8_t*)cmd);
                print("Enqueued command %s\n", cmd->cmd);
                // Downside, can't actually pass the data/len through;
                // this means we can't support variable sized commands
                break;
                // break if we match; the input can match at most 1 command
            }
        }

        return len;
    } else {
        return 0;
    }
}

void handle_pay_eps_req() {
    // Make a CAN req, add the appropriate response to the command
    // queue, create a callback which prints the output in a human
    // readable format.

    print("%s\n", __FUNCTION__);

    resume_mob(&pay_cmd_tx);
    while (!is_paused(&pay_cmd_tx));
    // TODO save and print all data, write to flash
}

void handle_read_flash() {
    print("%s\n", __FUNCTION__);
    // TODO
}

void handle_actuate_motor() {
    print("%s\n", __FUNCTION__);

    resume_mob(&pay_cmd_tx);
    while (!is_paused(&pay_cmd_tx));
}

int main(void) {
    init_uart();
    print("\n\nUART initialized\n");
    print("%d\n", sizeof(cmd_t));
    init_can();
    print("CAN initialized\n");

    init_rx_mob(&data_rx);
    print("Data RX MOb initialized\n");
    init_tx_mob(&pay_cmd_tx);
    print("PAY TX MOb initialized\n");
    init_tx_mob(&eps_cmd_tx);
    print("EPS TX MOb initialized\n");

    register_callback(handle_uart_cmd);
    print("Registered UART command handler\n");

    init_queue(&uart_cmd_queue);
    init_queue(&eps_tx_queue);
    init_queue(&pay_sci_tx_queue);
    init_queue(&pay_hk_tx_queue);
    init_queue(&pay_motor_tx_queue);

    print("Initialize UART command queue\n");

    print("Waiting for UART command...\n");
    while (1) {
        if (!is_empty(&uart_cmd_queue)) {
            // dequeue the latest UART command and execute it
            cmd_t cmd;
            print("Dequeueing\n");
            dequeue(&uart_cmd_queue, (uint8_t*)&cmd);
            print("Dequeued command\n");
            (cmd.fn)();
            // Now, callbacks are no longer executed in ISRs, so we
            // can actually resume/pause MObs inside them
        }

        _delay_ms(10);
    }

    return 0;
}
