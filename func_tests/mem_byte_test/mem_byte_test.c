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

    //erase all three memory chips
    for(uint8_t i = 0; i < MEM_NUM_CHIPS; i++) {
        erase_mem(i);
    }

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


        // Test chip rollover
        uint32_t address[] = {0x00FFFFFD, 0x01FFFFFD, 0x02FFFFFD};
        // Chip number: (address >> 24) & 0x03

        // Testing all 3 chip rollover conditions
        for (uint8_t i = 0; i < 3; i ++) {
            uint8_t write_data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
            uint8_t read_data[8] = {0};

            write_mem_bytes(address[i], write_data, 8);
            read_mem_bytes(address[i], read_data, 8);

            print ("Testing chip rollover for address %lx\n", address[i]);
            print ("Values in memory after writing:\n");
            print_bytes(read_data, 8);
        }
    }
}
