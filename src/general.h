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

void init_obc_core(void);
void init_obc_trans(void);

#endif
