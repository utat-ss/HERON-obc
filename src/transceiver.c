/*
Note that all/most setting configurations will return 1 if configuration successful
and 0 if not successful
*/

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

/*
Check if Command was successful by looking at first 3 string values in the
received command
Values: 'OK' means that command was successful. Return 1
Values: 'ERR' means that Command was not sucessful. Return 0
Other values are unexpected. Return 0
*/
uint8_t valid_cmd(char* string) {
    char valid[] = "OK"; //TODO: check if this declaration is valid
    char invalid[] = "ERR";

    if (string_cmp(valid, string, 2) == 1) {
        return 1;
    }
    else if (string_cmp(invalid, string, 3) ==1) {
        return 0;
    }
    else {
        return 0;
    }
}

//Compares strings, returns 1 if strings are the same, returns 0 otherwise
uint8_t string_cmp(char* string, char* string2, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        if (string[i] != string2[i]) {
            return 0;
        }
    }
    return 1;
}

//1. Write to status control register
uint8_t set_trans_scw(uint16_t reg) {
    print("ES+W%2X00%4X\r", TRANS_ADDR, reg);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //Check validity
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false;

    uint8_t validity = valid_cmd(cmnd);
    return validity;
}

//1. Read and return status control register
uint16_t read_trans_scw() {
    print("ES+R%2X00\r", TRANS_ADDR);
    //Answer is received through trans_cb

    while (!received_cmd_available) {}
    //Wait for response
    //response format: OK+[RR]0000[WWWW]<CR>
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false; //reset flag

    //check validity
    uint8_t validity = valid_cmd(cmnd); //command valid
    if (validity == 1) {
        uint8_t offset = 9;
        uint8_t count = 4;
        uint16_t scw = scan_uint(cmnd, offset, count); //TODO: check if truncation from 32-16 is ok
        return scw;
    }

    //Invalid response
    return 0;
}

//2. Set transceiver frequency default chosen - 435MHz
uint8_t set_trans_freq() {
    print("ES+W%2X01%8X\r", TRANS_ADDR, FREQ);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command
    //Check validity
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false;

    uint8_t validity = valid_cmd(cmnd);
    return validity;
}

//2.
uint32_t read_trans_freq() {
    print("ES+R%2X01\r", TRANS_ADDR);

    while (!received_cmd_available) {}
    //Answer format: OK+[RR][FFFFFF][NN]
    uint8_t cmnd[20];
    //store received command
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    int offset = 5;
    int count = 4;
    uint32_t freq = scan_uint(cmnd, offset, count);
    received_cmd_available = false; //reset flag
    return freq;
}

//Consider making TRANS_ADDR a variable if wanting to implement this function
//3. Change device address
uint8_t set_trans_addr(uint8_t addr) {
  if (addr != 0x23 && addr != 0x22) {
      return 0; //invalid address
  }
    print("ES+R%2XFC%2X\r", TRANS_ADDR, addr);

    //return 1 or 0 if failed or succeeded
    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command
    // Answer: OK+[NewAddr]<CR>

    //Check validity
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false;

    uint8_t validity = valid_cmd(cmnd);

    if (validity == 0) {
        return 0;
    }

    uint8_t new_addr = scan_uint(cmnd, 3, 2);
    //If new address is the same as desired
    if (new_addr == addr) {
        return 1;
    }
    return 0;

}

// from p.18
void set_trans_pipe_timeout(uint8_t timeout) {
    print("ES+W%2X06000000%2X\r", TRANS_ADDR, timeout);
}

/*
5. Set beacon transmission period - Default is 60s
Takes in desired period in seconds
Max val = 0xFFFF = 65535s = 1092min = 18.2h
*/
uint8_t set_beacon_period (uint16_t period) {
    print("ES+W%2X070000%4X\r", TRANS_ADDR, period);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //Check validity
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false;

    uint8_t validity = valid_cmd(cmnd);
    return validity;
}

uint16_t read_beacon_period () {
    print("ES+R%2X07%4X\r", TRANS_ADDR);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command
    // Answer: OK+[RR]0000[TTTT]<CR>

    //Check validity
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false;

    uint32_t period = scan_uint(cmnd, 9, 4);
    uint16_t period_16t = period; //TODO check truncation
    return period_16t;
}

/*
6. Sets beacon content. 'len' is the length of the message and 'content' is
the content in BIN format. Note: Avoid byte value 13 in content due to ESTTC
protocol, also len must be less than 128 bytes because of S/W version (hardware)
*/

//TODO: Are we expecting changing contents? Or need to make printing message
//variable to allow for different sizes in content
uint8_t set_beacon_content(uint32_t content, uint8_t len) {
    print("ES+W%2XFB%2X%4X\r", TRANS_ADDR, len, content);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //Check validity
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false;

    uint8_t validity = valid_cmd(cmnd);
    return validity;
}

//For now, it is assumed that beacon messages are uint32_t in length
uint32_t read_beacon_content() {
    print("ES+R%2XFB\r", TRANS_ADDR);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command
    // Answer: OK+LL HH...HH<CR>,

    //Check validity
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false;

    uint32_t message = scan_uint(cmnd, 3, 8);
    return message;
}

/*
7. Set destination call-sign
Callsign is 6 bytes long
*/
uint8_t set_destination_callsign(uint32_t callsign) {
    //callsign = (callsign2 << 16) + callsign1
    uint16_t callsign1 = callsign; //truncating callsign to be 24 bites
    uint8_t callsign2 = callsign >> (4*4);

    print("ES+W%2XF5%2X%4X\r", TRANS_ADDR, callsign2, callsign1);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //Check validity
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false;

    uint8_t validity = valid_cmd(cmnd);
    return validity;
}

uint32_t read_destination_callsign() {
    print("ES+R%2XF5\r", TRANS_ADDR);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command
    // Answer: OK+DDDDDD<CR>

    //Check validity
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false;

    uint16_t callsign1 = scan_uint(cmnd, 5, 4);
    uint8_t callsign2 = scan_uint(cmnd, 3, 2);
    uint32_t callsign = 0;

    //put together
    callsign = callsign | (callsign1) | (callsign2 << 16);
    return callsign;
}

// 8. Set source callsign
uint8_t set_source_callsign(uint32_t callsign) {
    //callsign = (callsign2 << 16) + callsign1
    uint16_t callsign1 = callsign; //truncating callsign to be 24 bites
    uint8_t callsign2 = callsign >> (4*4);

    print("ES+W%2XF6%2X%4X\r", TRANS_ADDR, callsign2, callsign1);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command

    //Check validity
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false;

    uint8_t validity = valid_cmd(cmnd);
    return validity;
}

//Get source callsign
uint32_t read_source_callsign() {
    print("ES+R%2XF6\r", TRANS_ADDR);

    while (!received_cmd_available) {}
    // TODO set timeout for waiting for command
    // Answer: OK+DDDDDD<CR>

    //Check validity
    uint8_t cmnd[20];
    for (uint8_t i = 0; i < 20; i ++) {
        cmnd[i] = received_cmd[i];
    }
    received_cmd_available = false;

    uint16_t callsign1 = scan_uint(cmnd, 5, 4);
    uint8_t callsign2 = scan_uint(cmnd, 3, 2);
    uint32_t callsign = 0;

    //put together
    callsign = callsign | (callsign1) | (callsign2 << 16);
    return callsign;
}

// 16. Get uptime
uint32_t get_trans_uptime(){
  print("ES+R%2X02", TRANS_ADDR);
  uint8_t offset = 5;
  uint8_t count = 8;

  while (!received_cmd_available) {}
  // TODO set timeout for waiting for command
  //Check validity
  uint8_t cmnd[20];
  for (uint8_t i = 0; i < 20; i ++) {
      cmnd[i] = received_cmd[i];
  }
  received_cmd_available = false;

  uint32_t uptime = scan_uint(cmnd, offset, count);

  return uptime;
}

// 17. Get number of transmitted packets
uint32_t get_transmitted_num_of_packets(){
  print("ES+R%2X03", TRANS_ADDR);
  uint8_t offset = 5;
  uint8_t count = 8;

  while (!received_cmd_available) {}
  // TODO set timeout for waiting for command
  //Check validity
  uint8_t cmnd[20];
  for (uint8_t i = 0; i < 20; i ++) {
      cmnd[i] = received_cmd[i];
  }
  received_cmd_available = false;

  uint32_t transmitted_num_of_packets = scan_uint(cmnd, offset, count);

  return transmitted_num_of_packets;
}

// 18. Get number of received packets
uint32_t get_received_num_of_packets(){
  print("ES+R%2X04", TRANS_ADDR);
  uint8_t offset = 5;
  uint8_t count = 8;

  while (!received_cmd_available) {}
  // TODO set timeout for waiting for command
  //Check validity
  uint8_t cmnd[20];
  for (uint8_t i = 0; i < 20; i ++) {
      cmnd[i] = received_cmd[i];
  }
  received_cmd_available = false;

  uint32_t received_num_of_packets = scan_uint(cmnd, offset, count);

  return received_num_of_packets;
}


// 19. Get number of packets with CRC error
uint32_t get_received_num_of_packets_CRC(){
  print("ES+R%2X05", TRANS_ADDR);
  uint8_t offset = 5;
  uint8_t count = 8;

  while (!received_cmd_available) {}
  // TODO set timeout for waiting for command
  //Check validity
  uint8_t cmnd[20];
  for (uint8_t i = 0; i < 20; i ++) {
      cmnd[i] = received_cmd[i];
  }
  received_cmd_available = false;

  uint32_t num_of_packets_CRC_error = scan_uint(cmnd, offset, count);

  return num_of_packets_CRC_error;
}
