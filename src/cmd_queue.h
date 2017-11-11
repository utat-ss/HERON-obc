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

// typedef struct cmd_node{
// 	Command command;
// } Cmd_node;

typedef struct cmd_queue{
	int currSize;
	int front;
	int rear;
	Command queueArray[MAXSIZE];
} Cmd_queue;

Cmd_queue initQueue();
int isFull(Cmd_queue *Q);
int isEmpty(Cmd_queue *Q);
void enqueue(Cmd_queue *Q, void (*func)(uint8_t, uint8_t), uint8_t receiver, uint8_t data);
int dequeue(Cmd_queue *Q, Command* cmd);
