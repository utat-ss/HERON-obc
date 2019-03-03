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
    t.ss = 40;
    t.mm = 59;
    t.hh = 23;
    set_rtc_time(t);

    rtc_date_t d;
    d.dd = 31;
    d.mm = 12;
    d.yy = 17;
    set_rtc_date(d);

    rtc_time_t t_alarm_1;
    t_alarm.ss = 00;
    t_alarm.mm = 00;
    t_alarm.hh = 00;

    rtc_date_t d_alarm_1;
    d_alarm.dd = 01;
    d_alarm.mm = 01;
    d_alarm.yy = 18;

    set_rtc_alarm(t_alarm_1, d_alarm_1, 1, alarm_1_test);

    rtc_time_t t_alarm_2;
    t_alarm.ss = 10;
    t_alarm.mm = 00;
    t_alarm.hh = 00;

    rtc_date_t d_alarm_2;
    d_alarm.dd = 01;
    d_alarm.mm = 01;
    d_alarm.yy = 18;

    set_rtc_alarm(t_alarm_2, d_alarm_2, 2, alarm_2_test);

    //disable_alarm(1); //comment out this line to test the functionality of the set_alarm function

    // run for 40 seconds, expect to see ALARM 1 ON followed by ALARM 2 ON
    for (uint8_t i=0; i<4; i++){
        _delay_ms(10000);

        t = read_rtc_time();
        d = read_rtc_date();
        print("\r\nTIME: %02d:%02d:%02d", t.hh, t.mm, t.ss);
        print("\r\nDATE: %02d:%02d:20%02d", d.dd, d.mm, d.yy);
        print("\r\n");
    }

}
