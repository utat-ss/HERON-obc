#include "mem.h"

uint32_t init_stack(uint8_t type){
    switch(type){
        case SCI_TYPE:
            return (uint32_t) SCI_INIT;
        case PAY_HK_TYPE:
            return (uint32_t) PAY_INIT;
        case EPS_HK_TYPE:
            return (uint32_t) EPS_INIT;
        default:
            return 0x00;
        /*case OBC_HK_TYPE:
        return OBC_INIT;
        case STATUS_TYPE:
        return STATUS_INIT;*/
    }
}

uint32_t* pointer(uint8_t type){
    switch(type){
        case SCI_TYPE:
            return (uint32_t*) SCI_CURR_PTR_ADDR;
        case PAY_HK_TYPE:
            return (uint32_t*) PAY_HK_CURR_PTR_ADDR;
        case EPS_HK_TYPE:
            return (uint32_t*) EPS_HK_CURR_PTR_ADDR;
        default:
            return 0x00;
        /*case OBC_HK_TYPE:
        OBC_HK_STACK_PTR;
        case STATUS_TYPE:
        STATUS_PTR;*/
    }
}

uint32_t block_size(uint8_t type){
    switch(type) {
        case SCI_TYPE:
            return (uint32_t) SCI_BLOCK_SIZE;
        case PAY_HK_TYPE:
            return (uint32_t) PAY_BLOCK_SIZE;
        case EPS_HK_TYPE:
            return (uint32_t) EPS_BLOCK_SIZE;
        default:
            return 0x00;
        /*  case OBC_HK_TYPE:
        return OBC_BLOCK_SIZE;
        case STATUS_TYPE:
        return STATUS_BLOCK_SIZE;*/
    }
}

void init_curr_stack_ptrs() {
    eeprom_write_dword ((uint32_t *) SCI_CURR_PTR_ADDR,SCI_INIT);
    eeprom_write_dword ((uint32_t *) PAY_HK_CURR_PTR_ADDR,PAY_INIT);
    eeprom_write_dword ((uint32_t *) EPS_HK_CURR_PTR_ADDR,EPS_INIT);
    print ("EPS_INIT: %x\n",EPS_INIT);
}

//read curr_ptr and update by BLOCK_SIZE
uint8_t init_block(uint8_t type){
    uint32_t curr_ptr;
    curr_ptr = eeprom_read_dword (pointer(type));
    curr_ptr = curr_ptr + block_size (type);
    eeprom_write_dword (pointer(type),curr_ptr);
    print ("Current pointer after init block is: %x\n",curr_ptr);
    return (uint8_t) ((curr_ptr-init_stack(type))/block_size(type));
}

void init_header(uint8_t *header, uint8_t type){
    uint32_t curr_ptr;
    curr_ptr = eeprom_read_dword (pointer(type));
    mem_write(curr_ptr, header, HEADER_SIZE*FIELD_SIZE);
}

// fields are indexed from ZERO
void write_to_flash(uint8_t type, uint8_t field_num, uint8_t * data) {
    uint32_t curr_ptr;
    if (field_num == 0x00) {
        uint8_t headerID = init_block(type);
        // time_t time = read_time();
        // date_t date = read_date();
        uint8_t error = 0xFF;
        // uint8_t header[8] = {time.hh, time.mm, time.ss, date.yy,
        //                      date.mm, date.dd, error, headerID};
        uint8_t header[8] = {0x02, 0x03, 0x04, 0x05,
                             0x06, 0x07, error, headerID};
        init_header(header, type);
    }
    curr_ptr = eeprom_read_dword (pointer(type));
    mem_write((curr_ptr + FIELD_SIZE*(field_num + HEADER_SIZE)), data, FIELD_SIZE);
    print ("Address written to is : %x\n*******",(curr_ptr + FIELD_SIZE*(field_num + HEADER_SIZE)));
}

void read_from_flash (uint8_t type,uint8_t* data,uint8_t data_len) {

    print ("Read from address: %x\n",init_stack(type)+block_size);
    mem_read(init_stack(type)+block_size(type), data, data_len);
    int i;
    for (i=0; i<data_len; i++) {
        print("%02x ",data[i]);
    }
}

void init_mem(){

    // initialize the Chip Select pin
    uint8_t i;
    for(i = 0; i < 3 ; i++)
    {
        init_cs((2 + i), &MEM_DDR);
        set_cs_high((2 + i), &MEM_PORT);
    }

    mem_unlock();

    for(i = 0; i < 3; i ++)
    {
        // mem_status_w(0x02, i);
        mem_erase(i);
    }

}

void mem_erase(uint8_t chip){

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
    uint16_t i = 0;
    uint8_t a1 = (((address + i) >> 16) & 0xFF);
    uint8_t a2 = (((address + i) >> 8) & 0xFF);
    uint8_t a3 = ((address + i) & 0xFF);
    uint8_t chip_num = ((address + i) >> 24) & 0x03;
    uint8_t busy = 1;
    uint32_t timeout = 1;

    for(i = 0; i < data_len; i ++){
        if(((address + i) % 256 == 0) || (i == 0))
        {
            set_cs_high((chip_num + 2), &MEM_PORT);

            while(busy && timeout)
            {
                busy = mem_status_r(chip_num) & 0x01;
                timeout += 1;
            }

            mem_command_short(MEM_WR_DISABLE, chip_num);

            a1 = (((address + i) >> 16) & 0xFF);
            a2 = (((address + i) >> 8) & 0xFF);
            a3 = ((address + i) & 0xFF);

            print("%x %x %x \n", a1, a2, a3);

            chip_num = ((address + i) >> 24) & 0x03;
            if(chip_num > 2)
            {
                chip_num = 0;
            }

            print("%x \n", chip_num);

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
    uint8_t i;
    uint8_t chip_num = (address >> 24) & 0x03; //Assume this starts at 0
    set_cs_low((chip_num + 2), &MEM_PORT);
    send_spi(MEM_R_BYTE);
    send_spi((address >> 16) & 0xFF);
    send_spi((address >> 8) & 0xFF);
    send_spi(address & 0xFF);

    for (i = 0; i < data_len; i++){
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
            print("%x \n", chip_num);
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
    for(int i = 0; i < 2; i++)
    {
        mem_command_short(MEM_WR_ENABLE, i);
        mem_command_short(0x98, i);
    }

}

uint8_t mem_status_r(uint8_t chip){
    return mem_command(MEM_READ_STATUS, 0x00, chip);
}

void mem_status_w(uint8_t status, uint8_t chip){
    set_cs_low((chip+2), &MEM_PORT);
    send_spi(MEM_WRITE_STATUS);
    send_spi(0x00);
    send_spi(status);
    set_cs_high((chip + 2), &MEM_PORT);
}

uint8_t mem_command(uint8_t command, uint8_t data, uint8_t chip){
    uint8_t value;
    set_cs_low((chip+2), &MEM_PORT);
    send_spi(command);
    value = send_spi(data);
    set_cs_high((chip + 2), &MEM_PORT);
    return value;
}

void mem_command_short(uint8_t command, uint8_t chip){
    set_cs_low((chip + 2), &MEM_PORT);
    send_spi(command);
    set_cs_high((chip + 2), &MEM_PORT);
}
