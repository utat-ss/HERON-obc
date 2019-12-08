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


// Subsystem numbers
#define CMD_OBC       1
#define CMD_EPS       2
#define CMD_PAY       3

// Block types
#define CMD_OBC_HK          1
#define CMD_EPS_HK          2
#define CMD_PAY_HK          3
#define CMD_PAY_OPT         4
#define CMD_PRIM_CMD_LOG    5
#define CMD_SEC_CMD_LOG     6

// Command opcodes
#define CMD_PING_OBC                    0x00
#define CMD_GET_RTC                     0x01
#define CMD_SET_RTC                     0x02
#define CMD_READ_OBC_EEPROM             0x03
#define CMD_ERASE_OBC_EEPROM            0x04
#define CMD_READ_OBC_RAM_BYTE           0x05
#define CMD_SET_BEACON_INHIBIT_ENABLE   0x06
#define CMD_READ_DATA_BLOCK             0x10
#define CMD_READ_PRIM_CMD_BLOCKS        0x11
#define CMD_READ_SEC_CMD_BLOCKS         0x12
#define CMD_READ_REC_STATUS_INFO        0x13
#define CMD_READ_REC_LOC_DATA_BLOCK     0x14
#define CMD_READ_RAW_MEM_BYTES          0x15
#define CMD_COL_DATA_BLOCK              0x20
#define CMD_GET_AUTO_DATA_COL_SETTINGS  0x21
#define CMD_SET_AUTO_DATA_COL_ENABLE    0x22
#define CMD_SET_AUTO_DATA_COL_PERIOD    0x23
#define CMD_RESYNC_AUTO_DATA_COL_TIMERS 0x24
#define CMD_GET_CUR_BLOCK_NUMS          0x30
#define CMD_SET_CUR_BLOCK_NUM           0x31
#define CMD_GET_MEM_SEC_ADDRS           0x32
#define CMD_SET_MEM_SEC_START_ADDR      0x33
#define CMD_SET_MEM_SEC_END_ADDR        0x34
#define CMD_ERASE_MEM_PHY_SECTOR        0x35
#define CMD_ERASE_MEM_PHY_BLOCK         0x36
#define CMD_ERASE_ALL_MEM               0x37
#define CMD_SEND_EPS_CAN_MSG            0x40
#define CMD_SEND_PAY_CAN_MSG            0x41
#define CMD_ACT_PAY_MOTORS              0x42
#define CMD_RESET_SUBSYS                0x43
#define CMD_SET_INDEF_LPM_ENABLE        0x44

// Mask to set MSB on opcode byte for response packets
#define CMD_RESP_CMD_ID_MASK            (0x1 << 15)

// ACK status bytes
#define CMD_ACK_STATUS_OK                   0x00
#define CMD_ACK_STATUS_RESET_CMD_ID         0x01
#define CMD_ACK_STATUS_INVALID_ENC_FMT      0x02
#define CMD_ACK_STATUS_INVALID_LEN          0x03
#define CMD_ACK_STATUS_INVALID_CSUM         0x04
#define CMD_ACK_STATUS_INVALID_DEC_FMT      0x05
#define CMD_ACK_STATUS_INVALID_CMD_ID       0x06
#define CMD_ACK_STATUS_DECREMENTED_CMD_ID   0x07
#define CMD_ACK_STATUS_REPEATED_CMD_ID      0x08
#define CMD_ACK_STATUS_INVALID_OPCODE       0x09
#define CMD_ACK_STATUS_INVALID_PWD          0x0A
#define CMD_ACK_STATUS_FULL_CMD_QUEUE       0x0B

// Response/command log status bytes
#define CMD_RESP_STATUS_OK              0x00
#define CMD_RESP_STATUS_INVALID_ARGS    0x01
#define CMD_RESP_STATUS_TIMED_OUT       0x02
#define CMD_RESP_STATUS_UNKNOWN         0xFF

// For unsuccessful ACKs where opcode/args are unknown
// This is OK because we don't ACK the auto enqueued commands
#define CMD_CMD_ID_UNKNOWN              0x0000
// When a command is automatically enqueued by OBC
#define CMD_CMD_ID_AUTO_ENQUEUED        0x0000

// TODO - change value?
#define CMD_TIMEOUT_DEF_PERIOD_S    30

// Default 6 hours
#define BEACON_INHIBIT_DEF_PERIOD_S (6 * 60 * 60)

// TODO
// Max memory read
#define CMD_READ_MEM_MAX_COUNT (TRANS_TX_DEC_MSG_MAX_SIZE - 13)


// Number of sections using auto data collection
#define NUM_AUTO_DATA_COL_SECTIONS 4

// Default period for automatic data collection for each block type
// (time between timer callbacks, in seconds)
#define OBC_HK_AUTO_DATA_COL_PERIOD     60
#define EPS_HK_AUTO_DATA_COL_PERIOD     60
#define PAY_HK_AUTO_DATA_COL_PERIOD     120
#define PAY_OPT_AUTO_DATA_COL_PERIOD    300

// Automatic data collection - default settings
#define OBC_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR    0x144
#define OBC_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR     0x148
#define EPS_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR    0x154
#define EPS_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR     0x158
#define PAY_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR    0x164
#define PAY_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR     0x168
#define PAY_OPT_AUTO_DATA_COL_ENABLED_EEPROM_ADDR   0x174
#define PAY_OPT_AUTO_DATA_COL_PERIOD_EEPROM_ADDR    0x178


// Automatic data collection for one block type
typedef struct {
    // True if we are currently collecting this type of data
    bool enabled;
    // Seconds between collection
    uint32_t period;
    // Number of seconds counted (start at 0, go to `period`)
    uint32_t count;
} auto_data_col_t;


extern queue_t cmd_opcode_queue;
extern queue_t cmd_args_queue;

extern volatile uint16_t current_cmd_id;
extern volatile cmd_t* volatile current_cmd;
extern volatile uint32_t current_cmd_arg1;
extern volatile uint32_t current_cmd_arg2;

extern volatile uint8_t cmd_timeout_count_s;
extern uint8_t cmd_timeout_period_s;

extern volatile bool beacon_inhibit_enabled;
extern volatile uint32_t beacon_inhibit_count_s;
extern uint32_t beacon_inhibit_period_s;

extern mem_header_t obc_hk_header;
extern uint32_t obc_hk_fields[];
extern mem_header_t eps_hk_header;
extern uint32_t eps_hk_fields[];
extern mem_header_t pay_hk_header;
extern uint32_t pay_hk_fields[];
extern mem_header_t pay_opt_header;
extern uint32_t pay_opt_fields[];
extern mem_header_t cmd_log_header;

extern volatile auto_data_col_t obc_hk_auto_data_col;
extern volatile auto_data_col_t eps_hk_auto_data_col;
extern volatile auto_data_col_t pay_hk_auto_data_col;
extern volatile auto_data_col_t pay_opt_auto_data_col;
extern volatile auto_data_col_t* all_auto_data_cols[];

extern rtc_date_t restart_date;
extern rtc_time_t restart_time;

extern bool print_cmds;
extern bool print_trans_tx_acks;


void handle_trans_rx_dec_msg(void);
void process_trans_tx_ack(void);

void start_trans_tx_resp(uint8_t status);
void append_to_trans_tx_resp(uint8_t byte);
void finish_trans_tx_resp(void);

cmd_t* cmd_opcode_to_cmd(uint8_t opcode);

void enqueue_cmd(uint16_t cmd_id, cmd_t* cmd, uint32_t arg1, uint32_t arg2);
void dequeue_cmd(uint16_t* cmd_id, cmd_t** cmd, uint32_t* arg1, uint32_t* arg2);

void execute_next_cmd(void);
void finish_current_cmd(uint8_t status);

void prepare_mem_section_curr_block(mem_section_t* section, uint32_t next_block);
void inc_and_prepare_mem_section_curr_block(mem_section_t* section);
void populate_header(mem_header_t* header, uint32_t block_num, uint8_t status);

void add_def_trans_tx_dec_msg(uint8_t status);
void append_header_to_tx_msg(mem_header_t* header);
void append_fields_to_tx_msg(uint32_t* fields, uint8_t num_fields);

void init_auto_data_col(void);
void auto_data_col_timer_cb(void);
void cmd_timeout_timer_cb(void);
void beacon_inhibit_timer_cb(void);

#endif
