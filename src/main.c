#include <uart/uart.h>
#include <timer/timer.h>
#include <can/can.h>
#include <can/ids.h>
#include <can/data_protocol.h>
#include <queue/queue.h>
#include <adc/adc.h>
#include <conversions/conversions.h>
//#include <heartbeat/heartbeat.h>

#include "mem.h"

#include <stdint.h>
#include <stdbool.h>
#include <util/delay.h>
/*
#define EPS_HK_FIELD_COUNT 3   // Number of fields
#define EPS_HK_TYPE 0x02

#define PAY_CMD_TX_MOB 3
#define EPS_CMD_TX_MOB 4
#define DATA_RX_MOB 5

queue_t pay_tx_queue;
queue_t eps_tx_queue;

//uint32_t pay_hk_data[CAN_PAY_HK_FIELD_COUNT] = { 0 };
//uint32_t pay_sci_data[CAN_PAY_SCI_FIELD_COUNT] = { 0 };
uint32_t eps_hk_data[EPS_HK_FIELD_COUNT] = { 0 };
uint8_t ssm_id;
uint8_t receiving_id;

mob_t status_rx_mob;
mob_t status_tx_mob;

// Declare fresh_start as global var. to keep track of fresh start and restart
uint8_t fresh_start;
void data_rx_callback(const uint8_t* data, uint8_t len);
void receive_pay_hk(const uint8_t* data, uint8_t len);
void receive_pay_sci(const uint8_t* data, uint8_t len);
void receive_eps_hk(const uint8_t* data, uint8_t len);
void receive_pay_motor(const uint8_t* data, uint8_t len);

void init_callbacks();
//void pay_cmd_tx_data_callback(uint8_t* data, uint8_t *len);
void eps_cmd_tx_data_callback(uint8_t* data, uint8_t *len);

void print_can_data(void) {
    // Payload
    print("%d C\n", (int16_t) temp_convert_raw_data_to_temperature(pay_hk_data[CAN_PAY_HK_TEMP]));
    print("%d %%RH\n", (int16_t) hum_convert_raw_humidity_to_humidity(pay_hk_data[CAN_PAY_HK_HUMID]));
    print("%d kPa\n", (int16_t) pres_convert_raw_pressure_to_pressure(pay_hk_data[CAN_PAY_HK_PRES]));

    print("\n");

    // Science
    for (uint8_t i = 0; i < CAN_PAY_SCI_FIELD_COUNT; i++) {
        print("#%u: %d%%\n", i + 1, (int16_t) ((double) pay_sci_data[i] / (double) 0xFFFFFF * 100.0));
    }

    print("\n");

    // Power
    print("Bat\n");
    print("+: %d mV\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BATT_VPOS_CH]) * 1000.0));
    print("-: %d mV\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BATT_VNEG_CH]) * 1000.0));
    print("%d mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BATT_IOUT_CH]) * 1000.0));
    // Converters
    print("BB: %f mV\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BB_VOUT_CH]) * 1000.0));
    print("BT: %f mV\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BT_VOUT_CH]) * 1000.0));
    print("BB1: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BB1_IOUT_CH]) * 1000.0));
    print("BB2: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BB2_IOUT_CH]) * 1000.0));
    print("BT1: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_BT1_IOUT_CH]) * 1000.0));
    print("PV\n");
    print("+X: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_PV_POS_X_IOUT_CH]) * 1000.0));
    print("-X: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_PV_NEG_X_IOUT_CH]) * 1000.0));
    print("+Y: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_PV_POS_Y_IOUT_CH]) * 1000.0));
    print("-Y: %f mA\n", (int16_t) (adc_eps_convert_raw_data_to_voltage(eps_hk_data[ADC_EPS_PV_NEG_Y_IOUT_CH]) * 1000.0));

    print("\n");
}

void print_raw_can_data(void) {
    // print("\n");
    //
    // print("PAY_HK\n");
    // for (uint8_t i = 0; i < CAN_PAY_HK_FIELD_COUNT; ++i) {
    //     print("#%u: %06x\n", i, pay_hk_data[i]);
    // }
    //
    // print("PAY_SCI\n");
    // for (uint8_t i = 0; i < CAN_PAY_SCI_FIELD_COUNT; ++i) {
    //     print("#%u: %06x\n", i, pay_sci_data[i]);
    // }
    //
    // print("EPS_HK\n");
    // for (uint8_t i = 0; i < EPS_HK_FIELD_COUNT; ++i) {
    //     print("#%u: %06x\n", i, eps_hk_data[i]);
    // }
    //
    // print("\n");
}




void data_rx_callback(const uint8_t* data, uint8_t len) {
    // print("RX Callback\n");
    print_hex_bytes((uint8_t *) data, len);

    //uint8_t boards = data[0];
    uint8_t message_type = data[1];

    switch (message_type) {
        case CAN_PAY_HK:
            receive_pay_hk(data, len);
            break;
        case CAN_PAY_SCI:
            receive_pay_sci(data, len);
            break;
        case CAN_EPS_HK:
            receive_eps_hk(data, len);
            break;
        case CAN_PAY_MOTOR:
            receive_pay_motor(data, len);
            break;
        default:
            print("Invalid RX\n");
            break;
    }
}

void receive_pay_hk(const uint8_t* data, uint8_t len){
    // print("%s\n", __FUNCTION__);
    uint8_t field_num = data[2];

    if (field_num < CAN_PAY_HK_FIELD_COUNT) {
        // Save data
        pay_hk_data[field_num] =
                ( ((uint32_t) data[3]) << 16 ) |
                ( ((uint32_t) data[4]) << 8  ) |
                ( ((uint32_t) data[5]) << 0  );

        if (field_num + 1 < CAN_PAY_HK_FIELD_COUNT) {
            uint8_t d[8] = { 0 };
            d[0] = 0;   // TODO
            d[1] = CAN_PAY_HK;
            d[2] = field_num + 1;
            enqueue(&pay_tx_queue, d);
            // print("Enqueued PAY_HK #%d\n", field_num + 1);
        } else {
            print("PAY_HK done\n");

            uint8_t d[8] = { 0 };
            d[0] = 0;   // TODO
            d[1] = CAN_PAY_SCI;
            d[2] = 0;
            enqueue(&pay_tx_queue, d);
        }
    }

    else {
        print("Unexpected\n");

}

void receive_pay_sci(const uint8_t* data, uint8_t len){
    uint8_t field_num = data[2];

    if (field_num < CAN_PAY_SCI_FIELD_COUNT) {
        // Save data
        pay_sci_data[field_num] =
                ( ((uint32_t) data[3]) << 16 ) |
                ( ((uint32_t) data[4]) << 8  ) |
                ( ((uint32_t) data[5]) << 0  );

        if (field_num + 1 < CAN_PAY_SCI_FIELD_COUNT){
            uint8_t d[8] = { 0 };
            d[0] = 0;   // TODO
            d[1] = CAN_PAY_SCI;
            d[2] = field_num + 1;
            enqueue(&pay_tx_queue, d);
        } else {
          //  print("PAY_SCI done\n");

            uint8_t d[8] = { 0 };
            d[0] = 0;   // TODO
            d[1] = CAN_EPS_HK;
            d[2] = 0;
            enqueue(&eps_tx_queue, d);
        }
    }
}

void receive_eps_hk(const uint8_t* data, uint8_t len){
    uint8_t field_num = data[2];

    if (field_num < EPS_HK_FIELD_COUNT) {
        // TODO: Save to flash, instead of to this array
        eps_hk_data[field_num] =
                ( ((uint32_t) data[3]) << 16 ) |
                ( ((uint32_t) data[4]) << 8  ) |
                ( ((uint32_t) data[5]) << 0  );

        if (field_num + 1 < EPS_HK_FIELD_COUNT) {
            // enqueue the next HK req
            uint8_t d[8] = { 0 };
            d[0] = 0;   // TODO
            d[1] = CAN_EPS_HK;
            d[2] = field_num + 1;
            enqueue(&eps_tx_queue, d);
            print("Enqueued\n");
        } else {
            print("EPS_HK done\n");

            print("\nDone CAN data\n\n");
            // print_raw_can_data();
            // print_can_data();
        }
    }
}

void receive_pay_motor(const uint8_t* data, uint8_t len){
    uint8_t field_num = data[2];
    if (field_num == CAN_PAY_MOTOR_ACTUATE) {
        print("PAY_MOTOR done\n");
    }
}




void pay_cmd_tx_data_callback(uint8_t* data, uint8_t *len) {
    if (!is_empty(&pay_tx_queue)) {
        dequeue(&pay_tx_queue, data);
        *len = 8;

        switch (data[1]) {
            case CAN_PAY_HK:
                print("Sending PAY_HK #%u\n", data[2]);
                break;
            case CAN_PAY_SCI:
                print("Sending PAY_SCI #%u\n", data[2]);
                break;
            case CAN_PAY_MOTOR:
                print("Sending PAY_MOTOR\n");
                break;
            default:
                break;
        }
    }

    else {
        *len = 0;
    }
}

void eps_cmd_tx_data_callback(uint8_t* data, uint8_t *len) {
    if (!is_empty(&eps_tx_queue)) {
        dequeue(&eps_tx_queue, data);
        print("%d\n", eps_tx_queue.size);
        *len = 8;

        print_hex_bytes(data, *len);
        print("Sending EPS_HK #%u\n", data[2]);
    }

    else {
        *len = 0;
    }
}


// CAN mob for sending commands to PAY
mob_t pay_cmd_tx = {
    .mob_num = PAY_CMD_TX_MOB,
    .mob_type = TX_MOB,
    .id_tag = OBC_PAY_CMD_TX_MOB_ID,
    .ctrl = default_tx_ctrl,
  //  .tx_data_cb = pay_cmd_tx_data_callback
};

// CAN mob for sending commands to EPS
mob_t eps_cmd_tx = {
    .mob_num = EPS_CMD_TX_MOB,
    .mob_type = TX_MOB,
    .id_tag = {0X0000},//OBC_EPS_CMD_TX_MOB_ID,
    .ctrl = default_tx_ctrl,
    .tx_data_cb = eps_cmd_tx_data_callback
};

// CAN mob for receiving data from any SSM
mob_t data_rx = {
    .mob_num = DATA_RX_MOB,
    .mob_type = RX_MOB,
    .dlc = 8, // this might change
    .id_tag = {0X000},//OBC_DATA_RX_MOB_ID,
    .id_mask = { 0x0000 },
    // .id_mask = CAN_RX_MASK_ID,
    .ctrl = default_rx_ctrl,
    .rx_cb = data_rx_callback
};




queue_t uart_cmd_queue;

typedef void(*uart_cmd_fn_t)(void);

typedef struct {
    char* cmd;
    uart_cmd_fn_t fn;
} cmd_t;

// Available UART commands

void handle_pay_eps_req();
cmd_t pay_eps_req = {
    .cmd = "REQ\r\n",
    .fn = handle_pay_eps_req
};

void handle_write_flash();
cmd_t write_flash = {
    .cmd = "WRITE\r\n",
    .fn = handle_write_flash
};

void handle_read_flash();
cmd_t read_flash = {
    .cmd = "READ\r\n",
    .fn = handle_read_flash
};

void handle_heartbeat_change ();
cmd_t heartbeat_change = {
    .cmd = "HEARTBEAT CHANGE\r\n",
    .fn = handle_heartbeat_change
};

void handle_actuate_motor();
cmd_t actuate_motor = {
    .cmd = "MOTOR\r\n",
    .fn = handle_actuate_motor
};

// TODO - do we even need variable input arguments?
// To accept variable input, e.g. a command like "LAST MSG 4" which
// returns the last 4 CAN msgs, set the .cmd member to be the prefix
// of the command, without the variable; in this case, "LAST MSG "

#define CMD_LIST_LEN 5
cmd_t* cmd_list[CMD_LIST_LEN] = {
    &pay_eps_req,
    &write_flash,
    &read_flash,
    &actuate_motor,
    &heartbeat_change
};




uint8_t handle_uart_cmd(const uint8_t* data, uint8_t len) {
    // print("\n%s\n", __FUNCTION__);
    // print_hex_bytes((uint8_t *) data, len);
    for (uint8_t i = 0; i < len; i++) {
        print("%c", data[i]);
    }
    print("\n");

    if (data[len - 1] == '\n') {
        for (uint8_t i = 0; i < CMD_LIST_LEN; i++) {
            cmd_t* cmd = cmd_list[i];
            uint8_t cmd_len = strlen(cmd->cmd);

            if (len < cmd_len) continue;

            // FIXME: might be len - 1 below
            uint8_t match = 1;
            for (uint8_t i = 0; i < cmd_len; i++) {
                if (data[i] != (cmd->cmd)[i]) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                // WOW: just enqueue the command directly into the queue!
                enqueue(&uart_cmd_queue, (uint8_t*)cmd);
                // print("Enqueued %s\n", cmd->cmd);
                // Downside, can't actually pass the data/len through;
                // this means we can't support variable sized commands
                break;
                // break if we match; the input can match at most 1 command
            }
        }

        return len;
    } else {
        return 0;
    }
}

void handle_pay_eps_req() {
    // Make a CAN req, add the appropriate response to the command
    // queue, create a callback which prints the output in a human
    // readable format.

    // print("%s\n", __FUNCTION__);

    // Send PAY_HK first
    uint8_t d[8] = { 0 };
    d[0] = 0;   // TODO
    d[1] = CAN_PAY_HK;
    d[2] = 0;
    enqueue(&pay_tx_queue, d);
    resume_mob(&pay_cmd_tx);
    while (!is_paused(&pay_cmd_tx));


    // Uncomment to send EPS_HK first

    uint8_t d[8] = { 0 };
    d[0] = 0;   // TODO
    d[1] = CAN_EPS_HK;
    d[2] = 0;
    enqueue(&eps_tx_queue, d);
    //resume_mob(&eps_cmd_tx);
    // while (!is_paused(&eps_cmd_tx));
}
void handle_heartbeat_change () {
  //*self_status += 1;
  //heartbeat();
}
void handle_write_flash() {

  print("%s\n", __FUNCTION__);
  print ("\nWriting to PAY_HK\n");
  write_to_flash(PAY_HK_TYPE,0,(uint8_t*) pay_hk_data);
  print ("\nWriting to SCI stack\n");
  write_to_flash(SCI_TYPE,0,(uint8_t*) pay_sci_data);
  print ("\nWriting to EPS_HK stack\n");
  write_to_flash(EPS_HK_TYPE,0,(uint8_t*) eps_hk_data);
  print ("Done\n");
}

void handle_read_flash() {
    // print("%s\n", __FUNCTION__);
    print("%s\n", __FUNCTION__);
    print ("\nReading from PAY HK\n");
    read_from_flash(PAY_HK_TYPE,(uint8_t*)pay_hk_data,CAN_PAY_HK_FIELD_COUNT*0X04);
    print ("\nReading from SCI\n");
    read_from_flash(SCI_TYPE, (uint8_t*)pay_sci_data,CAN_PAY_SCI_FIELD_COUNT*0x04);
    print ("\nReading from EPS HK\n");
    read_from_flash(EPS_HK_TYPE,(uint8_t*)eps_hk_data,EPS_HK_FIELD_COUNT*0x04 + 6); // Add 6 for header (unsure of why it's not 8)

}

void handle_actuate_motor() {
    // print("%s\n", __FUNCTION__);

    uint8_t d[8] = { 0 };
    d[0] = 0;   // TODO
    d[1] = CAN_PAY_MOTOR;
    d[2] = CAN_PAY_MOTOR_ACTUATE;
    enqueue(&pay_tx_queue, d);
    resume_mob(&pay_cmd_tx);
    while (!is_paused(&pay_cmd_tx));
}
*/
int main(void) {
    init_uart();
    print("\n\nUART\n");

    /*
    init_can();
    init_spi();

    init_rx_mob(&data_rx);
    //init_tx_mob(&pay_cmd_tx);
    init_tx_mob(&eps_cmd_tx);


    init_mem();
    init_rtc();
    init_curr_stack_ptrs();
    register_callback(handle_uart_cmd);

    init_queue(&uart_cmd_queue);
    init_queue(&eps_tx_queue);
    //init_queue(&pay_tx_queue);
    print("Initalized.\n");

    //init_heartbeat();

    print("Waiting...\n");

    handle_pay_eps_req();

    while (1) {
        if (!is_empty(&uart_cmd_queue)) {
            // dequeue the latest UART command and execute it
            cmd_t cmd;
            //print("Dequeueing\n");
            dequeue(&uart_cmd_queue, (uint8_t*)&cmd);
            //print("Dequeued command\n");
            (cmd.fn)();
            // Now, callbacks are no longer executed in ISRs, so we
            // can actually resume/pause MObs inside them
        }

        if (!is_empty(&pay_tx_queue)) {
            resume_mob(&pay_cmd_tx);
            while (!is_paused(&pay_cmd_tx));
        }

        if (!is_empty(&eps_tx_queue)) {
            resume_mob(&eps_cmd_tx);
            while (!is_paused(&eps_cmd_tx));
        }

        _delay_ms(100);
    }
    */

    return 0;
}
