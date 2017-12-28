#include "rtc.h"

void init_rtc(){

	init_cs(RTC_CS, &RTC_DDR);
    set_cs_high(RTC_CS, &RTC_PORT);

    uint8_t control_reg = 0x00;
    uint8_t status_reg = 0x00;

    control_reg |= _BV(INTCN);


    rtc_write(RTC_CTRL_R, control_reg, 1);
    rtc_write(RTC_STATUS_R, status_reg, 1);
    rtc_write(RTC_SEC_R, 0x00, 1);

}

uint8_t rtc_read(uint8_t reg_address, uint8_t * data, uint8_t data_len){

	uint8_t return_data;

	set_cs_low(RTC_CS, &RTC_PORT);

	send_spi(RTC_R | reg_address);


	// for (int i = 0; i < data_len; i++){
	// 	data[i] = send_spi(0x00);
	// }

	return_data = send_spi(0xFF);


	set_cs_high(RTC_CS, &RTC_PORT);

	return return_data;

	// print("\r\n \r\n");

	// print("Read from %x", (RTC_R | reg_address));
	// print("\r\n");
	// print("Value: %x", (return_data));
}

void rtc_write(uint8_t reg_address, uint8_t data, uint8_t data_len){

	set_cs_low(RTC_CS, &RTC_PORT);

	send_spi(RTC_W | reg_address);

	// for (int i = 0; i < data_len; i++){
	// 	send_spi(data[i]);
	// }

	send_spi(data);

	set_cs_high(RTC_CS, &RTC_PORT);


	print("\r\n \r\n");
	print("WRITE to %x", (RTC_W | reg_address));
	print("\r\n");
	print("Value: %x", (data));

}