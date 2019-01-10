#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/rtc.h"

int main(void){
    init_uart();
    init_spi();
    init_rtc();

    rtc_time_t t;
    t.ss = 50;
    t.mm = 59;
    t.hh = 23;
    set_rtc_time(t);

    rtc_date_t d;
    d.dd = 31;
    d.mm = 12;
    d.yy = 17;
    set_rtc_date(d);

    _delay_ms(5000);

    for (;;){
        _delay_ms(1000);

        t = read_rtc_time();
        d = read_rtc_date();
        print("\r\nTIME: %02d:%02d:%02d", t.hh, t.mm, t.ss);
        print("\r\nDATE: %02d:%02d:20%02d", d.dd, d.mm, d.yy);
        print("\r\n");
    }
}
