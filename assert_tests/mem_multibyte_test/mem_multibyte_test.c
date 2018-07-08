#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/mem.h"

int main(void) {
    init_uart();
    init_spi();
    init_mem();

    uint8_t write[10] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
    uint8_t read[10] = {0};
    uint8_t i;

    for(;;){
        _delay_ms(2000);
        print("\r\n\r\n");
        mem_write_multibyte(0x50, write, 9);
        mem_read(0x50, read, 9);
        for (i=0; i<9; i++){
            print("\r\nREAD:%x", read[i]);
        }
    }
}
