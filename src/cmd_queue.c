#include "cmd_queue.h"

Cmd_queue initQueue(){
	Cmd_queue Q;
	Q.currSize = 0;
	Q.front = 0;
	Q.rear = 0;

	return Q;
}

int isFull(Cmd_queue *Q){
	return Q->currSize == MAXSIZE;
}

int isEmpty(Cmd_queue *Q){
	return Q->currSize == 0;
}

void enqueue(Cmd_queue *Q, void (*func)(uint8_t, uint8_t), uint8_t receiver, uint8_t data){
	if(!isFull(Q)){
		Q->queueArray[Q->rear].func = func;
		Q->queueArray[Q->rear].receiver = receiver;
		Q->queueArray[Q->rear].data = data;
		Q->rear = (Q->rear + 1) % MAXSIZE;
		Q->currSize++;
	}
	else
		print("enqueue don't work\n");
}

int dequeue(Cmd_queue *Q, Command* cmd){
	if(!isEmpty(Q)){
		*cmd = Q->queueArray[Q->front];
		Q->front = (Q->front + 1) % MAXSIZE;
		Q->currSize--;
		return 0;
	}
	return 1; /* error! */
}
