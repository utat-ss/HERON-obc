#include "obc.h"

// Declare fresh_start as global var. to keep track of fresh start and restart
uint8_t fresh_start;

uint8_t ssm_id;
uint8_t receiving_id;

// Initializes everything in OBC, EXCEPT the transceiver/comms things that must not be turned on for the first 30 minutes
void init_obc_core(void) {
    init_uart();

    init_spi();

    init_can();
    init_rx_mob(&data_rx_mob);
    init_tx_mob(&pay_cmd_tx_mob);
    init_tx_mob(&eps_cmd_tx_mob);

    init_mem();
    init_rtc();

    init_queue(&eps_tx_msg_queue);
    init_queue(&pay_tx_msg_queue);
    init_queue(&data_rx_msg_queue);

    init_queue(&cmd_queue);

    //init_heartbeat();
}

// Initializes the comms/transceiver parts of OBC that must be delayed after initial startup
void init_obc_comms(void) {
    // TODO - some timer things
}

// If the command queue is not empty, dequeues the next command and executes it
void execute_next_cmd(void) {
    if (!queue_empty(&cmd_queue)) {
        cmd_t cmd;
        dequeue_cmd(&cmd_queue, &cmd);
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
