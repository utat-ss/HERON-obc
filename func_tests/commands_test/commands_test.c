/*
This is the highest-level (most comprehensive) test in the OBC repository.

It simulates the entire operation of OBC, except for simulating the transceiver
by adding commands to the command queue directly.

It provides a UART interface to select a command to enqueue, then executes it.

The `use_ext_can` variable controls whether to use actual CAN or simulate it.
If set to true, it assumes both the EPS and PAY PCBs are connected over CAN.
If set to false, it simulate both the EPS and PAY PCBs responding to CAN messages.
*/

#include <stdbool.h>

#include <adc/eps.h>
#include <can/data_protocol.h>
#include <conversions/conversions.h>
#include <uart/uart.h>
#include <spi/spi.h>

#include "../../src/commands.h"
#include "../../src/obc.h"

// Set to true to use an actual CAN connection
const bool use_ext_can = false;


// Normal command with a string description to print on UART
typedef struct {
    char* description;
    cmd_t* cmd;
} uart_cmd_t;


// Special command just for this test to print local data
void print_local_data_fn(void);
cmd_t print_local_data_cmd = {
    .fn = print_local_data_fn
};


// All possible commands
uart_cmd_t all_cmds[] = {
    {
        .description = "Request EPS HK",
        .cmd = &req_eps_hk_cmd
    },
    {
        .description = "Request PAY HK",
        .cmd = &req_pay_hk_cmd
    },
    {
        .description = "Request PAY SCI",
        .cmd = &req_pay_sci_cmd
    },
    {
        .description = "Actuate motor",
        .cmd = &actuate_motor_cmd
    },
    {
        .description = "Write local data to flash",
        .cmd = &write_flash_cmd
    },
    {
        .description = "Read flash to local data",
        .cmd = &read_flash_cmd
    },
    {
        .description = "Print local data",
        .cmd = &print_local_data_cmd
    }
};
// Length of array
const uint8_t all_cmds_len = sizeof(all_cmds) / sizeof(all_cmds[0]);


void print_cmds(void) {
    for (uint8_t i = 0; i < all_cmds_len; i++) {
        print("%u: %s\n", i, all_cmds[i].description);
    }
}


// TODO - convert and show units
void print_local_data_fn(void) {
    print("EPS HK:\n");
    for (uint8_t i = 0; i < CAN_EPS_HK_FIELD_COUNT; i++) {
        print("%lu ", eps_hk_data[i]);
    }
    print("\n\n");

    print("PAY HK:\n");
    for (uint8_t i = 0; i < CAN_PAY_HK_FIELD_COUNT; i++) {
        print("%lu ", pay_hk_data[i]);
    }
    print("\n\n");

    print("PAY SCI:\n");
    for (uint8_t i = 0; i < CAN_PAY_SCI_FIELD_COUNT; i++) {
        print("%lu ", pay_sci_data[i]);
    }
    print("\n\n");
}




uint8_t uart_cb(const uint8_t* data, uint8_t len) {
    if (len == 0) {
        print("No UART\n");
        return 0;
    }

    if (data[0] == 'h') {
        print_cmds();
        return 1;
    }

    // Check for a valid command number
    if (!('0' <= data[0] && data[0] < '0' + all_cmds_len)) {
        print("Invalid command\n");
        return 0;
    }

    // Enqueue the selected command
    uint8_t i = data[0] - '0';
    enqueue_cmd(&cmd_queue, all_cmds[i].cmd);
    return 1;
}


int main(void){
    init_obc_core();
    print("At any time, press h to show the command menu\n");
    print_cmds();
    register_callback(uart_cb);

    while (1) {}

    return 0;
}
