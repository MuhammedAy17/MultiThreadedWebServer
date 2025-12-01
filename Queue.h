#ifndef HW3_OS_QUEUE_H
#define HW3_OS_QUEUE_H

#include "segel.h"

enum Results {
    SUCCESS = 1,
    FAIL = 0
};

typedef struct timeval Time;

typedef struct request_t {
    int fd;
    Time arrivalTime;
    Time dispatchTime;
}* Request;

typedef struct node_t* Node;

typedef struct queue_t {
    Node root;
    Node last;
    int elementsNum;
    int queueSize;
    int runningThreads;
    pthread_cond_t condEnqueue;
    pthread_mutex_t mutex;
    pthread_cond_t condBlock;
    pthread_cond_t condBlockFlush;
}* Queue;


int createRequest(int fd, Time time, Request* requestPtr);
enum Results Check1(Queue queue);
enum Results Check2(Queue queue,Request request,Node *newNode);
int createQueue(int size, Queue* queuePtr);
int enqueue(Queue queue, Request request);
int dequeue(Queue queue, Request* request);
int removeHead(Queue queue);
int removeItem(Queue queue, int index);
int removeRandomHalf(Queue queue);

#endif //HW3_OS_QUEUE_H