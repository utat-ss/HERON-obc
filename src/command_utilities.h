#ifndef COMMAND_UTILITIES_H
#define COMMAND_UTILITIES_H

#include <stdbool.h>

#include <queue/queue.h>

#include "mem.h"
#include "transceiver.h"


// Callback function signature to run a command
typedef void(*cmd_fn_t)(void);

typedef struct {
    cmd_fn_t fn;
    uint8_t opcode;
    // true if password needs to be correct to execute
    bool pwd_protected;
} cmd_t;

// Need to declare `cmd_t` before this include to prevent errors from ordering
// of header includes
#include "commands.h"


// Subsystem
#define CMD_OBC       1
#define CMD_EPS       2
#define CMD_PAY       3

// Block types
#define CMD_OBC_HK    1
#define CMD_EPS_HK    2
#define CMD_PAY_HK    3
#define CMD_PAY_OPT   4

// Command types
#define CMD_PING                        0x00
#define CMD_GET_RTC                     0x01
#define CMD_SET_RTC                     0x02
#define CMD_READ_EEPROM                 0x03
#define CMD_ERASE_EEPROM                0x04
#define CMD_READ_RAM_BYTE               0x05
#define CMD_SEND_EPS_CAN_MSG            0x06
#define CMD_SEND_PAY_CAN_MSG            0x07
#define CMD_ACT_PAY_MOTORS              0x08
#define CMD_RESET_SUBSYS                0x09
#define CMD_SET_INDEF_LPM_ENABLE        0x0A
#define CMD_READ_REC_STATUS_INFO        0x10
#define CMD_READ_DATA_BLOCK             0x11
#define CMD_READ_REC_LOC_DATA_BLOCK     0x12
#define CMD_READ_PRIM_CMD_BLOCKS        0x13
#define CMD_READ_SEC_CMD_BLOCKS         0x14
#define CMD_READ_RAW_MEM_BYTES          0x15
#define CMD_SET_MEM_SEC_START_ADDR      0x16
#define CMD_SET_MEM_SEC_END_ADDR        0x17
#define CMD_ERASE_MEM_PHY_SECTOR        0x18
#define CMD_ERASE_MEM_PHY_BLOCK         0x19
#define CMD_ERASE_ALL_MEM               0x1A
#define CMD_COL_DATA_BLOCK              0x20
#define CMD_GET_CUR_BLOCK_NUM           0x21
#define CMD_SET_CUR_BLOCK_NUM           0x22
#define CMD_SET_AUTO_DATA_COL_ENABLE    0x23
#define CMD_SET_AUTO_DATA_COL_PERIOD    0x24
#define CMD_RESYNC_AUTO_DATA_COL        0x25

// Max memory read
#define CMD_READ_MEM_MAX_COUNT (TRANS_TX_DEC_MSG_MAX_SIZE - 13)

// Default period for automatic data collection for each block type
// (time between timer callbacks, in seconds)
#define EPS_HK_AUTO_DATA_COL_PERIOD     60
#define PAY_HK_AUTO_DATA_COL_PERIOD     120
#define PAY_OPT_AUTO_DATA_COL_PERIOD    300

// TODO change
#define EPS_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR    ((uint32_t*) 0x100)
#define EPS_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR     ((uint32_t*) 0x104)
#define PAY_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR    ((uint32_t*) 0x110)
#define PAY_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR     ((uint32_t*) 0x114)
#define PAY_OPT_AUTO_DATA_COL_ENABLED_EEPROM_ADDR   ((uint32_t*) 0x120)
#define PAY_OPT_AUTO_DATA_COL_PERIOD_EEPROM_ADDR    ((uint32_t*) 0x124)


// Automatic data collection for one block type
typedef struct {
    // True if we are currently collecting this type of data
    bool enabled;
    // Seconds between collection
    uint32_t period;
    // Number of seconds counted (start at 0, go to `period`)
    uint32_t count;
} auto_data_col_t;


extern queue_t cmd_queue;
extern queue_t cmd_args_queue;

extern volatile cmd_t* volatile current_cmd;
extern volatile uint32_t current_cmd_arg1;
extern volatile uint32_t current_cmd_arg2;
extern volatile bool prev_cmd_succeeded;

extern volatile uint8_t can_countdown;

extern mem_header_t eps_hk_header;
extern uint32_t eps_hk_fields[];
extern mem_header_t pay_hk_header;
extern uint32_t pay_hk_fields[];
extern mem_header_t pay_opt_header;
extern uint32_t pay_opt_fields[];

extern volatile auto_data_col_t eps_hk_auto_data_col;
extern volatile auto_data_col_t pay_hk_auto_data_col;
extern volatile auto_data_col_t pay_opt_auto_data_col;

extern rtc_date_t restart_date;
extern rtc_time_t restart_time;


void handle_trans_rx_dec_msg(void);

void start_trans_tx_dec_msg(void);
void append_to_trans_tx_dec_msg(uint8_t byte);
void finish_trans_tx_dec_msg(void);

cmd_t* trans_msg_type_to_cmd(uint8_t msg_type);
uint8_t trans_cmd_to_msg_type(cmd_t* cmd);

void enqueue_cmd(cmd_t* cmd, uint32_t arg1, uint32_t arg2);
void dequeue_cmd(void);

void execute_next_cmd(void);
void finish_current_cmd(bool succeeded);

void populate_header(mem_header_t* header, uint32_t block_num, uint8_t error);

void append_header_to_tx_msg(mem_header_t* header);
void append_fields_to_tx_msg(uint32_t* fields, uint8_t num_fields);

void init_auto_data_col(void);
void auto_data_col_timer_cb(void);
void can_timer_cb(void);

#endif
