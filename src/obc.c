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
    init_queue(&cmd_arg_queue);

    //init_heartbeat();
    print("Initalized.\n");
}

void execute_next_cmd(void) {
    if (!queue_empty(&cmd_queue)) {
        // dequeue the latest command and execute it
        cmd_fn_t cmd;
        //print("Dequeueing\n");
        dequeue(&cmd_queue, (uint8_t*) &cmd);
        //print("Dequeued command\n");
        (cmd)();
        // Now, callbacks are no longer executed in ISRs, so we
        // can actually resume/pause MObs inside them
    }
}
