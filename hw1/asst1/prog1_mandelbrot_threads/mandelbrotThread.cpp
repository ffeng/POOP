#include <stdio.h>
#include <pthread.h>

#include "CycleTimer.h"

typedef struct {
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
} WorkerArgs;


extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);


//
// workerThreadStart --
//
// Thread entrypoint.
void* workerThreadStart(void* threadArgs) {
    double start = CycleTimer::currentSeconds();
    double end;
    WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);

    printf("Hello world from thread %d height: %d \n", args->threadId, 
                                                       args->height);
    
    /*
    int srow = (args->height)/3*(args->threadId);
    if (args->threadId == 1) {
        rows = 133;
    }    

    if (args->threadId == 2) {
        srow = (args->height)/3 + 133;
        rows = 133;
    }

    if (args->threadId == 3) {
        srow = (args->height)/3*((args->threadId)-1);
        rows = 268;
    }
    */
    for (int i = args->threadId; i<(args->height); i = i+4) {
    mandelbrotSerial(args->x0, args->y0, args->x1, args->y1,
                     args->width, args->height,
                     i, 1,
                     args->maxIterations,
                     args->output);
    }
    end = CycleTimer::currentSeconds();
    printf("Thread %d Time:  %f\n",args->threadId,end-start);
    return NULL;
}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Multi-threading performed via pthreads.
void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    const static int MAX_THREADS = 32;
    numThreads = 4;

    if (numThreads > MAX_THREADS)
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    pthread_t workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    for (int i=0; i<numThreads; i++) {
        args[i].threadId = i;
        args[i].x0 = x0;
        args[i].y0 = y0;
        args[i].x1 = x1;
        args[i].y1 = y1;
        args[i].width = width;
        args[i].height = height;
        args[i].maxIterations = maxIterations;
        args[i].output = output;
    }

    //2 threads: 1.90x, 1.71x
    //3 threads: 1.66x, 1.73x
    //4 threads: 3.30x, 2.71x
    //4 threads: 3.50x, 3.31x
    // Fire up the worker threads.  Note that numThreads-1 pthreads
    // are created and the main app thread is used as a worker as
    // well.

    for (int i=1; i<numThreads; i++)
        pthread_create(&workers[i], NULL, workerThreadStart, &args[i]);

    //workerThreadStart(&args[0]);
    printf("Hello world from thread %d height: %d \n", args->threadId,
                                                       args->height);
    double start = CycleTimer::currentSeconds();
    double end;
    for (int i = 0; i<(args->height); i = i+4) {
    mandelbrotSerial(args->x0, args->y0, args->x1, args->y1,
                     args->width, args->height,
                     i, 1,
                     args->maxIterations,
                     args->output);
    }
    end = CycleTimer::currentSeconds();
    printf("Thread %d Time:  %f\n",args->threadId,end-start);

    // wait for worker threads to complete
    for (int i=1; i<numThreads; i++)
        pthread_join(workers[i], NULL);
}
