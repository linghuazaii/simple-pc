#include <pthread.h>
#include <iostream>
#include <string>
#include <queue>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
using namespace std;

#define CONSUMER_COUNT 2
#define PRODUCER_COUNT 1
#define QUEUE_SIZE 100

typedef struct pool_t {
    pthread_mutex_t mutex;
    pthread_cond_t consumer_notify;
    pthread_cond_t producer_notify;
    queue<string> message;
    int queue_size;
    int count;
    pthread_t producer_thread[PRODUCER_COUNT];
    pthread_t consumer_thread[CONSUMER_COUNT];
} pool_t;

pool_t *pool_init() {
    pool_t *pool = new pool_t;
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->consumer_notify, NULL);
    pthread_cond_init(&pool->producer_notify, NULL);
    pool->queue_size = QUEUE_SIZE;
    pool->count = 0;
}

void destroy_pool(pool_t *pool) {
    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->consumer_notify);
    pthread_cond_destroy(&pool->producer_notify);
}

void *produce(void *arg) {
    pool_t *pool = (pool_t *)arg;
    char buffer[128];
    pthread_t thread = pthread_self();
    char file_name[128];
    sprintf(file_name, "P-%lu", thread);
    FILE *file = fopen(file_name, "w+");
    assert(file != NULL);
    for (;;) {
        pthread_mutex_lock(&pool->mutex);
        while (pool->count == pool->queue_size)
            pthread_cond_wait(&pool->producer_notify, &pool->mutex);
        ++pool->count;
        sprintf(buffer, "P->%d\n",pool->count);
        fwrite(buffer, strlen(buffer), 1, file);
        pool->message.push(string(buffer));
        pthread_cond_signal(&pool->consumer_notify);
        pthread_mutex_unlock(&pool->mutex);
    }
}

void start_producer(pool_t *pool) {
    for (int i = 0; i < PRODUCER_COUNT; ++i) {
        pthread_create(&pool->producer_thread[i], NULL, produce, (void *)pool);
    }
}

void stop_producer(pool_t *pool) {

}

void *consume(void *arg) {
    pool_t *pool = (pool_t *)arg;
    char buffer[128];
    pthread_t thread = pthread_self();
    char file_name[128];
    sprintf(file_name, "C-%lu", thread);
    FILE *file = fopen(file_name, "w+");
    assert(file != NULL);
    for (;;) {
        pthread_mutex_lock(&pool->mutex);
        while (pool->count == 0)
            pthread_cond_wait(&pool->consumer_notify, &pool->mutex);
        --pool->count;
        string msg = pool->message.front();
        fwrite(msg.c_str(), msg.length(), 1, file);
        pool->message.pop();
        pthread_cond_signal(&pool->producer_notify);
        pthread_mutex_unlock(&pool->mutex);
    }
}

void start_consumer(pool_t *pool) {
    for (int i = 0; i < CONSUMER_COUNT; ++i) {
        pthread_create(&pool->consumer_thread[i], NULL, consume, (void *)pool);
    }
}

void stop_consumer(pool_t * pool) {

}
