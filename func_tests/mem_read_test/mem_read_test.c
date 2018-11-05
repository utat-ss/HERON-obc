#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/mem.h"

int main(void) {
    init_uart();
    init_spi();
    init_mem();

    uint8_t ctrl;

    // for(;;){
    //     _delay_ms(20000);
    //     print("\r\n\r\n");
    //     mem_write_byte(0x000010, 0xA0);
    mem_read(0x000010, &ctrl, 1);
    //     print("\r\nREAD:%x", ctrl);
    // }
}
