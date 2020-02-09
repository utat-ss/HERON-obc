#include "general.h"


phase2_delay_t phase2_delay = {
    .in_progress = false,
    .done = false,
    .prev_uptime_s = 0,
    .period_s = PHASE2_DELAY_PERIOD_S,
};


// Initializes everything in OBC, EXCEPT the transceiver/comms things that must
// not be turned on for the first 30 minutes
void init_obc_phase1_core(void) {
    init_uart();

    init_spi();
    init_i2c();

    init_rtc();
    init_mem();

    init_ant();

    init_queue(&eps_tx_msg_queue);
    init_queue(&pay_tx_msg_queue);
    init_queue(&data_rx_msg_queue);

    init_queue(&cmd_queue_1);
    init_queue(&cmd_queue_2);

    init_can();
    init_rx_mob(&cmd_rx_mob);
    init_tx_mob(&pay_cmd_tx_mob);
    init_tx_mob(&eps_cmd_tx_mob);

    restart_date = read_rtc_date();
    restart_time = read_rtc_time();

    init_uptime();
    init_com_timeout();

    init_auto_data_col();
    add_uptime_callback(cmd_timeout_timer_cb);
}

void init_obc_phase1_comms(void) {
    init_phase2_delay();

    init_trans();
    
    uart_baud_rate_t previous_baud = UART_BAUD_9600;
    correct_transceiver_baud_rate(UART_BAUD_9600, &previous_baud);
    set_uart_baud_rate(UART_BAUD_9600);

    set_def_trans_beacon_content();
}

// Initializes the transceiver parts of OBC that must be delayed after initial startup
void init_obc_phase2(void) {
    // If either or both enables in EEPROM are set, turn on the beacon
    uint32_t enable_1 = read_eeprom_or_default(BEACON_ENABLE_1_EEPROM_ADDR, 1);
    uint32_t enable_2 = read_eeprom_or_default(BEACON_ENABLE_2_EEPROM_ADDR, 1);
    if ((enable_1 != 0) || (enable_2 != 0)) {
        print("Turn on beacon\n");
        turn_on_trans_beacon();
    }

    deploy_antenna();
}

void set_def_trans_beacon_content(void) {
    uint8_t data_bytes[11];
    data_bytes[0] = (restart_count >> 24) & 0xFF;
    data_bytes[1] = (restart_count >> 16) & 0xFF;
    data_bytes[2] = (restart_count >> 8) & 0xFF;
    data_bytes[3] = (restart_count >> 0) & 0xFF;
    data_bytes[4] = restart_reason;
    data_bytes[5] = restart_date.yy;
    data_bytes[6] = restart_date.mm;
    data_bytes[7] = restart_date.dd;
    data_bytes[8] = restart_time.hh;
    data_bytes[9] = restart_time.mm;
    data_bytes[10] = restart_time.ss;
    
    // Need space for the terminating character
    char content[28];
    // Use snprintf to populate the string buffer
    // https://www.microchip.com/webdoc/AVRLibcReferenceManual/group__avr__stdio_1ga77070c245d4ca4f7ec7d7144260fb875.html
    // Add 1 extra to the byte count because it includes the NUL character,
    // which will be overwritten by the next call to snprintf
    snprintf(&content[0], 6, "%s", "HERON");
    for (uint8_t i = 0; i < sizeof(data_bytes) / sizeof(data_bytes[0]); i++) {
        snprintf(&content[5 + (2 * i)], 3, "%02X", data_bytes[i]);
    }
    
    // Terminate string just in case something goes wrong
    content[27] = '\0';

    set_trans_beacon_content(content);
}

void init_phase2_delay(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        phase2_delay.in_progress = true;

        phase2_delay.done = false;
        if (read_eeprom(PHASE2_DELAY_DONE_EEPROM_ADDR) == PHASE2_DELAY_DONE_FLAG) {
            print("Skipping phase 2 delay\n");
            phase2_delay.done = true;
        } else {
            print("Starting phase 2 delay\n");
        }

        phase2_delay.prev_uptime_s = uptime_s;
    }
}

// Delays 30 minutes before we can init comms
// Fetches previous value in EEPROM to see if we have already finished this
void run_phase2_delay(void) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (!phase2_delay.in_progress) {
            return;
        }

        // Only print the uptime once per second
        if (uptime_s > phase2_delay.prev_uptime_s) {
            phase2_delay.prev_uptime_s = uptime_s;

            // Blink antenna warning LED on/off for even/odd seconds
            if (uptime_s % 2 == 0) {
                set_pin_high(ANT_DEP_WARN, &PORT_ANT_WARN);
            } else {
                set_pin_low(ANT_DEP_WARN, &PORT_ANT_WARN);
            }

            // This message will be seen once if the done flag was already set
            // in EEPROM
            print("Delay: %lu/%lu s elapsed\n", uptime_s, phase2_delay.period_s);
        }

        if (uptime_s >= phase2_delay.period_s) {
            phase2_delay.done = true;
            write_eeprom(PHASE2_DELAY_DONE_EEPROM_ADDR, PHASE2_DELAY_DONE_FLAG);
            print("Phase 2 delay done\n");
        }

        if (!phase2_delay.done) {
            return;
        }

        // If we don't return early and get here, it is time to init phase 2
        phase2_delay.in_progress = false;
        phase2_delay.done = false;
    }

    // This MUST be outside the atomic block because it needs UART RX interrupts
    // to function properly
    init_obc_phase2();
}
