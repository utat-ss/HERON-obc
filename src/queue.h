/*
Author: Shimi Smith
*/

/*
 CAN messages that go TO obc enqueue new commands
 CAN messages FROM obc should be the result of a dequeue
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define MAXSIZE 50

typedef struct command_t{
	void (*func)(uint8_t _receiver, uint8_t _data);
	uint8_t receiver;
	uint8_t data;
} Command;

typedef struct Node{
	Command command;
} Node;

typedef struct Queue{
	int currSize;
	int front;
	int rear;
	Node queueArray[MAXSIZE];
} Queue;

Queue initQueue();
int isFull(Queue *Q);
int isEmpty(Queue *Q);
void enqueue(Queue *Q, void (*func)(uint8_t, uint8_t), uint8_t receiver, uint8_t data);
Command dequeue(Queue *Q);
