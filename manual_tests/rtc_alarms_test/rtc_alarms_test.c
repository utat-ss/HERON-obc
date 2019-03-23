/*
TODO - get this test working
*/

#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/rtc.h"

void alarm_1_test(){
    print("ALARM 1 ON");
}

void alarm_2_test(){
    print("ALARM 2 ON");
}

int main(void){
    init_uart();
    init_spi();
    init_rtc();

    rtc_time_t t;
    t.ss = 21;
    t.mm = 59;
    t.hh = 23;
    set_rtc_time(t);

    rtc_date_t d;
    d.dd = 31;
    d.mm = 12;
    d.yy = 17;
    set_rtc_date(d);

    rtc_time_t t_alarm_1;
    t_alarm_1.ss = 50;
    t_alarm_1.mm = 59;
    t_alarm_1.hh = 23;

    rtc_date_t d_alarm_1;
    d_alarm_1.dd = 31;
    d_alarm_1.mm = 12;
    d_alarm_1.yy = 17;

    set_rtc_alarm(t_alarm_1, d_alarm_1, RTC_ALARM_1, alarm_1_test);

    rtc_time_t t_alarm_2;
    t_alarm_2.ss = 30;
    t_alarm_2.mm = 00;
    t_alarm_2.hh = 00;

    rtc_date_t d_alarm_2;
    d_alarm_2.dd = 01;
    d_alarm_2.mm = 01;
    d_alarm_2.yy = 18;

    set_rtc_alarm(t_alarm_2, d_alarm_2, RTC_ALARM_2, alarm_2_test);
    //disable_alarm(1); //comment out this line to test the functionality of the set_alarm function

    // run for 40 seconds, expect to see ALARM 1 ON followed by ALARM 2 ON
    while(1){
        _delay_ms(5000);

        t = read_rtc_time();
        d = read_rtc_date();
        print("\r\nTIME: %02d:%02d:%02d", t.hh, t.mm, t.ss);
        print("\r\nDATE: %02d:%02d:20%02d", d.dd, d.mm, d.yy);
        print("\r\n");
    }

}
