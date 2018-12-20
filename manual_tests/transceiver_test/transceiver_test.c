/*
Program to test the functionality of the transceiver functions. Expected results
are written as comments.
*/

#include <uart/uart.h>
#include <utilities/utilities.h>
#include "../../src/transceiver.h"

//Note that read functions return the value - however we first must test how
//transceiver deals with weird input before arbitrarily using print statements

int main(void){
    init_uart();
    print("\n\n\r"); //for avoiding programming gibberish

    //Might need to add a delay function here

    init_trans();
    uint16_t scw;
    uint8_t ret;

    ret = get_trans_scw(&scw);
    print("scw = %.4X\n", scw);

    // Turn off beacon mode
    scw &= ~_BV(TRANS_BCN);
    ret = set_trans_scw(scw);
    print("set SCW: %u\n", ret);

    ret = get_trans_scw(&scw);
    print("scw = %.4X\n", scw);

    // uint32_t freq;
    // get_trans_freq(&freq);
    // uint32_t dest_call_sign;
    // get_trans_dest_call_sign(&dest_call_sign);
    // uint32_t src_call_sign;
    // get_trans_src_call_sign(&src_call_sign);
    // uint32_t uptime;
    // get_trans_uptime(&uptime);

/*
    //Go through functions
    set_trans_freq();

    uint8_t pipeline_timeout = 15; //in seconds
    set_trans_pipe_timeout(pipeline_timeout);

    char callsign[6] = "VA3ZZZ"; //Random callsign - get a legit one later
    set_trans_dest_call_sign(callsign);


    char source_callsign[6] = "VA3ZBR"; //Brytni's callsign
    set_trans_src_call_sign(source_callsign);


    //set_trans_pipeline();
    //delay(15500); //wait for timeout
    //Should return the same value as what was set - 15
    //get_trans_scw()

    //Turn on Beacon
    //turn_on_trans_beacon();
    //Turn off Beacon
    //turn_off_trans_beacon();
    */
}
