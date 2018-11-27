/*
Flash memory Library
SST26VF016B
Datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/20005262D.pdf

This library controls interfacing with the 3 flash memory chips connected to the OBC microcontroller.

The organization of data memory follows the OBC Flash Memory Protocol:
https://utat-ss.readthedocs.io/en/master/our-protocols/obc-mem.html
*/

#include "mem.h"

//TODO: FIX NUMBERS
// TODO - make sure EEPROM addresses don't conflict with heartbeat

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
    .start_addr = 0x0DB0UL,
    .curr_block = 0,
    .curr_block_eeprom_addr = (uint32_t*) 0x20,
    .fields_per_block = CAN_EPS_HK_FIELD_COUNT   // Should be 12
};

mem_section_t pay_hk_mem_section = {
    .start_addr = 0x10000UL,
    .curr_block = 0,
    .curr_block_eeprom_addr = (uint32_t*) 0x24,
    .fields_per_block = CAN_PAY_HK_FIELD_COUNT   // Should be 3
};

mem_section_t pay_sci_mem_section = {
    .start_addr = 0x20000UL,
    .curr_block = 0,
    .curr_block_eeprom_addr = (uint32_t*) 0x28,
    .fields_per_block = CAN_PAY_SCI_FIELD_COUNT   // Should be 33
};




void init_mem(void){

/*
    intializes the chip select pin, unlocks (un-write protects) and erases memory
*/
    // initialize the Chip Select pins
    for(uint8_t i = 0; i < MEM_NUM_CHIPS; i++)
    {
        init_cs(mem_cs[i].pin, mem_cs[i].ddr);
        set_cs_high(mem_cs[i].pin, mem_cs[i].port);
    }

    unlock_mem(); //use global unlock to unlock memory

    // TODO - initialize block numbers from EEPROM
}


// TODO - update current stack pointers after block writes, store current address variables
void write_curr_block_to_eeprom(mem_section_t* section) {
    /*
    writes the current block number of `section` to its designated address in EEPROM
    */
    eeprom_write_dword (section->curr_block_eeprom_addr, section->curr_block);
}


void read_curr_block_from_eeprom(mem_section_t* section) {
    /*
    reads the current block number from its designated address in EEPROM and stores it in `section`
    */
    section->curr_block = eeprom_read_dword (section->curr_block_eeprom_addr);
}


void increment_curr_block(mem_section_t* section) {
    /*
    Increments the section's current block number by 1 and updates the value stored in EEPROM
    */
    section->curr_block++;
    write_curr_block_to_eeprom(section);
}


/*
Calculates and returns the address of the start of a block (where the header starts).
*/
uint32_t mem_block_addr(mem_section_t* section, uint32_t block_num) {
    uint32_t bytes_per_block = BYTES_PER_HEADER +
        (((uint32_t) section->fields_per_block) * BYTES_PER_FIELD);
    uint32_t block_address = section->start_addr + (block_num * bytes_per_block);
    return block_address;
}


/*
Calculates and returns the address of the start of a field in a block (after the header).
*/
uint32_t mem_field_addr(mem_section_t* section, uint32_t block_num, uint32_t field_num) {
    uint32_t block_address = mem_block_addr(section, block_num);
    uint32_t field_address = block_address +
        BYTES_PER_HEADER + (field_num * BYTES_PER_FIELD);
    return field_address;
}


void write_mem_header(mem_section_t* section, uint32_t block_num){
    /*
    writes the header information array (which contains metadata such as
    block number, timestamp, and error codes) into the current block of the section
    */

    time_t time = read_time();
    date_t date = read_date();
    uint8_t error = 0x00;   // TODO
    uint8_t header[BYTES_PER_HEADER] = {
        block_num,
        error,
        date.yy, date.mm, date.dd,
        time.hh, time.mm, time.ss
    };

    write_mem_bytes(mem_block_addr(section, block_num), header, BYTES_PER_HEADER);
}


/*
Reads the header data for the specified block number.
data - must be already allocated (`BYTES_PER_HEADER` bytes long) and passed to this function; this function will
populate the data in it
*/
void read_mem_header(mem_section_t* section, uint32_t block_num, uint8_t* data) {
    read_mem_bytes(mem_block_addr(section, block_num), data, BYTES_PER_HEADER);
}


// fields are indexed from ZERO
void write_mem_field(mem_section_t* section, uint32_t block_num, uint8_t field_num, uint32_t data) {

/*
    writes a field (3 bytes) to flash, and automatically creates a new
    block in the stack whenever field zero is reached

    fields are indexed from zero and blocks are indexed from one

    data - the least significant 24 bits will be written to memory
*/

    uint32_t address = mem_field_addr(section, block_num, field_num);

    // Split the data into 3 bytes
    uint8_t data_bytes[BYTES_PER_FIELD] = {
        (data >> 16) & 0xFF,
        (data >> 8) & 0xFF,
        data & 0xFF
    };

    write_mem_bytes(address, data_bytes, BYTES_PER_FIELD);
}


uint32_t read_mem_field(mem_section_t* section, uint32_t block_num, uint8_t field_num) {

/*
    Reads and returns the 24-bit data for the specified section, block, and field
*/

    uint32_t address = mem_field_addr(section, block_num, field_num);

    uint8_t data_bytes[BYTES_PER_FIELD];
    read_mem_bytes(address, data_bytes, BYTES_PER_FIELD);

    uint32_t data = 0;
    for (uint8_t i = 0; i < BYTES_PER_FIELD; ++i) {
        data = data << 8;
        data = data | data_bytes[i];
    }
    return data;
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

    uint8_t chip_num = (address >> 24) & 0x03; //calculate the initial chip number

    //initialize counter
    for(uint16_t i = 0; i < data_len; i++){

        //rollover condition for chip and page boundaries
        if(((address + i) % 256 == 0) || (i == 0))
        {
            set_cs_high(mem_cs[chip_num].pin, mem_cs[chip_num].port); //end previous write operation

            //used to determine the end of a write operation
            uint8_t busy = 1;
            //ensures that the polling loop terminates
            uint16_t timeout = UINT16_MAX;
            while(busy && timeout > 0)
            {
                //wait for previous operation to terminate
                busy = read_mem_status(chip_num) & 0x01;
                timeout--;
            }

            send_short_mem_command(MEM_WR_DISABLE, chip_num);

            //calculate address bytes
            uint8_t a1 = (((address + i) >> 16) & 0xFF);
            uint8_t a2 = (((address + i) >> 8) & 0xFF);
            uint8_t a3 = ((address + i) & 0xFF);

            //recalculate and rollover the chip number,
            //stored in the most significant bits of the address

            chip_num = ((address + i) >> 24) & 0x03;
            if(chip_num > 2)
            {
                chip_num = 0;
            }

            send_short_mem_command(MEM_WR_ENABLE, chip_num); //enable writing to chip
            set_cs_low(mem_cs[chip_num].pin, mem_cs[chip_num].port);
            /* all bytes to be written must be
            proceeded by the Page Program command */

            send_spi(MEM_PG_PRG);

            send_spi(a1);
            send_spi(a2);
            send_spi(a3);
        }

        send_spi(data[i]);
    }

    set_cs_high(mem_cs[chip_num].pin, mem_cs[chip_num].port);

    //used to determine the end of a write operation
    uint8_t busy = 1;
    uint16_t timeout = UINT16_MAX;

    while(busy && timeout > 0)
    {
        busy = read_mem_status(chip_num) & 0x01;
        timeout--;
    }

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

    //Assume this starts at 0, not 2 (as in the actual hardware)
    uint8_t chip_num = (address >> 24) & 0x03;

    set_cs_low(mem_cs[chip_num].pin, mem_cs[chip_num].port);
    send_spi(MEM_R_BYTE);
    send_spi((address >> 16) & 0xFF);
    send_spi((address >> 8) & 0xFF);
    send_spi(address & 0xFF);

    for (uint8_t i = 0; i < data_len; i++){

        //checks from chip rollover condition
        if (((address + i) >> 24 & 0x03) != chip_num)
        {
            set_cs_high(mem_cs[chip_num].pin, mem_cs[chip_num].port);

            if (chip_num >= MEM_NUM_CHIPS - 1) {
                //ensure wrap-around back to chip 0
                chip_num = 0;
            }
            else {
                chip_num++;
            }

            set_cs_low(mem_cs[chip_num].pin, mem_cs[chip_num].port);
            /* Begin read on new chip */
            send_spi(MEM_R_BYTE);
            send_spi(0x00);
            send_spi(0x00);
            send_spi(0x00);
        }

        data[i] = send_spi(0x00);
    }

    set_cs_high(mem_cs[chip_num].pin, mem_cs[chip_num].port);
}


void erase_mem(uint8_t chip){
/*
    erase the specified memory chip (overwrite all data to ones)
*/
    send_short_mem_command(MEM_WR_ENABLE, chip);
    send_short_mem_command(MEM_ERASE, chip);
    send_short_mem_command(MEM_WR_DISABLE, chip);

    uint8_t busy = 1;
    uint32_t timeout = UINT16_MAX;
    while(busy && timeout > 0)
    {
        timeout--;
        busy = read_mem_status(chip) & 0x01;
    }
}


void erase_mem_sector(uint8_t sector, uint8_t chip){

    /*
    erase a specific sector in memory, on indicated chip
    functionality not used in the higher-level implementation
    */

    uint32_t address = sector * 4096;

    send_short_mem_command(MEM_WR_ENABLE, chip);

    set_cs_low(mem_cs[chip].pin, mem_cs[chip].port);
    send_spi(MEM_SECTOR_ERASE);
    send_spi(address >> 16);
    send_spi((address >> 8) & 0xFF);
    send_spi(address & 0xFF);
    set_cs_high(mem_cs[chip].pin, mem_cs[chip].port);

    send_short_mem_command(MEM_WR_DISABLE, chip);
}


void unlock_mem(void){

    // send the global mem unlock command to enable write operations
    for(uint8_t i = 0; i < MEM_NUM_CHIPS; i++) {
        send_short_mem_command(MEM_WR_ENABLE, i);
        send_short_mem_command(0x98, i);
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
