#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/mem.h"

int main(void) {
    init_uart();
    init_spi();
    init_mem();

    uint8_t i;
    uint8_t write[255] = {0x00};

    for(i = 0; i < 255; i++)
    {
        write[i] = 0x05;
    }

    uint8_t read[255] = {0};

    for(;;){
        _delay_ms(2000);
        print("\r\n\r\n");
        mem_write(0x01FFFFF0, write, 255);
        mem_read(0x01FFFFF0, read, 255);
        for (i=0; i<255; i++){
            print("\r\nREAD:%x", read[i]);
        }
    }

}
