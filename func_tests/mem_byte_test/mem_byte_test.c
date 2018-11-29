/*
This program tests the capability to write bytes to and read bytes from specific
memory addresses, independent of any memory layout/section scheme.
*/

#include <stdlib.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/mem.h"


#define TEST1_COUNT 5
#define TEST1_ADDR 0x1FFF

void test1(void) {
    print("\nStarting test 1\n\n");

    _delay_ms(2000);

    // Create some sample data to write
    uint8_t write[TEST1_COUNT] = {0x00};
    // Data read back, should be the same
    uint8_t read[TEST1_COUNT] = {0x00};

    //flag for if values were as expected. 1 = expected, 0 = unexpected
    uint8_t expected = 1;

    // Generate random numbers to write
    for(uint8_t i = 0; i < TEST1_COUNT; i++) {
        // write[i] = rand() & 0xFF;
        write[i] = i + 10;
    }

    // Write data and read it back
    write_mem_bytes(TEST1_ADDR, write, TEST1_COUNT);
    _delay_ms(1000);
    read_mem_bytes(TEST1_ADDR, read, TEST1_COUNT);

    for (uint8_t i = 0; i < TEST1_COUNT; i++){
        print("i = %u, expected %.2x, read %.2x", i, write[i], read[i]);
        //Not as expected
        if (read[i] != write[i]){
            expected = 0;
            print(" Not expected");
        }
        print("\n");
    }

    //check if values were as expected
    if (expected == 0){
        print("\nThere were unexpected values\n");
    }
    else {
        print("\nAll values were as expected\n");
    }

    print("\nDone test 1\n");
}


#define TEST2_COUNT 5
#define TEST2_ADDR 0x3ABCDE

void test2(void) {
    print("\nStarting test 2\n\n");

    _delay_ms(2000);

    uint8_t write[TEST2_COUNT] = {0x00};
    uint8_t read[TEST2_COUNT] = {0x00};

    uint8_t expected = 1;

    //write different data to block
    for(uint8_t i = 0; i < TEST2_COUNT; i++) {
        write[i] = i + 13;
    }

    //read and write
    write_mem_bytes(TEST2_ADDR, write, TEST2_COUNT);
    _delay_ms(1000);
    read_mem_bytes(TEST2_ADDR, read, TEST2_COUNT);

    //Check values
    for (uint8_t i = 0; i < TEST2_COUNT; i++){
        print("i = %u, expected %.2x, read %.2x", i, write[i], read[i]);
        //Not as expected
        if (read[i] != write[i]){
            expected = 0;
            print(" Not expected");
        }
        print("\n");
    }

    //check if values were as expected
    if (expected == 0){
        print("\nThere were unexpected values\n");
    }
    else {
        print("\nAll values were as expected\n");
    }

    print("\nDone test 2\n");
}


#define TEST3_ADDR_COUNT 3
#define TEST3_DATA_COUNT 8

void test3(void) {
    print("\nStarting test 3\n\n");

    // Test chip rollover
    uint32_t address[TEST3_ADDR_COUNT] = {0x00FFFFFD, 0x01FFFFFD, 0x02FFFFFD};
    // Chip number: (address >> 24) & 0x03

    // Testing all 3 chip rollover conditions
    for (uint8_t i = 0; i < TEST3_ADDR_COUNT; i ++) {
        uint8_t write[TEST3_DATA_COUNT] = {1, 2, 3, 4, 5, 6, 7, 8};
        uint8_t read[TEST3_DATA_COUNT] = {0};

        write_mem_bytes(address[i], write, TEST3_DATA_COUNT);
        read_mem_bytes(address[i], read, TEST3_DATA_COUNT);

        print ("Testing chip rollover for address %lx\n", address[i]);
        print ("Values in memory after writing:\n");
        print_bytes(read, TEST3_DATA_COUNT);
    }

    print("\nDone test 3\n");
}

int main(void) {
    init_uart();
    init_spi();
    init_mem();

    print("\n\n\nStarting memory test\n\n");

    //erase all three memory chips
    for(uint8_t i = 0; i < MEM_NUM_CHIPS; i++) {
        erase_mem(i);
    }

    print("Erased memory\n\n");

    test1();
    test2();
    test3();

    print("\nDone all tests\n");
}
