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
#include <queue/queue.h>
#include <uart/uart.h>
#include <spi/spi.h>

#include "../../src/commands.h"
#include "../../src/general.h"

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

// Special command just for this test to clear local data (set to all 0s)
void clear_local_data_fn(void);
cmd_t clear_local_data_cmd = {
    .fn = clear_local_data_fn
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
    },
    {
        .description = "Clear local data",
        .cmd = &clear_local_data_cmd
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
    for (uint8_t i = 0; i < CAN_EPS_HK_GET_COUNT; i++) {
        print("%.3lu ", eps_hk_data[i]);
    }
    print("\n");

    print("PAY HK:\n");
    for (uint8_t i = 0; i < CAN_PAY_HK_GET_COUNT; i++) {
        print("%.4lu ", pay_hk_data[i]);
    }
    print("\n");

    print("PAY SCI:\n");
    for (uint8_t i = 0; i < CAN_PAY_SCI_GET_COUNT; i++) {
        print("%.6lu ", pay_sci_data[i]);
    }
    print("\n");
}

void clear_local_data_fn(void) {
    for (uint8_t i = 0; i < CAN_EPS_HK_GET_COUNT; i++) {
        eps_hk_data[i] = 0;
    }
    print("Cleared local EPS_HK\n");

    for (uint8_t i = 0; i < CAN_PAY_HK_GET_COUNT; i++) {
        pay_hk_data[i] = 0;
    }
    print("Cleared local PAY_HK\n");

    for (uint8_t i = 0; i < CAN_PAY_SCI_GET_COUNT; i++) {
        pay_sci_data[i] = 0;
    }
    print("Cleared local PAY_SCI\n");
}


// Generates a random number in the range [0, 2^31)
// This is because rand() caps at RAND_MAX (0x7FFF)
uint32_t rand_uint32(void) {
    uint32_t msb = rand();
    uint32_t lsb = rand();
    uint32_t ret = (msb << 15) | lsb;
    return ret;
}

// Generates a random number in the range [start, end]
uint32_t rand_in_range(uint32_t start, uint32_t end) {
    uint32_t count = end - start + 1;
    uint32_t ret = start + (rand_uint32() % count);
    return ret;
}

// Generates a random number with the specified number of bits (all other bits will always be 0)
uint32_t rand_bits(uint8_t num_bits) {
    // Generate `num_bits` number of 1s
    uint32_t mask = (1UL << num_bits) - 1;
    uint32_t ret = rand_uint32() & mask;
    return ret;
}


// Implements _delay_ms() with a variable parameter
// (_delay_ms() only takes constants)
void delay_ms(uint16_t ms) {
    for (uint16_t i = 0; i < ms; i++) {
        _delay_ms(1);
    }
}

// Delays for a random number of ms in the range [1, max_ms]
void delay_random_ms(uint16_t max_ms) {
    delay_ms(rand_in_range(1, max_ms));
}

// Splits up 24 bit data and populates msg[3], msg[4], and msg[5] with it
void populate_msg_data(uint8_t* msg, uint32_t data) {
    msg[3] = (data >> 16) & 0xFF;
    msg[4] = (data >> 8) & 0xFF;
    msg[5] = data & 0xFF;
}

// Simulates sending an EPS TX message and getting a response back
void sim_eps_tx_msg(void) {
    if (queue_empty(&eps_tx_msg_queue)) {
        return;
    }

    // TX and RX defined from OBC's perspective
    uint8_t tx_msg[8];
    dequeue(&eps_tx_msg_queue, tx_msg);

    // Construct the message EPS would send back
    uint8_t rx_msg[8];
    rx_msg[0] = 0;
    rx_msg[1] = tx_msg[1];
    rx_msg[2] = tx_msg[2];

    // Can return early to not send a message back
    switch (tx_msg[1]) {
        case CAN_EPS_HK:
            if (0 <= tx_msg[2] && tx_msg[2] < CAN_EPS_HK_GET_COUNT) {
                // All fields are 12-bit ADC data
                populate_msg_data(rx_msg, rand_bits(12));
            } else {
                return;
            }
            break;
        default:
            return;
    }

    // Simulate waiting to receive the message
    delay_random_ms(100);
    print("Enqueued to data_rx_msg_queue\n");
    enqueue(&data_rx_msg_queue, rx_msg);
}

// Simulates sending an EPS TX message and getting a response back
void sim_pay_tx_msg(void) {
    if (queue_empty(&pay_tx_msg_queue)) {
        return;
    }

    // TX and RX defined from OBC's perspective
    uint8_t tx_msg[8];
    dequeue(&pay_tx_msg_queue, tx_msg);

    // Construct the message EPS would send back
    uint8_t rx_msg[8];
    rx_msg[0] = 0;
    rx_msg[1] = tx_msg[1];
    rx_msg[2] = tx_msg[2];

    // Can return early to not send a message back
    switch (tx_msg[1]) {
        case CAN_PAY_HK:
            if (tx_msg[2] == CAN_PAY_HK_TEMP) {
                populate_msg_data(rx_msg, rand_bits(16));
            } else if (tx_msg[2] == CAN_PAY_HK_HUM) {
                populate_msg_data(rx_msg, rand_bits(14));
            } else if (tx_msg[2] == CAN_PAY_HK_PRES) {
                populate_msg_data(rx_msg, rand_bits(24));
            } else {
                return;
            }
            break;
        case CAN_PAY_OPT:
            if (0 <= tx_msg[2] && tx_msg[2] < CAN_PAY_SCI_GET_COUNT) {
                // All fields are 24-bit ADC data
                populate_msg_data(rx_msg, rand_bits(24));
            } else {
                return;
            }
            break;
        case CAN_PAY_EXP:
            if (tx_msg[2] == CAN_PAY_EXP_POP) {
                // Don't need to populate anything
            } else {
                return;
            }
            break;
        default:
            return;
    }

    // Simulate waiting to receive the message
    delay_random_ms(100);
    print("Enqueued to data_rx_msg_queue\n");
    enqueue(&data_rx_msg_queue, rx_msg);
}




uint8_t uart_cb(const uint8_t* data, uint8_t len) {
    if (len == 0) {
        return 0;
    }

    // Print the typed character
    print("%c\n", data[0]);

    // Check for printing the help menu
    if (data[0] == 'h') {
        print_cmds();
    }

    // Check for a valid command number
    else if ('0' <= data[0] && data[0] < '0' + all_cmds_len) {
        // Enqueue the selected command
        uint8_t i = data[0] - '0';
        enqueue_cmd(&cmd_queue, all_cmds[i].cmd);
    }

    else {
        print("Invalid command\n");
    }

    // Processed 1 character
    return 1;
}


int main(void){
    init_obc_core();
    print("\n\n\nStarting commands test\n\n");
    print("Initialized OBC core\n\n");

    print("At any time, press h to show the command menu\n\n");
    print_cmds();
    set_uart_rx_cb(uart_cb);

    while (1) {
        // If we are using external CAN, physically send the messages
        if (use_ext_can) {
            send_next_eps_tx_msg();
            send_next_pay_tx_msg();
        }
        // If we are not using external CAN, simulate sending messages
        else {
            sim_eps_tx_msg();
            sim_pay_tx_msg();
        }

        // print("data_rx_msg_queue: head = %u, tail = %u\n", data_rx_msg_queue.head, data_rx_msg_queue.tail);
        // if (!queue_empty(&data_rx_msg_queue)) {
        //     uint8_t msg[8];
        //     peek_queue(&data_rx_msg_queue, msg);
        //     print("data_rx_msg_queue: peek = ");
        //     print_bytes(msg, 8);
        // }
        process_next_rx_msg();

        execute_next_cmd();
    }

    return 0;
}
