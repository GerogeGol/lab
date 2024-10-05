#ifndef MY_RWLOCK
#define MY_RWLOCK

#include <pthread.h>
#include <stdbool.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t readers;
    pthread_cond_t writers;
    int readers_count;
    int writers_waiting;
    int writer_active;
} rwlock_t;

void rwlock_init(rwlock_t* lock) {
    pthread_mutex_init(&lock->mutex, NULL);
    pthread_cond_init(&lock->readers, NULL);
    pthread_cond_init(&lock->writers, NULL);
    lock->readers_count = 0;
    lock->writers_waiting = 0;
    lock->writer_active = 0;
}

void rwlock_rdlock(rwlock_t* lock) {
    pthread_mutex_lock(&lock->mutex);
    while (lock->writer_active || lock->writers_waiting > 0) {
        pthread_cond_wait(&lock->readers, &lock->mutex);
    }
    lock->readers_count++;
    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_wrlock(rwlock_t* lock) {
    pthread_mutex_lock(&lock->mutex);
    lock->writers_waiting++;
    while (lock->writer_active || lock->readers_count > 0) {
        pthread_cond_wait(&lock->writers, &lock->mutex);
    }
    lock->writer_active = 1;
    lock->writers_waiting--;
    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_unlock(rwlock_t* lock) {
    pthread_mutex_lock(&lock->mutex);
    if (lock->writer_active) {
        lock->writer_active = 0;
        pthread_cond_broadcast(&lock->readers);
        pthread_cond_signal(&lock->writers);
    } else {
        lock->readers_count--;
        if (lock->readers_count == 0) {
            pthread_cond_signal(&lock->writers);
        }
    }
    pthread_mutex_unlock(&lock->mutex);
}

void rwlock_destroy(rwlock_t* lock) {
    pthread_mutex_destroy(&lock->mutex);
    pthread_cond_destroy(&lock->readers);
    pthread_cond_destroy(&lock->writers);
}
#endif