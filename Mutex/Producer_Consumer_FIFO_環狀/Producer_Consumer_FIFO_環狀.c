#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
int buffer[BUFFER_SIZE];
int front = 0;  // 消費者取資料位置
int rear = 0;   // 生產者放資料位置

pthread_mutex_t lock;
pthread_cond_t not_full, not_empty;

// 判斷是否空或滿
int isEmpty() {
    return front == rear;
}

int isFull() {
    return (rear + 1) % BUFFER_SIZE == front;
}

void* producer(void* arg) {
    for (int i = 1; i <= 10; i++) {
        pthread_mutex_lock(&lock);

        while (isFull())
            pthread_cond_wait(&not_full, &lock);

        buffer[rear] = i;
        rear = (rear + 1) % BUFFER_SIZE;
        printf("Produced: %d\n", i);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&lock);

        sleep(1);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 1; i <= 10; i++) {
        pthread_mutex_lock(&lock);

        while (isEmpty())
            pthread_cond_wait(&not_empty, &lock);

        int item = buffer[front];
        front = (front + 1) % BUFFER_SIZE;
        printf("Consumed: %d\n", item);

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&lock);

        sleep(2);
    }
    return NULL;
}

int main() {
    pthread_t prod, cons;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);

    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    return 0;
}
