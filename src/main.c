#include "general.h"

int main(void) {
    WDT_OFF();
    WDT_ENABLE_SYS_RESET(WDTO_8S);

    init_obc_core();
    
    run_comms_delay();
    deploy_antenna();
    init_obc_comms();

    while (1) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);

        send_next_eps_tx_msg();
        send_next_pay_tx_msg();
        process_next_rx_msg();

        execute_next_cmd();

        send_trans_tx_enc_msg();
        encode_trans_tx_msg();
        handle_trans_rx_dec_msg();
        decode_trans_rx_msg();
    }

    return 0;
}
