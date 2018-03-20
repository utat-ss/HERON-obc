#include "mem.h"

uint8_t init_stack(uint8_t type){
  switch(type){
    case SCI_TYPE:
      SCI_INIT;
    case PAY_HK_TYPE:
      PAY_INIT;
    case EPS_HK_TYPE:
      EPS_INIT;
    case OBC_HK_TYPE:
      OBC_INIT;
    case STATUS_TYPE:
      STATUS_INIT;
    }
}

uint8_t pointer(uint8_t type){
  switch(type){
    case SCI_TYPE:
      SCI_STACK_PTR;
    case PAY_HK_TYPE:
      PAY_HK_STACK_PTR;
    case EPS_HK_TYPE:
      EPS_HK_STACK_PTR;
    case OBC_HK_TYPE:
      OBC_HK_STACK_PTR;
    case STATUS_TYPE:
      STATUS_PTR;
    }
}

uint8_t block_size(uint8_t type){
  switch(type){
    case SCI_TYPE:
      SCI_BLOCK_SIZE;
    case PAY_HK_TYPE:
      PAY_BLOCK_SIZE;
    case EPS_HK_TYPE:
      EPS_BLOCK_SIZE;
    case OBC_HK_TYPE:
      OBC_BLOCK_SIZE;
    case STATUS_TYPE:
      STATUS_BLOCK_SIZE;
    }
}

void init_stacks(){
  uint8_t a1 = ((SCI_INIT >> 16) & 0xFF);
  uint8_t a2 = ((SCI_INIT >> 8) & 0xFF);
  uint8_t a3 = (SCI_INIT & 0xFF);

  uint8_t a4 = ((PAY_INIT >> 16) & 0xFF);
  uint8_t a5 = ((PAY_INIT >> 8) & 0xFF);
  uint8_t a6 = (PAY_INIT & 0xFF);

  uint8_t a7 = ((EPS_INIT >> 16) & 0xFF);
  uint8_t a8 = ((EPS_INIT >> 8) & 0xFF);
  uint8_t a9 = (EPS_INIT & 0xFF);

  uint8_t a10 = ((OBC_INIT >> 16) & 0xFF);
  uint8_t a11 = ((OBC_INIT >> 8) & 0xFF);
  uint8_t a12 = (OBC_INIT & 0xFF);

  uint8_t a13 = ((STATUS_INIT >> 16) & 0xFF);
  uint8_t a14 = ((STATUS_INIT >> 8) & 0xFF);
  uint8_t a15 = (STATUS_INIT & 0xFF);

  uint8_t INIT[15] = {a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12,
                      a13, a14, a15};

  mem_write_multibyte(SCI_STACK_PTR, INIT, 0x0F);
}

//read curr_ptr and update by BLOCK_SIZE
uint8_t init_block(uint8_t type){
  uint32_t curr_ptr;
  uint8_t sector[REFRESH_SECTOR];
  mem_read(0x00, sector, REFRESH_SECTOR_SIZE);

  curr_ptr = (uint32_t) sector[pointer(type)] << 16 + (uint32_t)(sector[pointer(type) + 1] << 8)
            + (uint32_t)(sector[pointer(type) + 2]);

  curr_ptr += block_size(type);

  mem_sector_erase(0x01);

  sector[pointer(type) + 2] = (curr_ptr & 0xFF);
  sector[pointer(type) + 1] = ((curr_ptr >> 8) & 0xFF);
  sector[pointer(type)] = (curr_ptr >> 16) & 0xFF;

  mem_write_multibyte(0x00, sector, REFRESH_SECTOR);
  return (uint8_t) ((curr_ptr-init_stack(type))/block_size(type));
}

void init_header(uint8_t *header, uint8_t type){
  uint32_t curr_ptr;
  mem_read(pointer(type), &curr_ptr, 0x03);
  mem_write_multibyte(curr_ptr, header, HEADER_SIZE*FIELD_SIZE);
}

// fields are indexed from ZERO
void write_to_flash(uint8_t type, uint8_t field_num, uint8_t * data) {
  uint32_t curr_ptr;
  mem_read(pointer(type), &curr_ptr, 0x03);

  if(field_num == 0x00){
    uint8_t headerID = init_block(type);

    time_t time = read_time();
    date_t date = read_date();
    uint8_t error = 0xFF;
    uint8_t header[8] = {time.hh, time.mm, time.ss, date.yy,
                        date.mm, date.dd, error, headerID};
    init_header(header, type);
  }
    mem_write_multibyte((curr_ptr + FIELD_SIZE*(field_num + HEADER_SIZE)), data, FIELD_SIZE);
}


void read_sci_block(uint8_t block_num, uint8_t * data){
}

void read_field(uint8_t block_num, uint8_t * data){
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
}

void mem_write_multibyte(uint32_t address, uint8_t * data, uint8_t data_len){
  uint8_t a1 = ((address >> 16) & 0xFF);
  uint8_t a2 = ((address >> 8) & 0xFF);
  uint8_t a3 = (address & 0xFF);
  uint8_t i = 0; // counter for the loop
  uint8_t mem_busy = 1; //default assumption is that mem is busy
  uint8_t end;

  mem_read(((uint32_t) address + data_len), &end, 1);
  /* AAI only except data in pairs, so if an odd number of bytes are to be written,
  we do not want to overwrite the last byte */

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
    send_spi(end);
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
      send_spi(end);
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
  send_spi(address >> 16);
	send_spi((address >> 8) & 0xFF);
	send_spi(address & 0xFF);

  for (i = 0; i < data_len; i++){
    *(data + i) = send_spi(0x00);
  }

	set_cs_high(MEM_CS, &MEM_PORT);
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
