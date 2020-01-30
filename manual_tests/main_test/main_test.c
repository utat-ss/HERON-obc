/*
This is the highest-level (most comprehensive) test in the OBC repository.

It runs all the code in the main OBC program, with some extra debugging features
for special modes, to enable/disable/modify specific features, etc.
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
// Denominator of probability the simulated EPS/PAY will respond to a data
// collection field request (e.g. 5 means 1/5 probability)
uint32_t data_col_prob_denom = 3;

bool reset_phase2_delay_eeprom = false;
bool skip_phase2_delay = false;
bool skip_phase2_init = false;

bool disable_hb = false;


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

    // Normally there would be several iterations of the main loop before EPS
    // responds, so pick a chance of this being the iteration where it responds
    // Most of the time it will not respond so return
    if ((random() % data_col_prob_denom) != 0) {
        return;
    }

    // TX and RX defined from OBC's perspective
    uint8_t tx_msg[8] = {0x00};
    dequeue(&eps_tx_msg_queue, tx_msg);

    if (print_can_msgs) {
        print("CAN TX (EPS): ");
        print_bytes(tx_msg, 8);
    }

    uint8_t opcode = tx_msg[0];
    uint8_t field_num = tx_msg[1];

    // Construct the message EPS would send back
    uint8_t rx_msg[8] = {0x00};
    rx_msg[0] = opcode;
    rx_msg[1] = field_num;

    uint8_t rx_status = CAN_STATUS_OK;

    // Can return early to not send a message back
    switch (opcode) {
        case CAN_EPS_HK:
            if (CAN_EPS_HK_GYR_UNCAL_X <= field_num && field_num <= CAN_EPS_HK_GYR_CAL_Z) {
                // 16-bit data - IMU gyro
                populate_msg_data(rx_msg, rand_bits(16));
            } else if (field_num < CAN_EPS_HK_FIELD_COUNT) {
                // TODO
                populate_msg_data(rx_msg, rand_bits(32));
            } else {
                rx_status = CAN_STATUS_INVALID_FIELD_NUM;
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
                rx_status = CAN_STATUS_INVALID_FIELD_NUM;
            }
            break;

        default:
            rx_status = CAN_STATUS_INVALID_OPCODE;
            break;
    }

    rx_msg[2] = rx_status;

    // print("Enqueued to data_rx_msg_queue\n");
    enqueue(&data_rx_msg_queue, rx_msg);
}

// Simulates sending a PAY TX message and getting a response back
void sim_send_next_pay_tx_msg(void) {
    if (queue_empty(&pay_tx_msg_queue)) {
        return;
    }

    // Normally there would be several iterations of the main loop before PAY
    // responds, so pick a chance of this being the iteration where it responds
    // Most of the time it will not respond so return
    if ((random() % data_col_prob_denom) != 0) {
        return;
    }

    // TX and RX defined from OBC's perspective
    uint8_t tx_msg[8] = {0x00};
    dequeue(&pay_tx_msg_queue, tx_msg);

    if (print_can_msgs) {
        print("CAN TX (PAY): ");
        print_bytes(tx_msg, 8);
    }

    uint8_t opcode = tx_msg[0];
    uint8_t field_num = tx_msg[1];

    // Construct the message PAY would send back
    uint8_t rx_msg[8] = {0x00};
    rx_msg[0] = opcode;
    rx_msg[1] = field_num;

    uint8_t rx_status = CAN_STATUS_OK;

    // Can return early to not send a message back
    switch (opcode) {
        case CAN_PAY_HK:
            if (field_num == CAN_PAY_HK_HUM) {
                populate_msg_data(rx_msg, rand_bits(14));
            } else if (field_num == CAN_PAY_HK_PRES) {
                populate_msg_data(rx_msg, rand_bits(24));
            } else if (field_num < CAN_PAY_HK_FIELD_COUNT) {
                // TODO
                populate_msg_data(rx_msg, rand_bits(32));
            } else {
                rx_status = CAN_STATUS_INVALID_FIELD_NUM;
            }
            break;

        case CAN_PAY_OPT:
            if (0 <= field_num && field_num < CAN_PAY_OPT_TOT_FIELD_COUNT) {
                // All fields are 24-bit ADC data
                populate_msg_data(rx_msg, rand_bits(24));
            } else {
                rx_status = CAN_STATUS_INVALID_FIELD_NUM;
            }
            break;

        case CAN_PAY_CTRL:
            if ((field_num == CAN_PAY_CTRL_PING) ||
                (field_num == CAN_PAY_CTRL_MOTOR_UP) ||
                (field_num == CAN_PAY_CTRL_MOTOR_DOWN) ||
                (field_num == CAN_PAY_CTRL_ERASE_EEPROM)) {
                // Don't need to populate anything
            } else if (field_num == CAN_PAY_CTRL_RESET_SSM) {
                // Don't send a message back for reset
                return;
            } else if (field_num == CAN_PAY_CTRL_READ_EEPROM) {
                populate_msg_data(rx_msg, rand_bits(32));
            } else if (field_num < CAN_PAY_CTRL_FIELD_COUNT) {
                // TODO
                populate_msg_data(rx_msg, rand_bits(32));
            } else {
                rx_status = CAN_STATUS_INVALID_FIELD_NUM;
            }
            break;

        default:
            rx_status = CAN_STATUS_INVALID_OPCODE;
            break;
    }

    rx_msg[2] = rx_status;

    // print("Enqueued to data_rx_msg_queue\n");
    enqueue(&data_rx_msg_queue, rx_msg);
}


int main(void){
    WDT_OFF();
    WDT_ENABLE_SYS_RESET(WDTO_8S);

    init_obc_phase1();

    print("\n\n\nStarting OBC main test\n\n");

    sim_eps = true;
    sim_pay = true;
    phase2_delay.period_s = 30;
    reset_phase2_delay_eeprom = false;
    skip_phase2_delay = true;
    skip_phase2_init = true;
    com_timeout_period_s = 600;
    hb_req_period_s = 300;
    hb_resp_wait_time_s = 5;
    disable_hb = false;
    print_can_msgs = true;
    print_cmds = true;
    print_trans_msgs = true;
    print_trans_tx_acks = true;

    // NOTE: Leaving all these print statements in will likely overflow the data section/stack, so generally they should be commented out
    // Run `avr-size main_test.elf`
    // In one case, 2676 worked but 2926 did not

    // print("sim_eps = %u\n", sim_eps);
    // print("sim_pay = %u\n", sim_pay);
    // print("skip_phase2_delay = %u\n", skip_phase2_delay);
    // print("skip_phase2_init = %u\n", skip_phase2_init);
    // print("cmd_timer_period_s = %lu\n", cmd_timer_period_s);
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

    if (reset_phase2_delay_eeprom) {
        write_eeprom(PHASE2_DELAY_DONE_EEPROM_ADDR, EEPROM_DEF_DWORD);
        print("Reset phase2 delay EEPROM\n");
    }
    if (skip_phase2_delay) {
        phase2_delay.done = true;
    }
    if (skip_phase2_init) {
        phase2_delay.in_progress = false;
        phase2_delay.done = false;
    }

    while (1) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);

        run_phase2_delay();

        if (!disable_hb) {
            run_hb();
        }

        run_auto_data_col();

        // Trans RX (encoded)
        decode_trans_rx_msg();
        // Trans RX (decoded)
        handle_trans_rx_dec_msg();

        // Trans TX ACK
        process_trans_tx_ack();
        // Trans TX (decoded)
        encode_trans_tx_msg();
        // Trans TX (encoded)
        send_trans_tx_enc_msg();

        // Command
        execute_next_cmd();

        // EPS TX
        // Either simulate EPS over CAN or actually send the CAN message
        if (sim_eps) {
            sim_send_next_eps_tx_msg();
        }  else {
            send_next_eps_tx_msg();
        }

        // PAY TX
        // Either simulate PAY over CAN or actually send the CAN message
        if (sim_pay) {
            sim_send_next_pay_tx_msg();
        }  else {
            send_next_pay_tx_msg();
        }

        // EPS/PAY RX
        process_next_rx_msg();

        // Trans TX (decoded)
        encode_trans_tx_msg();
        // Trans TX (encoded)
        send_trans_tx_enc_msg();
    }

    return 0;
}
