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
    uint8_t currSize;
    uint8_t front;
    uint8_t rear;
    struct Node queueArray[MAXQUEUESIZE];
};

void initQueue(struct Queue ** qPtr);

uint8_t isEmpty(struct Queue * Q);

uint8_t isFull(struct Queue * Q);

uint8_t enqueue(struct Queue * Q, struct Command c);

uint8_t dequeue(struct Queue * Q, struct Command * c);

#endif // QUEUE_H_INCLUDED
