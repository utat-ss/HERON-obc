#define MAXBUFFERSIZE 32

#include  <stdbool.h>
#include  <stdint.h>

// data buffer for commands
struct data_buffer{
  uint8_t buffer[MAXBUFFERSIZE];
  int curr_size;
  int total_size;
};

void init_data_buffer(struct data_buffer* dptr);
void set_total_size(struct data_buffer* dptr,int size);
int add_to_buffer(struct data_buffer* dptr, uint8_t data);
bool is_full(struct data_buffer* dptr);
void send_to_flash(struct data_buffer* dptr);

/*
    struct data_buffer data_packages;
    init_data_buffer(&data_packages);
*/
