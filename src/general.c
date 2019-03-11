#include "general.h"

// Count up to the time we can initialize comms - preserved between restarts
volatile uint32_t comms_time_s = 0;
// The threshold time that comms_time_s is counting up to
volatile uint32_t comms_thresh_s = COMMS_TIME_DELAY;
// Number of seconds since we last updated comms time in EEPROM
volatile uint32_t comms_eeprom_update_time_s = 0;


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

    init_comms_time();

    rtc_date_t date = read_rtc_date();
    rtc_time_t time = read_rtc_time();
    init_uptime(date, time);
}

// Initializes the comms/transceiver parts of OBC that must be delayed after initial startup
void init_obc_comms(void) {
    // TODO
    // init_trans();
}

void init_comms_time(void) {
    // Read the comms time stored in EEPROM
    comms_time_s = eeprom_read_dword(COMMS_TIME_EEPROM_ADDR);
    // If the EEPROM is cleared (read default all 1's), we know that we have not
    // written to this address yet and the existing value should be ignored
    if (comms_time_s == EEPROM_DEF_DWORD) {
        comms_time_s = 0;
    }
}

// Delays 30 minutes before we can init comms
// Fetches previous value in EEPROM to see if we have already spent some time
void delay_comms(void) {
    // print("Starting %s\n", __FUNCTION__);

    // Use delays as a backup to the timer (upper bound)
    // Use 100ms increments because that it tolerable for delay precision
    // Use the constant COMMS_TIME_DELAY as a backup,
    // but use the modifiable comms_thresh_s as the intended condition

    // Keep a cached (saved) version of the global OBC uptime
    uint32_t cached_uptime_s = uptime_s;

    // print("Starting delay loop\n");

    // Multiply by 10 because we are using delays of 1/10 seconds
    for (uint32_t i = 0; (i < COMMS_TIME_DELAY * 10) &&
            (comms_time_s < comms_thresh_s); i++) {

        _delay_ms(100);

        // Check if the uptime has changed from what we have cached
        if (uptime_s > cached_uptime_s) {
            uint32_t diff = uptime_s - cached_uptime_s;
            comms_time_s += diff;
            comms_eeprom_update_time_s += diff;

            cached_uptime_s = uptime_s;

            print("Updated comms_time_s = %lu\n", comms_time_s);
            print("Updated comms_eeprom_update_time_s = %lu\n",
                comms_eeprom_update_time_s);
        }

        // If enough time has elapsed, save the time to EEPROM
        if (comms_eeprom_update_time_s >= COMMS_TIME_EEPROM_UPDATE_THRESH) {
            eeprom_write_dword(COMMS_TIME_EEPROM_ADDR, comms_time_s);
            print("Wrote comms_time_s = %lu to EEPROM\n", comms_time_s);
            comms_eeprom_update_time_s = 0;
        }
    }

    // print("Done %s\n", __FUNCTION__);
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
