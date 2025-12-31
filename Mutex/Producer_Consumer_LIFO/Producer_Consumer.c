#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define STACK_SIZE 5

int stack[STACK_SIZE];
int top = 0;   // 指向下一個可放位置（元素數量）

pthread_mutex_t lock;
pthread_cond_t not_full;
pthread_cond_t not_empty;

/* 判斷 stack 是否空或滿 */
int isEmpty(void)
{
    return top == 0;
}

int isFull(void)
{
    return top == STACK_SIZE;
}

/* Producer：push */
void* producer(void* arg)
{
    int item = 0;

    while (1)
    {
        item++;

        pthread_mutex_lock(&lock);

        while (isFull())
            pthread_cond_wait(&not_full, &lock);

        stack[top++] = item;   // push
        printf("Produced: %d (top=%d)\n", item, top);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&lock);

        sleep(1);
    }
    return NULL;
}

/* Consumer：pop */
void* consumer(void* arg)
{
    int item;

    while (1)
    {
        pthread_mutex_lock(&lock);

        while (isEmpty())
            pthread_cond_wait(&not_empty, &lock);

        item = stack[--top];   // pop
        printf("Consumed: %d (top=%d)\n", item, top);

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&lock);

        sleep(2);
    }
    return NULL;
}

int main(void)
{
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

