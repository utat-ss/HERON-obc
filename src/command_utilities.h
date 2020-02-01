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
#define CMD_PAY_OPT_OD      5
#define CMD_PAY_OPT_FL      6
#define CMD_PRIM_CMD_LOG    7
#define CMD_SEC_CMD_LOG     8

// Command opcodes
#define CMD_PING_OBC                    0x00
#define CMD_GET_RTC                     0x01
#define CMD_SET_RTC                     0x02
#define CMD_READ_OBC_EEPROM             0x03
#define CMD_ERASE_OBC_EEPROM            0x04
#define CMD_READ_OBC_RAM_BYTE           0x05
#define CMD_SET_INDEF_BEACON_ENABLE     0x06
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
#define CMD_RESET_SUBSYS                0x42

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
#define CMD_RESP_STATUS_OK                      0x00
#define CMD_RESP_STATUS_INVALID_ARGS            0x01
#define CMD_RESP_STATUS_TIMED_OUT               0x02
#define CMD_RESP_STATUS_DATA_COL_IN_PROGRESS    0x03
#define CMD_RESP_STATUS_UNKNOWN                 0xFF

// For unsuccessful ACKs where opcode/args are unknown
// This is OK because we don't ACK the auto enqueued commands
#define CMD_CMD_ID_UNKNOWN              0x0000
// When a command is automatically enqueued by OBC
#define CMD_CMD_ID_AUTO_ENQUEUED        0x0000

#define CMD_TIMEOUT_DEF_PERIOD_S            60
// Maximum number of seconds between fields of a collect data block command
// before it times out
#define CMD_COL_DATA_BLOCK_FIELD_TIMEOUT_S  10


// Max number of command log blocks
#define CMD_READ_CMD_BLOCKS_MAX_COUNT   5
// Max memory read
#define CMD_READ_MEM_MAX_COUNT          106
// Minimum auto data collection period in seconds
#define CMD_AUTO_DATA_COL_MIN_PERIOD    60


// Number of sections using data collection
#define NUM_DATA_COL_SECTIONS 4

// Default period for automatic data collection for each block type
// (time between timer callbacks, in seconds)
#define OBC_HK_AUTO_DATA_COL_PERIOD     600
#define EPS_HK_AUTO_DATA_COL_PERIOD     600
#define PAY_HK_AUTO_DATA_COL_PERIOD     1200
#define PAY_OPT_AUTO_DATA_COL_PERIOD    7200

// Beacon enables in EEPROM
#define BEACON_ENABLE_1_EEPROM_ADDR                 0x110
#define BEACON_ENABLE_2_EEPROM_ADDR                 0x114

// Automatic data collection - settings in EEPROM
#define OBC_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR    0x130
#define OBC_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR     0x134
#define EPS_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR    0x150
#define EPS_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR     0x154
#define PAY_HK_AUTO_DATA_COL_ENABLED_EEPROM_ADDR    0x170
#define PAY_HK_AUTO_DATA_COL_PERIOD_EEPROM_ADDR     0x174
#define PAY_OPT_AUTO_DATA_COL_ENABLED_EEPROM_ADDR   0x190
#define PAY_OPT_AUTO_DATA_COL_PERIOD_EEPROM_ADDR    0x194


// Automatic data collection for one block type
typedef struct {
    // String name for section (longest is PAY_OPT, 7 characters + terminating character)
    char name[8];
    // True if we are currently collecting this type of data
    bool auto_enabled;
    // EEPROM address to store enabled bit
    uint16_t auto_enabled_eeprom_addr;
    // Seconds between collection
    uint32_t auto_period;
    // EEPROM addresss to store period
    uint16_t auto_period_eeprom_addr;
    // Value of `uptime_s` when we last started collecting a block of this type of data (only applied for automatically, this value is not affected by manually triggering data collection)
    uint32_t prev_auto_col_uptime_s;
    // Value of `uptime_s` when we last received a field of this type of data
    uint32_t prev_field_col_uptime_s;
    // Header for this section
    mem_header_t header;
    // Array of field data
    uint32_t* fields;
    // Corresponding section in memory to write data to
    mem_section_t* mem_section;
    // Keep track of the block number this command corresponds to in the
    // command log (because multiple of these command will be executing
    // simultaneously and overlapping each other)
    // Should be primary command log
    uint32_t cmd_log_block_num;
    // Value of arg1 for this memory section/block type
    uint32_t cmd_arg1;
    // Queue to enqueue CAN TX messages to
    queue_t* can_tx_queue;
    // Opcode byte in CAN messages
    uint8_t can_opcode;
} data_col_t;


extern queue_t cmd_queue_1;
extern queue_t cmd_queue_2;

extern volatile uint16_t current_cmd_id;
extern volatile cmd_t* volatile current_cmd;
extern volatile uint32_t current_cmd_arg1;
extern volatile uint32_t current_cmd_arg2;

extern volatile uint8_t cmd_timeout_count_s;
extern uint8_t cmd_timeout_period_s;

extern mem_header_t cmd_log_header;

extern data_col_t obc_hk_data_col;
extern data_col_t eps_hk_data_col;
extern data_col_t pay_hk_data_col;
extern data_col_t pay_opt_data_col;
extern data_col_t* all_data_cols[];

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
mem_section_t* mem_section_for_cmd(cmd_t* cmd);

void cmd_to_bytes(uint16_t cmd_id, cmd_t* cmd, uint32_t arg1, uint32_t arg2,
        uint8_t* bytes1, uint8_t* bytes2);
bool enqueue_cmd(uint16_t cmd_id, cmd_t* cmd, uint32_t arg1, uint32_t arg2);
bool enqueue_cmd_front(uint16_t cmd_id, cmd_t* cmd, uint32_t arg1, uint32_t arg2);
void bytes_to_cmd(uint16_t* cmd_id, cmd_t** cmd, uint32_t* arg1, uint32_t* arg2,
        uint8_t* bytes1, uint8_t* bytes2);
bool dequeue_cmd(uint16_t* cmd_id, cmd_t** cmd, uint32_t* arg1, uint32_t* arg2);
bool cmd_queue_contains_col_data_block(uint8_t block_type);

void execute_next_cmd(void);
void finish_current_cmd(uint8_t status);

void prepare_mem_section_curr_block(mem_section_t* section, uint32_t next_block);
void inc_and_prepare_mem_section_curr_block(mem_section_t* section);
void populate_header(mem_header_t* header, uint32_t block_num, uint8_t status);

void add_def_trans_tx_dec_msg(uint8_t status);
void append_header_to_tx_msg(mem_header_t* header);
void append_fields_to_tx_msg(uint32_t* fields, uint8_t num_fields);

void init_auto_data_col(void);
void run_auto_data_col(void);
void cmd_timeout_timer_cb(void);

#endif
