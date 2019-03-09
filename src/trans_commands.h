#ifndef TRANS_COMMANDS_H
#define TRANS_COMMANDS_H

// Subsystem
#define TRANS_CMD_OBC       0
#define TRANS_CMD_EPS       1
#define TRANS_CMD_PAY       2

// Block types
#define TRANS_CMD_EPS_HK    0
#define TRANS_CMD_PAY_HK    1
#define TRANS_CMD_PAY_OPT   2

// Command types
#define TRANS_CMD_PING                  0x00
#define TRANS_CMD_GET_RESTART_UPTIME    0x01
#define TRANS_CMD_GET_RTC               0x02
#define TRANS_CMD_SET_RTC               0x03
#define TRANS_CMD_READ_MEM              0x04
#define TRANS_CMD_ERASE_MEM             0x05
#define TRANS_CMD_COL_BLOCK             0x06
#define TRANS_CMD_READ_LOC_BLOCK        0x07
#define TRANS_CMD_READ_MEM_BLOCK        0x08
#define TRANS_CMD_AUT_DATA_COL_ENABLE   0x09
#define TRANS_CMD_AUT_DATA_COL_PERIOD   0x0A
#define TRANS_CMD_AUT_DATA_COL_RESYNC   0x0B
#define TRANS_CMD_EPS_HEAT_SP           0x0C
#define TRANS_CMD_PAY_HEAT_SP           0x0D
#define TRANS_CMD_PAY_CTRL_ACT_MOTORS   0x0E
#define TRANS_CMD_RESET                 0x0F

#endif
