/*
A program to configure OBC before launch (e.g. erase memory, check EEPROM erased,
set RTC to date/time 0).
*/

#include "../../src/general.h"
#include "../../src/transceiver.h"

// TODO - add asserts to check expected values/state
int main(void){
    init_uart();

    print("\n\n");
    print("Starting pre launch configuration\n");

    init_spi();

    init_rtc();
    init_mem();

    init_trans();

    rtc_date_t date = { .yy = 0, .mm = 0, .dd  = 0 };
    rtc_time_t time = { .hh = 0, .mm = 0, .ss  = 0 };
    set_rtc_date(date);
    set_rtc_time(time);

    erase_mem();

    // TODO - maybe check all used EEPROM addrs are erased?
    
    uart_baud_rate_t previous_baud = UART_BAUD_9600;
    correct_transceiver_baud_rate(UART_BAUD_9600, &previous_baud);
    set_uart_baud_rate(UART_BAUD_9600);

    set_trans_scw(TRANS_DEF_SCW);
    set_trans_freq(TRANS_DEF_FREQ);
    set_def_trans_beacon_content();
    set_trans_beacon_period(TRANS_BEACON_DEF_PERIOD_S);

    print("Done pre launch configuration\n");

    return 0;
}
