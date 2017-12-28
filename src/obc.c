#include "obc.h"



int main(void) {

    init_uart();

    print("\r\nUART Initialized \r\n");

    init_spi();

    init_rtc();

    uint8_t * read;
 

    uint8_t seconds;
    uint8_t sec_tens;
    uint8_t sec_ones;

    for (;;){
  
        seconds = rtc_read(RTC_SEC_R, read, 1);


        sec_tens = ((seconds & 0x70) >> 4) * 10;
        sec_ones = seconds % 16;

        print("Tens: %d. Ones: %d \r\n", sec_tens, sec_ones);


        _delay_ms(10000);


    }
}