#include "mem.h"

void mem_byte_write_demo(){

    init_uart();
    init_spi();
    init_mem();

    uint8_t ctrl;


    for(;;){
        _delay_ms(20000);
        print("\r\n\r\n");
        mem_write_byte(0x000010, 0xA0);
        ctrl = mem_read_byte(0x000010);
        print("\r\nREAD:%x", ctrl);
    }
}


void init_mem(){

	// initialize the Chip Select pin
	init_cs(MEM_CS, &MEM_DDR);
    set_cs_high(MEM_CS, &MEM_PORT);

    mem_status_w(( _BV(BPL) | _BV(BP0) | _BV(BP1) | _BV(BP2) | _BV(BP3) ));
    mem_command_short(MEM_WR_DISABLE);

}


void mem_write_multibyte(uint32_t address, uint8_t * data, uint8_t data_len){
  uint8_t a1 = ((address >> 16) & 0xFF);
  uint8_t a2 = ((address >> 8) & 0xFF);
  uint8_t a3 = (address & 0xFF);
  uint8_t i;
  data_len = (uint32_t) data_len;

  uint8_t end = mem_read_byte(address + data_len);

  mem_unlock(MEM_ALL_SECTORS);
  mem_command_short(MEM_BUSY_ENABLE);
  mem_command_short(MEM_WR_ENABLE);

  set_cs_low(MEM_CS, &MEM_PORT);
  send_spi(MEM_WR_AAI);
  send_spi(a1);
  send_spi(a2);
  send_spi(a3);
  send_spi(*(data));

  if (data_len >= 2){
    send_spi(*(data+1));
  }
  else{
    send_spi(end);
  }

  set_cs_high(MEM_CS, &MEM_PORT);

  for(i = 2; i < data_len; i +=2){
    while (bit_is_set(PINB, PB0)){
      continue;

    if(i == (data_len - 1)){
      set_cs_low(MEM_CS, &MEM_PORT);
      send_spi(MEM_WR_AAI);
      send_spi(*(data +i));
      send_spi(end);
      set_cs_high(MEM_CS, &MEM_PORT);
    }
    else{
      send_spi(*(data + i));
      send_spi(*(data + i + 1));
      set_cs_high(MEM_CS, &MEM_PORT);
    }
    }
  }

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

uint8_t mem_read_byte(uint32_t address){
	set_cs_low(MEM_CS, &MEM_PORT);
	send_spi(MEM_R_BYTE);	send_spi(address >> 16);
	send_spi((address >> 8) & 0xFF);
	send_spi(address & 0xFF);
	uint8_t data = send_spi(0x00);
	set_cs_high(MEM_CS, &MEM_PORT);

	return data;
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
