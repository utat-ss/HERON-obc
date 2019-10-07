/*
 * Harness based test for the OBC flash memory.
 *
 * Tests functionality of writing and reading memory at the byte level, writing to and reading from eeprom,
 * writing and reading headers and fields from memory sections.
 *
 * NOTE: See mem.c notes on flash memory architecture - generally need to do an erase before every test function
 *
 * Most tests check that the memory is all 1s before doing a write - make sure the data isn't just left over from a previous run of the test
 */

#include <stdlib.h>

#include <test/test.h>
#include <uart/uart.h>
#include <spi/spi.h>
#include "../../src/mem.h"

#define ERASE_ADDR_COUNT        20
#define DATA_LENGTH             5
#define RANDOM_MAX_LEN          255
#define ROLLOVER_ADDR_1         0x1FFFFC
#define ROLLOVER_ADDR_2         0x3FFFFB
#define NUM_ROLLOVER            2
#define ROLLOVER_DATA           {0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xF0, 0x0D, 0x12}
#define ROLLOVER_DATA_LEN       8

// Macros to compare structs without having to do it for each of the fields every time
#define ASSERT_EQ_DATE(date1, date2) \
    ASSERT_EQ(date1.yy, date2.yy); \
    ASSERT_EQ(date1.mm, date2.mm); \
    ASSERT_EQ(date1.dd, date2.dd);

#define ASSERT_EQ_TIME(time1, time2) \
    ASSERT_EQ(time1.hh, time2.hh); \
    ASSERT_EQ(time1.mm, time2.mm); \
    ASSERT_EQ(time1.ss, time2.ss);

#define ASSERT_EQ_ARRAY(array1, array2, count) \
    for (uint8_t i = 0; i < count; i++) { \
        ASSERT_EQ((array1)[i], (array2)[i]); \
    }

#define ASSERT_NEQ_ARRAY(array1, array2, count) \
    for (uint8_t i = 0; i < count; i++) { \
        ASSERT_NEQ((array1)[i], (array2)[i]); \
    }


rtc_date_t rand_rtc_date(void) {
    rtc_date_t date = {
        .yy = random() & 0xFF,
        .mm = random() & 0xFF,
        .dd = random() & 0xFF
    };
    return date;
}

rtc_time_t rand_rtc_time(void) {
    rtc_time_t time = {
        .hh = random() & 0xFF,
        .mm = random() & 0xFF,
        .ss = random() & 0xFF
    };
    return time;
}

void populate_ones(uint8_t* array, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        array[i] = 0xFF;
    }
}




// Test the erase_mem()

void erase_mem_test(void) {
	erase_mem();
	// take a random sample of the memory to see if erase works
	// values should all be 0xff
	uint32_t num_addrs = MEM_NUM_CHIPS * (1UL << MEM_CHIP_ADDR_WIDTH);

	for(uint8_t i = 0; i < ERASE_ADDR_COUNT; i++){
		uint32_t addr = random() % num_addrs;
		uint8_t read[1] = { 0x00 };
		read_mem_bytes(addr, read, 1);
		ASSERT_EQ(read[0], 0xFF);
	}
}


//Test single write/read for consistency
void single_write_read_test(void) {
    erase_mem();

	uint32_t address = 0x0ADF00;
	uint8_t data = 0xDD;
	uint8_t write[1] = {data};
	uint8_t read[1] = {0x00};

    uint8_t ones[1];
    populate_ones(ones, 1);
    read_mem_bytes(address, read, 1);
	ASSERT_EQ_ARRAY(ones, read, 1);

    write_mem_bytes(address, write, 1);
	read_mem_bytes(address, read, 1);
	ASSERT_EQ_ARRAY(write, read, 1);
}

//Test multiple write/read for consistency
void multiple_write_read_test(void) {
    erase_mem();

	uint32_t address = 0x05ABEEF;
	uint8_t data[5] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
	uint8_t read[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

    uint8_t ones[5];
    populate_ones(ones, 5);
    read_mem_bytes(address, read, 5);
	ASSERT_EQ_ARRAY(ones, read, 5);

	write_mem_bytes(address, data, 5);
	read_mem_bytes(address, read, 5);

	for(uint8_t i=0; i<5; i++){
		ASSERT_EQ(data[i], read[i]);
	}
}

//More read/write tests for different address
void multiple_write_read_test_2(void) {
    erase_mem();

	uint32_t address = 0x300000;

	uint8_t write[20];
	for(uint8_t i=0; i<20; i++)
		write[i] = i%16;

	uint8_t read[20];
	for(uint8_t i=0; i<20; i++)
		read[i] = 0x00;

    uint8_t ones[20];
    populate_ones(ones, 20);
    read_mem_bytes(address, read, 20);
	ASSERT_EQ_ARRAY(ones, read, 20);

	write_mem_bytes(address, write, 20);
	read_mem_bytes(address, read, 15);

    ASSERT_EQ_ARRAY(write, read, 15);
	//read shouldn't touch the last 5 bytes in the read array
    ASSERT_EQ_ARRAY(&ones[15], &read[15], 5);
}


// Test random read and write capabilities

void random_read_write_test(void) {
    erase_mem();

	uint32_t num_addrs = MEM_NUM_CHIPS * (1UL << MEM_CHIP_ADDR_WIDTH);
	uint32_t addr = random() % num_addrs;

    // Use a random number of bytes
	uint8_t len = (random() % RANDOM_MAX_LEN) + 1;
    // Array needs to be big enough to hold the maximum number of bytes
	uint8_t write[RANDOM_MAX_LEN];
	for(uint8_t i = 0; i<len ; i++) {
		write[i] = random() % 0XFF;
	}

	uint8_t read[RANDOM_MAX_LEN];

    uint8_t ones[RANDOM_MAX_LEN];
    populate_ones(ones, RANDOM_MAX_LEN);
    read_mem_bytes(addr, read, RANDOM_MAX_LEN);
	ASSERT_EQ_ARRAY(ones, read, RANDOM_MAX_LEN);

	write_mem_bytes(addr, write, len);
	read_mem_bytes(addr, read, len);

	for(uint8_t i=0; i<len; i++) {
		ASSERT_EQ(read[i], write[i]);
	}
}

// Test memory roll over capabilities. Board has three memory chips, so two roll overs
// TODO - test what happens at the end of chip 3

void roll_over_test(void) {
    erase_mem();

	uint32_t addr[NUM_ROLLOVER] = {ROLLOVER_ADDR_1, ROLLOVER_ADDR_2};
	uint8_t write[ROLLOVER_DATA_LEN] = ROLLOVER_DATA;
	uint8_t read[ROLLOVER_DATA_LEN];

    uint8_t ones[ROLLOVER_DATA_LEN];
    populate_ones(ones, ROLLOVER_DATA_LEN);

	for(uint8_t i=0; i<NUM_ROLLOVER; i++){
        read_mem_bytes(addr[i], read, ROLLOVER_DATA_LEN);
        ASSERT_EQ_ARRAY(ones, read, ROLLOVER_DATA_LEN);

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

	uint32_t eps_hk_block_prev = read_eeprom_or_default(eps_hk_mem_section.curr_block_eeprom_addr, 0);
	uint32_t pay_hk_block_prev = read_eeprom_or_default(pay_hk_mem_section.curr_block_eeprom_addr, 0);
	uint32_t pay_opt_block_prev = read_eeprom_or_default(pay_opt_mem_section.curr_block_eeprom_addr, 0);

	set_mem_section_curr_block(&eps_hk_mem_section, eps_hk_mem_section.curr_block + 1);
    set_mem_section_curr_block(&pay_hk_mem_section, pay_hk_mem_section.curr_block + 1);
    set_mem_section_curr_block(&pay_opt_mem_section, pay_opt_mem_section.curr_block + 1);

    ASSERT_EQ(eps_hk_block_prev + 1, read_eeprom(eps_hk_mem_section.curr_block_eeprom_addr));
    ASSERT_EQ(pay_hk_block_prev + 1, read_eeprom(pay_hk_mem_section.curr_block_eeprom_addr));
    ASSERT_EQ(pay_opt_block_prev + 1, read_eeprom(pay_opt_mem_section.curr_block_eeprom_addr));
}


// Test headers for each of the mem_sections (metadata)
void mem_header_test_individual( mem_section_t* section ) {
    erase_mem();

	// eps_hk_mem_section
	mem_header_t write = {
        .block_num = section->curr_block,
        .date = rand_rtc_date(),
        .time = rand_rtc_time(),
        .status = 0x00,
    };
    write_mem_header(section, section->curr_block, &write);

    mem_header_t read;
    read_mem_header(section, section->curr_block, &read);

    ASSERT_EQ(write.block_num, read.block_num);
    ASSERT_EQ(write.status, read.status);
    ASSERT_EQ_DATE(write.date, read.date);
    ASSERT_EQ_TIME(write.time, read.time);
}

void mem_header_test(void){
	mem_header_test_individual(&eps_hk_mem_section);
	mem_header_test_individual(&pay_hk_mem_section);
	mem_header_test_individual(&pay_opt_mem_section);
}


//Test field (metadata)
void mem_field_test_individual( mem_section_t* section) {
    erase_mem();

	uint8_t field_num = random() % section->fields_per_block;
    // Random 24-bit number
	uint32_t write = (random() % 0xFFFFFF) + 1;
	uint32_t read = 0x00000000;

    read = read_mem_field(section, section->curr_block, field_num);
	ASSERT_EQ(0xFFFFFF, read);

	write_mem_field(section, section->curr_block, field_num, write);
	read = read_mem_field(section, section->curr_block, field_num);
	ASSERT_EQ(write, read);
}

void mem_field_test(void) {
	mem_field_test_individual(&eps_hk_mem_section);
	mem_field_test_individual(&pay_hk_mem_section);
	mem_field_test_individual(&pay_opt_mem_section);
}

//test blocks
void mem_block_test_1(void){
    erase_mem();

    uint32_t write_fields_1[obc_hk_mem_section.fields_per_block];
    uint32_t write_fields_2[eps_hk_mem_section.fields_per_block];
    uint32_t write_fields_3[pay_hk_mem_section.fields_per_block];
    uint32_t write_fields_4[pay_opt_mem_section.fields_per_block];
    //random 32 bit numbers
    for (int a = 0; a < obc_hk_mem_section.fields_per_block; a++){
        write_fields_1[a] = (random() & 0xFFFFFF);
    }
    for (int a = 0; a < eps_hk_mem_section.fields_per_block; a++){
        write_fields_2[a] = (random() & 0xFFFFFF);
    }
    for (int a = 0; a < pay_hk_mem_section.fields_per_block; a++){
        write_fields_3[a] = (random() & 0xFFFFFF);
    }
    for (int a = 0; a < pay_opt_mem_section.fields_per_block; a++){
        write_fields_4[a] = (random() & 0xFFFFFF);
    }
    uint32_t* write_test_fields[4] = {write_fields_1, write_fields_2, write_fields_3, write_fields_4};

    mem_header_t write_header[4];

    uint32_t block_num[4];
    //write to all sections
    for (int i = 0; i < 4; i++){
        mem_section_t* section = all_mem_sections[i];
        section->curr_block = 1;////////////////
        block_num[i] = section->curr_block;
        (write_header[i]).block_num = section->curr_block;
        (write_header[i]).date = rand_rtc_date();
        (write_header[i]).time = rand_rtc_time();
        (write_header[i]).status = 0x00;
        uint32_t prev_block = section->curr_block;
        write_mem_data_block(section, block_num[i], &(write_header[i]), write_test_fields[i]);
        ASSERT_EQ(prev_block, block_num[i]);///////////////
    }

    uint32_t read_fields_1[obc_hk_mem_section.fields_per_block];
    uint32_t read_fields_2[eps_hk_mem_section.fields_per_block];
    uint32_t read_fields_3[pay_hk_mem_section.fields_per_block];
    uint32_t read_fields_4[pay_opt_mem_section.fields_per_block];
    uint32_t* read_test_fields[4] = {read_fields_1, read_fields_2, read_fields_3, read_fields_4};

    mem_header_t read_header[4];
    uint32_t read_block_num[4];

    //read from all sections
    for (int i = 0; i < 4; i++){
        mem_section_t* section = all_mem_sections[i];
        section->curr_block = 1;////////////////////
        read_block_num[i] = section->curr_block;
        read_mem_data_block(section, read_block_num[i], &(read_header[i]), read_test_fields[i]);
    }

    for (int i=0; i<4; i++){
        ASSERT_EQ(block_num[i], read_block_num[i]);
    }

    //check headers
    for (int i = 0; i < 4; i++){
        ASSERT_EQ((write_header[i]).block_num, (read_header[i]).block_num); //this should be the case right.
        ASSERT_EQ_DATE((write_header[i]).date, (read_header[i]).date);
        ASSERT_EQ_TIME((write_header[i]).time, (read_header[i]).time);
        ASSERT_EQ((write_header[i]).status, (read_header[i]).status);
    }

    //check fields
    for (uint8_t a = 0; a < obc_hk_mem_section.fields_per_block; a++){
        ASSERT_EQ(write_fields_1[a], read_fields_1[a]);
    }
    for (uint8_t a = 0; a < eps_hk_mem_section.fields_per_block; a++){
        ASSERT_EQ(write_fields_2[a], read_fields_2[a]);
    }
    for (uint8_t a = 0; a < pay_hk_mem_section.fields_per_block; a++){
        ASSERT_EQ(write_fields_3[a], read_fields_3[a]);
    }
    for (uint8_t a = 0; a < pay_opt_mem_section.fields_per_block; a++){
        ASSERT_EQ(write_fields_4[a], read_fields_4[a]);
    }
}

//actually test blocks
void mem_block_test_2(void){
    erase_mem();

    // TODO - add OBC_HK

    uint32_t write_fields_1[eps_hk_mem_section.fields_per_block];
    uint32_t write_fields_2[pay_hk_mem_section.fields_per_block];
    uint32_t write_fields_3[pay_opt_mem_section.fields_per_block];

    uint32_t read_fields_1[eps_hk_mem_section.fields_per_block];
    uint32_t read_fields_2[pay_hk_mem_section.fields_per_block];
    uint32_t read_fields_3[pay_opt_mem_section.fields_per_block];

    //populate with random data
    for (uint8_t i=0; i<eps_hk_mem_section.fields_per_block; i++){
        write_fields_1[i] = 0x765432;
        read_fields_1[i] = 0x234567;
    }
    for (uint8_t i=0; i<pay_hk_mem_section.fields_per_block; i++){
        write_fields_2[i] = 0x765432;
        read_fields_2[i] = 0x234567;
    }
    for (uint8_t i=0; i<pay_opt_mem_section.fields_per_block; i++){
        write_fields_3[i] = 0x765432;
        read_fields_3[i] = 0x234567;
    }

    //test eps housekeeping
    mem_section_t* section = all_mem_sections[1];
    section->curr_block = 0;//

    uint32_t block_num = section->curr_block;
    mem_header_t write_header;
    mem_header_t read_header;
    write_header.block_num = section->curr_block;
    write_header.date = rand_rtc_date();
    write_header.time = rand_rtc_time();
    write_header.status = 0x00;

    write_mem_data_block(section, block_num, &write_header, write_fields_1);
    ASSERT_EQ(block_num,0);
    read_mem_data_block(section, block_num, &read_header, read_fields_1);
    ASSERT_EQ(block_num,0);

    ASSERT_EQ(write_header.block_num, read_header.block_num);
    ASSERT_EQ(write_header.status, read_header.status);

    for (uint8_t i=0; i<eps_hk_mem_section.fields_per_block; i++){
        ASSERT_EQ(write_fields_1[i], read_fields_1[i]);
    }

    //test pay housekeeping
    section = all_mem_sections[2];
    section->curr_block = 0;//

    block_num = section->curr_block;
    write_header.block_num = section->curr_block;
    write_header.date = rand_rtc_date();
    write_header.time = rand_rtc_time();
    write_header.status = 0x00;

    write_mem_data_block(section, block_num, &write_header, write_fields_2);
    ASSERT_EQ(block_num,0);
    read_mem_data_block(section, block_num, &read_header, read_fields_2);
    ASSERT_EQ(block_num,0);

    ASSERT_EQ(write_header.block_num, read_header.block_num);
    ASSERT_EQ(write_header.status, read_header.status);

    for (uint8_t i=0; i<pay_hk_mem_section.fields_per_block; i++){
        ASSERT_EQ(write_fields_2[i], read_fields_2[i]);
    }

    //test pay optical
    section = all_mem_sections[3];
    section->curr_block = 0;//

    block_num = section->curr_block;
    write_header.block_num = section->curr_block;
    write_header.date = rand_rtc_date();
    write_header.time = rand_rtc_time();
    write_header.status = 0x00;

    write_mem_data_block(section, block_num, &write_header, write_fields_3);
    ASSERT_EQ(block_num,0);
    read_mem_data_block(section, block_num, &read_header, read_fields_3);
    ASSERT_EQ(block_num,0);

    ASSERT_EQ(write_header.block_num, read_header.block_num);
    ASSERT_EQ(write_header.status, read_header.status);

    for (uint8_t i=0; i<pay_opt_mem_section.fields_per_block; i++){
        ASSERT_EQ(write_fields_3[i], read_fields_3[i]);
    }
}

void section_byte_isolation_test(void) {
    erase_mem();

    uint8_t ones[10];
    populate_ones(ones, 10);

    // Write PAY_HK

    uint8_t write_pay_hk[5] = {0x00};
    for (uint8_t i = 0; i < 5; i++) {
        write_pay_hk[i] = i + 50;
    }
    uint8_t read_pay_hk[5] = {0x00};

    read_mem_section_bytes(&pay_hk_mem_section, 0, read_pay_hk, 5);
    ASSERT_EQ_ARRAY(read_pay_hk, ones, 5);

    ASSERT_TRUE(write_mem_section_bytes(&pay_hk_mem_section, 0, write_pay_hk, 5));
    read_mem_section_bytes(&pay_hk_mem_section, 0, read_pay_hk, 5);
    ASSERT_EQ_ARRAY(read_pay_hk, write_pay_hk, 5);


    // Write EPS_HK, don't overwrite PAY_HK

    // Address offset from beginning of EPS_HK, should be 0x0FFFFB
    uint32_t eps_hk_section_addr = eps_hk_mem_section.end_addr - eps_hk_mem_section.start_addr - 4;
    uint8_t write_eps_hk[10] = {0x00};
    for (uint8_t i = 0; i < 10; i++) {
        write_eps_hk[i] = i + 13;
    }
    uint8_t read_eps_hk[10] = {0x00};

    read_mem_section_bytes(&eps_hk_mem_section, eps_hk_section_addr, read_eps_hk, 5);
    ASSERT_EQ_ARRAY(read_eps_hk, ones, 5);

    // TODO - probably need to erase in between writes
    ASSERT_FALSE(write_mem_section_bytes(&eps_hk_mem_section, eps_hk_section_addr, write_eps_hk, 10));
    ASSERT_FALSE(write_mem_section_bytes(&eps_hk_mem_section, eps_hk_section_addr, write_eps_hk, 6));
    ASSERT_TRUE(write_mem_section_bytes(&eps_hk_mem_section, eps_hk_section_addr, write_eps_hk, 5));
    read_mem_section_bytes(&eps_hk_mem_section, eps_hk_section_addr, read_eps_hk, 5);
    ASSERT_EQ_ARRAY(read_eps_hk, write_eps_hk, 5);

    // Check that PAY_HK is unchanged
    read_mem_section_bytes(&pay_hk_mem_section, 0, read_pay_hk, 5);
    ASSERT_EQ_ARRAY(read_pay_hk, write_pay_hk, 5);
}

void cmd_block_test(void) {
    erase_mem();

    // eps_hk_mem_section
	mem_header_t write_header = {
        .block_num = prim_cmd_log_mem_section.curr_block,
        .date = rand_rtc_date(),
        .time = rand_rtc_time(),
        .status = 0x00,
    };
    mem_header_t read_header;

    uint32_t block_num;

    uint8_t write_cmd_num;
    uint32_t write_arg1;
    uint32_t write_arg2;

    uint8_t read_cmd_num;
    uint32_t read_arg1;
    uint32_t read_arg2;

    block_num = 13542;
    write_cmd_num = 5;
    write_arg1 = 1000000000;
    write_arg2 = 132497;
    ASSERT_TRUE(write_mem_cmd_block(&prim_cmd_log_mem_section, block_num, &write_header, write_cmd_num, write_arg1, write_arg2));

    read_mem_cmd_block(&prim_cmd_log_mem_section, block_num, &read_header, &read_cmd_num, &read_arg1, &read_arg2);
    ASSERT_EQ(write_header.block_num, read_header.block_num);
    ASSERT_EQ(write_header.status, read_header.status);
    ASSERT_EQ_DATE(write_header.date, read_header.date);
    ASSERT_EQ_TIME(write_header.time, read_header.time);
    ASSERT_EQ(write_cmd_num, read_cmd_num);
    ASSERT_EQ(write_arg1, read_arg1);
    ASSERT_EQ(write_arg2, read_arg2);

    block_num = 1000000;
    ASSERT_FALSE(write_mem_cmd_block(&prim_cmd_log_mem_section, block_num, &write_header, write_cmd_num, write_arg1, write_arg2));
}

/* Test the ability to erase a 4kb sector of memory given an address */
void mem_sector_erase_test(void){
    uint8_t data[DATA_LENGTH] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    uint8_t read[1] = {0};

    /* Generate random address by seeding and calling random */
    uint32_t address = random() % MEM_NUM_ADDRESSES;
    /* Write to location in sector and verify that write worked */
    write_mem_bytes(address, data, DATA_LENGTH);
    for (uint32_t i = address; i < address + DATA_LENGTH; i++){
        read_mem_bytes(i, read, 1);
        ASSERT_EQ(read[0], data[i-address]);
    }

    /* Erase sector */
    erase_mem_sector(address);
    /* Read written bits in sector and verify that bits are all one */
    for (uint32_t i = address; i < address + DATA_LENGTH; i++){
        read_mem_bytes(i, read, 1);
        ASSERT_EQ(read[0], 0xFF);
    }
}

/* Test the ability to erase a block of memory given an address */
void mem_block_erase_test(void){
    uint8_t data[DATA_LENGTH] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    uint8_t read[1] = {0};

    uint32_t address = random() % MEM_NUM_ADDRESSES;

    /* Write to location in block and verify that write worked */
    write_mem_bytes(address, data, DATA_LENGTH);
    for (uint32_t i = address; i < address + DATA_LENGTH; i++){
        read_mem_bytes(i, read, 1);
        ASSERT_EQ(read[0], data[i-address]);
    }

    /* Erase block */
    erase_mem_block(address);
    /* Read written bits in block and verify that bits are all one */
    for (uint32_t i = address; i < address + DATA_LENGTH; i++){
        read_mem_bytes(i, read, 1);
        ASSERT_EQ(read[0], 0xFF);
    }
}



test_t t1 = { .name = "erase mem test", .fn = erase_mem_test };
test_t t2 = { .name = "single write read test", .fn = single_write_read_test };
test_t t3 = { .name = "multiple write read test", .fn = multiple_write_read_test };
test_t t4 = { .name = "multiple write read test 2", .fn = multiple_write_read_test_2 };
test_t t5 = { .name = "random read write test", .fn = random_read_write_test };
test_t t6 = { .name = "rollover test", .fn = roll_over_test };
test_t t7 = { .name = "eeprom test", .fn = eeprom_test };
test_t t8 = { .name = "mem header test", .fn = mem_header_test };
test_t t9 = { .name = "mem field test", .fn = mem_field_test };
test_t t10 = { .name = "mem block test 1", .fn = mem_block_test_1 };
test_t t11 = { .name = "mem block test 2", .fn = mem_block_test_2 };
test_t t12 = { .name = "section byte isolation test", .fn = section_byte_isolation_test };
test_t t13 = { .name = "cmd block test", .fn = cmd_block_test };
test_t t14 = { .name = "sector erase test", .fn = mem_sector_erase_test };
test_t t15 = { .name = "block erase test", .fn = mem_block_erase_test };

test_t* suite[] = { &t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9, &t10, &t11, &t12, &t13, &t14, &t15 };

int main(void) {
    init_uart();
    init_spi();
    init_mem();

    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
