#ifndef TIME_H
#define TIME_H


/*
  Sets Real Time Clock to a certain Date and time
*/
uint8_t set_rtc(uint8_t change_this);

/*
  Reads date and time from the Real Time Clock. Returns pointer.
*/
uint8_t * read_rtc();

/*
  Sends Current Time to receiver subsystem, as a result of REQ_TIME
  Data is garbage.
*/
uint8_t cmd_send_time(uint8_t receiver, uint8_t data);


#endif
