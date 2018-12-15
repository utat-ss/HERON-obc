#include "general.h"

int main(void) {
    init_obc_core();
    init_obc_comms();

    while (1) {
        send_next_eps_tx_msg();
        send_next_pay_tx_msg();

        process_next_rx_msg();

        execute_next_cmd();
    }

    return 0;
}
