#include "global_header.h"
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
			decode_HK_message(sender_id, message_id, pt_data, size);
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
