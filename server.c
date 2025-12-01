#include "segel.h"
#include "request.h"
#include "Queue.h"
// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//


// when then requests reach the limit, Handle the request according to the specific handling policy
void OverLoadHandler(Queue requestsQueue ,char *schedalg,int connfd,Time requestArrivalTime){
    int overallElemnts = requestsQueue->runningThreads + requestsQueue->elementsNum;
    if(strcmp(schedalg,"block") == 0){
        while (overallElemnts >= requestsQueue->queueSize){
            pthread_cond_wait(&requestsQueue->condBlock,&requestsQueue->mutex);
            overallElemnts = requestsQueue->runningThreads + requestsQueue->elementsNum;
        }
        Request request;
        if (createRequest(connfd, requestArrivalTime, &request) == FAIL) {
            exit(FAIL);
        }
        if (enqueue(requestsQueue, request) == FAIL) {
            exit(FAIL);
        }
        pthread_mutex_unlock((&requestsQueue->mutex));
    }
    else if (strcmp(schedalg,"bf") == 0){
        while (overallElemnts != 0){
            pthread_cond_wait(&requestsQueue->condBlockFlush,&requestsQueue->mutex);
            overallElemnts = requestsQueue->runningThreads + requestsQueue->elementsNum;
        }
        pthread_mutex_unlock((&requestsQueue->mutex));
        Close(connfd);
    }
    else if (strcmp(schedalg,"dt") == 0){
        Close(connfd);
        pthread_mutex_unlock(&requestsQueue->mutex);
    }
    else if (strcmp(schedalg,"dh") == 0){
        Request request;//NULL?
        if (createRequest(connfd, requestArrivalTime, &request) == FAIL) {
            exit(FAIL);
        }
        if(removeHead(requestsQueue) == FAIL){
            exit(FAIL);
        }
        if(enqueue(requestsQueue,request) == FAIL){
            exit(FAIL);
        }
        pthread_mutex_unlock(&requestsQueue->mutex);
    }
    else if (strcmp(schedalg,"random") == 0){
        if(removeRandomHalf(requestsQueue) == FAIL){
            exit(FAIL);
        }
        Request request;
        if (createRequest(connfd, requestArrivalTime, &request) == FAIL) {
            exit(FAIL);
        }
        if (enqueue(requestsQueue, request) == FAIL) {
            exit(FAIL);
        }
        pthread_mutex_unlock((&requestsQueue->mutex));
    }
    else{
        fprintf(stderr,"Overload Handling Error:Scheduling Algorithm Not Found");
        exit(FAIL);
    }
}

//Parse the new arguments
void getargs(int *port,int *numOfWorkerThreads,int *queueSize,char *schedalg, int argc, char *argv[])
{
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *numOfWorkerThreads = atoi(argv[2]);
    *queueSize = atoi(argv[3]);
    strcpy(schedalg,argv[4]);// Check After
    if((*numOfWorkerThreads) < 1 || (*queueSize) < 1){
        exit(1);
    }

}

//Makes the server multiThreaded, Master Thread gets the main request for creating the server, creates the queue and
// the Threads that handles the  requests simultaneously and then handles the request and then begins an infinite loop
//for accepting more requests.
int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen,queueSize,numOfWorkerThreads;
    char schedalg[8] = {0};
    struct sockaddr_in clientaddr;
    getargs(&port,&numOfWorkerThreads,&queueSize,schedalg, argc, argv);
    Queue requestsQueue = NULL;
    if(createQueue(queueSize,&requestsQueue) == FAIL){
        return FAIL;
    }
    for (int i = 0; i < numOfWorkerThreads ; ++i) {
        Thread thread = NULL;
        if(createThread(i,&thread) == FAIL) {
            return FAIL;
        }
        if(startThread(thread,requestsQueue) == FAIL) {
            return FAIL;
        }
    }
    listenfd = Open_listenfd(port);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *) &clientaddr, (socklen_t *) &clientlen);
        struct timeval requestArrivalTime;
        gettimeofday(&requestArrivalTime, NULL);
        pthread_mutex_lock(&requestsQueue->mutex);
        int overallElemnts = requestsQueue->runningThreads + requestsQueue->elementsNum;
        if (overallElemnts >= requestsQueue->queueSize) {
            OverLoadHandler(requestsQueue,schedalg,connfd,requestArrivalTime);
        } else {
            Request request;
            if (createRequest(connfd, requestArrivalTime, &request) == FAIL) {
                return FAIL;
            }
            if (enqueue(requestsQueue, request) == FAIL) {
                exit(FAIL);
            }
            pthread_mutex_unlock(&requestsQueue->mutex);
        }
    }
}


    


 
