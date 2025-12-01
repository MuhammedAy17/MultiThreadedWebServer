
#include "Queue.h"


struct node_t {
    Request request;
    struct node_t* prev;
    struct node_t* next;
};

//Gets The request prev , next and ths Current Node - Creates and connects the node and puts it in nodePtr ( equivalent to Constructor in C++ )
//Returns 1 on Success 0 otherwise
int createNode(Request request, Node prev, Node next, Node* nodePtr)
{
    *nodePtr = (Node)malloc(sizeof(**nodePtr));
    Node node = *nodePtr;
    if(node == NULL)
        return FAIL;
    node->request = request;
    node->prev = prev;
    node->next = next;
    return SUCCESS;
}

//Gets the fd, current time and a pointer to the request- Creates a request class and returns it in requestPtr
//Initializes the arrival and dispatch time in order to calculate for each request the statistics
//Returns 1 on success 0 otherwise
int createRequest(int fd, Time time, Request* requestPtr)
{
    *requestPtr = (Request)malloc(sizeof(**requestPtr));
    Request request = *requestPtr;
    if(request == NULL)
        return FAIL;
    request->fd = fd;
    request->arrivalTime = time;
    request->dispatchTime = time;
    return SUCCESS;
}

//Creates the initial Queue Without any requests
//Initializes the locks defined in the Queue class
//Returns 1 on success 0 otherwise
int createQueue(int queueSize, Queue* queuePtr)
{
    if(queueSize <= 0)
        return FAIL;
    Node root = NULL;
    if(createNode(NULL, NULL, NULL, &root) != SUCCESS)
        return FAIL;
    *queuePtr = (Queue)malloc(sizeof(**queuePtr));
    Queue queue = *queuePtr;
    if(queue == NULL)
    {
        return FAIL;
    }
    queue->root = root;
    queue->last = root;
    queue->queueSize = queueSize;
    queue->elementsNum = 0;
    queue->runningThreads = 0;

    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->condEnqueue, NULL);
    pthread_cond_init(&queue->condBlock, NULL);
    pthread_cond_init(&queue->condBlockFlush, NULL);
    return SUCCESS;
}

// Adds a Node for the queue if not full
// sends a signal for threads that wait for queue size to be >= 0
//Returns 1 on success 0 otherwise
//
int enqueue(Queue queue, Request request)
{
    // Assumes queue->mutex is locked.
    Node newNode = NULL;
    if(Check2(queue,request,&newNode) == FAIL)
        return FAIL;
    queue->root->next = newNode;
    if(newNode->next != NULL)
        newNode->next->prev = newNode;
    queue->elementsNum++;
    if(queue->elementsNum == 1)
        queue->last = queue->root->next;
    pthread_cond_signal(&queue->condEnqueue);

    return SUCCESS;
}

//Removes an element from the waiting requests queue , which means adding a thread to handle this request
// if the queue is empty, we wait using cond_wait until there are requests ti be handled ( gets a signal )
//Returns 1 on success 0 otherwise
int dequeue(Queue queue, Request* request)
{
    if(queue == NULL)
    {
        return FAIL;
    }
    pthread_mutex_lock(&queue->mutex);

    while(queue->elementsNum == 0)
    {
        pthread_cond_wait(&queue->condEnqueue, &queue->mutex);
    }
    Node toDelete = queue->last;
    queue->last = toDelete->prev;
    queue->last->next = NULL;
    *request = toDelete->request;
    queue->elementsNum--;
    queue->runningThreads++;//Check Later for subtraction ( when finishing a running connection )
    pthread_mutex_unlock(&queue->mutex);
    return SUCCESS;
}
// Used for Drop_head sched alg , Gets the waiting requests queue and deletes the oldest request from the queue,then
// and then closes the request using Close ( ignores it )
//Returns 1 on success 0 otherwise
int removeHead(Queue queue)
{
    // Assumes queue->mutex is locked.

    if(Check1(queue)==FAIL)
        return FAIL;
    Node tmp = queue->last;
    queue->last = queue->last->prev;
    queue->last->next = NULL;
    Close(tmp->request->fd);
    queue->elementsNum--;
    return SUCCESS;
}


//Gets the queue and specific index , deletes the element in queue ini this specific index
//Returns 1 on success 0 otherwise
int removeItem(Queue queue, int index)
{
    // Assumes queue->mutex is locked.
    if(Check1(queue) == FAIL || index < 0|| index >= queue->elementsNum )
        return FAIL;

    Node prev = queue->root;

    for(int i = 0; i < index; i++)
    {
        prev = prev->next;
    }

    Node toDelete = prev->next;
    prev->next = toDelete->next;
    if(toDelete->next != NULL)
        toDelete->next->prev = prev;
    else
        queue->last = toDelete->prev;
    Close(toDelete->request->fd);
    queue->elementsNum--;
    return SUCCESS;
}

// Used for Drop_Random sched alg , gets the waiting tasks queue and deletes half of the elements randomly
//return 1 on Success 0 otherwise
int removeRandomHalf(Queue queue)
{
    // Assumes queue->mutex is locked.
    if(queue == NULL)
        return FAIL;
    int numToDelete = (queue->elementsNum + 1) / 2;
    int finalRes = SUCCESS;
    srand(time(NULL));
    for(int i = 0; i < numToDelete; i++)
    {
        int index = ((int)rand()) % queue->elementsNum;
        int res = removeItem(queue, index);
        if(res == FAIL)
            finalRes = FAIL;
    }
    return finalRes;
}

//Additional Condition Checks for not duplicating
enum Results Check1(Queue queue){
    if(queue == NULL)
    {
        return FAIL;
    }

    if(queue->elementsNum == 0)
        return FAIL;

    return SUCCESS;
}
enum Results Check2(Queue queue,Request request,Node *newNode){
    if(queue == NULL)
        return FAIL;
    if(queue->elementsNum >= queue->queueSize)
    {
        return FAIL;
    }
    if(createNode(request, queue->root, queue->root->next, newNode) != SUCCESS)
    {
        return FAIL;
    }
    return SUCCESS;
}