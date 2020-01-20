#ifndef GENERAL_H
#define GENERAL_H

#include <stdint.h>

#include <can/can.h>
#include <heartbeat/heartbeat.h>
#include <queue/queue.h>
#include <spi/spi.h>
#include <uptime/uptime.h>
#include <watchdog/watchdog.h>

#include "antenna.h"
#include "can_interface.h"
#include "commands.h"
#include "i2c.h"
#include "mem.h"
#include "rtc.h"
#include "transceiver.h"


// EEPROM address to store whether comms delay has finished
#define PHASE2_DELAY_DONE_EEPROM_ADDR   0x100
// Write 1 when it has already been done
#define PHASE2_DELAY_DONE_FLAG          0xDEADBEEF
// Number of seconds to wait before initializing phase 2 (30 minutes)
#define PHASE2_DELAY_PERIOD_S           (30 * 60)


typedef struct {
    bool in_progress;
    bool done;
    uint32_t period_s;
    uint32_t prev_uptime_s;
} phase2_delay_t;

extern phase2_delay_t phase2_delay;


void init_obc_phase1(void);
void init_obc_phase2(void);
void init_phase2_delay(void);
void run_phase2_delay(void);

void set_def_trans_beacon_content(void);

#endif
