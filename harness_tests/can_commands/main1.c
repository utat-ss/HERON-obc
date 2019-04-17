/*
 * Harness based test for CAN commands
 */

#include <stdlib.h>
#include <test/test.h>
#include "general.h"

// check functionality of harness test
void harness_func_test( void ){
	ASSERT_EQUAL(1+1, 2);
}

// check functionality of processing rx messages for a variety of cases
void eps_hk_test( void ){
	// 8 byte data array for peeking into queues
	uint8_t data[8] = { 0 };
	// array holding all the data recieved from the fields
	uint32_t eps_hk_test_data[CAN_EPS_HK_FIELD_COUNT] = { 0 };
	// number of fields recieved
	uint8_t field_number = 0;

	// request hk data from eps
	req_eps_hk_fn();

	// recieve and request each successive field until completion
	while(field_number < CAN_EPS_HK_FIELD_COUNT) {
		if(!queue_empty(eps_tx_msg_queue)){
			peek_queue(eps_tx_msg_queue, data);
			// check if message is from the right source
			ASSERT_EQUAL(data[1], CAN_EPS_HK);
			send_next_eps_tx_msg();
		}
		if(!queue_empty(data_rx_msg_queue)){
			peek_queue(data_rx_msg_queue, data);
			// check if message is from the right source
			ASSERT_EQUAL(data[1], CAN_EPS_HK);
			// save the data read for later reference
			eps_hk_test_data[field_number] =
                (((uint32_t) data[3]) << 16) |
                (((uint32_t) data[4]) << 8) |
                ((uint32_t) data[5]);
            // process the next rx_msg
            process_next_rx_msg();
			field_number++;
		}
	}

	// after all fields are recieved, check if eps_hk_data matches global array
	for(uint8_t i=0; i<CAN_EPS_HK_FIELD_COUNT; i++){
		ASSERT_EQUAL(eps_hk_test_data[i], eps_hk_fields[i])
	}
}






test_t t1 = { .name = "harness functionality test", .fn = harness_func_test };
test_t t2 = { .name = "", .fn = };

test_t* suite[] = { &t1, &t2, &t3, &t4, &t5, &t6, &t7, &t8, &t9 };

int main( void ) {
    init_obc_core();
    init_obc_comms();

    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
