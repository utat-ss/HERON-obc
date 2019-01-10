#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/rtc.h"

int main(void){
    init_uart();
    init_spi();
    init_rtc();

    rtc_time_t t;
    t.ss = 30;
    t.mm = 59;
    t.hh = 23;
    set_rtc_time(t);

    rtc_date_t d;
    d.dd = 31;
    d.mm = 12;
    d.yy = 17;
    set_rtc_date(d);

    rtc_time_t t_alarm;
    t_alarm.ss = 00;
    t_alarm.mm = 00;
    t_alarm.hh = 00;

    rtc_date_t d_alarm;
    d_alarm.dd = 01;
    d_alarm.mm = 01;
    d_alarm.yy = 18;

    set_rtc_alarm(t_alarm, d_alarm, 1);
    //disable_alarm(1); //comment out this line to test the functionality of the set_alarm function

    for (;;){
        _delay_ms(10000);

        t = read_rtc_time();
        d = read_rtc_date();
        print("\r\nTIME: %02d:%02d:%02d", t.hh, t.mm, t.ss);
        print("\r\nDATE: %02d:%02d:20%02d", d.dd, d.mm, d.yy);
        print("\r\n");
    }

}
