#include "mem.h"

uint32_t init_stack(uint8_t type){

/*
    returns the initial address of the stack pointer of the input type
*/

    switch(type){
        case SCI_TYPE:
            return (uint32_t) SCI_INIT;
        case PAY_HK_TYPE:
            return (uint32_t) PAY_INIT;
        case EPS_HK_TYPE:
            return (uint32_t) EPS_INIT;
        default:
            return 0x00;
    }
}

uint32_t* pointer(uint8_t type){
/*

    returns pointer to the address in EEPROM
    which stores the value of the stack pointer for the input type

*/
    switch(type){
        case SCI_TYPE:
            return (uint32_t*) SCI_CURR_PTR_ADDR;
        case PAY_HK_TYPE:
            return (uint32_t*) PAY_HK_CURR_PTR_ADDR;
        case EPS_HK_TYPE:
            return (uint32_t*) EPS_HK_CURR_PTR_ADDR;
        default:
            return 0x00;
    }
}

uint32_t block_size(uint8_t type){
/*

    returns the size of a block (in units of fields, ie 4 bytes) for
    the given type
*/
    switch(type) {
        case SCI_TYPE:
            return (uint32_t) SCI_BLOCK_SIZE;
        case PAY_HK_TYPE:
            return (uint32_t) PAY_BLOCK_SIZE;
        case EPS_HK_TYPE:
            return (uint32_t) EPS_BLOCK_SIZE;
        default:
            return 0x00;
    }
}

void init_curr_stack_ptrs() {
    /*

    writes the initial addresses of the stack pointers to
    the addresses in EEPROM which store the stack pointers

    */
    eeprom_write_dword ((uint32_t *) SCI_CURR_PTR_ADDR,SCI_INIT);
    eeprom_write_dword ((uint32_t *) PAY_HK_CURR_PTR_ADDR,PAY_INIT);
    eeprom_write_dword ((uint32_t *) EPS_HK_CURR_PTR_ADDR,EPS_INIT);
}

uint8_t init_block(uint8_t type){
    /*

    reads value of current stack pointer, updates it by the block size of the
    given type, and returns the number of the current block

    */
    uint32_t curr_ptr;
    curr_ptr = eeprom_read_dword (pointer(type));
    curr_ptr = curr_ptr + block_size (type);
    eeprom_write_dword (pointer(type),curr_ptr);
    return (uint8_t) ((curr_ptr-init_stack(type))/block_size(type));
}

void init_header(uint8_t *header, uint8_t type){
    /*

    writes the header information array (which contains metadata such as
    timestamp, block number and error codes) into the header space of
    the current block

    */
    uint32_t curr_ptr;
    curr_ptr = eeprom_read_dword (pointer(type));
    mem_write(curr_ptr, header, HEADER_SIZE*FIELD_SIZE);
}

// fields are indexed from ZERO
void write_to_flash(uint8_t type, uint8_t field_num, uint8_t * data) {

/*

    writes a field (4 bytes) to flash, and automatically creates a new
    block in the stack whenever field zero is reached

    fields are indexed from zero and blocks are indexed from one

*/
    uint32_t curr_ptr;
    if (field_num == 0x00) {
        uint8_t headerID = init_block(type);
        time_t time = read_time();
        date_t date = read_date();
        uint8_t error = 0xFF;
        uint8_t header[8] = {time.hh, time.mm, time.ss, date.yy,
                             date.mm, date.dd, error, headerID};

        init_header(header, type);
    }
    curr_ptr = eeprom_read_dword (pointer(type));
    mem_write((curr_ptr + FIELD_SIZE*(field_num + HEADER_SIZE)), data, FIELD_SIZE);
}

void read_from_flash (uint8_t type,uint8_t* data,uint8_t data_len, uint8_t block_num, uint8_t field_num) {

/*
    fills data array with data of length data_len, starting from
    the block and field specified, for the given type
*/
    mem_read(init_stack(type)+block_size(type)*block_num+field_num*FIELD_SIZE, data, data_len);
}

void init_mem(){

/*
    intializes the chip select pin, unlocks (un-write protects) and erases memory
*/
    // initialize the Chip Select pin
    uint8_t i;
    for(i = 0; i < 3 ; i++)
    {
        init_cs((2 + i), &MEM_DDR);
        set_cs_high((2 + i), &MEM_PORT);
    }

    mem_unlock(); //use global unlock to unlock memory

    for(i = 0; i < 3; i ++)
    {
        mem_erase(i); //erase all three memory chips
    }

}

void mem_erase(uint8_t chip){
/*
    erase the specified memory chip (overwrite all data to ones)
*/
    mem_command_short(MEM_WR_ENABLE, chip);
    mem_command_short(MEM_ERASE, chip);
    mem_command_short(MEM_WR_DISABLE, chip);
    uint8_t busy = mem_status_r(chip);
    uint32_t timeout = 0x01;
    busy &= 0x01;
    while(busy && timeout)
    {
        timeout ++;
        busy = mem_status_r(chip);
        busy &= 0x01;
    }
}

 void mem_write(uint32_t address, uint8_t * data, uint8_t data_len){

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
        to be modified in the even of changes to the board design
*/

    uint16_t i = 0; //initialize counter

    chip_num = (address >> 24) & 0x03; //calculate the initial chip number

    //used to determine the end of a write operation
    uint8_t busy = 1;

    //ensures that the polling loop terminates through arithmetic rollover
    uint32_t timeout = 1;

    for(i = 0; i < data_len; i ++){

        //rollover condition for chip and page boundaries
        if(((address + i) % 256 == 0) || (i == 0))
        {
            set_cs_high((chip_num + 2), &MEM_PORT); //end previous write operation

            while(busy && timeout)
            {
                //wait for previous operation to terminate
                busy = mem_status_r(chip_num) & 0x01;
                timeout += 1;
            }

            mem_command_short(MEM_WR_DISABLE, chip_num);

            //calculate address bytes
            a1 = (((address + i) >> 16) & 0xFF);
            a2 = (((address + i) >> 8) & 0xFF);
            a3 = ((address + i) & 0xFF);

            //recalculate and rollover the chip number,
            //stored in the most significant bits of the address

            chip_num = ((address + i) >> 24) & 0x03;
            if(chip_num > 2)
            {
                chip_num = 0;
            }

            mem_command_short(MEM_WR_ENABLE, chip_num); //enable writing to chip
            set_cs_low((chip_num + 2), &MEM_PORT);
            /* all bytes to be written must be
            proceeded by the Page Program command */

            send_spi(MEM_PG_PRG);

            send_spi(a1);
            send_spi(a2);
            send_spi(a3);
        }

        send_spi(data[i]);
    }

    busy = 0x01;
    timeout = 0x01;

    set_cs_high((chip_num + 2), &MEM_PORT);

    while(busy && timeout)
    {
        busy = mem_status_r(chip_num) & 0x01;
        timeout += 1;
    }

    mem_command_short(MEM_WR_DISABLE, chip_num);
}


void mem_read(uint32_t address, uint8_t * data, uint8_t data_len){

/*
    Reads a continous block of memory of size data_len from the given address
    and places the result in the data array

    key features:
        highest address wrap-around (to zero)

        reads continously across chips (ie behaves as a continous address space)
*/
    uint8_t i;

    //Assume this starts at 0, not 2 (as in the actual hardware)
    uint8_t chip_num = (address >> 24) & 0x03;

    set_cs_low((chip_num + 2), &MEM_PORT);
    send_spi(MEM_R_BYTE);
    send_spi((address >> 16) & 0xFF);
    send_spi((address >> 8) & 0xFF);
    send_spi(address & 0xFF);

    for (i = 0; i < data_len; i++){

        //checks from chip rollover condition
        if (((address + i) >> 24 & 0x03) != chip_num)
        {
            if (chip_num == 2)
            {
            //ensure wrap-around back to chip 0
                set_cs_high((chip_num + 2), &MEM_PORT);
                chip_num = 0;
                set_cs_low((chip_num + 2), &MEM_PORT);
            }
            else
            {
                set_cs_high((chip_num + 2), &MEM_PORT);
                chip_num++;
                set_cs_low((chip_num + 2), &MEM_PORT);
            }
            /* Begin read on new chip */
            send_spi(MEM_R_BYTE);
            send_spi(0x00);
            send_spi(0x00);
            send_spi(0x00);
        }

        data[i] = send_spi(0x00);;
    }
    set_cs_high((chip_num + 2), &MEM_PORT);
}

void mem_sector_erase(uint8_t sector, uint8_t chip){

    /* erase a specific sector in memory, on indicated chip

    functionality not used in the higher-level implementation

    */

    uint32_t address = sector * 4096;

    mem_command_short(MEM_WR_ENABLE, chip);

    set_cs_low(chip + 2, &MEM_PORT);
    send_spi(MEM_SECTOR_ERASE);
    send_spi(address >> 16);
    send_spi((address >> 8) & 0xFF);
    send_spi(address & 0xFF);
    set_cs_high(chip + 2, &MEM_PORT);

    mem_command_short(MEM_WR_DISABLE, chip);

}

void mem_unlock(){

    // send the global mem unlock command to enable write operations
    for(int i = 0; i < 3; i++)
    {
        mem_command_short(MEM_WR_ENABLE, i);
        mem_command_short(0x98, i);
    }

}

uint8_t mem_status_r(uint8_t chip){

    // read from the status register

    return mem_command(MEM_READ_STATUS, 0x00, chip);
}

void mem_status_w(uint8_t status, uint8_t chip){

    // write to the configuration register

    set_cs_low((chip+2), &MEM_PORT);
    send_spi(MEM_WRITE_STATUS);
    send_spi(0x00);
    send_spi(status);
    set_cs_high((chip + 2), &MEM_PORT);
}

uint8_t mem_command(uint8_t command, uint8_t data, uint8_t chip){

    // send a command with an argument and return value to the device

    uint8_t value;
    set_cs_low((chip+2), &MEM_PORT);
    send_spi(command);
    value = send_spi(data);
    set_cs_high((chip + 2), &MEM_PORT);
    return value;
}

void mem_command_short(uint8_t command, uint8_t chip){

    // send a command without an argument or return value to the device
    set_cs_low((chip + 2), &MEM_PORT);
    send_spi(command);
    set_cs_high((chip + 2), &MEM_PORT);
}
