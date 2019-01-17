#include "commands.h"

void req_eps_hk_fn(void);
void req_pay_hk_fn(void);
void req_pay_opt_fn(void);
void pop_blister_packs_fn(void);
void write_flash_fn(void);
void read_flash_fn(void);


// Queue of commands that need to be executed but have not been executed yet
queue_t cmd_queue;

// All possible commands
cmd_t req_eps_hk_cmd = {
    .fn = req_eps_hk_fn
};
cmd_t req_pay_hk_cmd = {
    .fn = req_pay_hk_fn
};
cmd_t req_pay_opt_cmd = {
    .fn = req_pay_opt_fn
};
cmd_t pop_blister_packs_cmd = {
    .fn = pop_blister_packs_fn
};
cmd_t write_flash_cmd = {
    .fn = write_flash_fn
};
cmd_t read_flash_cmd = {
    .fn = read_flash_fn
};




// Command callback functions

// Starts requesting EPS HK data (field 0)
void req_eps_hk_fn(void) {
    enqueue_eps_hk_req_can_msg(0);
}

// Starts requesting PAY HK data (field 0)
void req_pay_hk_fn(void) {
    enqueue_pay_hk_req_can_msg(0);
}

// Starts requesting PAY SCI data (field 0)
void req_pay_opt_fn(void) {
    enqueue_pay_opt_req_can_msg(0);
}

// Sends the command to actuate the motors
void pop_blister_packs_fn(void) {
    enqueue_pop_blister_packs_can_msg();
}

// TODO
void write_flash_fn(void) {
  // write_to_flash(PAY_HK_TYPE,0,(uint8_t*) pay_hk_data);
  // write_to_flash(SCI_TYPE,0,(uint8_t*) pay_opt_data);
  // write_to_flash(EPS_HK_TYPE,0,(uint8_t*) eps_hk_data);
}

// TODO
void read_flash_fn(void) {
    // TODO
    // read_from_flash(PAY_HK_TYPE,(uint8_t*)pay_hk_data,CAN_PAY_HK_GET_COUNT*0X04);
    // TODO
    // read_from_flash(SCI_TYPE, (uint8_t*)pay_opt_data,CAN_PAY_SCI_GET_COUNT*0x04);
    // TODO
    // read_from_flash(EPS_HK_TYPE,(uint8_t*)eps_hk_data,EPS_HK_FIELD_COUNT*0x04 + 6); // Add 6 for header (unsure of why it's not 8)
}

// TODO
// void update_heartbeat_fn(void) {
//   *self_status += 1;
//   heartbeat();
// }




// We know that the microcontroller uses 16 bit addresses, so store a function
// pointer in the first 2 bytes of the queue entry (data[0] = MSB, data[1] = LSB)
// TODO - develop a func test and a harness test for enqueueing and dequeueing cmd_t structs

/*
queue - Queue to enqueue command onto
cmd - Command (with cmd->fn already set before calling this function) to enqueue
*/
void enqueue_cmd(queue_t* queue, cmd_t* cmd) {
    // Cast the cmd_fn_t function pointer to a uint16
    uint16_t fn_ptr = (uint16_t) cmd->fn;

    print("enqueue: fn_ptr = %x\n", fn_ptr);

    // Enqueue the command as an 8-byte array
    uint8_t data[8] = {0};
    data[0] = (fn_ptr >> 8) & 0xFF;
    data[1] = fn_ptr & 0xFF;
    enqueue(queue, data);
}

/*
queue - Queue to dequeue command from
cmd - The struct must already exist (be allocated) before calling this function,
      then this function sets the value of cmd->fn
*/
void dequeue_cmd(queue_t* queue, cmd_t* cmd) {
    // Dequeue the command as an 8-byte array
    uint8_t data[8] = {0};
    dequeue(queue, data);
    uint16_t fn_ptr = (((uint16_t) data[0]) << 8) | ((uint16_t) data[1]);

    print("dequeue: fn_ptr = %x\n", fn_ptr);

    // Cast the uint16 to a cmd_fn_t function pointer
    cmd->fn = (cmd_fn_t) fn_ptr;
}
