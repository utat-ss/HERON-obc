#include "general.h"


// Initializes everything in OBC, EXCEPT the transceiver/comms things that must
// not be turned on for the first 30 minutes
void init_obc_core(void) {
    init_uart();

    init_spi();
    init_i2c();

    init_rtc();

    init_mem();
    read_all_mem_sections_eeprom();

    init_queue(&eps_tx_msg_queue);
    init_queue(&pay_tx_msg_queue);
    init_queue(&data_rx_msg_queue);

    init_queue(&cmd_opcode_queue);
    init_queue(&cmd_args_queue);

    init_can();
    init_rx_mob(&data_rx_mob);
    init_tx_mob(&pay_cmd_tx_mob);
    init_tx_mob(&eps_cmd_tx_mob);

    restart_date = read_rtc_date();
    restart_time = read_rtc_time();

    init_uptime();
    init_com_timeout();

    init_auto_data_col();
    add_uptime_callback(cmd_timeout_timer_cb);
}

// Initializes the transceiver parts of OBC that must be delayed after initial startup
void init_obc_trans(void) {
    init_trans();

    uart_baud_rate_t previous_baud = UART_BAUD_9600;
    correct_transceiver_baud_rate(UART_BAUD_9600, &previous_baud);

    set_trans_freq(TRANS_DEF_FREQ);

    set_trans_beacon_period(TRANS_BEACON_DEF_PERIOD_S);
    set_trans_beacon_content(TRANS_BEACON_DEF_MSG);
    turn_on_trans_beacon();
}
