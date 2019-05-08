
/*
 * Harness based tests for ensuring functionality of CAN commands, commands to test include:
 * 		- EPS HK
 * 		- EPS Control
 * 		- PAY HK
 * 		- PAY Optical
 * 		- PAY Control
 */

#include <stdlib.h>
#include <test/test.h>
#include "general.h"

// Check functionality of harness test
void harness_func_test( void ){
	ASSERT_EQUAL(1+1, 2);
}

// EPS HK test
void eps_hk_test( void ){
	// generate a simulated EPS response that will later be used
	uint8_t eps_response[CAN_EPS_HK_FIELD_COUNT][8] = { 0x00 };
	for(uint8_t field=0; field<CAN_EPS_HK_FIELD_COUNT; field++){
		eps_response[field][0] = 0;
		eps_response[field][1] = CAN_EPS_HK;
		eps_response[field][2] = field;
		eps_response[field][3] = field;
		eps_response[field][4] = field+1;
		eps_response[field][5] = field+2;
		eps_response[field][6] = 0;
		eps_response[field][7] = 0;
	}

	// request data from EPS, and then proceed to simulate responses
	req_eps_hk_fn();
	uint8_t queue_data[8] = { 0 };
	uint8_t field_num = 0;

	while(field_num < CAN_EPS_HK_FIELD_COUNT){
		// dequeue from the tx_msg queue and check its contents
		dequeue(&eps_tx_msg_queue, data);
		ASSERT_EQUAL(queue_data[1], CAN_EPS_HK);
		ASSERT_EQUAL(queue_data[2], field_num);

		// insert simulated eps response into the rx_queue the response and process it
		enqueue(&data_rx_msg_queue, eps_response[field_num]);
		process_next_rx_msg();
		field_num++;
	}

	// check if eps_hk_fields holds the proper values
	for(uint8_t field=0; field<CAN_EPS_HK_FIELD_COUNT; field++){
	 	uint32_t eps_response_data = (eps_response[field][3] << 16) | 
	 									(eps_response[field][4] << 8) | 
	 									(eps_response[field][5]);
	 	ASSERT_EQUAL(eps_response_data, eps_hk_fields[field]);
	}
}

// PAY HK test
void pay_hk_test( void ){
	// generate a simulated PAY response that will later be used
	uint8_t pay_response[CAN_PAY_HK_FIELD_COUNT][8] = { 0x00 };
	for(uint8_t field=0; field<CAN_PAY_HK_FIELD_COUNT; field++){
		pay_response[field][0] = 0;
		pay_response[field][1] = CAN_PAY_HK;
		pay_response[field][2] = field;
		pay_response[field][3] = field;
		pay_response[field][4] = field+1;
		pay_response[field][5] = field+2;
		pay_response[field][6] = 0;
		pay_response[field][7] = 0;
	}

	// request data from PAY, and then proceed to simulate responses
	req_pay_hk_fn();
	uint8_t queue_data[8] = { 0 };
	uint8_t field_num = 0;

	while(field_num < CAN_PAY_HK_FIELD_COUNT){
		// dequeue from the tx_msg queue and check its contents
		dequeue(&pay_tx_msg_queue, data);
		ASSERT_EQUAL(queue_data[1], CAN_PAY_HK);
		ASSERT_EQUAL(queue_data[2], field_num);

		// insert simulated pay response into the rx_queue the response and process it
		enqueue(&data_rx_msg_queue, pay_response[field_num]);
		process_next_rx_msg();
		field_num++;
	}

	// check if pay_hk_fields holds the proper values
	for(uint8_t field=0; field<CAN_PAY_HK_FIELD_COUNT; field++){
	 	uint32_t pay_response_data = (pay_response[field][3] << 16) | 
	 									(pay_response[field][4] << 8) | 
	 									(pay_response[field][5]);
	 	ASSERT_EQUAL(pay_response_data, pay_hk_fields[field]);
	}
}

// PAY OPT test
void pay_opt_test( void ){
	// generate a simulated PAY_OPT response that will later be used
	uint8_t pay_response[CAN_PAY_OPT_FIELD_COUNT][8] = { 0x00 };
	for(uint8_t field=0; field<CAN_PAY_OPT_FIELD_COUNT; field++){
		pay_response[field][0] = 0;
		pay_response[field][1] = CAN_PAY_OPT;
		pay_response[field][2] = field;
		pay_response[field][3] = field;
		pay_response[field][4] = field+1;
		pay_response[field][5] = field+2;
		pay_response[field][6] = 0;
		pay_response[field][7] = 0;
	}

	// request data from PAY_OPT, and then proceed to simulate responses
	req_pay_opt_fn();
	uint8_t queue_data[8] = { 0 };
	uint8_t field_num = 0;

	while(field_num < CAN_PAY_OPT_FIELD_COUNT){
		// dequeue from the tx_msg queue and check its contents
		dequeue(&pay_tx_msg_queue, data);
		ASSERT_EQUAL(queue_data[1], CAN_PAY_OPT);
		ASSERT_EQUAL(queue_data[2], field_num);

		// insert simulated pay opt response into the rx_queue the response and process it
		enqueue(&data_rx_msg_queue, pay_response[field_num]);
		process_next_rx_msg();
		field_num++;
	}

	// check if pay_opt_fields holds the proper values
	for(uint8_t field=0; field<CAN_PAY_FIELD_COUNT; field++){
	 	uint32_t pay_response_data = (pay_response[field][3] << 16) | 
	 									(pay_response[field][4] << 8) | 
	 									(pay_response[field][5]);
	 	ASSERT_EQUAL(pay_response_data, pay_opt_fields[field]);
	}
}

test_t t1 = { .name = "harness functionality test", .fn = harness_func_test };
test_t t2 = { .name = "eps hk test", .fn = eps_hk_test };
test_t t3 = { .name = "pay hk test", .fn = pay_hk_test };
test_t t4 = { .name = "pay opt test", .fn = pay_opt_test }

test_t* suite[] = { &t1, &t2, &t3, &t4};

int main( void ) {
    init_obc_core();
    init_obc_comms();

    run_tests(suite, sizeof(suite) / sizeof(suite[0]));
    return 0;
}
