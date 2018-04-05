#ifndef CMD_QUEUE_H
#define CMD_QUEUE_H

#include <stdint.h>

#define CMDQ_MAXSIZE 50

typedef struct {
	void (*func)(uint8_t _receiver, uint8_t _data);
	uint8_t receiver;
	uint8_t data;
} OBC_Command;

typedef struct {
	int currSize;
	int front;
	int rear;
	OBC_Command queueArray[CMDQ_MAXSIZE];
} Cmd_queue;

Cmd_queue initCmdQueue();
int CMDQ_isFull(Cmd_queue *Q);
int CMDQ_isEmpty(Cmd_queue *Q);
void CMDQ_enqueue(Cmd_queue *Q, void (*func)(uint8_t, uint8_t), uint8_t receiver, uint8_t data);
int CMDQ_dequeue(Cmd_queue *Q, OBC_Command* cmd);

#endif
