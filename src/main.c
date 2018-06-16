#include "main.h"

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




typedef void(*uart_cmd_fn_t)(const uint8_t*, uint8_t);

typedef struct {
    char* cmd;
    uart_cmd_fn_t fn;
} cmd_t;

void handle_pay_hk_req(const uint8_t*, uint8_t);

cmd_t pay_hk_req = {
    .cmd = "PAY HK REQUEST\r\n",
    .fn = handle_pay_hk_req
};

// To accept variable input, e.g. a command like "LAST MSG 4" which
// returns the last 4 CAN msgs, set the .cmd member to be the prefix
// of the command, without the variable; in this case, "LAST MSG "

#define CMD_LIST_LEN 1
cmd_t* cmd_list[CMD_LIST_LEN] = { &pay_hk_req };




uint8_t next_pay_hk_field_num = 0;
bool send_next_pay_hk_field_num = false;

uint8_t next_pay_sci_field_num = 0;
bool send_next_pay_sci_field_num = false;

uint8_t next_eps_hk_field_num = 0;
bool send_next_eps_hk_field_num = false;








uint8_t handle_uart_cmd(const uint8_t* data, uint8_t len) {
    if (data[len - 1] == '\n') {
        for (uint8_t i = 0; i < CMD_LIST_LEN; i++) {
            cmd_t* cmd = cmd_list[i];
            uint8_t len = strlen(cmd->cmd);
            uint8_t match = 1;

            // FIXME: might be len - 1 below
            for (uint8_t i = 0; i < len; i++) {
                if (data[i] != (cmd->cmd)[i]) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                (cmd->fn)(data, len);
                break;
                // break if we match; the input can match at most 1 command
            }
        }

        return len;
    } else {
        return 0;
    }
}

void handle_pay_hk_req(const uint8_t* data, uint8_t len) {
    // Make a CAN req, add the appropriate response to the command
    // queue, create a callback which prints the output in a human
    // readable format.
}

int main(void) {
    init_uart();
    print("UART initialized\n");
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

    print("Waiting for UART command...");
    while (1) {};

    return 0;
}
