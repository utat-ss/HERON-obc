#include "can_handler.h"


// Returns 0 if no command executed
void can_handler ( uint16_t identifier, uint8_t* pt_data, uint8_t size) {


	uint16_t sender_id = identifier & TX_MASK;
	uint16_t receiver_id = identifier & RX_MASK;
	uint16_t msg_id = identifier & MSG_MASK;



	// Predefinined commands
	switch(msg_id)
	{

		case HK_DATA:
			print("Received HK Data");

			break;
		default:
			print("Unknown CAN message.");

			//Return error no command executed
	}


}
