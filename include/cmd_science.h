#ifndef SCIENCE_H
#define SCIENCE_H

// #include "global_header.h"
#include  <stdint.h>

#include "obc.h"
#include "can_queue.h"
#include "housekeeping.h"

#define SCI_BLOCK_SIZE 218
#define SCI_TYPE 0 // kamran will define in his code, delete this then

void req_sci_timer_callback();
void receive_science(uint8_t field_num, uint8_t* data);

#endif
