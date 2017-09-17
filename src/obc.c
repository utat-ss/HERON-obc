
#include "obc.h"

int main(void) {

    init_uart();

    //Initializing Housekeeping timer
    clock_t last_check = clock();
    int hk_req_flag = 0;
    char msg[] = "OBC Requesting Housekeeping From All SSMs\n";

    // Generating command_queue
    struct Queue * command_queue;
    struct Command c;

    struct data_buffer databuffer;
    data_buffer_pointer = &databuffer;
    struct housekeeping_data_buffer housekeepingbuffer;
    housekeeping_buffer = & housekeepingbuffer;


    initQueue(&command_queue);

    //1. add time based commands (hk_req_all, request_science)
    //2. dequeue commands

    for (;;) {

    // ** Adding time based commands to the command queue **

      clock_t difference = clock() - last_check;
      // This is where housekeeping is done
      if (difference/CLOCKS_PER_SEC > 300 && !hk_req_flag){
        hk_req_flag = 1;
      }

      //Send Housekeeping request and reset timer
      if (hk_req_flag) {

        // 2 Protocols - All & SSM specific

        // All

        // Checking receiver_id is unique for 3 different SSMs - flag when ssm is done

        hk_flag_pay = 0;
        hk_flag_comms = 0;
        hk_flag_eps = 0;
        hk_all_received = 0;
        hk_all_requested = 1;

        // Wait until all housekeeping is received
        while(!hk_req_all_received) {

          if(hk_flag_pay && hk_flag_comms && hk_flag_eps) {
            hk_req_all_received = 1;
          }

          hk_flag_pay = 0;
          hk_flag_comms = 0;
          hk_flag_eps = 0;

        }

        print("%s", msg);


        // Add can_send_message command for hk_req_all

        uint8_t size = 0;
        uint16_t id = OBC_TX | ALL_RX | HK_DATA;
        uint8_t data[] = {};
        can_send_message(data, size, id);
        last_check = clock();
        hk_req_flag = 0;

      }

    // ** Dequeueing the command queue **

      //Deques the command

      if(dequeue(command_queue, &c) == 1) {
        print("Empty queue");
      } else {
        print("Dequeueing...");
        // c is a uint16_t
        // C is a struct defined in queue.h, only element is uint_32t
        uint16_t command_sending = c.command_to_send;
        // Data array will be empty, only sending command, only identifier needed
        uint8_t data[] = {};
        // Sending can message i.e. sending the command
        can_send_message(data, 0, command_sending);
      }

  }
}
