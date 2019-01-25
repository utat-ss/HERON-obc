#ifndef GENERAL_H
#define GENERAL_H

#include <stdint.h>

#include <can/can.h>
#include <queue/queue.h>
#include <spi/spi.h>

#include "can_interface.h"
#include "commands.h"
#include "mem.h"
#include "rtc.h"
#include "transceiver.h"
#include "uptime.h"

// EEPROM address to store number of seconds - counting to comms init
#define COMMS_TIME_EEPROM_ADDR ((uint32_t*) 0x34)
// Threshold for number of seconds to wait before updating the value in EEPROM
#define COMMS_TIME_EEPROM_UPDATE_THRESH 60
// Number of seconds to wait before initializing comms (30 minutes)
#define COMMS_TIME_DELAY (30 * 60)


extern volatile uint32_t comms_time_s;
extern volatile uint32_t comms_thresh_s;
extern volatile uint32_t comms_eeprom_update_time_s;

void init_obc_core(void);
void init_obc_comms(void);
void init_comms_time(void);
void delay_comms(void);

void execute_next_cmd(void);

void process_next_rx_msg(void);
void send_next_eps_tx_msg(void);
void send_next_pay_tx_msg(void);

#endif
