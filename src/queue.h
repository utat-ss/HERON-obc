#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include "global_header.h"


#define MAXQUEUESIZE 50

struct Command {
    uint16_t command_to_send;
};

struct Node{
    struct Command command;
};

struct Queue{
    int currSize;
    int front;
    int rear;
    struct Node queueArray[MAXQUEUESIZE];
};

void initQueue(struct Queue ** qPtr);

int isEmpty(struct Queue * Q);

int isFull(struct Queue * Q);

int enqueue(struct Queue * Q, struct Command c);

int dequeue(struct Queue * Q, struct Command * c);

#endif // QUEUE_H_INCLUDED
