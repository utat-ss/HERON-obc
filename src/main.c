#include "general.h"

int main(void) {
    WDT_OFF();
    WDT_ENABLE_SYS_RESET(WDTO_8S);

    init_obc_phase1_core();
    print("\n\nOBC main init (no dep version)\n\n");
    init_obc_phase1_comms();
    init_hb(HB_OBC);
    
    while (1) {
        WDT_ENABLE_SYS_RESET(WDTO_8S);

        run_phase2_delay();

        run_hb();

        run_auto_data_col();

        decode_trans_rx_msg();
        handle_trans_rx_dec_msg();

        process_trans_tx_ack();
        encode_trans_tx_msg();
        send_trans_tx_enc_msg();

        execute_next_cmd();

        send_next_eps_tx_msg();
        send_next_pay_tx_msg();
        process_next_rx_msg();

        encode_trans_tx_msg();
        send_trans_tx_enc_msg();
    }

    return 0;
}
