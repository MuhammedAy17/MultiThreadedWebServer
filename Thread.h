//
// Created by Muham on 3/5/2024.
//

#ifndef WEBSERVER_FILES_THREAD_H
#define WEBSERVER_FILES_THREAD_H

#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include "Queue.h"

enum REQUEST_TYPE {
    OTHER = 2,
    DYNAMIC = 1,
    STATIC = 0
};

typedef struct thread_t{
    int id;
    int overallThreads;
    int staticThreads;
    int dynamicThreads;
    pthread_t  m_thread;
}* Thread;

int createThread(int id,Thread *thread);
int startThread(Thread thread,Queue req_queue);
int addThread(Thread thread,enum REQUEST_TYPE type);
void* runThread(void *args);
#endif //WEBSERVER_FILES_THREAD_H
