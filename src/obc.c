#include "obc.h"

int main(void) {
    init_uart();
    init_spi();
    init_mem();
    init_rtc();
    init_stacks();

    uint8_t i;

    for(i=0; i<SCI_BLOCK_SIZE; i++){
        uint8_t data[FIELD_SIZE] = {i,i,i,i};
        write_to_flash(SCI_TYPE, i, data);
    }



    // uint8_t data_read[SCI_BLOCK_SIZE*FIELD_SIZE];
    //
    // mem_read(0x1000, data_read, SCI_BLOCK_SIZE);
    //
    // for(i=0; i<SCI_BLOCK_SIZE; i++){
    //   print("%d\n", data_read[i]);
    // }

}
