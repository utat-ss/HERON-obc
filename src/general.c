#include "general.h"

// Declare fresh_start as global var. to keep track of fresh start and restart
uint8_t fresh_start;

// TODO - update these update and restart values
// OBC uptime (in seconds) - total for satellite's life
volatile uint32_t obc_total_uptime_s = 0;
// OBC uptime (in seconds) - since most recent restart
volatile uint32_t obc_current_uptime_s = 0;
// Number of times OBC has started up (includes the first time)
volatile uint32_t obc_restart_count = 0;

// Count up to the time we can initialize comms - preserved between restarts
// Must be volatile since it is modified inside the timer interrupt
volatile uint32_t obc_comms_time_s = 0;
// Number of seconds since we last updated comms time in EEPROM
volatile uint32_t obc_comms_eeprom_update_time_s = 0;
// Set to true if we are currently in the process of waiting to init comms
volatile bool delaying_obc_comms = false;

void obc_1m_timer_cb(void);


// Initializes everything in OBC, EXCEPT the transceiver/comms things that must not be turned on for the first 30 minutes
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

    // TODO - use 1 second
    // Initialize timer to go off every minute
    init_timer_8bit(1, obc_1m_timer_cb);

    //init_heartbeat();
}

// Initializes the comms/transceiver parts of OBC that must be delayed after initial startup
void init_obc_comms(void) {
    init_trans();
}

// Delays 30 minutes before we can init comms
// Fetches previous value in EEPROM to see if we have already spent some time
void delay_obc_comms(void) {
    delaying_obc_comms = true;

    // See what is in EEPROM
    uint32_t read = eeprom_read_dword(OBC_COMMS_TIME_EEPROM_ADDR);
    // If the EEPROM is cleared (read default all F's), we know that we have not
    // written to this yet and it should be ignored
    if (read == EEPROM_DEF_DWORD) {
        print("EEPROM for comms not written yet\n");
        obc_comms_time_s = 0;
    } else {
        obc_comms_time_s = read;
    }
    print("obc_comms_time_s = %lu\n", obc_comms_time_s);

    // Use delays as a backup to the timer (upper bound)
    // Use 100ms increments because that it tolerable for delay precision
    print("Starting delaying for comms\n");
    for (uint32_t i = 0; (i < OBC_COMMS_TIME_DELAY * 10) &&
            (obc_comms_time_s < OBC_COMMS_TIME_DELAY); i++) {
        print("obc_comms_time_s = %lu\n", obc_comms_time_s);
        _delay_ms(100);
    }
    delaying_obc_comms = false;
    print("Done delaying for comms\n");
}

// This timer should be called repeatedly (every minute) to keep track of uptime, etc.
// TODO - should be every second
void obc_1m_timer_cb(void) {
    // Only do comms time things if we are in the process of delaying comms
    // (this timer callback will keep being called for the entire lifetime of the satellite)
    if (delaying_obc_comms) {
        // TODO - use 1s
        obc_comms_time_s += 60;
        obc_comms_eeprom_update_time_s += 60;

        // If enough time has elapsed to write the new time to EEPROM, do it
        if (obc_comms_eeprom_update_time_s >= OBC_COMMS_TIME_EEPROM_UPDATE_PERIOD) {
            eeprom_write_dword(OBC_COMMS_TIME_EEPROM_ADDR, obc_comms_time_s);
            print("Wrote obc_comms_time_s = %lu to EEPROM\n", obc_comms_time_s);
        }
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
