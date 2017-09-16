#include <avr/io.h>
#include <util/delay.h>

#include <uart/uart.h>
#include <uart/log.h>
#include <can/can.h>
#include <spi/spi.h>


int main(void) {
  init_uart();
  //
  //   can_init(0);
  //   CANBT1 = 0b01111110;
  //   init_uart();
  //   uint8_t data[8];
  // 	init_rx_mob(&rx_mob, data, 2, 0x08);
  // 	init_rx_interrupts(rx_mob);
    char msg[] = "This is an OBC board\n";
    DDRD |= _BV(PD0);
    PORTD |= _BV(PD0);
    while (1) {
        print("%s", msg);
    }


    /*
    uint8_t data[2];
    can_init(0);
    CANBT1 = 0b01111110;
    while(1){

        data[0] = 0x2A;
        data[1] = 0x2A;

        can_send_message(data, 2, (PAY_TX | OBC_RX | HK_DATA));

    }
    */

}
