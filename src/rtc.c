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