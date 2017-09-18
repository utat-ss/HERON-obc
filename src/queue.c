#include "global_header.h"

void initQueue(struct Queue ** qPtr)
{
    struct Queue * Q = (struct Queue *)malloc(sizeof(struct Queue));
    Q->currSize = 0;
    Q->front = 0;
    Q->rear = 0;
    *qPtr = Q;
}

uint8_t isEmpty(struct Queue * Q)
{
    return Q->currSize == 0;
}

uint8_t isFull(struct Queue * Q)
{
    return Q->currSize == MAXQUEUESIZE;
}

uint8_t enqueue(struct Queue * Q, struct Command c)
{
    if (!isFull(Q)){
        Q->queueArray[Q->rear].command = c;
        Q->rear = (Q->rear + 1) % MAXQUEUESIZE;
        Q->currSize++;
        return 0;
    }
    else{
        return 1;
    }
}

uint8_t dequeue(struct Queue * Q, struct Command * c)
{
    if (!isEmpty(Q)){
        *c = Q->queueArray[Q->front].command;
        Q->front = (Q->front + 1) % MAXQUEUESIZE;
        Q->currSize--;
        return 0;
    }
    else{
        return 1;
    }
}
