#include "obc.h"

int main(void) {
    queue_t *sci_tx_queue, *pay_hk_tx_queue, *eps_hk_tx_queue, *obc_queue;
    init_uart();
    init_callbacks();
    // change these times
    init_timer(1,req_hk_timer_callback);
    init_timer(1,req_sci_timer_callback);

    print("starting main\n");

    //Initializing Housekeeping timer
    uint8_t hk_req_flag = 0;
    char msg[] = "OBC Requesting Housekeeping From All SSMs\n";

    // Generating command_queue
    Cmd_queue command_queue;
    Command c;

    struct data_buffer databuffer;
    struct data_buffer * data_buffer_pointer  = &databuffer;
    struct housekeeping_data_buffer housekeepingbuffer;
    struct housekeeping_data_buffer * housekeeping_buffer = & housekeepingbuffer;

    command_queue = initQueue();

    //1. add time based commands (hk_req_all, reques,&ctscience)
    for(uint8_t i = 0; i < 30; i++) // garbage test to see if stops at 50 cmds (it did)
      enqueue(&command_queue,request_tc,i,i);
    for(uint8_t i = 0; i < 5; i++){
      if(dequeue(&command_queue,&c))
        print("Empty queue\n");
      else
        print("data: %d\n",c.data);
    }
    for(uint8_t i = 0; i < 26; i++)
      enqueue(&command_queue,request_tc,i,i);
    for(uint8_t i = 0; i < 52; i++){
      if(dequeue(&command_queue,&c))
        print("Empty queue\n");
      else
        print("data: %d\n",c.data);
    }

    //2. dequeue commands
    while (1) {
      /*
      if(dequeue(&command_queue,&c)){
        //print("Empty queue\n");
      } else {
        print("Dequeueing...\n");
        (*c.func)(c.receiver,c.data);
        // Sending can message i.e. sending the command
        // can_send_message(data, 0, command_sending);
      }

      if (!is_empty(sci_tx_queue)) {
          int field_num = dequeue(sci_tx_queue);
          GLOBAL_SCI_FIELD_NUM = field_num;
          resume_mob(&PAY_CMD_Tx_MOb);
      }
      if (!is_empty(pay_hk_tx_queue)) {
          int field_num = dequeue(pay_hk_tx_queue);
          GLOBAL_PAY_HK_FIELD_NUM = field_num;
          resume_mob(&PAY_CMD_Tx_MOb);
      }
      if (!is_empty(eps_hk_tx_queue)) {
          int field_num = dequeue(eps_hk_tx_queue);
          GLOBAL_EPS_HK_FIELD_NUM = field_num;
          resume_mob(&EPS_CMD_Tx_MOb);
      }
      if (!is_empty(obc_queue)) {
          int obc_queue_date = dequeue(obc_queue);
          // handle here
      }
      */
    }
}
