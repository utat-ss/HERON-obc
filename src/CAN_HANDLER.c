// Returns 0 if no command executed 

int CAN_HANDLER ( uint16_t identifier, uint8_t* pt_data, uint8_t size) {

	// Get 11 bit identifier (priority)

	// Get data 

	// Switch statements 

	uint16_t command; 

	uint16_t sender_id = identifier & TX_MASK;
	uint16_t receiver_id = identifier & RX_MASK;
	uint16_t msg_id = identifier & MSG_MASK;

	command = identifier; 

	// Predefinined commands
	switch(command) 
	{

		case command_1:
		// code to be executed 

			break;

		case command_2:
		// code to be executed 

			break;

		case command_3:
		// code to be executed 

			break;

		default: 

			return 0; 
			//Return error no command executed 
	}


}

