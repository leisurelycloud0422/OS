#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5
int buffer[BUFFER_SIZE];
int count = 0;       // 當前 buffer 中元素數量
int front = 0;       // 消費者取資料位置
int rear = 0;        // 生產者放資料位置

pthread_mutex_t lock;
pthread_cond_t not_full, not_empty;

void* producer(void* arg) {
    for (int i = 1; i <= 10; i++) {
        pthread_mutex_lock(&lock);

        // buffer 滿了就等待
        while (count == BUFFER_SIZE)
            pthread_cond_wait(&not_full, &lock);

        // 放入資料
        buffer[rear] = i;
        rear = (rear + 1) % BUFFER_SIZE;  // 循環
        count++;
        printf("Produced: %d (count=%d)\n", i, count);

        pthread_cond_signal(&not_empty);  // 喚醒消費者
        pthread_mutex_unlock(&lock);

        sleep(1);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 1; i <= 10; i++) {
        pthread_mutex_lock(&lock);

        // buffer 空了就等待
        while (count == 0)
            pthread_cond_wait(&not_empty, &lock);

        // 取出資料
        int item = buffer[front];
        front = (front + 1) % BUFFER_SIZE;  // 循環
        count--;
        printf("Consumed: %d (count=%d)\n", item, count);

        pthread_cond_signal(&not_full);  // 喚醒生產者
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
