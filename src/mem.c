/*
Flash memory Library
SST26VF016B
Datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/20005262D.pdf

This library controls interfacing with the 3 flash memory chips connected to the OBC microcontroller.

The organization of data memory follows the OBC Flash Memory Protocol:
https://utat-ss.readthedocs.io/en/master/our-protocols/obc-mem.html

Addresses are composed as follows (as uint32_t):
{ 0 (9 bits), chip_num (2 bits), chip_addr (21 bits)}

NOTES ON FLASH MEMORY ARCHITECTURE:
Flash is designed so that you can erase large sections at once, which means setting all bits to 1s. If you do a write, it changes some of the bits to 0s. This can only change 1s to 0s, not 0s to 1s. If you try to write a second time, it only changes some of the bits - 1s change to 0s, but 0s don't change back to 1s. Therefore, writing to it a second time may produce a value that is very different from what you tried to write. You can’t change 0s back to 1s with a write - this can only be accomplished by doing an erase before writing.
*/

#include "mem.h"

// Useful to comment/uncomment for debugging
// #define MEM_DEBUG


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


mem_section_t obc_hk_mem_section = {
    .start_addr = MEM_OBC_HK_START_ADDR,
    .end_addr = MEM_OBC_HK_END_ADDR,
    .curr_block = 0,
    .curr_block_eeprom_addr = MEM_OBC_HK_CURR_BLOCK_EEPROM_ADDR,
    .fields_per_block = CAN_OBC_HK_FIELD_COUNT
};

mem_section_t eps_hk_mem_section = {
    .start_addr = MEM_EPS_HK_START_ADDR,
    .end_addr = MEM_EPS_HK_END_ADDR,
    .curr_block = 0,
    .curr_block_eeprom_addr = MEM_EPS_HK_CURR_BLOCK_EEPROM_ADDR,
    .fields_per_block = CAN_EPS_HK_FIELD_COUNT
};

mem_section_t pay_hk_mem_section = {
    .start_addr = MEM_PAY_HK_START_ADDR,
    .end_addr = MEM_PAY_HK_END_ADDR,
    .curr_block = 0,
    .curr_block_eeprom_addr = MEM_PAY_HK_CURR_BLOCK_EEPROM_ADDR,
    .fields_per_block = CAN_PAY_HK_FIELD_COUNT
};

mem_section_t pay_opt_mem_section = {
    .start_addr = MEM_PAY_OPT_START_ADDR,
    .end_addr = MEM_PAY_OPT_END_ADDR,
    .curr_block = 0,
    .curr_block_eeprom_addr = MEM_PAY_OPT_CURR_BLOCK_EEPROM_ADDR,
    .fields_per_block = CAN_PAY_OPT_FIELD_COUNT
};

mem_section_t prim_cmd_log_mem_section = {
    .start_addr = MEM_PRIM_CMD_LOG_START_ADDR,
    .end_addr = MEM_PRIM_CMD_LOG_END_ADDR,
    .curr_block = 0,
    .curr_block_eeprom_addr = MEM_PRIM_CMD_LOG_CURR_BLOCK_EEPROM_ADDR,
    .fields_per_block = 1   // don't care
};

mem_section_t sec_cmd_log_mem_section = {
    .start_addr = MEM_SEC_CMD_LOG_START_ADDR,
    .end_addr = MEM_SEC_CMD_LOG_END_ADDR,
    .curr_block = 0,
    .curr_block_eeprom_addr = MEM_SEC_CMD_LOG_CURR_BLOCK_EEPROM_ADDR,
    .fields_per_block = 1   // don't care
};

// All memory sections
mem_section_t* all_mem_sections[MEM_NUM_SECTIONS] = {
    &obc_hk_mem_section,
    &eps_hk_mem_section,
    &pay_hk_mem_section,
    &pay_opt_mem_section,
    &prim_cmd_log_mem_section,
    &sec_cmd_log_mem_section,
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

    // Read all previously stored EEPROM data
    read_all_mem_sections_eeprom();
}

/*
Sets all struct fields to 0 in the header.
*/
void clear_mem_header(mem_header_t* header) {
    if (header == NULL) {
        return;
    }

    header->block_num = 0;
    header->error = 0;
    header->date.yy = 0;
    header->date.mm = 0;
    header->date.dd = 0;
    header->time.hh = 0;
    header->time.mm = 0;
    header->time.ss = 0;
}



/*
writes the current block number of `section` to its designated address in EEPROM
*/
void write_mem_section_eeprom(mem_section_t* section) {
    eeprom_write_dword (section->curr_block_eeprom_addr, section->curr_block);
}

/*
reads the current block number from its designated address in EEPROM and stores it in `section`
*/
void read_mem_section_eeprom(mem_section_t* section) {
    section->curr_block = eeprom_read_dword (section->curr_block_eeprom_addr);
    if (section->curr_block == EEPROM_DEF_DWORD) {
        section->curr_block = 0;
    }
}

/*
Reads data for all memory sections from EEPROM.
*/
void read_all_mem_sections_eeprom(void) {
    for (uint8_t i = 0; i < MEM_NUM_SECTIONS; i++) {
        read_mem_section_eeprom(all_mem_sections[i]);
    }
}

/*
Increments the section's current block number by 1 and writes it to EEPROM.
*/
void inc_mem_section_curr_block(mem_section_t* section) {
    section->curr_block++;
    write_mem_section_eeprom(section);
}



void write_mem_data_block(mem_section_t* section, uint32_t block_num,
    mem_header_t* header, uint32_t* fields) {

    // print("%s: ", __FUNCTION__);
    // print("start_addr = 0x%.8lX, block_num = %lu\n", section->start_addr,
    //     block_num);

    // Write header
    write_mem_header(section, block_num, header);
    // Write data fields
    for (uint8_t i = 0; i < section->fields_per_block; i++) {
        write_mem_field(section, block_num, i, fields[i]);
        // i is field number; fields[i] corresponds to associated field data
    }
}

void read_mem_data_block(mem_section_t* section, uint32_t block_num,
    mem_header_t* header, uint32_t* fields) {

    // print("%s: ", __FUNCTION__);
    // print("start_addr = 0x%.8lX, block_num = %lu\n", section->start_addr,
    //     block_num);

    // Read header
    read_mem_header(section, block_num, header);
    // Read fields
    for (uint8_t i = 0; i < section->fields_per_block; i++) {
        fields[i] = read_mem_field(section, block_num, i);
    }
}

// Command was a success
void write_mem_cmd_success(uint32_t block_num) {
    //uint32_t start_address = mem_cmd_section_addr(block_num); // header starting addr
    //uint8_t bytes[1] = { 0x00 };
    //write_mem_section_bytes(&cmd_log_mem_section, (start_address + 3), bytes, 1);
    uint8_t success = 0x00;
    mem_header_t dummy_header = {
        .block_num = 0xFFFFFFFF,
        .error = success,
        .date.yy = 0xFF,
        .date.mm = 0xFF,
        .date.dd = 0xFF,
        .time.hh = 0xFF,
        .time.mm = 0xFF,
        .time.ss = 0xFF,
    };
    write_mem_header(&cmd_log_mem_section, block_num, &dummy_header);
}

uint8_t write_mem_cmd_block(mem_section_t* section, uint32_t block_num, mem_header_t* header,
    uint8_t cmd_num, uint32_t arg1, uint32_t arg2) {
    /*
     * Writes to cmd_log section in the flash memory. Each entry is 19 bytes. 10 bytes for
     * the header, 1 byte for command type, 4 bytes for arg 1, and 4 bytes for arg 2.
     * This format differs from the rest of the memory sections, which has standardized 3 bytes/field and multiple
     * fields forming a block
     * Returns a 1 if write was successful, 0 if not
     */

    write_mem_header(section, block_num, header);

    // calculate the address based on block number. This is the offset address from the start of the section
    uint32_t start_address = mem_cmd_section_addr(section, block_num);

    // write the 19 bytes of information and check if write was successful
    uint8_t bytes[MEM_BYTES_PER_CMD] = {
        cmd_num,
        (arg1 >> 24) & 0xFF, (arg1 >> 16) & 0xFF, (arg1 >> 8) & 0xFF, arg1 & 0xFF,
        (arg2 >> 24) & 0xFF, (arg2 >> 16) & 0xFF, (arg2 >> 8) & 0xFF, arg2 & 0xFF
    };
    if (write_mem_section_bytes(section, start_address,
        bytes, MEM_BYTES_PER_CMD)) {
        inc_mem_section_curr_block(&cmd_log_mem_section);
        return 1;
    } else {
        return 0;
    }
}


void read_mem_cmd_block(mem_section_t* section, uint32_t block_num, mem_header_t* header,
    uint8_t* cmd_num, uint32_t* arg1, uint32_t* arg2){
    /*
     * Reads the cmd log from flash memory. Each cmd log is 19 bytes
     */

    // Read 10 byte header
    read_mem_header(section, block_num, header);

    // read the 9 bytes that constitute the cmd_log block
    uint8_t bytes[MEM_BYTES_PER_CMD] = {0};
    read_mem_section_bytes(section, mem_cmd_section_addr(section, block_num), bytes, MEM_BYTES_PER_CMD);

    // deconstruct the 9 bytes into the specific components(cmd_num, arg1, and arg2)
    *cmd_num = bytes[0];
    *arg1 =
        ((uint32_t) bytes[1] << 24) |
        ((uint32_t) bytes[2] << 16) |
        ((uint32_t) bytes[3] << 8) |
        ((uint32_t) bytes[4]);
    *arg2 =
        ((uint32_t) bytes[5] << 24) |
        ((uint32_t) bytes[6] << 16) |
        ((uint32_t) bytes[7] << 8) |
        ((uint32_t) bytes[8]);
}



void write_mem_header(mem_section_t* section, uint32_t block_num,
    mem_header_t* header) {

    /*
    writes the header information array (which contains metadata such as
    block number, timestamp, and error codes) into the current block of the
        section
    */

    uint8_t bytes[MEM_BYTES_PER_HEADER] = {
        (header->block_num >> 16) & 0xFF,
        (header->block_num >> 8) & 0xFF,
        header->block_num & 0xFF,
        header->error,
        header->date.yy, header->date.mm, header->date.dd,
        header->time.hh, header->time.mm, header->time.ss
    };

    write_mem_section_bytes(section, mem_block_section_addr(section, block_num), bytes, MEM_BYTES_PER_HEADER);
}


/*
Reads the header data for the specified block number.
block_num - the expected block number (determines the address to start reading
    from) - expected to be equal to header->block_num set by this function
header - will be set by this function to the results
*/
void read_mem_header(mem_section_t* section, uint32_t block_num,
    mem_header_t* header) {

    uint8_t bytes[MEM_BYTES_PER_HEADER];
    read_mem_section_bytes(section, mem_block_section_addr(section, block_num), bytes,
        MEM_BYTES_PER_HEADER);

    header->block_num =
        (((uint32_t) bytes[0]) << 16) |
        (((uint32_t) bytes[1]) << 8) |
        ((uint32_t) bytes[2]);
    header->error = bytes[3];
    header->date.yy = bytes[4];
    header->date.mm = bytes[5];
    header->date.dd = bytes[6];
    header->time.hh = bytes[7];
    header->time.mm = bytes[8];
    header->time.ss = bytes[9];
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

    uint32_t address = mem_field_section_addr(section, block_num, field_num);

    // Split the data into 3 bytes
    uint8_t data_bytes[MEM_BYTES_PER_FIELD] = {
        (data >> 16) & 0xFF,
        (data >> 8) & 0xFF,
        data & 0xFF
    };

    write_mem_section_bytes(section, address, data_bytes, MEM_BYTES_PER_FIELD);
}

uint32_t read_mem_field(mem_section_t* section, uint32_t block_num,
        uint8_t field_num) {
/*
    Reads and returns the 24-bit data for the specified section, block, and field
*/

    uint32_t address = mem_field_section_addr(section, block_num, field_num);

    uint8_t data_bytes[MEM_BYTES_PER_FIELD];
    read_mem_section_bytes(section, address, data_bytes, MEM_BYTES_PER_FIELD);

    uint32_t data = 0;
    for (uint8_t i = 0; i < MEM_BYTES_PER_FIELD; ++i) {
        data <<= 8;
        data |= data_bytes[i];
    }
    return data;
}




/*
Calculates the number of bytes per block for the section.
*/
uint32_t mem_block_size(mem_section_t* section) {
    if (section == &prim_cmd_log_mem_section ||
        section == &sec_cmd_log_mem_section) {
        return MEM_BYTES_PER_HEADER + MEM_BYTES_PER_CMD;
    } else {
        return MEM_BYTES_PER_HEADER +
            (((uint32_t) section->fields_per_block) * MEM_BYTES_PER_FIELD);
    }
}

/*
Calculates and returns the address of the start of a block (where the header starts).
This is an offset from the beginning of the section.
*/
uint32_t mem_block_section_addr(mem_section_t* section, uint32_t block_num) {
    uint32_t block_address = block_num * mem_block_size(section);
    return block_address;
}


/*
Calculates and returns the address of the start of a field in a block (after the header).
Only applies to data sections.
*/
uint32_t mem_field_section_addr(mem_section_t* section, uint32_t block_num,
        uint32_t field_num) {
    uint32_t block_address = mem_block_section_addr(section, block_num);
    uint32_t field_address = block_address +
        MEM_BYTES_PER_HEADER + (field_num * MEM_BYTES_PER_FIELD);
    return field_address;
}

/*
Calculates and returns the address of the start of a command in a block (after the header).
Only applies to the command sections.
*/
uint32_t mem_cmd_section_addr(mem_section_t* section, uint32_t block_num) {
    uint32_t cmd_address =
        mem_block_section_addr(section, block_num) +
        MEM_BYTES_PER_HEADER;
    return cmd_address;
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

uint8_t write_mem_section_bytes(mem_section_t *section, uint32_t address, uint8_t* data, uint8_t data_len){
/*
    writes an array of bytes to the specific section where address is the offset from the start of
    the section. Data will only be written if the array fits into the section. Returns 1 if write was success.
*/
    if(address < 0 || data_len <= 0)
        return 0;

    if((section->start_addr + address + data_len - 1) <= section->end_addr ){
        write_mem_bytes(address + section->start_addr, data, data_len);
        return 1;
    } else {
        return 0;
    }
}

void read_mem_section_bytes(mem_section_t *section, uint32_t address, uint8_t* data, uint8_t data_len){
/*
    reads an array of bytes from a section where address is the offset from the start of the section
*/
    read_mem_bytes(address + section->start_addr, data, data_len);
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

#ifdef MEM_DEBUG
    print("%s: ", __FUNCTION__);
    print("address = 0x%.8lX, data_len = %u\n", address, data_len);
    print("data = ");
    print_bytes(data, data_len);
#endif

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

#ifdef MEM_DEBUG
    print("%s: ", __FUNCTION__);
    print("address = 0x%.8lX, data_len = %u\n", address, data_len);
    print("data = ");
    print_bytes(data, data_len);
#endif
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

#ifdef MEM_DEBUG
    if (timeout == 0) {
        print("MEM TIMEOUT\n");
    }
#endif
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

/* Takes an address and chip as input and erases the appropriate sector */
/* Each sector is 4kb, see pg. 24 for more info on sector erase */
 void erase_mem_sector(uint32_t address){
     uint8_t chip_num;
     uint8_t addr1;
     uint8_t addr2;
     uint8_t addr3;
     process_mem_addr(address, &chip_num, &addr1, &addr2, &addr3);

     send_short_mem_command(MEM_WR_ENABLE, chip_num);

     set_cs_low(mem_cs[chip_num].pin, mem_cs[chip_num].port);
     send_spi(MEM_SECTOR_ERASE);
     /* Remaining bits after sector address can be either high or low, so
        the entire address can be sent as bit 23 is the MSB */
    send_spi(addr1);
    send_spi(addr2);
    send_spi(addr3);
     set_cs_high(mem_cs[chip_num].pin, mem_cs[chip_num].port);

     send_short_mem_command(MEM_WR_DISABLE, chip_num);
     wait_for_mem_not_busy(chip_num);
 }

/* Takes an address and chip as input and deletes the appropriate block.
   The block size can range from 8kb to 64kb */
/* See pg. 5 of data sheet for memory map and pg. 25 for more details on block erase*/
 void erase_mem_block(uint32_t address){
     uint8_t chip_num;
     uint8_t addr1;
     uint8_t addr2;
     uint8_t addr3;
     process_mem_addr(address, &chip_num, &addr1, &addr2, &addr3);

     send_short_mem_command(MEM_WR_ENABLE, chip_num);

     set_cs_low(mem_cs[chip_num].pin, mem_cs[chip_num].port);
     send_spi(MEM_BLOCK_ERASE);
     /* Remaining bits after block address can be either high or low, so
        the entire address can be sent as bit 23 is the MSB*/
     send_spi(addr1);
     send_spi(addr2);
     send_spi(addr3);
     set_cs_high(mem_cs[chip_num].pin, mem_cs[chip_num].port);

     send_short_mem_command(MEM_WR_DISABLE, chip_num);
     wait_for_mem_not_busy(chip_num);

 }
