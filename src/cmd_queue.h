/*
Author: Shimi Smith
*/

/*
 CAN messages that go TO obc enqueue new commands
 CAN messages FROM obc should be the result of a dequeue
*/

/*
	Local queue versus TM queue
	1) How often do we make hk requests?
	2) How long are we connected to Earth?
	3) How large does each queue need to be, if we have 2?
	4) Are we overusing/underusing flash; is it inefficient?
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
