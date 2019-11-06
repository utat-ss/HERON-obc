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

bool reset_comms_delay_eeprom = false;
bool skip_comms_delay = false;
bool skip_deploy_antenna = false;

bool disable_hb = false;

// Set to true to print TX and RX CAN messages
bool print_can_msgs = false;
// Set to true to print commands and arguments
bool print_cmds = false;
// Set to true to print transceiver messages
bool print_trans_msgs = false;
// Set to true to print ACKs
bool print_trans_tx_acks = false;


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
        .cmd = &ping_obc_cmd,
        .arg1 = 0,
        .arg2 = 0,
        .bypass_trans = false
    },
    {
        .description = "Get restart and uptime",
        .cmd = &read_rec_status_info_cmd,
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
        .cmd = &col_data_block_cmd,
        .arg1 = CMD_EPS_HK,
        .arg2 = 0,
        .bypass_trans = false
    },
    {
        .description = "Request PAY HK",
        .cmd = &col_data_block_cmd,
        .arg1 = CMD_PAY_HK,
        .arg2 = 0,
        .bypass_trans = false
    },
    {
        .description = "Request PAY OPT",
        .cmd = &col_data_block_cmd,
        .arg1 = CMD_PAY_OPT,
        .arg2 = 0,
        .bypass_trans = false
    },
    {
        .description = "Actuate motors up",
        .cmd = &act_pay_motors_cmd,
        .arg1 = CAN_PAY_CTRL_ACT_UP,
        .arg2 = 0,
        .bypass_trans = false
    },
    {
        .description = "Actuate motors down",
        .cmd = &act_pay_motors_cmd,
        .arg1 = CAN_PAY_CTRL_ACT_DOWN,
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
    print(" 0x%.3X = %f V\n", raw_data, adc_raw_to_circ_vol(raw_data, 1e4, 1e4));
}

void print_current(uint16_t raw_data) {
    print(" 0x%.3X = %f A\n", raw_data, adc_raw_to_circ_cur(raw_data, 0.008, 0.0));
}

void print_therm_temp(uint16_t raw_data) {
    print(" 0x%.3X = %f C\n", raw_data, adc_raw_to_therm_temp(raw_data));
}

void print_gyro_data(uint16_t raw_data) {
    print(" 0x%.4X = %.3f rad/s\n", raw_data, imu_raw_data_to_gyro(raw_data));
}

void print_header(mem_header_t header) {
    print("block_num = %lu, status = %u, ", header.block_num, header.status);
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

    print("Bat Temp 1:");
    print_therm_temp(eps_hk_fields[CAN_EPS_HK_BAT_TEMP1]);
    print("Bat Temp 2:");
    print_therm_temp(eps_hk_fields[CAN_EPS_HK_BAT_TEMP2]);
    print("Bat Vol:");
    print_voltage(eps_hk_fields[CAN_EPS_HK_BAT_VOL]);
    print("Bat Cur:");
    print_current(eps_hk_fields[CAN_EPS_HK_BAT_CUR]);
    print("Gyro (uncal) X:");
    print_gyro_data(eps_hk_fields[CAN_EPS_HK_GYR_UNCAL_X]);
    print("Gyro (uncal) Y:");
    print_gyro_data(eps_hk_fields[CAN_EPS_HK_GYR_UNCAL_Y]);
    print("Gyro (uncal) Z:");
    print_gyro_data(eps_hk_fields[CAN_EPS_HK_GYR_UNCAL_Z]);
    print("Gyro (cal) X:");
    print_gyro_data(eps_hk_fields[CAN_EPS_HK_GYR_CAL_X]);
    print("Gyro (cal) Y:");
    print_gyro_data(eps_hk_fields[CAN_EPS_HK_GYR_CAL_Y]);
    print("Gyro (cal) Z:");
    print_gyro_data(eps_hk_fields[CAN_EPS_HK_GYR_CAL_Z]);
    

    print("\nPAY HK:\n");

    // print("Header: ");
    print_header(pay_hk_header);

    // print("Fields: ");
    for (uint8_t i = 0; i < CAN_PAY_HK_FIELD_COUNT; i++) {
        print("0x%.6lX ", pay_hk_fields[i]);
    }
    print("\n");

    print("Hum: 0x%.6lX = %.3f %%RH\n", pay_hk_fields[CAN_PAY_HK_HUM],
        hum_raw_data_to_humidity(pay_hk_fields[CAN_PAY_HK_HUM]));
    print("Pres: 0x%.6lX = %.3f kPa\n", pay_hk_fields[CAN_PAY_HK_PRES],
        pres_raw_data_to_pressure(pay_hk_fields[CAN_PAY_HK_PRES]));


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

    finish_current_cmd(CMD_STATUS_OK);
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

void print_next_trans_tx_ack(void) {
    if (!print_trans_tx_acks) {
        return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!trans_tx_ack_avail) {
            return;
        }
        print("ACK: op = 0x%.2x, arg1 = 0x%.8lx, arg2 = 0x%.8lx, stat = 0x%.2x\n", trans_tx_ack_opcode, trans_tx_ack_arg1, trans_tx_ack_arg2, trans_tx_ack_status);
    }
}

void print_next_cmd(void) {
    if (!print_cmds) {
        return;
    }
    // Only print if we are open to start a new command, or else it will spam
    // print this in every main loop iteration until the current command is done
    if (current_cmd != &nop_cmd) {
        return;
    }

    uint8_t cmd[8] = { 0x00 };
    uint8_t args[8] = { 0x00 };
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (queue_empty(&cmd_opcode_queue)) {
            return;
        }
        if (queue_empty(&cmd_args_queue)) {
            return;
        }
        peek_queue(&cmd_opcode_queue, cmd);
        peek_queue(&cmd_args_queue, args);
    }

    print("Cmd:  ");
    print_bytes(cmd, 8);
    print("Args: ");
    print_bytes(args, 8);
}


void print_next_trans_tx_enc_msg(void) {
    if (!print_trans_msgs) {
        return;
    }
    if (!trans_tx_enc_avail) {
        return;
    }

    print("Trans TX (Encoded): %u bytes: ", trans_tx_enc_len);
    print_bytes((uint8_t*) trans_tx_enc_msg, trans_tx_enc_len);
}

void print_next_trans_tx_dec_msg(void) {
    if (!print_trans_msgs) {
        return;
    }
    if (!trans_tx_dec_avail) {
        return;
    }

    print("Trans TX (Decoded): %u bytes: ", trans_tx_dec_len);
    print_bytes((uint8_t*) trans_tx_dec_msg, trans_tx_dec_len);
}

void print_next_trans_rx_dec_msg(void) {
    if (!print_trans_msgs) {
        return;
    }
    if (!trans_rx_dec_avail) {
        return;
    }

    print("Trans RX (Decoded): %u bytes: ", trans_rx_dec_len);
    print_bytes((uint8_t*) trans_rx_dec_msg, trans_rx_dec_len);
}

void print_next_trans_rx_enc_msg(void) {
    if (!print_trans_msgs) {
        return;
    }
    if (!trans_rx_enc_avail) {
        return;
    }

    print("\n");
    print("Trans RX (Encoded): %u bytes: ", trans_rx_enc_len);
    print_bytes((uint8_t*) trans_rx_enc_msg, trans_rx_enc_len);
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

    finish_current_cmd(CMD_STATUS_OK);
}

void read_all_mem_blocks_to_local_fn(void) {
    // TODO
    enqueue_cmd(&read_data_block_cmd, CMD_EPS_HK,
        eps_hk_mem_section.curr_block - 1);
    enqueue_cmd(&read_data_block_cmd, CMD_PAY_HK,
        pay_hk_mem_section.curr_block - 1);
    enqueue_cmd(&read_data_block_cmd, CMD_PAY_OPT,
        pay_opt_mem_section.curr_block - 1);

    finish_current_cmd(CMD_STATUS_OK);
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

// Splits up 32 bit data and populates msg[4], msg[5], msg[6], and msg[7] with it
void populate_msg_data(uint8_t* msg, uint32_t data) {
    msg[4] = (data >> 24) & 0xFF;
    msg[5] = (data >> 16) & 0xFF;
    msg[6] = (data >> 8) & 0xFF;
    msg[7] = data & 0xFF;
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
    rx_msg[0] = 0x00;
    rx_msg[1] = 0x00;
    rx_msg[2] = tx_msg[2];
    rx_msg[3] = tx_msg[3];

    uint8_t msg_type = tx_msg[2];
    uint8_t field_num = tx_msg[3];

    // Can return early to not send a message back
    switch (msg_type) {
        case CAN_EPS_HK:
            if (CAN_EPS_HK_GYR_UNCAL_X <= field_num && field_num <= CAN_EPS_HK_GYR_CAL_Z) {
                // 16-bit data - IMU gyro
                populate_msg_data(rx_msg, rand_bits(16));
            } else if (field_num < CAN_EPS_HK_FIELD_COUNT) {
                // TODO
                populate_msg_data(rx_msg, rand_bits(32));
            } else {
                return;
            }
            break;

        case CAN_EPS_CTRL:
            if (field_num == CAN_EPS_CTRL_PING) {
                // Nothing
            } else if (field_num == CAN_EPS_CTRL_RESET) {
                // Don't send a message back for reset
                return;
            } else if (field_num == CAN_EPS_CTRL_READ_EEPROM) {
                populate_msg_data(rx_msg, rand_bits(32));
            } else if (field_num == CAN_EPS_CTRL_ERASE_EEPROM) {
                // Nothing
            } else if (field_num < CAN_EPS_CTRL_FIELD_COUNT) {
                // TODO
                populate_msg_data(rx_msg, rand_bits(32));
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
    rx_msg[0] = 0x00;
    rx_msg[1] = 0x00;
    rx_msg[2] = tx_msg[2];
    rx_msg[3] = tx_msg[3];

    uint8_t msg_type = tx_msg[2];
    uint8_t field_num = tx_msg[3];

    // Can return early to not send a message back
    switch (msg_type) {
        case CAN_PAY_HK:
            if (field_num == CAN_PAY_HK_HUM) {
                populate_msg_data(rx_msg, rand_bits(14));
            } else if (field_num == CAN_PAY_HK_PRES) {
                populate_msg_data(rx_msg, rand_bits(24));
            } else if (field_num < CAN_PAY_HK_FIELD_COUNT) {
                // TODO
                populate_msg_data(rx_msg, rand_bits(32));
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
            if ((field_num == CAN_PAY_CTRL_PING) ||
                (field_num == CAN_PAY_CTRL_ACT_UP) ||
                (field_num == CAN_PAY_CTRL_ACT_DOWN) ||
                (field_num == CAN_PAY_CTRL_ERASE_EEPROM)) {
                // Don't need to populate anything
            } else if (field_num == CAN_PAY_CTRL_RESET) {
                // Don't send a message back for reset
                return;
            } else if (field_num == CAN_PAY_CTRL_READ_EEPROM) {
                populate_msg_data(rx_msg, rand_bits(32));
            } else if (field_num < CAN_PAY_CTRL_FIELD_COUNT) {
                // TODO
                populate_msg_data(rx_msg, rand_bits(32));
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
            uint8_t opcode = all_cmds[i].cmd->opcode;
            uint32_t arg1 = all_cmds[i].arg1;
            uint32_t arg2 = all_cmds[i].arg2;

            // Encode one byte to two ASCII hex bytes
            trans_rx_enc_msg[0] = 0x00;
            trans_rx_enc_msg[1] = 9 * 2;
            trans_rx_enc_msg[2] = hex_to_char((opcode >> 4) & 0x0F);
            trans_rx_enc_msg[3] = hex_to_char((opcode >> 0) & 0x0F);
            trans_rx_enc_msg[4] = hex_to_char((arg1 >> 28) & 0x0F);
            trans_rx_enc_msg[5] = hex_to_char((arg1 >> 24) & 0x0F);
            trans_rx_enc_msg[6] = hex_to_char((arg1 >> 20) & 0x0F);
            trans_rx_enc_msg[7] = hex_to_char((arg1 >> 16) & 0x0F);
            trans_rx_enc_msg[8] = hex_to_char((arg1 >> 12) & 0x0F);
            trans_rx_enc_msg[9] = hex_to_char((arg1 >> 8) & 0x0F);
            trans_rx_enc_msg[10] = hex_to_char((arg1 >> 4) & 0x0F);
            trans_rx_enc_msg[11] = hex_to_char((arg1 >> 0) & 0x0F);
            trans_rx_enc_msg[12] = hex_to_char((arg2 >> 28) & 0x0F);
            trans_rx_enc_msg[13] = hex_to_char((arg2 >> 24) & 0x0F);
            trans_rx_enc_msg[14] = hex_to_char((arg2 >> 20) & 0x0F);
            trans_rx_enc_msg[15] = hex_to_char((arg2 >> 16) & 0x0F);
            trans_rx_enc_msg[16] = hex_to_char((arg2 >> 12) & 0x0F);
            trans_rx_enc_msg[17] = hex_to_char((arg2 >> 8) & 0x0F);
            trans_rx_enc_msg[18] = hex_to_char((arg2 >> 4) & 0x0F);
            trans_rx_enc_msg[19] = hex_to_char((arg2 >> 0) & 0x0F);

            trans_rx_enc_len = 20;
            trans_rx_enc_avail = true;
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

    init_obc_phase1();

    print("\n\n\nStarting commands test\n\n");

    sim_eps = true;
    sim_pay = true;
    sim_trans = true;
    sim_trans_uart = false;
    comms_delay_s = 30;
    reset_comms_delay_eeprom = false;
    skip_comms_delay = true;
    skip_deploy_antenna = true;
    com_timeout_period_s = 600;
    beacon_inhibit_period_s = 15;
    hb_ping_period_s = 30;
    disable_hb = true;
    print_can_msgs = true;
    print_cmds = true;
    print_trans_msgs = true;
    print_trans_tx_acks = true;

    // NOTE: Leaving all these print statements in will likely overflow the data section/stack, so generally they should be commented out
    // Run `avr-size main_test.elf`
    // In one case, 2676 worked but 2926 did not

    // print("sim_eps = %u\n", sim_eps);
    // print("sim_pay = %u\n", sim_pay);
    // print("sim_trans = %u\n", sim_trans);
    // print("sim_trans_uart = %u\n", sim_trans_uart);
    // print("comms_delay_s = %lu\n", comms_delay_s);
    // print("reset_comms_delay_eeprom = %u\n", reset_comms_delay_eeprom);
    // print("skip_comms_delay = %u\n", skip_comms_delay);
    // print("skip_deploy_antenna = %u\n", skip_deploy_antenna);
    // print("cmd_timer_period_s = %lu\n", cmd_timer_period_s);
    // print("beacon_inhibit_period_s = %lu\n", beacon_inhibit_period_s);
    // print("hb_ping_period_s = %lu\n", hb_ping_period_s);
    // print("disable_hb = %u\n", disable_hb);
    // print("print_can_msgs = %u\n", print_can_msgs);
    // print("print_cmds = %u\n", print_cmds);
    // print("print_trans_msgs = %u\n", print_trans_msgs);
    // print("\n");

    // Initialize heartbeat separately so we have the option to disable it for debugging
    if (!disable_hb) {
        init_hb(HB_OBC);
    }

    // print("Mem blocks: eps_hk = %lu, pay_hk = %lu, pay_opt = %lu\n",
    //     eps_hk_mem_section.curr_block,
    //     pay_hk_mem_section.curr_block,
    //     pay_opt_mem_section.curr_block);
    // print("\n");

    if (reset_comms_delay_eeprom) {
        write_eeprom(COMMS_DELAY_DONE_EEPROM_ADDR, EEPROM_DEF_DWORD);
        print("Reset comms delay EEPROM\n");
    }
    if (!skip_comms_delay) {
        run_comms_delay();
    }
    if (!skip_deploy_antenna) {
        deploy_antenna();
    }

    if (sim_trans) {
        if (sim_trans_uart) {
            print("Overwriting UART RX cb\n");
            set_uart_rx_cb(uart_cb);
            // print("Press h to list commands\n\n");
            print_uart_cmds();
        } else {
            print("Init trans UART\n");
            init_trans_uart();
        }
    } else {
        print("Init OBC trans\n");
        init_obc_phase2();
    }
    print("\n");

    while (1) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);

        if (!disable_hb) {
            run_hb();
        }

        // Trans RX (encoded)
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_trans_rx_enc_msg();
            decode_trans_rx_msg();
        }
        // Trans RX (decoded)
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_trans_rx_dec_msg();
            handle_trans_rx_dec_msg();
        }

        // Trans TX ACK
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_trans_tx_ack();
            process_trans_tx_ack();
        }
        // TODO - better way to do this than to repeat TX twice in a loop iteration?
        // Trans TX (decoded)
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_trans_tx_dec_msg();
            encode_trans_tx_msg();
        }
        // Trans TX (encoded)
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_trans_tx_enc_msg();
            send_trans_tx_enc_msg();
        }

        // Command
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_cmd();
            execute_next_cmd();
        }

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
        // EPS/PAY RX
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_rx_msg();
            process_next_rx_msg();
        }

        // Trans TX (decoded)
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_trans_tx_dec_msg();
            encode_trans_tx_msg();
        }
        // Trans TX (encoded)
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            print_next_trans_tx_enc_msg();
            send_trans_tx_enc_msg();
        }
    }

    return 0;
}
