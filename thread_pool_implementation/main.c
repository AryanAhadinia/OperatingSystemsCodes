#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

typedef struct
{
    void *(*worker)(void *);
    void *args;
} Task;

typedef struct
{
    Task **tasks;
    int start;
    int end;
    int size;
} TaskQueue;

typedef struct
{
    Task **tasks;
    int (*priority)(void *);
    int end;
    int size;
} PriorityTaskQueue;

typedef struct
{
    TaskQueue *taskQueue;
    int numThreads;

    PriorityTaskQueue *priorityTaskQueue;
    int open;

    pthread_t masterThread;
    pthread_t *workerThreads;
} ThreadPool;

typedef struct
{
    int id;
    ThreadPool *threadPool;
    Task *task;
    int permission_to_close;
} NameSpace;

typedef struct
{
    int m;
    int n;
} AckermannArg;

// Constants

Task NULL_TASK = (Task){NULL, NULL};

// Queue functions

void createTaskQueue(TaskQueue *taskQueue, int size);

void enqueueTaskQueue(TaskQueue *taskQueue, Task *task);

Task *dequeueTaskQueue(TaskQueue *taskQueue);

int isTaskQueueEmpty(TaskQueue *taskQueue);

// Priority Queue functions

void createPriorityTaskQueue(PriorityTaskQueue *priorityTaskQueue, int (*priority)(void *), int size);

void enqueuePriorityTaskQueue(PriorityTaskQueue *priorityTaskQueue, Task *task);

Task *dequeuePriorityTaskQueue(PriorityTaskQueue *priorityTaskQueue);

int isPriorityTaskQueueEmpty(PriorityTaskQueue *priorityTaskQueue);

// Thread Pool functions

void createThreadPool(ThreadPool *threadPool, TaskQueue *taskQueue, int (*priority)(void *), int numThreads, int queueSize);

void *master(void *args);

void *worker(void *args);

void waitThreadPool(ThreadPool *threadPool);

// Ackermann functions

int ackermannPriority(void *args);

int ackermann(int m, int n);

void *ackermannWorker(void *args);

// main

int main()
{
    TaskQueue taskQueue;
    createTaskQueue(&taskQueue, 256);

    ThreadPool threadPool;
    createThreadPool(&threadPool, &taskQueue, ackermannPriority, 4, 1024);

    printf("Enter inputs in form of `m, n`. Enter `-1, -1` for termination\n");

    while (1)
    {
        int m, n;
        scanf("%d, %d", &m, &n);
        if (m == -1 && n == -1)
            break;
        Task *task = (Task *)malloc(sizeof(Task));
        AckermannArg *args = (AckermannArg *)malloc(sizeof(AckermannArg));
        task->worker = &ackermannWorker;
        task->args = args;
        args->m = m;
        args->n = n;
        enqueueTaskQueue(&taskQueue, task);
    }

    waitThreadPool(&threadPool);

    return 0;
}

// Queue functions

void createTaskQueue(TaskQueue *taskQueue, int size)
{
    taskQueue->tasks = (Task **)malloc(sizeof(Task *) * size);
    for (size_t i = 0; i < size; i++)
        taskQueue->tasks[i] = NULL;
    taskQueue->start = 0;
    taskQueue->end = 0;
    taskQueue->size = size;
}

void enqueueTaskQueue(TaskQueue *taskQueue, Task *task)
{
    taskQueue->tasks[taskQueue->end] = task;
    taskQueue->end++;
}

Task *dequeueTaskQueue(TaskQueue *taskQueue)
{
    Task *task = taskQueue->tasks[taskQueue->start];
    taskQueue->tasks[taskQueue->start] = NULL;
    taskQueue->start++;
    return task;
}

int isTaskQueueEmpty(TaskQueue *taskQueue)
{
    return taskQueue->start == taskQueue->end;
}

// Priority Queue functions

void createPriorityTaskQueue(PriorityTaskQueue *priorityTaskQueue, int (*priority)(void *), int size)
{
    priorityTaskQueue->tasks = (Task **)malloc(sizeof(Task *) * size);
    for (size_t i = 0; i < size; i++)
        priorityTaskQueue->tasks[i] = NULL;
    priorityTaskQueue->priority = priority;
    priorityTaskQueue->end = 0;
    priorityTaskQueue->size = size;
}

void enqueuePriorityTaskQueue(PriorityTaskQueue *priorityTaskQueue, Task *task)
{
    priorityTaskQueue->tasks[priorityTaskQueue->end] = task;
    priorityTaskQueue->end++;
}

Task *dequeuePriorityTaskQueue(PriorityTaskQueue *priorityTaskQueue)
{
    int indexMaxPriority = 0;
    int maxPriority = priorityTaskQueue->priority(priorityTaskQueue->tasks[indexMaxPriority]->args);
    for (size_t i = 1; i < priorityTaskQueue->end; i++)
    {
        int priority = priorityTaskQueue->priority(priorityTaskQueue->tasks[i]->args);
        if (priority > maxPriority)
        {
            indexMaxPriority = i;
            maxPriority = priority;
        }
    }
    Task *task = priorityTaskQueue->tasks[indexMaxPriority];
    priorityTaskQueue->tasks[indexMaxPriority] = priorityTaskQueue->tasks[priorityTaskQueue->end - 1];
    priorityTaskQueue->end--;
    return task;
}

int isPriorityTaskQueueEmpty(PriorityTaskQueue *priorityTaskQueue)
{
    return priorityTaskQueue->end == 0;
}

// Thread Pool function

void createThreadPool(ThreadPool *threadPool, TaskQueue *taskQueue, int (*priority)(void *), int numThreads, int queueSize)
{
    threadPool->taskQueue = taskQueue;
    threadPool->numThreads = numThreads;
    threadPool->open = 1;
    threadPool->priorityTaskQueue = (PriorityTaskQueue *)malloc(sizeof(PriorityTaskQueue));
    createPriorityTaskQueue(threadPool->priorityTaskQueue, priority, queueSize);
    pthread_create(&threadPool->masterThread, NULL, master, threadPool);
}

void *master(void *args)
{
    ThreadPool *threadPool = (ThreadPool *)args;

    NameSpace *nameSpace = (NameSpace *)malloc(sizeof(NameSpace) * threadPool->numThreads);
    for (size_t i = 0; i < threadPool->numThreads; i++)
    {
        nameSpace[i].id = i;
        nameSpace[i].threadPool = threadPool;
        nameSpace[i].task = &NULL_TASK;
        nameSpace[i].permission_to_close = 0;
    }

    threadPool->workerThreads = (pthread_t *)malloc(sizeof(pthread_t) * threadPool->numThreads);
    for (size_t i = 0; i < threadPool->numThreads; i++)
        pthread_create(&threadPool->workerThreads[i], NULL, worker, &nameSpace[i]);

    while (threadPool->open || !isTaskQueueEmpty(threadPool->taskQueue) || !isPriorityTaskQueueEmpty(threadPool->priorityTaskQueue))
    {
        if (!isTaskQueueEmpty(threadPool->taskQueue))
        {
            Task *task = dequeueTaskQueue(threadPool->taskQueue);
            enqueuePriorityTaskQueue(threadPool->priorityTaskQueue, task);
        }
        if (!isPriorityTaskQueueEmpty(threadPool->priorityTaskQueue))
        {
            for (size_t i = 0; i < threadPool->numThreads; i++)
            {
                if (nameSpace[i].task == &NULL_TASK)
                {
                    nameSpace[i].task = dequeuePriorityTaskQueue(threadPool->priorityTaskQueue);
                    break;
                }
            }
        }
    }

    for (size_t i = 0; i < threadPool->numThreads; i++)
        nameSpace[i].permission_to_close = 1;

    return NULL;
}

void *worker(void *args)
{
    NameSpace *nameSpace = (NameSpace *)args;
    int num_exec = 0;
    while (!nameSpace->permission_to_close || nameSpace->task != &NULL_TASK)
    {
        if (nameSpace->task != &NULL_TASK)
        {
            nameSpace->task->worker(nameSpace->task->args);
            nameSpace->task = &NULL_TASK;
            num_exec++;
        }
    }
    return NULL;
}

void waitThreadPool(ThreadPool *threadPool)
{
    threadPool->open = 0;
    pthread_join(threadPool->masterThread, NULL);
    for (size_t i = 0; i < threadPool->numThreads; i++)
        pthread_join(threadPool->workerThreads[i], NULL);
}

// Ackermann functions

int ackermannPriority(void *args)
{
    AckermannArg *ackermannArgs = (AckermannArg *)args;
    return -1 * (ackermannArgs->m + ackermannArgs->n);
}

int ackermann(int m, int n)
{
    if (m == 0)
        return n + 1;
    else if (n == 0)
        return ackermann(m - 1, 1);
    else
        return ackermann(m - 1, ackermann(m, n - 1));
}

void *ackermannWorker(void *args)
{
    AckermannArg *ackermannArgs = (AckermannArg *)args;
    int result = ackermann(ackermannArgs->m, ackermannArgs->n);
    printf("A(%d, %d) = %d\n", ackermannArgs->m, ackermannArgs->n, result);
    return NULL;
}
