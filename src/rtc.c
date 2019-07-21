/*
RTC (real-time clock)
DS3234
Datasheet: https://www.sparkfun.com/datasheets/BreakoutBoards/DS3234.pdf

- 24-hour clock
- Can use SPI mode 1 or 3 (choose 1)
*/

#include "rtc.h"

void no_op(void){
    //default call, do nothing
}

static alarm_fn_t alarm_1_cmd = no_op;
static alarm_fn_t alarm_2_cmd = no_op;

void init_rtc(void){
    // initialize the Chip Select pin
    init_cs(RTC_CS, &RTC_DDR);
    set_cs_high(RTC_CS, &RTC_PORT);

    // Write defaults to the control and status registers
    rtc_write(RTC_CTRL_R, RTC_CTRL_DEF);
    rtc_write(RTC_STATUS_R, RTC_STATUS_DEF); 
}

void set_rtc_time(rtc_time_t time){
    // Uses a 24-hr clock

    rtc_write(RTC_SEC_R, rtc_dec_to_bcd(time.ss));
    rtc_write(RTC_MIN_R, rtc_dec_to_bcd(time.mm));
    rtc_write(RTC_HOUR_R, rtc_dec_to_bcd(time.hh));
}

rtc_time_t read_rtc_time(void){
    // Reads time and returns result in decimal format

    rtc_time_t time;
    time.ss = rtc_bcd_to_dec(rtc_read(RTC_SEC_R));
    time.mm = rtc_bcd_to_dec(rtc_read(RTC_MIN_R));
    time.hh = rtc_bcd_to_dec(rtc_read(RTC_HOUR_R));
    return time;
}

rtc_date_t read_rtc_date(void){
    //Reads time and returns result in decimal format
    rtc_date_t date;
    date.yy = rtc_bcd_to_dec(rtc_read(RTC_YEAR_R));
    date.mm = rtc_bcd_to_dec(rtc_read(RTC_MONTH_R));
    date.dd = rtc_bcd_to_dec(rtc_read(RTC_DAY_R));
    return date;
}

void set_rtc_date(rtc_date_t date){
    //Write date to appropriate registers in bcd format
    rtc_write(RTC_DAY_R, rtc_dec_to_bcd(date.dd));
    rtc_write(RTC_MONTH_R, rtc_dec_to_bcd(date.mm));
    rtc_write(RTC_YEAR_R, rtc_dec_to_bcd(date.yy));
}

uint8_t set_rtc_alarm(rtc_time_t time, rtc_date_t date,
    rtc_alarm_t alarm_number, alarm_fn_t cmd) {

    /*
    Enables interrupts from respective alarm (1 or 2),
    and writes to alarm time to the appropriate register. Configures alarm to
    trigger once PER MONTH on given seconds, minutes, hours, date
    (only minutes, hours, date for alarm 2). Alternate configurations
     (with more frequent triggers) could be implemented.

    Alarm 1 can be set to trigger on a specific second
    Alarm 2 can only be set to trigger on a specific minute.
    */
    uint8_t RTC_CTRL = rtc_read(RTC_CTRL_R);
    if (alarm_number == RTC_ALARM_1){
        // select mask register 0
        PCICR |= (1 << PCIE0);
        // tell mcu to look for pin changes on PCINT6 (pin PB6)
        PCMSK0 |= (1 << PCINT6);
        // enable global interrupts
        sei();

        rtc_write(RTC_CTRL_R, (RTC_CTRL | _BV(RTC_A1IE)));
        rtc_write(RTC_ALARM_1_SEC_R, rtc_dec_to_bcd(time.ss));
        rtc_write(RTC_ALARM_1_MIN_R, rtc_dec_to_bcd(time.mm));
        rtc_write(RTC_ALARM_1_HOUR_R, rtc_dec_to_bcd(time.hh));
        rtc_write(RTC_ALARM_1_DAY_R, rtc_dec_to_bcd(date.dd));

        alarm_1_cmd = cmd;

        return 1;
    }
    else if(alarm_number == RTC_ALARM_2){
        // select mask register 0
        PCICR |= (1 << PCIE0);
        // tell mcu to look for pin changes on PCINT6 (pin PB6)
        PCMSK0 |= (1 << PCINT6);
        // enable global interrupts
        sei();

        rtc_write(RTC_CTRL_R, (RTC_CTRL | _BV(RTC_A2IE)));
        rtc_write(RTC_ALARM_2_MIN_R, rtc_dec_to_bcd(time.mm));
        rtc_write(RTC_ALARM_2_HOUR_R, rtc_dec_to_bcd(time.hh));
        rtc_write(RTC_ALARM_2_DAY_R, rtc_dec_to_bcd(date.dd));

        alarm_2_cmd = cmd;

        return 1;
    }
    return 0; //if the alarm is not set, return 0
}

uint8_t disable_rtc_alarm(rtc_alarm_t alarm_number){
    //disabling the alarm and clearing the interrupt are NOT the same thing
    //disabling the alarm simply prevents any interrupts from being sent in the
    // future
    uint8_t RTC_CTRL = rtc_read(RTC_CTRL_R);
    if (alarm_number == RTC_ALARM_1){
        //write 0 to the interrupt enable bit
        rtc_write(RTC_CTRL_R, (RTC_CTRL & ~_BV(RTC_A1IE)));
        return 1;
    }
    else if (alarm_number == RTC_ALARM_2){
        rtc_write(RTC_CTRL_R, (RTC_CTRL & ~_BV(RTC_A2IE)));
        return 1;
    }
    return 0; // if no alarm is disabled, return 0
}

uint8_t rtc_read(uint8_t reg_address){
    // transmit address, get data back
    start_rtc_spi();
    send_spi(RTC_R | reg_address);
    uint8_t return_data = send_spi(0xFF);
    end_rtc_spi();

    return return_data;
}

void rtc_write(uint8_t reg_address, uint8_t data){
    //writes data to reg_address on the RTC chip
    start_rtc_spi();
    send_spi(RTC_W | reg_address);
    send_spi(data);
    end_rtc_spi();
}

/*
Starts a SPI transmission for the RTC (using SPI mode 1).
*/
void start_rtc_spi(void) {
    // adjust the phase
    // the DS3234 requires this phase setting, which is not our default
    set_spi_mode(1);
    set_cs_low(RTC_CS, &RTC_PORT);
}

/*
Ends a SPI transmission for the RTC (resets SPI mode to 0).
*/
void end_rtc_spi(void) {
    set_cs_high(RTC_CS, &RTC_PORT);
    reset_spi_mode();
}

uint8_t rtc_bcd_to_dec(uint8_t bcd){
    //binary-coded-decimal to decimal converter
    //binary-coded decimal uses four bits to store each digit of a decimal number
    bcd &= 0x7F;
    return (10*(bcd >> 4) + (bcd & 0x0F));
}

uint8_t rtc_dec_to_bcd(uint8_t dec){
    //decimal to binary-coded-decimal converter
    uint8_t bcd = dec % 10;
    return ((dec-bcd)/10 << 4) + bcd;
}

/* 
 * Interrupt servce routine. Check the pin values manually to ensure the response is correct. 
 * For this interrupt to be valid, the logic needs to be going from HIGH -> LOW. This is determined according to the RTC datasheet 
 */
ISR(PCINT0_vect) {
    // Only act if PINB6 is driven low, otherwise do nothing
    if(!get_pin_val(6, &RTC_PORT)) {
        uint8_t RTC_STATUS = rtc_read(RTC_STATUS_R);
        if(RTC_STATUS & _BV(RTC_A1F)) {
            // perform actions for alarm 1...
            (alarm_1_cmd)();
            // clear the interrupt flag so that pin B6 gets driven high
            rtc_write(RTC_STATUS_R, (RTC_STATUS & ~_BV(RTC_A1F)));
        }
        else if(RTC_STATUS & _BV(RTC_A2F)) {
            // perform actions for alarm 2...
            (alarm_2_cmd)();
            // clear the interrupt flag so that pin B6 gets driven high
            rtc_write(RTC_STATUS_R, (RTC_STATUS & ~_BV(RTC_A2F)));
        }
    }
}
