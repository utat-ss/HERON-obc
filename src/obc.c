#include "obc.h"

int main(void) {
    init_uart();
    init_spi();
    init_mem();
    init_rtc();
  //  _delay_ms(1000);
    init_stacks();
    uint8_t i;


    uint8_t read[15] = {0};
    mem_read(SCI_STACK_PTR,read, 15);

    for(i=0; i<15; i++){
      print("****************");
      print("%x\n", read[i]);
    }

  /*  uint8_t test [4]= {0x08, 0x07, 0x06, 0x08};
    uint8_t read_test [4] = {0};

    _delay_ms(1500);
    mem_write_multibyte(0x00, &test, 4);

    mem_read(0x00, &read_test, 4);
    for(i=0; i<4; i++){
      print("****************");
      print("%x\n", read_test[i]);
    }*/



    // uint8_t data_read[SCI_BLOCK_SIZE*FIELD_SIZE];
    //
    // mem_read(0x1000, data_read, SCI_BLOCK_SIZE);
    //
    // for(i=0; i<SCI_BLOCK_SIZE; i++){
    //   print("%d\n", data_read[i]);
    // }

}
