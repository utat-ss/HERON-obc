#include "can_handler.h"

// Returns 0 if no command executed
void can_handler (uint16_t identifier, uint8_t* pt_data, uint8_t size) {

	// Bit masks defined in can.h to get sender_id, receiver_id, and msg_id
	uint16_t sender_id = identifier & TX_MASK;
	uint16_t receiver_id = identifier & RX_MASK;
	uint16_t msg_id = identifier & MSG_MASK;


	// Confirm RX == SSM, redundancy
	if(receiver_id != OBC_RX) return;

	// Predefinined commands
	switch(msg_id)
	{

		case HK_DATA:
			print("Received HK Data");

			// 2 Protocols - All & SSM specific

			// Assuming 6 bytes will be sent from each SSM
			// Assuming each SSM will have 3 sensors
			// Assuming each sensor will have 2 bytes of information

			// Data Bufer = ( EPS | COMMS | PAY | OBC)
			// Each subsystem of the data buffer will appear as follows:
			// ( Sensor 1 | Sensor 2 | Sensor 3), as mentioned each sensor reading will be assumed to be 2 bytes

			// This command is hk_req_all since hk_all_requested = 1
			if(hk_all_requested) {

				if(sender_id == EPS_TX) {
					for(size_t i = 0; i < 5; i++){
						housekeeping_buffer -> buffer[i] = pt_data[i];
					}
					hk_flag_eps = 1;
				}

				else if(sender_id == COM_TX) {
					for(size_t i = 0; i < 5; i++){
						housekeeping_buffer -> buffer[i+6] = pt_data[i];
					}
					hk_flag_comms = 1;
				}

				else if(sender_id == PAY_TX) {
					for(size_t i = 0; i < 5; i++){
						housekeeping_buffer -> buffer[i+12] = pt_data[i];
					}
					hk_flag_pay = 1;
				}

		  }

			// This command is hk_req_ssm since hk_all_requested = 0
			else {

				// Find current size of the buffer
				// Note: Check if buffer is full before hk_req_ssm

				// Assuming buffer is not full, 8 bytes of space left (2 byte header + 3 sensors * 2 bytes of header)
				// The header shall be HK_DATA | sender_id, i.e. 0x0020 | 0x0500 for PAY_TX


				uint8_t index = data_buffer_pointer -> curr_size;
				//Adding header
				data_buffer_pointer -> buffer[index] = HK_DATA | sender_id;

				for(size_t i = 1; i < 6; i++){
					data_buffer_pointer -> buffer[index + i] = pt_data[i-1];
				}

			}

			break;

		case SCI_REQ:

			// Redundancy
			if(sender_id != PAY_TX) return;

			// Find current size of the buffer
			// Note: Check if buffer is full before hk_req_ssm

			// Assuming buffer is not full, 8 bytes of space left (2 byte header + 3 sensors * 2 bytes of header)
			// The header shall be HK_DATA | PAY_TX, i.e. 0x0020 | 0x0500

			uint8_t index = data_buffer_pointer -> curr_size;
			//Adding header
			data_buffer_pointer -> buffer[index] = HK_DATA | PAY_TX;

			for(size_t i = 1; i < 6; i++){
				data_buffer_pointer -> buffer[index + i] = pt_data[i-1];
			}

			break;

		default:
			print("Unknown CAN message.");

			//Return error no command executed
	}

	return;

}
