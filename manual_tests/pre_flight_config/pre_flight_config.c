/*
A program to configure OBC before launch (e.g. erase memory, check EEPROM erased,
set RTC to date/time 0).
*/

#include <test/test.h>

#include "../../src/general.h"
#include "../../src/transceiver.h"


#define ASSERT_BYTES_EQ(bytes1, bytes2, count)  \
    for (uint8_t __i = 0; __i < (count); __i++) { \
        ASSERT_EQ((bytes1)[__i], (bytes2)[__i]);    \
    }


void populate_bytes(uint8_t* bytes, uint8_t value, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        bytes[i] = value;
    }
}


void config_rtc(void) {
    rtc_date_t init_date = { .yy = 0, .mm = 1, .dd  = 1 };
    rtc_time_t init_time = { .hh = 0, .mm = 0, .ss  = 0 };
    set_rtc_date(init_date);
    set_rtc_time(init_time);

    _delay_ms(1000);
    rtc_date_t read_date = read_rtc_date();
    rtc_time_t read_time = read_rtc_time();
    ASSERT_EQ(read_date.yy, 0);
    ASSERT_EQ(read_date.mm, 1);
    ASSERT_EQ(read_date.dd, 1);
    ASSERT_EQ(read_time.hh, 0);
    ASSERT_EQ(read_time.mm, 0);
    ASSERT_EQ(read_time.ss, 1);
}

void config_mem(void) {
    erase_mem();

    uint8_t ones[4];
    populate_bytes(ones, 0xFF, 4);
    uint8_t read[4];

    populate_bytes(read, 0x00, 4);
    read_mem_bytes(0x000000, read, 4);
    ASSERT_BYTES_EQ(read, ones, 4);

    populate_bytes(read, 0x00, 4);
    read_mem_bytes(0x200000, read, 4);
    ASSERT_BYTES_EQ(read, ones, 4);

    populate_bytes(read, 0x00, 4);
    read_mem_bytes(0x400000, read, 4);
    ASSERT_BYTES_EQ(read, ones, 4);
}

void config_trans(void) {
    init_trans();

    uart_baud_rate_t previous_baud = UART_BAUD_9600;
    correct_transceiver_baud_rate(UART_BAUD_9600, &previous_baud);
    set_uart_baud_rate(UART_BAUD_9600);

    set_trans_scw(TRANS_DEF_SCW);
    set_trans_freq(TRANS_DEF_FREQ);
    set_trans_beacon_period(TRANS_BEACON_DEF_PERIOD_S);
    set_trans_src_call_sign("ABCDEF");
    set_trans_dest_call_sign("ABCDEF");
    turn_off_trans_beacon();

    // Check we can still read status and it is OK
    uint8_t rssi = 0;
    uint8_t reset_count = 0;
    uint16_t scw = 0;
    uint8_t ret = get_trans_scw(&rssi, &reset_count, &scw);
    ASSERT_TRUE(ret);
    ASSERT_EQ(rssi, 0);
    ASSERT_NEQ(reset_count, 0);
    ASSERT_EQ(scw, TRANS_DEF_SCW);
}


int main(void){
    WDT_OFF();
    init_obc_phase1_core();

    print("\n\n");
    print("OBC pre launch configuration init\n\n");

    config_rtc();
    config_mem();
    config_trans();

    print("Done pre launch configuration\n");

    return 0;
}
