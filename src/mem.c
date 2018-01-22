#include "mem.h"

void mem_multibyte_demo(){
  init_uart();
  init_spi();
  init_mem();

  uint8_t write[10] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
  uint8_t read[10] = {0};
  uint8_t i;

  for(;;){
      _delay_ms(2000);
      print("\r\n\r\n");
      mem_write_multibyte(0x50, write, 9);
      mem_read(0x50, read, 9);
        for (i=0; i<9; i++){
          print("\r\nREAD:%x", read[i]);
        }
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
