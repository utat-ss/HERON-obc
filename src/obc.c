#include "obc.h"

int main(void) {

    init_uart();

    print("starting main\n");

    init_timer(1,hk_req_all);
    //init_timer(1,request_science);

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
    // enqueue(&command_queue,request_tc,1,2);
    // enqueue(&command_queue,request_tc,3,4);
    // enqueue(&command_queue,request_tc,5,6);
    // enqueue(&command_queue,request_tc,7,8);

    //2. dequeue commands

    for (;;) {

      if(dequeue(&command_queue,&c)){
        print("Empty queue");
      } else {
        print("Dequeueing...");
        (*c.func)(c.receiver,c.data);
        // Sending can message i.e. sending the command
        // can_send_message(data, 0, command_sending);
      }

  }
}
