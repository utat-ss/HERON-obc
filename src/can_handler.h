#ifndef _can_handler_h
#define _can_handler_h

#include "global_header.h"

// Global variables, sorry, data buffer and housekeeping_data_buffer used in files can_handler and obc
struct data_buffer * data_buffer_pointer;
struct housekeeping_data_buffer * housekeeping_buffer;

// Global variables used in files can_handler and obc
uint8_t hk_flag_pay;
uint8_t hk_flag_comms;
uint8_t hk_flag_eps;
uint8_t hk_all_received;
uint8_t hk_all_requested;

void can_handler (uint16_t identifier, uint8_t* pt_data, uint8_t size);

#endif
