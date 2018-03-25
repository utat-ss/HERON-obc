#include "can_queue.h"

Can_queue initCanQueue(){
	Can_queue Q;
	Q.currSize = 0;
	Q.front = 0;
	Q.rear = 0;

	return Q;
}

int CANQ_isFull(Can_queue *Q){
	return Q->currSize == CANQ_MAXSIZE;
}

int CANQ_isEmpty(Can_queue *Q){
	return Q->currSize == 0;
}

void CANQ_enqueue(Can_queue *Q, uint8_t num){
	if(!CANQ_isFull(Q)){
		Q->field_num[Q->rear] = num;
		Q->rear = (Q->rear + 1) % CANQ_MAXSIZE;
		Q->currSize++;
	}
}

int CANQ_dequeue(Can_queue *Q, uint8_t* num){
	if(!CANQ_isEmpty(Q)){
		*num = Q->field_num[Q->front];
		Q->front = (Q->front + 1) % CANQ_MAXSIZE;
		Q->currSize--;
		return 0;
	}
	return 1; /* error! */
}
