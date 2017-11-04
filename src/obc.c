#include "obc.h"

int main(void) {

    init_uart();

    //Initializing Housekeeping timer
    uint8_t hk_req_flag = 0;
    char msg[] = "OBC Requesting Housekeeping From All SSMs\n";

    // Generating command_queue
    struct Queue * command_queue;
    struct Command c;

    struct data_buffer databuffer;
    struct data_buffer * data_buffer_pointer  = &databuffer;
    struct housekeeping_data_buffer housekeepingbuffer;
    struct housekeeping_data_buffer * housekeeping_buffer = & housekeepingbuffer;

    initQueue(&command_queue);

    //1. add time based commands (hk_req_all, request_science)
    //2. dequeue commands

    for (;;) {

      if(dequeue(command_queue, &c) == 1) {
        print("Empty queue");
      } else {
        print("Dequeueing...");
        // c is a uint16_t
        // C is a struct defined in queue.h, only element is uint_32t
        uint16_t command_sending = c.command_to_send;
        // Data array will be empty, only sending command, only identifier needed
        uint8_t data[] = {};

        send_command(command_sending);
        // Sending can message i.e. sending the command
        can_send_message(data, 0, command_sending);
      }

  }
}

uint8_t send_command(uint16_t command) {

  switch (command) {
    case OBC_TX | ALL_RX | HK_DATA:

      if(hk_all_received) {

        // Can_send_message command for hk_req_all
        uint8_t size = 0;
        uint8_t data[] = {};
        can_send_message(data, size, command);
        hk_all_received = 0;

      }

      else {

        // hk_req_all done, send message to flash
          return 1;

      }


      break;

    default:
      return 1;

  }


}
