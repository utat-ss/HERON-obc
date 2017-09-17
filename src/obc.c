
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

    initQueue(&command_queue);

    //1. dequeue commands
    //2. add time based commands (hk_req_all, request_science)
    for (;;) {


      clock_t difference = clock() - last_check;
      // This is where housekeeping is done
      if (difference/CLOCKS_PER_SEC > 300 && !hk_req_flag){
        hk_req_flag = 1;
      }

      //Send Housekeeping request and reset timer
      if (hk_req_flag){

        // 2 Protocols - All & SSM specific

        // All

        // Checking receiver_id is unique for 3 different SSMs - flag when ssm is done

        // Protocol for housekeeping data, start byte of housekeeping will be representation of the amount of bytes to be sent in hex
        // Ensure all bytes are received using a count
        // End of houskeeping will be FF --> confirm?
        // pt_data[0] gives size
        // total count = pt_data[0]
        // total count - size = bytes to wait for, do not send to comms just yet

        // Remember after the first CAN message is sent another SSM can send a message but

        // Data buffer

        // Can send can message to comms when buffer is full but still wait for incoming messages, send by ssm

        print("%s", msg);
        uint8_t size = 0;
        uint16_t id = OBC_TX | ALL_RX | HK_DATA;
        uint8_t data[] = {};
        can_send_message(data, size, id);
        last_check = clock();
        hk_req_flag = 0;

      }

      //Deques the command
      if(dequeue(command_queue, &c) == 1) {
        print("Empty queue");
      } else {
        print("Dequeueing...");
        // c is a uint32_t
        // C is a struct defined in queue.h, only element is uint_32t
        uint16_t command_sending = c.command_to_send;
        // Data array will be empty, only sending command, only identifier needed
        uint8_t data[] = {};
        // Sending can message i.e. sending the command
        can_send_message(data, 0, command_sending);
      }

  }
}
