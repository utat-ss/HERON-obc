#include "general.h"

// Date and time of the most recent restart
rtc_date_t restart_date = { .yy = 0, .mm = 0, .dd  = 0 };
rtc_time_t restart_time = { .hh = 0, .mm = 0, .ss  = 0 };


// Initializes everything in OBC, EXCEPT the transceiver/comms things that must
// not be turned on for the first 30 minutes
void init_obc_core(void) {
    init_uart();

    init_spi();
    init_i2c();

    init_rtc();

    init_mem();
    read_all_mem_sections_eeprom();

    init_queue(&eps_tx_msg_queue);
    init_queue(&pay_tx_msg_queue);
    init_queue(&data_rx_msg_queue);

    init_queue(&cmd_queue);
    init_queue(&cmd_args_queue);

    init_can();
    init_rx_mob(&data_rx_mob);
    init_tx_mob(&pay_cmd_tx_mob);
    init_tx_mob(&eps_cmd_tx_mob);

    restart_date = read_rtc_date();
    restart_time = read_rtc_time();
    init_uptime();

    add_uptime_callback(auto_data_col_timer_cb);
    add_uptime_callback(can_timer_cb);
}

// Initializes the comms/transceiver parts of OBC that must be delayed after initial startup
void init_obc_comms(void) {
    // TODO
    init_trans();
}


// If the command queue is not empty, dequeues the next command and executes it
void execute_next_cmd(void) {
    if (!queue_empty(&cmd_queue) && current_cmd == &nop_cmd) {
        print("Starting cmd\n");
        // Fetch the next command
        dequeue_cmd();
        // Run the command's function
        (current_cmd->fn)();
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
