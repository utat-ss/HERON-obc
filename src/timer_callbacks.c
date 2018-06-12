#include "timer_callbacks.h"

void req_sci_timer_callback(){
    CANQ_enqueue(&sci_tx_queue, 0);
    print("Enqueued Science TX Message\n");
}

void req_hk_timer_callback(){
    // CANQ_enqueue(&pay_hk_tx_queue, 0);
    // //CANQ_enqueue(&eps_hk_tx_queue, 0);
    // print("Enqueued Housekeeping TX Messages\n");
    print("\n2 minutes!!!!\n");
}
