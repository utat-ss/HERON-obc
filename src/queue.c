/*
Author: Shimi Smith

This queue implementation doesn't use malloc or free to adhere to the cubesat coding rules

EXAMPLE OF USE
--------------
Queue q = initQueue();
Queue *Q = &q;

enqueue(Q, 1);

printf("%d\n", dequeue(Q)); --> prints 1

*/

#include "queue.h"

Queue initQueue(){
	Queue Q;
	Q.currSize = 0;
	Q.front = 0;
	Q.rear = 0;

	return Q;
}

int isFull(Queue *Q){
	return Q->currSize == MAXSIZE;
}

int isEmpty(Queue *Q){
	return Q->currSize == 0;
}

void enqueue(Queue *Q, void (*func)(uint8_t, uint8_t), uint8_t receiver, uint8_t data){
	if(!isFull(Q)){
		Q->queueArray[Q->rear].func = func;
		Q->queueArray[Q->rear].receiver = receiver;
		Q->queueArray[Q->rear].data = data;
		Q->rear = (Q->rear + 1) % MAXSIZE;
		Q->currSize++;
	}
}

Command dequeue(Queue *Q){
	if(!isEmpty(Q)){
		Command c = Q->queueArray[Q->front].command;
		Q->front = (Q->front + 1) % MAXSIZE;
		Q->currSize--;
		return c;
	}

	// We should not get this far! ERROR
	Command garbage;
	garbage.func = NULL;
	return garbage;
}
