/*
This program tests the capability to write bytes to and read bytes from specific
memory addresses, independent of any memory layout/section scheme.
*/

#include <stdlib.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/mem.h"


// len - number of bytes in each of write and read
void print_write_read(uint32_t addr, uint8_t* write, uint8_t* read, uint8_t len) {
    print("addr = 0x%.8lx\n", addr);
    if (write != NULL) {
        print("write = ");
        print_bytes(write, len);
    }
    if (read != NULL) {
        print("read  = ");
        print_bytes(read, len);
    }
}

void compare_write_read(uint8_t* write, uint8_t* read, uint8_t len) {
    //flag for if values were as expected. 1 = expected, 0 = unexpected
    uint8_t pass = 1;
    for (uint8_t i = 0; i < len; i++){
        //Not as expected
        if (read[i] != write[i]){
            pass = 0;
        }
    }

    //check if values were as expected
    if (pass == 1){
        print("\nPASS: All values match\n");
    } else {
        print("\nFAIL: Not all values match\n");
    }
}


#define SINGLE_ADDR 0x3ABCDE
#define SINGLE_DATA 0xA7

// Write single bytes
void test_single_write_read(void) {
    print("\nStarting single write/read test\n\n");

    uint32_t addr = SINGLE_ADDR;
    uint8_t write[1] = { SINGLE_DATA };
    uint8_t read[1] = { 0x00 };

    write_mem_bytes(addr, write, 1);
    read_mem_bytes(addr, read, 1);
    print_write_read(addr, write, read, 1);

    print("\nDone single write/read test\n");
}


// NOTE: should change this seed periodically
#define ERASE_SEED          0x66BFAE12
#define ERASE_ADDR_COUNT    10

// Generate random numbers
// NOTE: srandom() and random() are for 32-bit integers,
// srand() and rand() are for 16-bit integers
void test_erase(void) {
    print("\nStarting erase test\n\n");

    //erase all memory chips
    erase_mem();
    print("Erased memory\n");

    srandom(ERASE_SEED);
    print("Random seed = 0x%.8lx\n", ERASE_SEED);

    uint32_t num_addrs = MEM_NUM_CHIPS * (1UL << MEM_CHIP_ADDR_WIDTH);
    print("num_addrs = 0x%.8lx\n", num_addrs);

    print("CHECK: all bytes are 0xff\n");

    for (uint8_t i = 0; i < ERASE_ADDR_COUNT; i++) {
        uint32_t addr = random() % num_addrs;
        uint8_t read[1] = { 0x00 };
        read_mem_bytes(addr, read, 1);
        print_write_read(addr, NULL, read, 1);
    }

    print("\nDone erase test\n");
}


#define MULTI_ADDR   0x3FE7FF
#define MULTI_LEN    5
#define MULTI_DATA   { 0x01, 0x00, 0xFF, 0x94, 0x79 }

// Write multiple bytes
void test_multi_write_read(void) {
    print("\nStarting multi write/read test\n\n");

    // Create some sample data to write
    uint8_t write[MULTI_LEN] = MULTI_DATA;
    // Data read back, should be the same
    uint8_t read[MULTI_LEN] = { 0x00 };

    // Write data and read it back
    write_mem_bytes(MULTI_ADDR, write, MULTI_LEN);
    read_mem_bytes(MULTI_ADDR, read, MULTI_LEN);

    print_write_read(MULTI_ADDR, write, read, MULTI_LEN);
    compare_write_read(write, read, MULTI_LEN);

    print("\nDone multi write/read test\n");
}


#define PATTERN_ADDR    0x5100FF
#define PATTERN_LEN     5
#define PATTERN_OFFSET  17

// Generate numbers with a specific pattern
void test_pattern_write_read(void) {
    print("\nStarting pattern test\n\n");

    // Create some numbers  to write
    uint8_t write[PATTERN_LEN] = { 0x00 };
    for(uint8_t i = 0; i < PATTERN_LEN; i++) {
        write[i] = i + PATTERN_OFFSET;
    }
    // Data read back, should be the same
    uint8_t read[PATTERN_LEN] = { 0x00 };

    // Write data and read it back
    write_mem_bytes(PATTERN_ADDR, write, PATTERN_LEN);
    read_mem_bytes(PATTERN_ADDR, read, PATTERN_LEN);

    print_write_read(PATTERN_ADDR, write, read, PATTERN_LEN);
    compare_write_read(write, read, PATTERN_LEN);

    print("\nDone pattern test\n");
}


// NOTE: should change this seed manually
#define RANDOM_SEED     0x68BCDE12
#define RANDOM_MAX_LEN  255

// Generate random numbers
// NOTE: srandom() and random() are for 32-bit integers,
// srand() and rand() are for 16-bit integers
void test_random_write_read(void) {
    print("\nStarting random test\n\n");

    srandom(RANDOM_SEED);
    print("Random seed = 0x%.8lx\n", RANDOM_SEED);

    uint32_t num_addrs = MEM_NUM_CHIPS * (1UL << MEM_CHIP_ADDR_WIDTH);
    print("num_addrs = 0x%.8lx\n", num_addrs);

    uint32_t addr = random() % num_addrs;
    print("addr = 0x%.8lx\n", addr);

    uint8_t len = (random() % RANDOM_MAX_LEN) + 1;
    print("len = %u\n", len);

    // Generate random data to write
    uint8_t write[RANDOM_MAX_LEN] = { 0x00 };
    for(uint8_t i = 0; i < len; i++) {
        write[i] = random() & 0xFF;
    }

    uint8_t read[RANDOM_MAX_LEN] = { 0x00 };

    //read and write
    write_mem_bytes(addr, write, len);
    read_mem_bytes(addr, read, len);

    print_write_read(addr, write, read, len);
    compare_write_read(write, read, len);

    print("\nDone random test\n");
}


#define ROLLOVER_ADDR_COUNT 3
#define ROLLOVER_LEN        8
#define ROLLOVER_DATA       { 1, 2, 19, 4, 5, 6, 7, 8 }

// Test chip rollover
// Chip number: (address >> 21) & 0x03
void test_chip_rollover(void) {
    print("\nStarting chip rollover test\n\n");

    // Test chip rollover
    uint32_t addr[ROLLOVER_ADDR_COUNT] = { 0x1FFFFD, 0x3FFFFE, 0x5FFFFF };

    // Testing all 3 chip rollover conditions
    for (uint8_t i = 0; i < ROLLOVER_ADDR_COUNT; i++) {
        uint8_t write[ROLLOVER_LEN] = ROLLOVER_DATA;
        uint8_t read[ROLLOVER_LEN] = { 0x00 };

        write_mem_bytes(addr[i], write, ROLLOVER_LEN);
        read_mem_bytes(addr[i], read, ROLLOVER_LEN);
        print_write_read(addr[i], write, read, ROLLOVER_LEN);
    }

    print("\nDone chip rollover test\n");
}


int main(void) {
    init_uart();
    init_spi();
    init_mem();

    print("\n\n\nStarting memory tests\n");

    test_single_write_read();
    test_erase();
    test_multi_write_read();
    test_pattern_write_read();
    test_random_write_read();
    test_chip_rollover();

    print("\nDone all memory tests\n\n\n");
}
