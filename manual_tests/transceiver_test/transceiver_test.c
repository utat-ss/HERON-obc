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
    read_trans_scw();
/*
    //Go through functions
    set_trans_freq();
    read_trans_freq();

    uint8_t pipeline_timeout = 15; //in seconds
    set_trans_pipe_timeout(pipeline_timeout);

    char callsign[6] = "VA3ZZZ"; //Random callsign - get a legit one later
    set_destination_callsign(callsign);
    read_destination_callsign();

    char source_callsign[6] = "VA3ZBR"; //Brytni's callsign
    set_source_callsign(source_callsign);
    read_source_callsign();

    //set_pipeline();
    //delay(15500); //wait for timeout
    //Should return the same value as what was set - 15
    //read_trans_scw()

    //Turn on Beacon
    //set_beacon();
    //Turn off Beacon
    //off_beacon();

    get_trans_uptime();*/
}
