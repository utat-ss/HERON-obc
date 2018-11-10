#include "transceiver.h"

//Default Address - may change to a variable so that we can set value
#define TRANS_ADDR  0x22

// Has '\0' termination
volatile uint8_t received_cmd[20];
volatile bool received_cmd_available = false;

void init_trans(void) {
  register_callback(trans_cb);
  //set_trans_scw
  set_trans_freq();
}

uint8_t trans_cb(const uint8_t* buf, uint8_t len) {
  for (uint8_t i = 0; i < len; ++i) {
    if (buf[i] == '\r') {
      for (uint8_t j = 0; j < i; j++) {
        received_cmd[j] = buf[j];
      }
      received_cmd[i] = '\0';
      received_cmd_available = true;

      return i + 1;
    }
  }

  return 0;
}

// Converts the ASCII representation of a hex number to the integer
uint8_t char_hex_to_dec(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    } else {
        // TODO
        return 0;
    }
}

/*
Extracts ASCII characters representing a hex number and returns the corresponding integer.
offset - starting index in the string
count - number of characters (between 1 and 8)
*/
uint32_t scan_uint(char* string, uint8_t offset, uint8_t count) {
    uint32_t value = 0;
    for (uint8_t i = offset; i < offset + count; i++) {
        value = value << 4;
        value = value + char_hex_to_dec(string[i]);
    }
    return value;
}

//Write to status control register
void set_trans_scw(uint16_t reg) {
    print("ES+W%2X00%4X\r", TRANS_ADDR, reg);

    while (!received_cmd_available) {}
    // TODO process the message in received_cmd
    // TODO set timeout for waiting for command
    received_cmd_available = false;
}

//Read and return status control register
uint32_t read_trans_scw() {
    print("ES+R%2X00\r", TRANS_ADDR);
    //Answer is received through trans_cb

    while (!received_cmd_available) {}
    //Wait for response
    //response format: OK+[RR]0000[WWWW]<CR>
    int offset = 9;
    int count = 4;
    uint32_t scw = scan_uint(received_cmd, offset, count);

    received_cmd_available = false;

    return scw;
}

//Set transceiver frequency default chosen - 435MHz
uint8_t set_trans_freq() {
    print("ES+W%2X01%8X\r", TRANS_ADDR, FREQ);

    //read and return 0 or 1 depending if it was able to configure
}

uint64_t read_trans_freq() {
    print("ES+R%2X01\r", TRANS_ADDR);

    while (!received_cmd_available) {}
    //Answer format: OK+[RR][FFFFFF][NN]
    uint8_t cmd [20];
    cmd = received_cmd;
    int offset = 5;
    int count = 4;
    uint32_t freq1 = scan_uint(received_cmd, offset, count);
    offset += 4;
    uint32_t freq2 = scan_uint(received_cmd, offset, count);

    received_cmd_available = false; //reset flag

    uint64_t freq = (freq2 << 4) | freq1;
    return freq;
}

//Consider making TRANS_ADDR a variable if wanting to implement this function
uint8_t set_trans_addr(uint8_t addr) {
  if (addr != 0x23 && addr != 0x22) {
      return 0; //invalid address
  }
    print("ES+R%2XFC%2X\r", TRANS_ADDR, addr);

    //return 1 or 0 if failed or succeeded
}

// from p.18
void set_trans_pipe_timeout(uint8_t timeout) {
    print("ES+W%2X06000000%2X\r", TRANS_ADDR, timeout);
}



//7-8, beacon mode code, 16-19 : Process responses
