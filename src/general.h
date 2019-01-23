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

// EEPROM address to store number of seconds - counting to comms init
#define OBC_COMMS_TIME_EEPROM_ADDR ((uint32_t*) 0x34)
// Number of seconds between updating the value in EEPROM
// i.e. should only update EEPROM when obc_comms_time_s is a multiple of this
#define OBC_COMMS_TIME_EEPROM_UPDATE_PERIOD 60
// Number of seconds to wait before initializing comms
#define OBC_COMMS_TIME_DELAY (30 * 60)

// Default double word (4-byte) value in EEPROM
#define EEPROM_DEF_DWORD 0xFFFFFFFF

void init_obc_core(void);
void init_obc_comms(void);

void execute_next_cmd(void);

void process_next_rx_msg(void);
void send_next_eps_tx_msg(void);
void send_next_pay_tx_msg(void);

#endif
