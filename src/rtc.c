#include "rtc.h"

void rtc_time_demo(){
	init_uart();
    init_spi();
    init_rtc();

    time_t t;
    t.ss = 50;
    t.mm = 59;
    t.hh = 23;
    set_time(t);

    date_t d;
    d.dd = 31;
    d.mm = 12;
    d.yy = 17;
    set_date(d);

    _delay_ms(5000);

    for (;;){
        _delay_ms(10000);

        t = read_time();
        d = read_date();
        print("\r\nTIME: %02d:%02d:%02d", t.hh, t.mm, t.ss);
        print("\r\nDATE: %02d:%02d:20%02d", d.dd, d.mm, d.yy);
        print("\r\n");

    }
}

void alarms_demo(){
	init_uart();
	init_spi();
	init_rtc();

	time_t t;
	t.ss = 30;
	t.mm = 59;
	t.hh = 23;
	set_time(t);

	date_t d;
	d.dd = 31;
	d.mm = 12;
	d.yy = 17;
	set_date(d);

	time_t t_alarm;
	t_alarm.ss = 00;
	t_alarm.mm = 00;
	t_alarm.hh = 00;

	date_t d_alarm;
	d_alarm.dd = 01;
	d_alarm.mm = 01;
	d_alarm.yy = 18;

	set_alarm(t_alarm, d_alarm, 1);
	disable_alarm(1); //comment out this line to test the functionality of the set_alarm function

	for (;;){
			_delay_ms(10000);

			t = read_time();
			d = read_date();
			print("\r\nTIME: %02d:%02d:%02d", t.hh, t.mm, t.ss);
			print("\r\nDATE: %02d:%02d:20%02d", d.dd, d.mm, d.yy);
			print("\r\n");

	}


}

void init_rtc(){

	// the DS3234 requires this phase setting, which is not our default
	SPCR |= _BV(CPHA);

	// initialize the Chip Select pin
	init_cs(RTC_CS, &RTC_DDR);
    set_cs_high(RTC_CS, &RTC_PORT);

    // Write defaults to the control and status registers
    rtc_write(RTC_CTRL_R, RTC_CTRL_DEF);
    rtc_write(RTC_STATUS_R, RTC_STATUS_DEF);

    // Set back the phase
    SPCR &= ~(_BV(CPHA));

}

void set_time(time_t time){
	// since we are not savages, we will be using a 24h clock.
	rtc_write(RTC_SEC_R, dec_to_bcd(time.ss));
	rtc_write(RTC_MIN_R, dec_to_bcd(time.mm));
	rtc_write(RTC_HOUR_R, dec_to_bcd(time.hh));

}

time_t read_time (){

	time_t time;
	time.ss = bcd_to_dec(rtc_read(RTC_SEC_R));
	time.mm = bcd_to_dec(rtc_read(RTC_MIN_R));
	time.hh = bcd_to_dec(rtc_read(RTC_HOUR_R));
	return time;
}

date_t read_date(){
	date_t date;
	date.yy = bcd_to_dec(rtc_read(RTC_YEAR_R));
	date.mm = bcd_to_dec(rtc_read(RTC_MONTH_R));
	date.dd = bcd_to_dec(rtc_read(RTC_DAY_R));
	return date;
}

void set_date(date_t date){
	rtc_write(RTC_DAY_R, dec_to_bcd(date.dd));
	rtc_write(RTC_MONTH_R, dec_to_bcd(date.mm));
	rtc_write(RTC_YEAR_R, dec_to_bcd(date.yy));
}

uint8_t set_alarm(time_t time, date_t date, uint8_t alarm_number){
	uint8_t RTC_CTRL = rtc_read(RTC_CTRL_R);
	if (alarm_number == 1){
		rtc_write(RTC_CTRL_R, (RTC_CTRL | _BV(A1IE))); //enable interrupts from alarm 1
		rtc_write(RTC_ALARM_1_SEC_R, dec_to_bcd(time.ss));
		rtc_write(RTC_ALARM_1_MIN_R, dec_to_bcd(time.mm));
		rtc_write(RTC_ALARM_1_HOUR_R, dec_to_bcd(time.hh));
		rtc_write(RTC_ALARM_1_DAY_R, dec_to_bcd(date.dd));
		return 1;
	}
	else if(alarm_number == 2){
		rtc_write(RTC_CTRL_R, (RTC_CTRL | _BV(A2IE)));
		rtc_write(RTC_ALARM_2_MIN_R, dec_to_bcd(time.mm));
		rtc_write(RTC_ALARM_2_HOUR_R, dec_to_bcd(time.hh));
		rtc_write(RTC_ALARM_2_DAY_R, dec_to_bcd(date.dd));
		return 1;
	}
	return 0; //if the alarm is not set, return 0
}

uint8_t disable_alarm(uint8_t alarm_number){ //disabling the alarm and clearing the interrupt are NOT the same thing
	uint8_t RTC_CTRL = rtc_read(RTC_CTRL_R);
	if (alarm_number == 1){
		rtc_write(RTC_CTRL_R, (RTC_CTRL & ~_BV(A1IE))); //write 0 to the interrupt enable bit
		RTC_CTRL = rtc_read(RTC_CTRL_R);
		return 1;
	}
	else if (alarm_number == 2){
		rtc_write(RTC_CTRL_R, (RTC_CTRL & ~_BV(A2IE)));
		return 1;
	}
	return 0; // if no alarm is disabled, return 0
}

uint8_t rtc_read(uint8_t reg_address){

	// adjust the phase
	SPCR |= _BV(CPHA);

	uint8_t return_data;

	// transmit address, get data back
	set_cs_low(RTC_CS, &RTC_PORT);
	send_spi(RTC_R | reg_address);
	return_data = send_spi(0xFF);
	set_cs_high(RTC_CS, &RTC_PORT);

	SPCR &= ~(_BV(CPHA));

	return return_data;
}

void rtc_write(uint8_t reg_address, uint8_t data){

	set_cs_low(RTC_CS, &RTC_PORT);

	send_spi(RTC_W | reg_address);

	send_spi(data);

	set_cs_high(RTC_CS, &RTC_PORT);

}

uint8_t bcd_to_dec(uint8_t bcd){

	bcd &= 0x7F;
	return ( 10*(bcd >> 4) + (bcd & 0x0F)) ;

}

uint8_t dec_to_bcd(uint8_t dec){

	uint8_t bcd;
	bcd = dec % 10;
	return ((dec-bcd)/10 << 4) + bcd;

}
