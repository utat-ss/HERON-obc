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

#include <can/data_protocol.h>
#include <conversions/conversions.h>
#include <queue/queue.h>
#include <uart/uart.h>
#include <spi/spi.h>

#include "../../src/commands.h"
#include "../../src/general.h"

// Set to true to simulate communicating with EPS (without EPS actually being
// connected over CAN)
bool sim_eps = false;
// Set to true to simulate communicating with PAY (without PAY actually being
// connected over CAN)
bool sim_pay = false;
// Set to true to simulate using the transceiver
bool sim_trans = false;
// Set to true to simulate getting UART from the transceiver
bool sim_trans_uart = false;

// Set to true to print TX and RX CAN messages
bool print_can_msgs = false;
// Set to true to print commands and arguments
bool print_cmds = false;
// Set to true to print transceiver messages
bool print_trans_msgs = false;


// Normal command with a string description to print on UART
typedef struct {
    char* description;
    cmd_t* cmd;
    uint32_t arg1;
    uint32_t arg2;
    // true to bypass enqueuing a transceiver message and just enqueue directly
    // in command queue
    bool bypass_trans;
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

void read_all_mem_blocks_to_local_fn(void);
cmd_t read_all_mem_blocks_to_local_cmd = {
    .fn = read_all_mem_blocks_to_local_fn
};


// All possible commands
uart_cmd_t all_cmds[] = {
    {
        .description = "Ping OBC",
        .cmd = &ping_cmd,
        .arg1 = 0,
        .arg2 = 0,
        .bypass_trans = false
    },
    {
        .description = "Get restart and uptime",
        .cmd = &get_restart_uptime_cmd,
        .arg1 = 0,
        .arg2 = 0,
        .bypass_trans = false
    },
    {
        .description = "Print all local data blocks",
        .cmd = &print_local_data_cmd,
        .arg1 = 0,
        .arg2 = 0,
        .bypass_trans = true
    },
    {
        .description = "Clear local data",
        .cmd = &clear_local_data_cmd,
        .arg1 = 0,
        .arg2 = 0,
        .bypass_trans = true
    },
    {
        .description = "Read all mem blocks to local data",
        .cmd = &read_all_mem_blocks_to_local_cmd,
        .arg1 = 0,
        .arg2 = 0,
        .bypass_trans = true
    },
    {
        .description = "Request EPS HK",
        .cmd = &collect_block_cmd,
        .arg1 = CMD_BLOCK_EPS_HK,
        .arg2 = 0,
        .bypass_trans = false
    },
    {
        .description = "Request PAY HK",
        .cmd = &collect_block_cmd,
        .arg1 = CMD_BLOCK_PAY_HK,
        .arg2 = 0,
        .bypass_trans = false
    },
    {
        .description = "Request PAY OPT",
        .cmd = &collect_block_cmd,
        .arg1 = CMD_BLOCK_PAY_OPT,
        .arg2 = 0,
        .bypass_trans = false
    },
    {
        .description = "Actuate motors up",
        .cmd = &actuate_pay_motors_cmd,
        // TODO - constants
        .arg1 = 1,
        .arg2 = 0,
        .bypass_trans = false
    },
    {
        .description = "Actuate motors down",
        .cmd = &actuate_pay_motors_cmd,
        // TODO - constants
        .arg1 = 2,
        .arg2 = 0,
        .bypass_trans = false
    }
};

// Length of array
const uint8_t all_cmds_len = sizeof(all_cmds) / sizeof(all_cmds[0]);


void print_uart_cmds(void) {
    for (uint8_t i = 0; i < all_cmds_len; i++) {
        print("%u: %s\n", i, all_cmds[i].description);
    }
}

void print_voltage(uint16_t raw_data) {
    print(" 0x%.3X = %f V\n", raw_data, adc_raw_data_to_eps_vol(raw_data));
}

void print_current(uint16_t raw_data) {
    print(" 0x%.3X = %f A\n", raw_data, adc_raw_data_to_eps_cur(raw_data));
}

void print_therm_temp(uint16_t raw_data) {
    print(" 0x%.3X = %f C\n", raw_data,
        therm_res_to_temp(
        therm_vol_to_res(
        adc_raw_data_to_raw_vol(raw_data))));
}

void print_imu_data(uint16_t raw_data) {
    print(" 0x%.4X\n", raw_data);
}

void print_header(mem_header_t header) {
    print("block_num = %lu, error = %u, ", header.block_num, header.error);
    print("date = %02u:%02u:%02u, time = %02u:%02u:%02u\n",
        header.date.yy, header.date.mm, header.date.dd,
        header.time.hh, header.time.mm, header.time.ss);
}

void print_local_data_fn(void) {
    print("\nEPS HK:\n");

    // print("Header: ");
    print_header(eps_hk_header);

    // print("Fields: ");
    for (uint8_t i = 0; i < CAN_EPS_HK_FIELD_COUNT; i++) {
        print("0x%.6lX ", eps_hk_fields[i]);
    }
    print("\n");

    print("BB Vol:");
    print_voltage(eps_hk_fields[CAN_EPS_HK_BB_VOL]);
    print("BB Cur:");
    print_current(eps_hk_fields[CAN_EPS_HK_BB_CUR]);
    print("BT Vol:");
    print_voltage(eps_hk_fields[CAN_EPS_HK_BT_VOL]);
    print("BT Cur:");
    print_current(eps_hk_fields[CAN_EPS_HK_BT_CUR]);
    print("+X Cur:");
    print_current(eps_hk_fields[CAN_EPS_HK_PX_CUR]);
    print("-X Cur:");
    print_current(eps_hk_fields[CAN_EPS_HK_NX_CUR]);
    print("+Y Cur:");
    print_current(eps_hk_fields[CAN_EPS_HK_PY_CUR]);
    print("-Y Cur:");
    print_current(eps_hk_fields[CAN_EPS_HK_NY_CUR]);
    print("Bat Vol:");
    print_voltage(eps_hk_fields[CAN_EPS_HK_BAT_VOL]);
    print("Bat Cur:");
    print_current(eps_hk_fields[CAN_EPS_HK_BAT_CUR]);
    print("Bat Temp 1:");
    print_therm_temp(eps_hk_fields[CAN_EPS_HK_BAT_TEMP1]);
    print("Bat Temp 2:");
    print_therm_temp(eps_hk_fields[CAN_EPS_HK_BAT_TEMP2]);
    // print("Acc X:");
    // print_imu_data(eps_hk_fields[CAN_EPS_HK_IMU_ACC_X]);
    // print("Acc Y:");
    // print_imu_data(eps_hk_fields[CAN_EPS_HK_IMU_ACC_Y]);
    // print("Acc Z:");
    // print_imu_data(eps_hk_fields[CAN_EPS_HK_IMU_ACC_Z]);
    // print("Gyr X:");
    // print_imu_data(eps_hk_fields[CAN_EPS_HK_IMU_GYR_X]);
    // print("Gyr Y:");
    // print_imu_data(eps_hk_fields[CAN_EPS_HK_IMU_GYR_Y]);
    // print("Gyr Z:");
    // print_imu_data(eps_hk_fields[CAN_EPS_HK_IMU_GYR_Z]);
    // print("Mag X:");
    // print_imu_data(eps_hk_fields[CAN_EPS_HK_IMU_MAG_X]);
    // print("Mag Y:");
    // print_imu_data(eps_hk_fields[CAN_EPS_HK_IMU_MAG_Y]);
    // print("Mag Z:");
    // print_imu_data(eps_hk_fields[CAN_EPS_HK_IMU_MAG_Z]);
    print("Bat Temp Setpt 1:");
    print_therm_temp(eps_hk_fields[CAN_EPS_HK_HEAT_SP1]);
    print("Bat Temp Setpt 2:");
    print_therm_temp(eps_hk_fields[CAN_EPS_HK_HEAT_SP2]);

    print("\nPAY HK:\n");

    // print("Header: ");
    print_header(pay_hk_header);

    // print("Fields: ");
    for (uint8_t i = 0; i < CAN_PAY_HK_FIELD_COUNT; i++) {
        print("0x%.6lX ", pay_hk_fields[i]);
    }
    print("\n");

    print("Temp: 0x%.6lX = %.3f C\n", pay_hk_fields[CAN_PAY_HK_TEMP],
        temp_raw_data_to_temperature(pay_hk_fields[CAN_PAY_HK_TEMP]));
    print("Hum: 0x%.6lX = %.3f %%RH\n", pay_hk_fields[CAN_PAY_HK_HUM],
        hum_raw_data_to_humidity(pay_hk_fields[CAN_PAY_HK_HUM]));
    print("Pres: 0x%.6lX = %.3f kPa\n", pay_hk_fields[CAN_PAY_HK_PRES],
        pres_raw_data_to_pressure(pay_hk_fields[CAN_PAY_HK_PRES]));
    for (uint8_t i = 0; i < 10; i++) {
        print("Temp %u:", i);
        print_therm_temp(pay_hk_fields[CAN_PAY_HK_THERM0 + i]);
    }
    print("Temp Setpt 1:");
    print_therm_temp(pay_hk_fields[CAN_PAY_HK_HEAT_SP1]);
    print("Temp Setpt 2:");
    print_therm_temp(pay_hk_fields[CAN_PAY_HK_HEAT_SP2]);

    print("\nPAY OPT:\n");

    // print("Header: ");
    print_header(pay_opt_header);

    // print("Fields: ");
    for (uint8_t i = 0; i < CAN_PAY_OPT_FIELD_COUNT; i++) {
        print("0x%.6lX ", pay_opt_fields[i]);
    }
    print("\n");

    for (uint8_t i = 0; i < CAN_PAY_OPT_FIELD_COUNT; i++) {
        print("Well %u: 0x%.6lX = %.6f %%\n", i, pay_opt_fields[i],
            ((double) pay_opt_fields[i]) / 0xFFFFFF * 100.0);
    }

    finish_current_cmd(true);
}




void print_next_eps_tx_msg(void) {
    if (!print_can_msgs) {
        return;
    }

    uint8_t tx_msg[8] = { 0x00 };
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&eps_tx_msg_queue)) {
            return;
        }
        peek_queue(&eps_tx_msg_queue, tx_msg);
    }

    print("CAN TX (EPS): ");
    print_bytes(tx_msg, 8);
}

void print_next_pay_tx_msg(void) {
    if (!print_can_msgs) {
        return;
    }

    uint8_t tx_msg[8] = { 0x00 };
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&pay_tx_msg_queue)) {
            return;
        }
        peek_queue(&pay_tx_msg_queue, tx_msg);
    }

    print("CAN TX (PAY): ");
    print_bytes(tx_msg, 8);
}

void print_next_rx_msg(void) {
    if (!print_can_msgs) {
        return;
    }

    uint8_t rx_msg[8] = { 0x00 };
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&data_rx_msg_queue)) {
            return;
        }
        peek_queue(&data_rx_msg_queue, rx_msg);
    }

    // Extra spaces to align with CAN TX messages
    print("CAN RX:       ");
    print_bytes(rx_msg, 8);
}

void print_next_cmd(void) {
    if (!print_cmds) {
        return;
    }

    uint8_t cmd[8] = { 0x00 };
    uint8_t args[8] = { 0x00 };
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&cmd_queue)) {
            return;
        }
        if (queue_empty(&cmd_args_queue)) {
            return;
        }
        peek_queue(&cmd_queue, cmd);
        peek_queue(&cmd_args_queue, args);
    }

    print("Cmd:  ");
    print_bytes(cmd, 8);
    print("Args: ");
    print_bytes(args, 8);
}


void print_next_trans_encoded_tx_msg(void) {
    if (!print_trans_msgs) {
        return;
    }
    if (!trans_encoded_tx_msg_avail) {
        return;
    }

    print("Trans TX (Encoded): %u bytes: ", trans_encoded_tx_msg_len);
    print_bytes((uint8_t*) trans_encoded_tx_msg, trans_encoded_tx_msg_len);
}

void print_next_trans_decoded_tx_msg(void) {
    if (!print_trans_msgs) {
        return;
    }
    if (!trans_decoded_tx_msg_avail) {
        return;
    }

    print("Trans TX (Decoded): %u bytes: ", trans_decoded_tx_msg_len);
    print_bytes((uint8_t*) trans_decoded_tx_msg, trans_decoded_tx_msg_len);
}

void print_next_trans_decoded_rx_msg(void) {
    if (!print_trans_msgs) {
        return;
    }
    if (!trans_decoded_rx_msg_avail) {
        return;
    }

    print("Trans RX (Decoded): %u bytes: ", trans_decoded_rx_msg_len);
    print_bytes((uint8_t*) trans_decoded_rx_msg, trans_decoded_rx_msg_len);
}

void print_next_trans_encoded_rx_msg(void) {
    if (!print_trans_msgs) {
        return;
    }
    if (!trans_encoded_rx_msg_avail) {
        return;
    }

    print("Trans RX (Encoded): %u bytes: ", trans_encoded_rx_msg_len);
    print_bytes((uint8_t*) trans_encoded_rx_msg, trans_encoded_rx_msg_len);
}






void clear_local_data_fn(void) {
    clear_mem_header(&eps_hk_header);
    for (uint8_t i = 0; i < CAN_EPS_HK_FIELD_COUNT; i++) {
        eps_hk_fields[i] = 0;
    }
    clear_mem_header(&pay_hk_header);
    for (uint8_t i = 0; i < CAN_PAY_HK_FIELD_COUNT; i++) {
        pay_hk_fields[i] = 0;
    }
    clear_mem_header(&pay_opt_header);
    for (uint8_t i = 0; i < CAN_PAY_OPT_FIELD_COUNT; i++) {
        pay_opt_fields[i] = 0;
    }

    print("Cleared local data\n");

    finish_current_cmd(true);
}

void read_all_mem_blocks_to_local_fn(void) {
    // TODO
    enqueue_cmd(&read_mem_block_cmd, CMD_BLOCK_EPS_HK,
        eps_hk_mem_section.curr_block - 1);
    enqueue_cmd(&read_mem_block_cmd, CMD_BLOCK_PAY_HK,
        pay_hk_mem_section.curr_block - 1);
    enqueue_cmd(&read_mem_block_cmd, CMD_BLOCK_PAY_OPT,
        pay_opt_mem_section.curr_block - 1);

    finish_current_cmd(true);
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
void sim_send_next_eps_tx_msg(void) {
    if (queue_empty(&eps_tx_msg_queue)) {
        return;
    }

    // TX and RX defined from OBC's perspective
    uint8_t tx_msg[8] = {0x00};
    dequeue(&eps_tx_msg_queue, tx_msg);

    // Construct the message EPS would send back
    uint8_t rx_msg[8] = {0x00};
    rx_msg[0] = 0;
    rx_msg[1] = tx_msg[1];
    rx_msg[2] = tx_msg[2];

    uint8_t msg_type = tx_msg[1];
    uint8_t field_num = tx_msg[2];

    // Can return early to not send a message back
    switch (msg_type) {
        case CAN_EPS_HK:
            if (0 <= field_num && field_num < CAN_EPS_HK_FIELD_COUNT) {
                // All fields are 12-bit ADC data
                populate_msg_data(rx_msg, rand_bits(12));
            } else {
                return;
            }
            break;

        case CAN_EPS_CTRL:
            if ((field_num == CAN_EPS_CTRL_HEAT_SP1) ||
                (field_num == CAN_EPS_CTRL_HEAT_SP2)) {
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
    // print("Enqueued to data_rx_msg_queue\n");
    enqueue(&data_rx_msg_queue, rx_msg);
}

// Simulates sending a PAY TX message and getting a response back
void sim_send_next_pay_tx_msg(void) {
    if (queue_empty(&pay_tx_msg_queue)) {
        return;
    }

    // TX and RX defined from OBC's perspective
    uint8_t tx_msg[8] = {0x00};
    dequeue(&pay_tx_msg_queue, tx_msg);

    // Construct the message EPS would send back
    uint8_t rx_msg[8] = {0x00};
    rx_msg[0] = 0;
    rx_msg[1] = tx_msg[1];
    rx_msg[2] = tx_msg[2];

    uint8_t msg_type = tx_msg[1];
    uint8_t field_num = tx_msg[2];

    // Can return early to not send a message back
    switch (msg_type) {
        case CAN_PAY_HK:
            if (field_num == CAN_PAY_HK_TEMP) {
                populate_msg_data(rx_msg, rand_bits(16));
            } else if (field_num == CAN_PAY_HK_HUM) {
                populate_msg_data(rx_msg, rand_bits(14));
            } else if (field_num == CAN_PAY_HK_PRES) {
                populate_msg_data(rx_msg, rand_bits(24));
            } else if (CAN_PAY_HK_THERM0 <= field_num &&
                    field_num <= CAN_PAY_HK_THERM9) {
                populate_msg_data(rx_msg, rand_bits(12));
            } else if (field_num == CAN_PAY_HK_HEAT_SP1 ||
                    field_num == CAN_PAY_HK_HEAT_SP2) {
                populate_msg_data(rx_msg, rand_bits(12));
            } else if (field_num == CAN_PAY_HK_PROX_LEFT ||
                    field_num == CAN_PAY_HK_PROX_RIGHT) {
                populate_msg_data(rx_msg, rand_bits(12));
            } else {
                return;
            }
            break;

        case CAN_PAY_OPT:
            if (0 <= field_num && field_num < CAN_PAY_OPT_FIELD_COUNT) {
                // All fields are 24-bit ADC data
                populate_msg_data(rx_msg, rand_bits(24));
            } else {
                return;
            }
            break;

        case CAN_PAY_CTRL:
            if ((field_num == CAN_PAY_CTRL_HEAT_SP1) ||
                (field_num == CAN_PAY_CTRL_HEAT_SP2) ||
                (field_num == CAN_PAY_CTRL_ACT_UP) ||
                (field_num == CAN_PAY_CTRL_ACT_DOWN)) {
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
    // print("Enqueued to data_rx_msg_queue\n");
    enqueue(&data_rx_msg_queue, rx_msg);
}




uint8_t uart_cb(const uint8_t* data, uint8_t len) {
    if (len == 0) {
        return 0;
    }

    // Print the typed character
    print("%c\n", data[0]);
    print("0x%.2X\n", data[0]);

    // Check for printing the help menu
    if (data[0] == 'h') {
        print_uart_cmds();
    }

    // Check for a valid command number
    else if ('0' <= data[0] && data[0] < '0' + all_cmds_len) {
        // Enqueue the selected command
        uint8_t i = data[0] - '0';

        if (all_cmds[i].bypass_trans) {
            enqueue_cmd(all_cmds[i].cmd, all_cmds[i].arg1, all_cmds[i].arg2);
        } else {
            trans_encoded_rx_msg[0] = 0x00;
            trans_encoded_rx_msg[1] = 9;
            trans_encoded_rx_msg[2] = trans_cmd_to_msg_type(all_cmds[i].cmd);
            trans_encoded_rx_msg[3] = (all_cmds[i].arg1 >> 24) & 0xFF;
            trans_encoded_rx_msg[4] = (all_cmds[i].arg1 >> 16) & 0xFF;
            trans_encoded_rx_msg[5] = (all_cmds[i].arg1 >> 8) & 0xFF;
            trans_encoded_rx_msg[6] = all_cmds[i].arg1 & 0xFF;
            trans_encoded_rx_msg[7] = (all_cmds[i].arg2 >> 24) & 0xFF;
            trans_encoded_rx_msg[8] = (all_cmds[i].arg2 >> 16) & 0xFF;
            trans_encoded_rx_msg[9] = (all_cmds[i].arg2 >> 8) & 0xFF;
            trans_encoded_rx_msg[10] = all_cmds[i].arg2 & 0xFF;

            trans_encoded_rx_msg_len = 11;
            trans_encoded_rx_msg_avail = true;
        }
    }

    else {
        print("Invalid cmd\n");
    }

    // Processed 1 character
    return 1;
}


int main(void){
    WDT_OFF();
    WDT_ENABLE_SYS_RESET(WDTO_8S);

    init_obc_core();

    print("\n\n\nStarting commands test\n\n");

    sim_local_actions = false;
    sim_eps = false;
    sim_pay = false;
    sim_trans = true;
    sim_trans_uart = false;
    print_can_msgs = true;
    print_cmds = true;
    print_trans_msgs = true;

    print("sim_local_actions = %u\n", sim_local_actions);
    print("sim_eps = %u\n", sim_eps);
    print("sim_pay = %u\n", sim_pay);
    print("sim_trans = %u\n", sim_trans);
    print("sim_trans_uart = %u\n", sim_trans_uart);
    print("print_can_msgs = %u\n", print_can_msgs);
    print("print_cmds = %u\n", print_cmds);
    print("print_trans_msgs = %u\n", print_trans_msgs);
    print("\n");

    print("Mem blocks: eps_hk = %lu, pay_hk = %lu, pay_opt = %lu\n",
        eps_hk_mem_section.curr_block,
        pay_hk_mem_section.curr_block,
        pay_opt_mem_section.curr_block);
    print("\n");

    if (sim_trans) {
        if (sim_trans_uart) {
            print("Overwriting UART RX callback\n");
            set_uart_rx_cb(uart_cb);
            // print("Press h to list commands\n\n");
            print_uart_cmds();
        } else {
            print("Init trans UART\n");
            init_trans_uart();
        }
    } else {
        print("Init OBC comms\n");
        init_obc_comms();
    }
    print("\n");

    while (1) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);

        // EPS TX
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_eps_tx_msg();
            // Either simulate EPS over CAN or actually send the CAN message
            if (sim_eps) {
                sim_send_next_eps_tx_msg();
            }  else {
                send_next_eps_tx_msg();
            }
        }

        // PAY TX
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_pay_tx_msg();
            // Either simulate PAY over CAN or actually send the CAN message
            if (sim_pay) {
                sim_send_next_pay_tx_msg();
            }  else {
                send_next_pay_tx_msg();
            }
        }

        // CAN RX
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_rx_msg();
            process_next_rx_msg();
        }

        // Command
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_cmd();
            execute_next_cmd();
        }

        // Trans TX (encoded)
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_trans_encoded_tx_msg();
            send_trans_encoded_tx_msg();
        }

        // Trans TX (decoded)
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_trans_decoded_tx_msg();
            encode_trans_tx_msg();
        }

        // Trans RX (decoded)
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_trans_decoded_rx_msg();
            handle_trans_decoded_rx_msg();
        }

        // Trans RX (encoded)
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_trans_encoded_rx_msg();
            decode_trans_rx_msg();
        }
    }

    return 0;
}