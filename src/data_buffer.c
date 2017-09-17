#include "data_buffer.h"

// initialize data buffer
void init_data_buffer(struct data_buffer* dptr){
  // only initialize it to Max for initialzation, should get changed in future
  dptr->total_size = MAXBUFFERSIZE;
  dptr->curr_size = 0;
  return;
}

void set_total_size(struct data_buffer* dptr,int size){
  dptr->total_size = size;
  return;
}

// add new data to the buffer
int add_to_buffer(struct data_buffer* dptr, uint8_t data){
  if (!is_full(dptr)){
    dptr->buffer[dptr->curr_size] = data;
    dptr->curr_size++;
    return 0;
  }
  else{
    return 1;
  }
}

// check if the buffer is full or not (i.e. reach its max size)
bool is_full(struct data_buffer* dptr){
  return dptr->curr_size == dptr->total_size;
}

void send_to_flash(struct data_buffer* dptr){
  //can_send_message() to flash
  //D->curr_size = 0;
  return;
}
