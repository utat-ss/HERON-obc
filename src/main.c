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


// Available UART commands

void handle_pay_eps_req(const uint8_t*, uint8_t);
cmd_t pay_eps_req = {
    .cmd = "PAY EPS REQUEST\r\n",
    .fn = handle_pay_eps_req
};

void handle_read_flash(const uint8_t*, uint8_t);
cmd_t read_flash = {
    .cmd = "READ FLASH\r\n",
    .fn = handle_read_flash
};

void handle_actuate_motor(const uint8_t*, uint8_t);
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




uint8_t next_pay_hk_field_num = 0;
bool send_next_pay_hk_field_num = false;

uint8_t next_pay_sci_field_num = 0;
bool send_next_pay_sci_field_num = false;

uint8_t next_eps_hk_field_num = 0;
bool send_next_eps_hk_field_num = false;

bool send_pay_motor_actuate = false;





// Will be set to true if OBC is still executing a UART command
// (e.g. true if still waiting for a CAN response)
// EACH COMMAND IS RESPONSIBLE FOR SETTING THIS TO FALSE WHEN DONE
bool uart_cmd_busy = false;

uint8_t handle_uart_cmd(const uint8_t* data, uint8_t len) {
    print("%s\n", __FUNCTION__);
    print("data = ");
    print_hex_bytes((uint8_t *) data, len);
    print("len = %u\n", len);

    if (uart_cmd_busy) {
        print("UART CMD BUSY\n");
        return 0;
    }

    if (data[len - 1] == '\n') {
        for (uint8_t i = 0; i < CMD_LIST_LEN; i++) {
            cmd_t* cmd = cmd_list[i];
            uint8_t cmd_len = strlen(cmd->cmd);

            if (len < cmd_len) {
                continue;
            }

            // FIXME: might be len - 1 below
            uint8_t match = 1;
            for (uint8_t i = 0; i < cmd_len; i++) {
                if (data[i] != (cmd->cmd)[i]) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                uart_cmd_busy = true;
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

void handle_pay_eps_req(const uint8_t* data, uint8_t len) {
    // Make a CAN req, add the appropriate response to the command
    // queue, create a callback which prints the output in a human
    // readable format.

    print("%s\n", __FUNCTION__);

    next_pay_hk_field_num = 0;
    send_next_pay_hk_field_num = true;

    // TODO save and print all data, write to flash
}

void handle_read_flash(const uint8_t* data, uint8_t len) {
    print("%s\n", __FUNCTION__);

    // TODO

    uart_cmd_busy = false;
}

void handle_actuate_motor(const uint8_t* data, uint8_t len) {
    print("%s\n", __FUNCTION__);

    send_pay_motor_actuate = true;
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
    while (1) {
        if (send_next_pay_hk_field_num) {
            resume_mob(&pay_cmd_tx);
        }
        else if (send_next_pay_sci_field_num) {
            resume_mob(&pay_cmd_tx);
        }
        else if (send_next_eps_hk_field_num) {
            resume_mob(&eps_cmd_tx);
        }
        else if (send_pay_motor_actuate) {
            resume_mob(&pay_cmd_tx);
        }

        _delay_ms(10);
    }

    return 0;
}
