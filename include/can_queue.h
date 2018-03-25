#ifndef CAN_QUEUE_H
#define CAN_QUEUE_H

// #include "global_header.h"
#include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>

#define CANQ_MAXSIZE 3

typedef struct {
	int currSize;
	int front;
	int rear;
	uint8_t field_num[CANQ_MAXSIZE];
} Can_queue;

Can_queue initCanQueue();
int CANQ_isFull(Can_queue *Q);
int CANQ_isEmpty(Can_queue *Q);
void CANQ_enqueue(Can_queue *Q, uint8_t num);
int CANQ_dequeue(Can_queue *Q, uint8_t* num);

#endif
