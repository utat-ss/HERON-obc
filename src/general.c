#include "general.h"

// All time-related variables must be volatile since they are modified inside
// the timer interrupt
// Note that 1 billion seconds is about 31.7 years

// Number of times OBC has started up (includes 1 for the first time)
volatile uint32_t restart_count = 0;
// OBC uptime (in seconds) - since most recent restart
volatile uint32_t uptime_s = 0;

// Set to true if we are currently inside the function `delay_comms()`
// (in the process of waiting to init comms)
volatile bool delaying_comms = false;
// Count up to the time we can initialize comms - preserved between restarts
volatile uint32_t comms_time_s = 0;
// The threshold time that comms_time_s is counting up to
volatile uint32_t comms_time_threshold_s = COMMS_TIME_DELAY;
// Number of seconds since we last updated comms time in EEPROM
volatile uint32_t comms_eeprom_update_time_s = 0;


void timer_cb_1m(void);


// Initializes everything in OBC, EXCEPT the transceiver/comms things that must
// not be turned on for the first 30 minutes
void init_obc_core(void) {
    init_uart();

    init_spi();

    init_rtc();

    init_mem();
    read_all_mem_sections_eeprom();

    init_queue(&eps_tx_msg_queue);
    init_queue(&pay_tx_msg_queue);
    init_queue(&data_rx_msg_queue);

    init_queue(&cmd_queue);

    init_can();
    init_rx_mob(&data_rx_mob);
    init_tx_mob(&pay_cmd_tx_mob);
    init_tx_mob(&eps_cmd_tx_mob);

    // Read the restart count stored in EEPROM
    restart_count = eeprom_read_dword(RESTART_COUNT_EEPROM_ADDR);
    if (restart_count == EEPROM_DEF_DWORD) {
        restart_count = 0;
    }
    // Increment the restart count and write it back to EEPROM
    restart_count++;
    eeprom_write_dword(RESTART_COUNT_EEPROM_ADDR, restart_count);

    // Read the comms time stored in EEPROM
    comms_time_s = eeprom_read_dword(COMMS_TIME_EEPROM_ADDR);
    // If the EEPROM is cleared (read default all 1's), we know that we have not
    // written to this address yet and the existing value should be ignored
    if (comms_time_s == EEPROM_DEF_DWORD) {
        comms_time_s = 0;
    }

    // TODO - use 1 second
    // Initialize timer to go off every minute
    init_timer_8bit(1, timer_cb_1m);
}

// Initializes the comms/transceiver parts of OBC that must be delayed after initial startup
void init_obc_comms(void) {
    init_trans();
}

// Delays 30 minutes before we can init comms
// Fetches previous value in EEPROM to see if we have already spent some time
void delay_comms(void) {
    // print("Starting %s\n", __FUNCTION__);
    delaying_comms = true;

    // Use delays as a backup to the timer (upper bound)
    // Use 100ms increments because that it tolerable for delay precision
    // print("Starting delay loop\n");
    // Use the constant COMMS_TIME_DELAY as a backup,
    // but use the modifiable comms_time_threshold_s as the intended condition
    for (uint32_t i = 0; (i < COMMS_TIME_DELAY * 10) &&
            (comms_time_s < comms_time_threshold_s); i++) {
        _delay_ms(100);

        // If enough time has elapsed to write the new time to EEPROM, do it
        if (comms_eeprom_update_time_s >= COMMS_EEPROM_UPDATE_TIME_THRESH) {
            eeprom_write_dword(COMMS_TIME_EEPROM_ADDR, comms_time_s);
            print("Wrote comms_time_s = %lu to EEPROM\n", comms_time_s);
            comms_eeprom_update_time_s = 0;
        }
    }

    delaying_comms = false;
    // print("Done %s\n", __FUNCTION__);
}

// This timer should be called repeatedly (every minute) to keep track of uptime, etc.
// TODO - should be every second
void timer_cb_1m(void) {
    print("1 min timer cb\n");

    // TODO - 1s
    uptime_s += 60;
    print("uptime_s = %lu\n", uptime_s);

    // Only do comms time things if we are in the process of delaying comms
    // (this timer callback will keep being called for the entire lifetime of the satellite but this block should only execute in the first 30 minutes)
    if (delaying_comms) {
        // TODO - use 1s
        comms_time_s += 60;
        comms_eeprom_update_time_s += 60;
    }
}

// If the command queue is not empty, dequeues the next command and executes it
void execute_next_cmd(void) {
    if (!queue_empty(&cmd_queue) && current_cmd.fn == nop_fn) {
        // Fetch the next command
        cmd_t cmd;
        dequeue_cmd(&cmd_queue, &cmd);

        // Set the global current command to prevent other commands from running
        current_cmd = cmd;

        // Run the command's function
        print("Starting cmd\n");
        (cmd.fn)();
    }
}

// If there is an RX messsage in the queue, handle it
void process_next_rx_msg(void) {
    if (!queue_empty(&data_rx_msg_queue)) {
        handle_rx_msg();
    }
}

/*
If there is a TX message in the EPS queue, sends it

When resume_mob(mob name) is called, it:
1) resumes the MOB
2) triggers an interrupt (callback function) to get the data to transmit
3) sends the data
4) pauses the mob
*/
void send_next_eps_tx_msg(void) {
    if (!queue_empty(&eps_tx_msg_queue)) {
        resume_mob(&eps_cmd_tx_mob);
    }
}

/*
If there is a TX message in the PAY queue, sends it

When resume_mob(mob name) is called, it:
1) resumes the MOB
2) triggers an interrupt (callback function) to get the data to transmit
3) sends the data
4) pauses the mob
*/
void send_next_pay_tx_msg(void) {
    if (!queue_empty(&pay_tx_msg_queue)) {
        resume_mob(&pay_cmd_tx_mob);
    }
}
