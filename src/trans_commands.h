#ifndef TRANS_COMMANDS_H
#define TRANS_COMMANDS_H

#include <util/atomic.h>

#include "commands.h"
#include "transceiver.h"


// Subsystem
#define TRANS_CMD_OBC       0
#define TRANS_CMD_EPS       1
#define TRANS_CMD_PAY       2

// Block types
#define TRANS_CMD_EPS_HK    0
#define TRANS_CMD_PAY_HK    1
#define TRANS_CMD_PAY_OPT   2

// Command types
#define TRANS_CMD_PING                      0x00
#define TRANS_CMD_GET_RESTART_UPTIME        0x01
#define TRANS_CMD_GET_RTC                   0x02
#define TRANS_CMD_SET_RTC                   0x03
#define TRANS_CMD_READ_MEM_BYTES            0x04
#define TRANS_CMD_ERASE_MEM_SECTOR          0x05
#define TRANS_CMD_COL_BLOCK                 0x06
#define TRANS_CMD_READ_LOC_BLOCK            0x07
#define TRANS_CMD_READ_MEM_BLOCK            0x08
#define TRANS_CMD_AUTO_DATA_COL_ENABLE      0x09
#define TRANS_CMD_AUTO_DATA_COL_PERIOD      0x0A
#define TRANS_CMD_AUTO_DATA_COL_RESYNC      0x0B
#define TRANS_CMD_EPS_HEAT_SP               0x0C
#define TRANS_CMD_PAY_HEAT_SP               0x0D
#define TRANS_CMD_PAY_ACT_MOTORS            0x0E
#define TRANS_CMD_RESET_SUBSYS              0x0F
#define TRANS_CMD_EPS_CAN                   0x10
#define TRANS_CMD_PAY_CAN                   0x11
#define TRANS_CMD_READ_EEPROM               0x12
#define TRANS_CMD_GET_CUR_BLOCK_NUM         0x13
#define TRANS_CMD_SET_CUR_BLOCK_NUM         0x14
#define TRANS_CMD_SET_MEM_SEC_START_ADDR    0x15
#define TRANS_CMD_SET_MEM_SEC_END_ADDR      0x16
#define TRANS_CMD_ERASE_EEPROM              0x17
#define TRANS_CMD_SET_EPS_HEAT_CUR_THRESH   0x18
#define TRANS_CMD_ERASE_ALL_MEM             0x19
#define TRANS_CMD_ERASE_MEM_PHY_BLOCK       0x1A

void handle_trans_rx_dec_msg(void);

void start_trans_tx_dec_msg(void);
void append_to_trans_tx_dec_msg(uint8_t byte);
void finish_trans_tx_dec_msg(void);

#endif
