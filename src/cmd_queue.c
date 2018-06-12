#include "cmd_queue.h"

Cmd_queue initCmdQueue(){
	Cmd_queue Q;
	Q.currSize = 0;
	Q.front = 0;
	Q.rear = 0;

	return Q;
}

int CMDQ_isFull(Cmd_queue *Q){
	return Q->currSize == CMDQ_MAXSIZE;
}

int CMDQ_isEmpty(Cmd_queue *Q){
	return Q->currSize == 0;
}

void CMDQ_enqueue(Cmd_queue *Q, void (*func)(uint8_t, uint8_t), uint8_t receiver, uint8_t data){
	if(!CMDQ_isFull(Q)){
		Q->queueArray[Q->rear].func = func;
		Q->queueArray[Q->rear].receiver = receiver;
		Q->queueArray[Q->rear].data = data;
		Q->rear = (Q->rear + 1) % CMDQ_MAXSIZE;
		Q->currSize++;
	}
}

int CMDQ_dequeue(Cmd_queue *Q, OBC_Command* cmd){
	if(!CMDQ_isEmpty(Q)){
		*cmd = Q->queueArray[Q->front];
		Q->front = (Q->front + 1) % CMDQ_MAXSIZE;
		Q->currSize--;
		return 0;
	}
	return 1; /* error! */
}
