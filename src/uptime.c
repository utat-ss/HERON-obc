/*
Functionality to track a global uptime (in seconds since last restart) across
all of OBC using the 8-bit timer. Also tracks the number of times OBC has
restarted using EEPROM.
*/

#include "uptime.h"

// All time-related variables must be volatile since they are modified inside
// the timer interrupt
// Note that 1 billion seconds is about 31.7 years

// Number of times OBC has started up (includes 1 for the first time)
volatile uint32_t restart_count = 0;
// OBC uptime (in seconds) - since most recent restart
volatile uint32_t uptime_s = 0;

void uptime_timer_cb(void);


void init_restart_count(void) {
    // Read the restart count stored in EEPROM
    restart_count = eeprom_read_dword(RESTART_COUNT_EEPROM_ADDR);
    if (restart_count == EEPROM_DEF_DWORD) {
        restart_count = 0;
    }
    // Increment the restart count and write it back to EEPROM
    restart_count++;
    eeprom_write_dword(RESTART_COUNT_EEPROM_ADDR, restart_count);
}

void start_uptime_timer(void) {
    // TODO - use 1 second
    // Initialize timer to go off every minute
    init_timer_8bit(1, uptime_timer_cb);
}

// This timer should be called repeatedly (every minute) to keep track of uptime
// TODO - should be every second
void uptime_timer_cb(void) {
    print("uptime timer cb\n");

    // TODO - 1s
    uptime_s += 60;
    print("uptime_s = %lu\n", uptime_s);
}
