#include "commands.h"

queue_t cmd_queue;
queue_t cmd_arg_queue;



// Available UART commands

void resume_mob_fn(void) {
    mob_t* mob;
    dequeue(&cmd_arg_queue, (uint8_t *) &mob);
    resume_mob(mob);
    while (!is_paused(mob));
}

cmd_t resume_mob_cmd = {
    .fn = resume_mob_fn
};


void req_eps_hk_fn(void) {
    // Make a CAN req, add the appropriate response to the command
    // queue, create a callback which prints the output in a human
    // readable format.

    // print("%s\n", __FUNCTION__);

    uint8_t d[8] = { 0 };
    d[0] = 0;   // TODO
    d[1] = CAN_EPS_HK;
    d[2] = 0;
    enqueue(&eps_tx_msg_queue, d);
    enqueue(&cmd_queue, (uint8_t *) &resume_mob_cmd);
    enqueue(&cmd_arg_queue, (uint8_t *) &pay_cmd_tx_mob);
    //resume_mob(&eps_cmd_tx);
    // while (!is_paused(&eps_cmd_tx));
}

cmd_t req_eps_hk_cmd = {
    .fn = req_eps_hk_fn
};



void req_pay_hk_fn(void) {
    // Make a CAN req, add the appropriate response to the command
    // queue, create a callback which prints the output in a human
    // readable format.

    // print("%s\n", __FUNCTION__);

    // Send PAY_HK first
    uint8_t d[8] = { 0 };
    d[0] = 0;   // TODO
    d[1] = CAN_PAY_HK;
    d[2] = 0;
    enqueue(&pay_tx_msg_queue, d);
    enqueue(&cmd_queue, (uint8_t *) &resume_mob_cmd);
    enqueue(&cmd_arg_queue, (uint8_t *) &pay_cmd_tx_mob);
}

cmd_t req_pay_hk_cmd = {
    .fn = req_pay_hk_fn
};


// TODO
void write_flash_fn(void) {
  print("%s\n", __FUNCTION__);
  print ("\nWriting to PAY_HK\n");
  // write_to_flash(PAY_HK_TYPE,0,(uint8_t*) pay_hk_data);
  print ("\nWriting to SCI stack\n");
  // write_to_flash(SCI_TYPE,0,(uint8_t*) pay_sci_data);
  print ("\nWriting to EPS_HK stack\n");
  // write_to_flash(EPS_HK_TYPE,0,(uint8_t*) eps_hk_data);
  print ("Done\n");
}

cmd_t write_flash_cmd = {
    .fn = write_flash_fn
};


void read_flash_fn(void) {
    // print("%s\n", __FUNCTION__);
    print("%s\n", __FUNCTION__);
    print ("\nReading from PAY HK\n");
    // TODO
    // read_from_flash(PAY_HK_TYPE,(uint8_t*)pay_hk_data,CAN_PAY_HK_FIELD_COUNT*0X04);
    print ("\nReading from SCI\n");
    // TODO
    // read_from_flash(SCI_TYPE, (uint8_t*)pay_sci_data,CAN_PAY_SCI_FIELD_COUNT*0x04);
    print ("\nReading from EPS HK\n");
    // TODO
    // read_from_flash(EPS_HK_TYPE,(uint8_t*)eps_hk_data,EPS_HK_FIELD_COUNT*0x04 + 6); // Add 6 for header (unsure of why it's not 8)
}

cmd_t read_flash_cmd = {
    .fn = read_flash_fn
};


// TODO
void update_heartbeat_fn(void) {
  //*self_status += 1;
  //heartbeat();
}

cmd_t update_heartbeat_cmd = {
    .fn = update_heartbeat_fn
};


void actuate_motor_fn(void) {
    // print("%s\n", __FUNCTION__);

    uint8_t d[8] = { 0 };
    d[0] = 0;   // TODO
    d[1] = CAN_PAY_MOTOR;
    d[2] = CAN_PAY_MOTOR_ACTUATE;
    enqueue(&pay_tx_msg_queue, d);
    enqueue(&cmd_queue, (uint8_t*) &resume_mob_cmd);
    enqueue(&cmd_arg_queue, (uint8_t*) &pay_cmd_tx_mob);
}

cmd_t actuate_motor_cmd = {
    .fn = actuate_motor_fn
};


// TODO - do we even need variable input arguments?
// To accept variable input, e.g. a command like "LAST MSG 4" which
// returns the last 4 CAN msgs, set the .cmd member to be the prefix
// of the command, without the variable; in this case, "LAST MSG "

cmd_t* cmd_list[] = {
    &resume_mob_cmd,
    &req_eps_hk_cmd,
    &req_pay_hk_cmd,
    &write_flash_cmd,
    &read_flash_cmd,
    &update_heartbeat_cmd,
    &actuate_motor_cmd
};
const uint8_t cmd_list_len = sizeof(cmd_list) / sizeof(cmd_list[0]);
