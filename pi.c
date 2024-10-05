#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

long incircle = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* pi(void* args) {
    long points = (long)args;
    long incircle_thread = 0;

    unsigned int rand_state = rand();
    for (long i = 0; i < points; i++) {
        double x = rand_r(&rand_state) / ((double)RAND_MAX);
        double y = rand_r(&rand_state) / ((double)RAND_MAX);
        if (x * x + y * y < 1) {
            incircle_thread++;
        }
    }

    pthread_mutex_lock(&mutex);
    incircle += incircle_thread;
    pthread_mutex_unlock(&mutex);
}

pthread_t* create_threads(long nthreads, long ntrials) {
    long points_per_thread = ntrials / nthreads;

    pthread_t* threads = malloc(nthreads * sizeof(pthread_t));
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&threads[0], &attr, pi,
                   (void*)(points_per_thread + ntrials % nthreads));

    for (long i = 1; i < nthreads; i++) {
        pthread_create(&threads[i], &attr, pi, (void*)points_per_thread);
    }
    return threads;
}

void join_threads(pthread_t* threads, long nthreads) {
    for (long i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }
    free(threads);
}

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: ./pi <total points> <threads>\n");
        exit(1);
    }

    srand(time(NULL));
    long nthreads = atoi(argv[1]);
    long ntrials = atol(argv[2]);

    double start, end;
    GET_TIME(start);

    pthread_t* threads = create_threads(nthreads, ntrials);
    join_threads(threads, nthreads);

    pthread_mutex_destroy(&mutex);
    GET_TIME(end);

    printf("%f ", (4. * (double)incircle) / ((double)ntrials));
    printf("%f\n", (end - start));

    return 0;
}