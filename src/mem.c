#include "mem.h"
#include <can/can_data_protocol.h>

uint32_t init_stack(uint8_t type){
  switch(type){
    case SCI_TYPE:
      return (uint32_t) SCI_INIT;
    case PAY_HK_TYPE:
      return (uint32_t) PAY_INIT;
    case EPS_HK_TYPE:
      return (uint32_t) EPS_INIT;
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
  /*  case OBC_HK_TYPE:
      return OBC_BLOCK_SIZE;
    case STATUS_TYPE:
      return STATUS_BLOCK_SIZE;*/
  }
}

void init_curr_stack_ptrs() {
  uint32_t stack_addr = SCI_CURR_PTR_ADDR;
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
  mem_write_multibyte(curr_ptr, header, HEADER_SIZE*FIELD_SIZE);
}

// fields are indexed from ZERO
void write_to_flash(uint8_t type, uint8_t field_num, uint8_t * data) {
  uint32_t curr_ptr;
  if (field_num == 0x00) {
    time_t time = read_time();
    date_t date = read_date();
    uint8_t error = 0xFF;
    uint8_t header[8] = {time.hh, time.mm, time.ss, date.yy,
                        date.mm, date.dd, error, 0};
    init_header(header, type);
    uint8_t headerID = init_block(type);
  }
  curr_ptr = eeprom_read_dword (pointer(type));
  //mem_write_multibyte((curr_ptr + FIELD_SIZE*(field_num + HEADER_SIZE)), data, FIELD_SIZE);
  mem_write_multibyte((curr_ptr + FIELD_SIZE*(HEADER_SIZE)), data, block_size(type)*FIELD_SIZE);
  print ("Address written to is : %x\n*******",(curr_ptr + FIELD_SIZE*HEADER_SIZE));
}

void read_from_flash (uint8_t type,uint8_t* data,uint8_t data_len) {
  uint32_t curr_ptr;
  curr_ptr = eeprom_read_dword (pointer(type));
  print ("Read from address: %x\n",init_stack(type)+block_size);
  mem_read(init_stack(type)+block_size(type)+0x05, data, data_len);
  int i;
  for (i=0; i<data_len; i++) {
      print("%02x ",data[i]);
  }
}

void init_mem(){

	// initialize the Chip Select pin
	  init_cs(MEM_CS, &MEM_DDR);
    set_cs_high(MEM_CS, &MEM_PORT);
    mem_status_w(( _BV(BPL) | _BV(BP0) | _BV(BP1) | _BV(BP2) | _BV(BP3) ));
    mem_command_short(MEM_WR_DISABLE);
    mem_erase();

}

void mem_erase(){
  mem_unlock(MEM_ALL_SECTORS);
  mem_command_short(MEM_WR_ENABLE);
  mem_command_short(MEM_ERASE);
  mem_command_short(MEM_WR_DISABLE);
  mem_lock(MEM_ALL_SECTORS);
  _delay_ms(1000);
}

void mem_write_multibyte(uint32_t address, uint8_t * data, uint8_t data_len){
  uint8_t a1 = ((address >> 16) & 0xFF);
  uint8_t a2 = ((address >> 8) & 0xFF);
  uint8_t a3 = (address & 0xFF);
  uint8_t i = 0; // counter for the loop
  uint8_t mem_busy = 1; //default assumption is that mem is busy

  mem_unlock(MEM_ALL_SECTORS);
  mem_command_short(MEM_BUSY_ENABLE); //enables hardware end-of-write detection
  mem_command_short(MEM_WR_ENABLE);

  set_cs_low(MEM_CS, &MEM_PORT);
  send_spi(MEM_WR_AAI); //all bytes to be written must be proceeded by the AAI command
  send_spi(a1);
  send_spi(a2);
  send_spi(a3);
  send_spi(*(data));

  if(data_len > 1){
    send_spi(*(data+1));
  }

  else{
    send_spi(0xFF);
  }

  while (mem_busy){
    set_cs_high(MEM_CS, &MEM_PORT);
    mem_busy = (~(bit_is_set(PINB, PB0)) & 0x01);
  }

  for(i = 2; i < data_len; i +=2){
    mem_command_short(MEM_WR_ENABLE);
    set_cs_low(MEM_CS, &MEM_PORT);
    send_spi(MEM_WR_AAI);

    if(i == (data_len - 1)){
      send_spi(*(data +i));
      send_spi(0xFF);
    }

    else{
      send_spi(*(data + i));
      send_spi(*(data + i + 1));
    }

    while (mem_busy){
      set_cs_high(MEM_CS, &MEM_PORT);
      mem_busy = (~(bit_is_set(PINB, PB0)) & 0x01);
    }

  }

  set_cs_high(MEM_CS, &MEM_PORT);
  mem_command_short(MEM_WR_DISABLE);
  mem_command_short(MEM_BUSY_DISABLE);
  mem_lock(MEM_ALL_SECTORS);
}

void mem_write_byte(uint32_t address, uint8_t data){

	uint8_t a1 = ((address >> 16) & 0xFF);
	uint8_t a2 = ((address >> 8) & 0xFF);
	uint8_t a3 = (address & 0xFF);

	mem_unlock(MEM_ALL_SECTORS); //change this at some point
	mem_command_short (MEM_WR_ENABLE);

	set_cs_low(MEM_CS, &MEM_PORT);
	send_spi(MEM_WR_BYTE);
	send_spi(a1);
	send_spi(a2);
	send_spi(a3);
	send_spi(data);
	set_cs_high(MEM_CS, &MEM_PORT);

	mem_lock(MEM_ALL_SECTORS);
	mem_command_short(MEM_WR_DISABLE);

}

void mem_read(uint32_t address, uint8_t * data, uint8_t data_len){
  uint8_t i;

	set_cs_low(MEM_CS, &MEM_PORT);
	send_spi(MEM_R_BYTE);
  send_spi((address >> 16) & 0xFF);
	send_spi((address >> 8) & 0xFF);
	send_spi(address & 0xFF);

  for (i = 0; i < data_len; i++){
    *(data + i) = send_spi(0x00);
  }

	set_cs_high(MEM_CS, &MEM_PORT);
}

void mem_sector_erase(uint8_t sector){
  uint32_t address = sector * 4096;
  mem_unlock(MEM_ALL_SECTORS);
  mem_command_short(MEM_WR_ENABLE);

  set_cs_low(MEM_CS, &MEM_PORT);
  send_spi(MEM_SECTOR_ERASE);
  send_spi(address >> 16);
	send_spi((address >> 8) & 0xFF);
	send_spi(address & 0xFF);
  set_cs_high(MEM_CS, &MEM_PORT);

	mem_lock(MEM_ALL_SECTORS);
	mem_command_short(MEM_WR_DISABLE);

}

void mem_unlock(uint8_t sector){
	uint8_t status = mem_command(MEM_READ_STATUS, 0x00);
	status &= ~(_BV(BPL));
	mem_status_w(status);
	mem_status_w(status & ~sector | _BV(BPL));

}

void mem_lock(uint8_t sector){
	uint8_t status = mem_status_r();
	status &= ~(_BV(BPL));
	mem_status_w(status);
	mem_status_w(status | sector | _BV(BPL));
}

uint8_t mem_status_r(){
	return mem_command(MEM_READ_STATUS, 0x00);
}

void mem_status_w(uint8_t status){
	mem_command_short(MEM_WR_STATUS_ENABLE);
	mem_command	(MEM_WRITE_STATUS, status);
}

uint8_t mem_command(uint8_t command, uint8_t data){
	uint8_t value;
	set_cs_low(MEM_CS, &MEM_PORT);
	send_spi(command);
	value = send_spi(data);
	set_cs_high(MEM_CS, &MEM_PORT);
	return value;
}

void mem_command_short(uint8_t command){
	set_cs_low(MEM_CS, &MEM_PORT);
	send_spi(command);
	set_cs_high(MEM_CS, &MEM_PORT);
}
