#include "can_handler.h"

// Returns 0 if no command executed
void can_handler (uint16_t identifier, uint8_t* pt_data, uint8_t size) {

	// // Bit masks defined in can.h to get sender_id, receiver_id, and msg_id
	// uint16_t sender_id = identifier & TX_MASK;
	// uint16_t receiver_id = identifier & RX_MASK;
	// uint16_t msg_id = identifier & MSG_MASK;
  //
  //
	// // Confirm RX == SSM, redundancy
	// if(receiver_id != OBC_RX) return;
  //
	// /*
  //
	// The following are the definitions from can.h
  //
	// #define HK_DATA       0x0020
	// #define HK_SENSOR     0x0024
	// #define HK_REQ        0x0026
	// #define HK_REQ_SENSOR 0x0028
	// #define SCI_REQ       0x02A
	// #define SCI_DATA      0x02C
  //
  // */
  //
	// // Predefinined commands
	// switch(msg_id)
	// {
  //
	// 	case HK_DATA:
	// 		print("Received HK Data");
  //
	// 		// 2 Protocols - All & SSM specific
  //
	// 		// All
  //
	// 		// Checking receiver_id is unique for 3 different SSMs - flag when ssm is done
  //
	// 		// Protocol for housekeeping data, start byte of housekeeping will be representation of the amount of bytes to be sent in hex
	// 		// Ensure all bytes are received using a count
	// 		// End of houskeeping will be FF --> confirm?
	// 		// pt_data[0] gives size
	// 		// total count = pt_data[0]
	// 		// total count - size = bytes to wait for, do not send to comms just yet
  //
	// 		// Remember after the first CAN message is sent another SSM can send a message but
  //
	// 		// Data buffer
  //
	// 		// Can send can message to comms when buffer is full but still wait for incoming messages, send by ssm
  //
  //
	// 		break;
	// 	default:
	// 		print("Unknown CAN message.");
  //
	// 		//Return error no command executed
	// }


}
