#ifndef DATABUFFER
#define DATABUFFER

#define HOUSEKEEPINGPACKET 24
#define MAXBUFFERSIZE 32

#include  "global_header.h"

// data buffer for housekeeping
struct data_buffer{
  uint8_t buffer[MAXBUFFERSIZE];
  uint8_t curr_size;
  uint8_t total_size;
};

// data buffer for misc. data
struct housekeeping_data_buffer{
  uint8_t buffer[HOUSEKEEPINGPACKET];
  uint8_t curr_size;
  uint8_t total_size;
};

void init_data_buffer(struct data_buffer* dptr);
void set_total_size(struct data_buffer* dptr,uint8_t size);
uint8_t add_to_buffer(struct data_buffer* dptr, uint8_t data);
uint8_t is_full(struct data_buffer* dptr);
void send_to_flash(struct data_buffer* dptr);

/*
    struct data_buffer data_packages;
    init_data_buffer(&data_packages);
*/
#endif
