#include "transceiver.h"

//Default Address - may change to a variable so that we can set value
#define TRANS_ADDR  0x22

//Write to status control register
void set_trans_scw(uint16_t reg) {
    print("ES+W%2X00%4X\r", TRANS_ADDR, reg);
}

//Read and return status control register
uint16_t read_teans_scw() {
    print("ES+R%2X00\r", TRANS_ADDR);
    //get answer
    //if answer OK+[RR]0000[WWWW]<CR>
    //return WWWW
    //else return something that tells you value is wrong
}

//Set transceiver frequency
uint8_t set_trans_freq(int freq) {
    uint16_t Fcfrac;
    uint8_t Fcinte;
    //refer to p.17
    //uint8_t NPRESC = 2;
    //uint8_t outdiv = 8;
    //int Freq_xo = 26000000;
    //rfChannel = (Fcinte + Fcfrac/(2^19))*NPRESC*Freq_xo/outdiv;

//TODO: Figure out how to convert

    print("ES+W%2X01%4X%2X\r", TRANS_ADDR, Fcfrac, Fcinte);

    //read and return 0 or 1 depending if it was able to configure
}

uint16_t read_trans_freq() {
    print("ES+R%2X01\r", TRANS_ADDR);
    //read answer
}

//Consider making TRANS_ADDR a variable if wanting to implement this function
uint8_t set_trans_addr(uint8_t addr) {
  if (addr != 23 && addr != 22) {
      return 0; //invalid address
  }
    print("ES+R%2XFC%2X\r", TRANS_ADDR, addr);

    //return 1 or 0 if failed or succeeded
}

// from p.18
void set_trans_pipe_timeout(uint8_t timeout) {
    print("ES+W%2X06000000%2X\r", TRANS_ADDR, timeout);
}
