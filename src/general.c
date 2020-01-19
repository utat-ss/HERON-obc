#include "general.h"


// Initializes everything in OBC, EXCEPT the transceiver/comms things that must
// not be turned on for the first 30 minutes
void init_obc_phase1(void) {
    init_uart();

    init_spi();
    init_i2c();

    init_rtc();
    init_mem();

    init_queue(&eps_tx_msg_queue);
    init_queue(&pay_tx_msg_queue);
    init_queue(&data_rx_msg_queue);

    init_queue(&cmd_queue_1);
    init_queue(&cmd_queue_2);

    init_can();
    init_rx_mob(&cmd_rx_mob);
    init_tx_mob(&pay_cmd_tx_mob);
    init_tx_mob(&eps_cmd_tx_mob);

    init_ant();

    restart_date = read_rtc_date();
    restart_time = read_rtc_time();

    init_uptime();
    init_com_timeout();

    init_auto_data_col();
    add_uptime_callback(cmd_timeout_timer_cb);
    add_uptime_callback(beacon_inhibit_timer_cb);

    init_trans();
    uart_baud_rate_t previous_baud = UART_BAUD_9600;
    correct_transceiver_baud_rate(UART_BAUD_9600, &previous_baud);
    set_uart_baud_rate(UART_BAUD_9600);
    set_def_trans_beacon_content();
}

// Initializes the transceiver parts of OBC that must be delayed after initial startup
void init_obc_phase2(void) {
    // TODO - move to flight config program
    set_trans_freq(TRANS_DEF_FREQ);
    set_trans_beacon_period(TRANS_BEACON_DEF_PERIOD_S);

    turn_on_trans_beacon();
    // TODO - deploy antenna
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
