/*
This program tests the capability to write bytes to and read bytes from specific
memory addresses, independent of any memory layout/section scheme.
*/

#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/mem.h"

int main(void) {
    init_uart();
    init_spi();
    init_mem();

    // Create some sample data to write
    uint8_t write[255] = {0x00};
    for(uint8_t i = 0; i < 255; i++) {
        write[i] = 0x05;
    }

    // Data read back, should be the same
    uint8_t read[255] = {0};

    //flag for if values were as expected. 1 = expected, 0 = unexpected
    uint8_t expected;

    for(;;){
        _delay_ms(2000);
        print("\r\n\r\n");
        expected = 1; //set flag

        // Write data and read it back
        write_mem_bytes(0x01FFFFF0, write, 255);
        read_mem_bytes(0x01FFFFF0, read, 255);

        for (uint8_t i = 0; i < 255; i++){
            print("\r\nREAD:%x", read[i]);
            //Not as expected
            if (read[i] != 0x05){
                expected = 0;
                print(" Value not as expected");
            }
        }
        //check if values were as expected
        if (expected == 0){
            print("\nThere were unexpected values");
        }
        else {
            print("\nAll values were as expected");
        }

        _delay_ms(2000);
        print("\r\n\r\n");
        expected = 1;

        //write different data to block
        for(uint8_t i = 0; i < 255; i++) {
            write[i] = i;
        }

        //read and write
        write_mem_bytes(0x01FFFFF0, write, 255);
        read_mem_bytes(0x01FFFFF0, read, 255);

        //Check values
        for (uint8_t i = 0; i < 255; i++){
            print("\r\nREAD:%x", read[i]);

            if (read[i] != i){
                expected = 0;
                print(" Value not as expected");
            }
        }

        //check if values were as expected
        if (expected == 0){
            print("\nThere were unexpected values");
        }
        else {
            print("\nAll values were as expected");
        }
    }
}
