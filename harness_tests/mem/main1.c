/*
 * Harness based test for the OBC flash memory.
 *
 * Tests functionality of writing and reading memory at the byte level, writing to and reading from eeprom,
 * writing and reading headers and fields from memory sections 	
 */

#include <test/test.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/mem.h"

void basic_test(void) {
    ASSERT_EQ(1 + 1, 2);
}

//Test single write/read for consistency
void single_write_read_test(void) {
	uint32_t address = 0x0ADF00;
	uint8_t data = 0xDD;
	uint8_t write[1] = {data};
	uint8_t read[1] = {0x00};

	write_mem_bytes(address, write, 1);
	read_mem_bytes(address, read, 1);
	ASSERT_EQ(data, read[1]);
}

//Test multiple write/read for consistency
void multiple_write_read_test(void) {
	uint32_t address = 0x05ABEEF;
	uint8_t data[5] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
	uint8_t read[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

	write_mem_bytes(address, data, 5);
	read_mem_bytes(address, read, 5)

	for(uint8_t i=0; i<5; i++){
		ASSERT_EQ(data[i], read[i]);
	}
}

//More read/write tests for different address
void multiple_write_read_test_2(void) {
	uint32_t address = 0x300000;

	uint8_t data[20];
	for(uint8_t i=0; i<20; i++)
		data[i] = i%16;

	uint8_t read[20];
	for(uint8_t i=0; i<20; i++)
		data[i] = 0x00;

	write_mem_bytes(address, data, 20);
	read_mem_bytes(address, read, 15);

	for(uint8_t i=0; i<15; i++){
		ASSERT_EQ(data[i], read[i]);
	}

	//read shouldn't touch the last 5 bytes in the read array
	for(uint8_t i=15; i<20; i++){
		ASSERT_EQ(0x00, read[i]);
	}
}

// Test the erase_mem()
// NOTE: should change this seed periodically
#define ERASE_SEED          0x162FAF13
#define ERASE_ADDR_COUNT    20

void erase_mem_test(void) {
	erase_mem();
	// take a random sample of the memory to see if erase works
	// values should all be 0xff
	srandom(ERASE_SEED);
	uint32_t num_addrs = MEM_NUM_CHIPS * (1UL << MEM_CHIP_ADDR_WIDTH);

	for(uint8_t i = 0; i < ERASE_ADDR_COUNT; i++){
		uint32_t addr = random() % num_addrs;
		uint8_t read[1] = { 0x00 };
		read_mem_bytes(addr, read, 1);
		ASSERT_EQ(read[0], 0xFF);	
	}
}

// Test random read and write capabilities
#define RANDOM_SEED 0x5729AB7D
#define RANDOM_MAX_LEN 255

void random_read_write_test(void) {
	srandom(RANDOM_SEED);
	uint32_t num_addrs = MEM_NUM_CHIPS * (1UL << MEM_CHIP_ADDR_WIDTH);
	uint32_t addr = random() % num_addrs;

	uint8_t len = random() % RANDOM_ADDR_COUNT;
	uint8_t write[RANDOM_MAX_LEN];
	for(uint8_t i = 0; i<len ; i++) {
		write[i] = random() % 0XFF;
	}

	uint8_t read[RANDOM_MAX_LEN];

	write_mem_bytes(addr, write, len);
	read_mem_bytes(addr, read, len);

	for(uint8_t i=0; i<len; i++) {
		ASSERT_EQ(read[i], write[i]);
	}
}

// Test memory roll over capabilities. Board has three memory chips, so three roll overs
#define ROLLOVER_ADDR_1 0x1FFFFC
#define ROLLOVER_ADDR_2 0x3FFFFB
#define ROLLOVER_ADDR_3 0x5FFFFA
#define NUM_ROLLOVER 3
#define ROLLOVER_DATA {0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xF0, 0x0D, 0x12}
#define ROLLOVER_DATA_LEN 8

void roll_over_test(void) {
	uint32_t addr[NUM_ROLLOVER] = {ROLLOVER_ADDR_1, ROLLOVER_ADDR_2, ROLLOVER_ADDR_3};
	uint8_t write[ROLLOVER_DATA_LEN] = ROLLOVER_DATA;
	uint8_t read[ROLLOVER_DATA_LEN];

	for(uint8_t i=0; i<NUM_ROLLOVER; i++){
		write_mem_bytes(addr[i], write, ROLLOVER_DATA_LEN);
		read_mem_bytes(addr[i], read, ROLLOVER_DATA_LEN);
		for(uint8_t j=0; j<ROLLOVER_DATA_LEN; j++){
			ASSERT_EQ(write[j], read[j]);
		}
	}
}

// EEPROM test
void eeprom_test(void) {
	// load all section data from eeprom
	read_all_mem_sections_eeprom();

	uint32_t eps_hk_block_prev = eeprom_read_dword(&eps_hk_mem_section);
	uint32_t pay_hk_block_prev = eeprom_read_dword(&pay_hk_mem_section);
	uint32_t pay_opt_block_prev = eeprom_read_dword(&pay_opt_mem_section);

	inc_mem_section_curr_block(&eps_hk_mem_section);
    inc_mem_section_curr_block(&pay_hk_mem_section);
    inc_mem_section_curr_block(&pay_opt_mem_section);

    write_all_mem_sections_eeprom();

    ASSERT_EQ(eps_hk_block_prev + 1, eeprom_read_dword(&eps_hk_mem_section));
    ASSERT_EQ(pay_hk_block_prev + 1, eeprom_read_dword(&pay_hk_mem_section));
    ASSERT_EQ(pay_opt_block_prev + 1, eeprom_read_dword(&pay_opt_mem_section));
}


// Test headers for each of the mem_sections (metadata)
void mem_header_test(void){
	mem_header_test_individual(&eps_hk_mem_section);
	mem_header_test_individual(&pay_hk_mem_section);
	mem_header_test_individual(&pay_opt_mem_section);
}

void mem_header_test_individual( mem_section_t* section ) {
	// eps_hk_mem_section
	mem_header_t write = {
        .block_num = section->curr_block,
        .error = 0x00,
        .date = read_date(),
        .time = read_time()
    };
    write_mem_header(section, section->curr_block, &write);

    mem_header_t read;
    read_mem_header(section, section->curr_block, &read);

    ASSERT_EQ(write.block_num, read.block_num);
    ASSERT_EQ(write.error, read.error);
    ASSERT_EQ(write.date, read.date);
    ASSERT_EQ(write.time, read.time);
}

//Test field (metadata)
#define FIELD_TEST_RANDOM_SEED 0x4357D43A

void mem_field_test(void) {
	srandom(FIELD_TEST_RANDOM_SEED);
	mem_field_test_individual(&eps_hk_mem_section);
	mem_field_test_individual(&pay_hk_mem_section);
	mem_field_test_individual(&pay_opt_mem_section);
}

void mem_field_test_individual( mem_section_t* section) {
	uint8_t field_num = random() % section->fields_per_block;
	uint32_t write = (random() % 0xFFFFFFFF) + 1;
	uint32_t read = 0x00000000;

	write_mem_field(section, section->curr_block, field_num, write);
	read = read_mem_field(section, section->curr_block, field_num);

	ASSERT_EQ(write, read);
}	

test_t t1 = { .name = "Basic", .fn = basic_test };
test_t t2 = { .name = "single write read test", .fn = single_write_read_test };
test_t t3 = { .name = "multiple write read test", .fn = multiple_write_read_test };
test_t t4 = { .name = "multiple write read test 2", .fn = multiple_write_read_test_2 };
test_t t5 = { .name = "erase mem test", .fn = erase_mem_test };
test_t t6 = { .name = "random read write test", .fn = random_read_write_test };
test_t t7 = { .name = "rollover test", .fn = roll_over_test };
test_t t8 = { .name = "eeprom test", .fn = eeprom_test };
test_t t9 = { .name = "mem header test", .fn = mem_header_test };
test_t t10 = { .name = "mem field test", .fn = mem_field_test };

test_t* suite[10] = { &t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9, &t10 };

int main() {
    init_uart();
    init_spi();
    init_mem();
    init_rtc();

    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
