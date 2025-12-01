//
// Created by Muham on 3/5/2024.
//

#include "Thread.h"
#include "request.h"
#include <unistd.h>

typedef struct arguments_t{
   Queue args_Queue;
   Thread args_Thread;
}* FUNC_Arguments;

//Allocates and Creates a new Worker Thread, gets the id and a pointer to thread ( equivalent to Constructor in C++ )
//returns 1 on Success 0 Otherwise
int createThread(int id, Thread *thread) {
    *thread = (Thread)malloc(sizeof(**thread));
    Thread new_thread = *thread;
    if(new_thread == NULL){
        return FAIL;
    }
    new_thread->id = id;
    new_thread->overallThreads = 0;
    new_thread->dynamicThreads = 0;
    new_thread->staticThreads = 0;
    return SUCCESS;
}
//creates a new thread that starts to run the function run_thread
//this function does all the work for the thread which is processing the new request when available
//returns 1 on Success 0 Otherwise
int startThread(Thread thread,Queue req_queue) {
    if(req_queue == NULL || thread == NULL){
        return FAIL;
    }
    FUNC_Arguments arguments = (FUNC_Arguments)malloc(sizeof (*arguments));
    arguments->args_Queue = req_queue;
    arguments->args_Thread = thread;
    if(pthread_create(&thread->m_thread,NULL,runThread,arguments) != 0){
        return FAIL;
    }
    return SUCCESS;
}
//adds the counter of threads for Usage Statistics according to the request type( Dynamic / static )
//returns 1 on Success 0 Otherwise
int addThread(Thread thread, enum REQUEST_TYPE type) {
    if (thread == NULL){
        return FAIL;
    }
    if(type == STATIC){
        thread->staticThreads++;
    }
    if(type == DYNAMIC){
        thread->dynamicThreads++;
    }
    thread->overallThreads++;
    return SUCCESS;
}

//this function that every thread executes which does all the job of receiving a new request
//which the master thread has added to the queue, this function calls the function request handle
//which handles all of the requests, and then calculates the dispatch time of every request when handled
//this function sends signals for block and block flush algorithms because we need to know when
// the queue is empty for block flush to know when it is allowed for the master thread to accepts and reveive
// new requests, and a signal for block algorithm to know when its possible to enqueue a request
void *runThread(void *args) {
    if(args == NULL){
        return NULL;
    }
    FUNC_Arguments arguments =args;
    Thread thread = arguments->args_Thread;
    Queue req_queue = arguments->args_Queue;
    if(thread == NULL || req_queue == NULL) {
        return NULL;
    }
    while(1){
        Request request = NULL;
        if(dequeue(req_queue,&request) == FAIL){
            exit(FAIL);
        }
        Time Handle_Time;
        gettimeofday(&Handle_Time,NULL);
        timersub(&Handle_Time,&request->arrivalTime,&request->dispatchTime);
        requestHandle(request,thread);
        Close(request->fd);
        pthread_mutex_lock(&req_queue->mutex);
        req_queue->runningThreads--;
        int overallElements = req_queue->runningThreads + req_queue->elementsNum;
        if(overallElements == 0){
            pthread_cond_signal(&req_queue->condBlockFlush);
        }
        if(overallElements < req_queue->queueSize){
            pthread_cond_signal(&req_queue->condBlock);
        }
        pthread_mutex_unlock(&req_queue->mutex);
    }
    return NULL;
}
