/*
Flash memory Library
SST26VF016B
Datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/20005262D.pdf

This library controls interfacing with the 3 flash memory chips connected to the OBC microcontroller.

The organization of data memory follows the OBC Flash Memory Protocol:
https://utat-ss.readthedocs.io/en/master/our-protocols/obc-mem.html

Addresses are composed as follows (as uint32_t):
{ 0 (9 bits), chip_num (2 bits), chip_addr (21 bits)}

TODO - make sure EEPROM addresses don't conflict with heartbeat
TODO - develop harness-based test
*/

#include "mem.h"


// Chip selects for each of the memory chips
pin_info_t mem_cs[MEM_NUM_CHIPS] = {
    {
        .port = &MEM_CS_PORT,
        .ddr = &MEM_CS_DDR,
        .pin = MEM_CHIP0_CS_PIN
    },

    {
        .port = &MEM_CS_PORT,
        .ddr = &MEM_CS_DDR,
        .pin = MEM_CHIP1_CS_PIN
    },

    {
        .port = &MEM_CS_PORT,
        .ddr = &MEM_CS_DDR,
        .pin = MEM_CHIP2_CS_PIN
    }
};


mem_section_t eps_hk_mem_section = {
    .start_addr = 0x0DB00UL,
    .curr_block = 0,
    .curr_block_eeprom_addr = (uint32_t*) 0x20,
    .fields_per_block = CAN_EPS_HK_FIELD_COUNT   // Should be 12
};

mem_section_t pay_hk_mem_section = {
    .start_addr = 0x100000UL,
    .curr_block = 0,
    .curr_block_eeprom_addr = (uint32_t*) 0x24,
    .fields_per_block = CAN_PAY_HK_FIELD_COUNT   // Should be 3
};

mem_section_t pay_sci_mem_section = {
    .start_addr = 0x200000UL,
    .curr_block = 0,
    .curr_block_eeprom_addr = (uint32_t*) 0x28,
    .fields_per_block = CAN_PAY_SCI_FIELD_COUNT   // Should be 33
};

// All memory sections
mem_section_t* all_mem_sections[MEM_NUM_SECTIONS] = {
    &eps_hk_mem_section,
    &pay_hk_mem_section,
    &pay_sci_mem_section
};




void init_mem(void){
/*
    intializes the chip select pin, unlocks (un-write protects)
    NOTE: this function DOES NOT read from EEPROM - need to call
    write_all_mem_sections_eeprom() and read_all_mem_sections_eeprom() separately
*/
    // initialize the Chip Select pins
    for (uint8_t i = 0; i < MEM_NUM_CHIPS; i++) {
        init_cs(mem_cs[i].pin, mem_cs[i].ddr);
        set_cs_high(mem_cs[i].pin, mem_cs[i].port);
    }

    unlock_mem(); //use global unlock to unlock memory
}


void write_mem_section_eeprom(mem_section_t* section) {
    /*
    writes the current block number of `section` to its designated address in EEPROM
    */
    eeprom_write_dword (section->curr_block_eeprom_addr, section->curr_block);
}

/*
Writes data for all memory sections to EEPROM.
*/
void write_all_mem_sections_eeprom(void) {
    write_mem_section_eeprom(&eps_hk_mem_section);
    write_mem_section_eeprom(&pay_hk_mem_section);
    write_mem_section_eeprom(&pay_sci_mem_section);
}


void read_mem_section_eeprom(mem_section_t* section) {
    /*
    reads the current block number from its designated address in EEPROM and stores it in `section`
    */
    section->curr_block = eeprom_read_dword (section->curr_block_eeprom_addr);
}

/*
Reads data for all memory sections from EEPROM.
*/
void read_all_mem_sections_eeprom(void) {
    read_mem_section_eeprom(&eps_hk_mem_section);
    read_mem_section_eeprom(&pay_hk_mem_section);
    read_mem_section_eeprom(&pay_sci_mem_section);
}


void inc_mem_section_curr_block(mem_section_t* section) {
    /*
    Increments the section's current block number by 1.
    NOTE: this DOES NOT update the value stored in EEPROM
    */
    section->curr_block++;
}




void write_mem_header(mem_section_t* section, uint8_t block_num,
    mem_header_t* header) {

    /*
    writes the header information array (which contains metadata such as
    block number, timestamp, and error codes) into the current block of the
        section
    */

    uint8_t bytes[MEM_BYTES_PER_HEADER] = {
        header->block_num,
        header->error,
        header->date.yy, header->date.mm, header->date.dd,
        header->time.hh, header->time.mm, header->time.ss
    };

    write_mem_bytes(mem_block_addr(section, block_num), bytes, MEM_BYTES_PER_HEADER);
}


/*
Reads the header data for the specified block number.
block_num - the expected block number (determines the address to start reading
    from) - expected to be equal to header->block_num set by this function
header - will be set by this function to the results
*/
void read_mem_header(mem_section_t* section, uint8_t block_num,
    mem_header_t* header) {

    uint8_t bytes[MEM_BYTES_PER_HEADER];
    read_mem_bytes(mem_block_addr(section, block_num), bytes,
        MEM_BYTES_PER_HEADER);

    header->block_num = bytes[0];
    header->error = bytes[1];
    header->date.yy = bytes[2];
    header->date.mm = bytes[3];
    header->date.dd = bytes[4];
    header->time.hh = bytes[5];
    header->time.mm = bytes[6];
    header->time.ss = bytes[7];
}


// fields are indexed from ZERO
void write_mem_field(mem_section_t* section, uint32_t block_num,
        uint8_t field_num, uint32_t data) {
/*
    writes a field (3 bytes) to flash, and automatically creates a new
    block in the stack whenever field zero is reached

    fields are indexed from zero and blocks are indexed from one

    data - the least significant 24 bits will be written to memory
*/

    uint32_t address = mem_field_addr(section, block_num, field_num);

    // Split the data into 3 bytes
    uint8_t data_bytes[MEM_BYTES_PER_FIELD] = {
        (data >> 16) & 0xFF,
        (data >> 8) & 0xFF,
        data & 0xFF
    };

    write_mem_bytes(address, data_bytes, MEM_BYTES_PER_FIELD);
}


uint32_t read_mem_field(mem_section_t* section, uint32_t block_num,
        uint8_t field_num) {
/*
    Reads and returns the 24-bit data for the specified section, block, and field
*/

    uint32_t address = mem_field_addr(section, block_num, field_num);

    uint8_t data_bytes[MEM_BYTES_PER_FIELD];
    read_mem_bytes(address, data_bytes, MEM_BYTES_PER_FIELD);

    uint32_t data = 0;
    for (uint8_t i = 0; i < MEM_BYTES_PER_FIELD; ++i) {
        data <<= 8;
        data |= data_bytes[i];
    }
    return data;
}





/*
Calculates and returns the address of the start of a block (where the header starts).
*/
uint32_t mem_block_addr(mem_section_t* section, uint32_t block_num) {
    uint32_t bytes_per_block = MEM_BYTES_PER_HEADER +
        (((uint32_t) section->fields_per_block) * MEM_BYTES_PER_FIELD);
    uint32_t block_address = section->start_addr + (block_num * bytes_per_block);
    return block_address;
}


/*
Calculates and returns the address of the start of a field in a block (after the header).
*/
uint32_t mem_field_addr(mem_section_t* section, uint32_t block_num,
        uint32_t field_num) {
    uint32_t block_address = mem_block_addr(section, block_num);
    uint32_t field_address = block_address +
        MEM_BYTES_PER_HEADER + (field_num * MEM_BYTES_PER_FIELD);
    return field_address;
}


/*
Processes an address (virtual 23-bit address) and calculates (splits up) the
    chip number and physical address on that chip.
address - virtual 23-bit address
chip_num - gets set by this function to the chip number (0 to 2)
addr1 - gets set by this function to the MSB (5 bits) of the physical address
addr2 - gets set by this function to the middle (8 bits) of the physical address
addr3 - gets set by this function to the LSB (8 bits) of the physical address
*/
void process_mem_addr(uint32_t address, uint8_t* chip_num, uint8_t* addr1,
        uint8_t* addr2, uint8_t* addr3) {
    if (chip_num != NULL) {
        *chip_num = (address >> MEM_CHIP_ADDR_WIDTH) & 0x03;
    }
    if (addr1 != NULL) {
        *addr1 = (address >> 16) & 0x1F;
    }
    if (addr2 != NULL) {
        *addr2 = (address >> 8) & 0xFF;
    }
    if (addr3 != NULL) {
        *addr3 = address & 0xFF;
    }
}


void write_mem_bytes(uint32_t address, uint8_t* data, uint8_t data_len){
/*
    writes data to memory starting at the specified address
    data MUST be at least of length data_len
    all data at addresses in the array higher than data_len will be ignored

    two key features:
        overides page-write functionality (which limits write operations to
        256 bytes pages) to allow for continous write operations

        implements automatic roll-over functionality, creating a continous
        address space, combining three memory chips

        the continous roll-over functionality is hard-coded, and will need
        to be modified in the event of changes to the board design
*/

    uint8_t chip_num;
    uint8_t addr1;
    uint8_t addr2;
    uint8_t addr3;
    //calculate the initial chip number
    process_mem_addr(address, &chip_num, &addr1, &addr2, &addr3);

    //initialize counter
    for (uint16_t i = 0; i < data_len; i++) {

        //rollover condition for chip and page boundaries
        if (((address + i) % 256 == 0) || (i == 0)) {
            //end previous write operation
            set_cs_high(mem_cs[chip_num].pin, mem_cs[chip_num].port);

            //used to determine the end of a write operation
            //wait for previous operation to terminate
            wait_for_mem_not_busy(chip_num);

            send_short_mem_command(MEM_WR_DISABLE, chip_num);

            //calculate address bytes
            //recalculate and rollover the chip number,
            //stored in the most significant bits of the address
            process_mem_addr(address + i, &chip_num, &addr1, &addr2, &addr3);
            if (chip_num >= MEM_NUM_CHIPS) {
                chip_num = 0;
            }

            //enable writing to chip
            send_short_mem_command(MEM_WR_ENABLE, chip_num);
            set_cs_low(mem_cs[chip_num].pin, mem_cs[chip_num].port);
            /* all bytes to be written must be
            proceeded by the Page Program command */

            send_spi(MEM_PG_PRG);

            send_spi(addr1);
            send_spi(addr2);
            send_spi(addr3);
        }

        send_spi(data[i]);
    }

    set_cs_high(mem_cs[chip_num].pin, mem_cs[chip_num].port);

    //used to determine the end of a write operation
    wait_for_mem_not_busy(chip_num);

    send_short_mem_command(MEM_WR_DISABLE, chip_num);
}


void read_mem_bytes(uint32_t address, uint8_t* data, uint8_t data_len){
/*
    Reads a continous block of memory of size data_len from the given address
    and places the result in the data array

    key features:
        highest address wrap-around (to zero)

        reads continously across chips (ie behaves as a continous address space)
*/

    uint8_t chip_num;
    uint8_t addr1;
    uint8_t addr2;
    uint8_t addr3;
    process_mem_addr(address, &chip_num, &addr1, &addr2, &addr3);

    set_cs_low(mem_cs[chip_num].pin, mem_cs[chip_num].port);
    send_spi(MEM_R_BYTE);
    send_spi(addr1);
    send_spi(addr2);
    send_spi(addr3);

    for (uint8_t i = 0; i < data_len; i++) {
        // Get the chip number of the next address
        uint8_t new_chip_num;
        process_mem_addr(address + i, &new_chip_num, &addr1, &addr2, &addr3);

        // checks for chip rollover condition (moving to address on new chip)
        if (new_chip_num != chip_num)
        {
            // Stop reading from this chip
            set_cs_high(mem_cs[chip_num].pin, mem_cs[chip_num].port);

            chip_num = new_chip_num;
            //ensure wrap-around back to chip 0
            if (chip_num >= MEM_NUM_CHIPS) {
                chip_num = 0;
            }

            /* Begin read on new chip */
            set_cs_low(mem_cs[chip_num].pin, mem_cs[chip_num].port);
            send_spi(MEM_R_BYTE);
            send_spi(0x00);
            send_spi(0x00);
            send_spi(0x00);
        }

        data[i] = send_spi(0x00);
    }

    set_cs_high(mem_cs[chip_num].pin, mem_cs[chip_num].port);
}

/*
Erases all memory chips.
Erasing is defined as setting all bits to 1 (all bytes to 0xFF).
*/
void erase_mem(void) {
    for (uint8_t i = 0; i < MEM_NUM_CHIPS; i++) {
        erase_mem_chip(i);
    }
}

void erase_mem_chip(uint8_t chip){
/*
    erase the specified memory chip (overwrite all data to ones)
*/
    send_short_mem_command(MEM_WR_ENABLE, chip);
    send_short_mem_command(MEM_ERASE, chip);
    send_short_mem_command(MEM_WR_DISABLE, chip);

    wait_for_mem_not_busy(chip);
}


void unlock_mem(void){
    // send the global mem unlock command to enable write operations
    for(uint8_t i = 0; i < MEM_NUM_CHIPS; i++) {
        send_short_mem_command(MEM_WR_ENABLE, i);
        send_short_mem_command(MEM_UNLOCK, i);
    }
}




/*
Loops until the busy bit in the memory status is not set. Times out after 65,535
    cycles.
*/
void wait_for_mem_not_busy(uint8_t chip_num) {
    uint8_t busy = 1;

    uint16_t timeout;
    for (timeout = UINT16_MAX; busy && (timeout > 0); timeout--) {
        busy = read_mem_status(chip_num) & 0x01;
    }
}

uint8_t read_mem_status(uint8_t chip){
    // read from the status register

    return send_mem_command(MEM_READ_STATUS, 0x00, chip);
}


void write_mem_status(uint8_t status, uint8_t chip){
    // write to the configuration register

    set_cs_low(mem_cs[chip].pin, mem_cs[chip].port);
    send_spi(MEM_WRITE_STATUS);
    send_spi(0x00);
    send_spi(status);
    set_cs_high(mem_cs[chip].pin, mem_cs[chip].port);
}


uint8_t send_mem_command(uint8_t command, uint8_t data, uint8_t chip){
    // send a command with an argument and return value to the device

    uint8_t value;
    set_cs_low(mem_cs[chip].pin, mem_cs[chip].port);
    send_spi(command);
    value = send_spi(data);
    set_cs_high(mem_cs[chip].pin, mem_cs[chip].port);
    return value;
}


void send_short_mem_command(uint8_t command, uint8_t chip){
    // send a command without an argument or return value to the device
    set_cs_low(mem_cs[chip].pin, mem_cs[chip].port);
    send_spi(command);
    set_cs_high(mem_cs[chip].pin, mem_cs[chip].port);
}








// void erase_mem_sector(uint8_t sector, uint8_t chip){
//     /*
//     erase a specific sector in memory, on indicated chip
//     functionality not used in the higher-level implementation
//     UNTESTED!!
//     */
//
//     uint32_t address = sector * 4096;
//
//     send_short_mem_command(MEM_WR_ENABLE, chip);
//
//     set_cs_low(mem_cs[chip].pin, mem_cs[chip].port);
//     send_spi(MEM_SECTOR_ERASE);
//     send_spi((address >> 16) & 0xFF);
//     send_spi((address >> 8) & 0xFF);
//     send_spi(address & 0xFF);
//     set_cs_high(mem_cs[chip].pin, mem_cs[chip].port);
//
//     send_short_mem_command(MEM_WR_DISABLE, chip);
// }
