#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int in = 0, out = 0;

pthread_mutex_t mutex;   // 保護臨界區
sem_t empty;             // 可放入的空格數
sem_t full;              // 可取出的滿格數

void* producer(void* arg)
{
    int item = 0;
    while (1)
    {
        item++; // 產生新資料

        sem_wait(&empty);           // 等有空位
        pthread_mutex_lock(&mutex); // 鎖住 buffer

        buffer[in] = item;
        printf("Producer %lu -> item %d (in=%d)\n", pthread_self(), item, in);
        in = (in + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex); // 解鎖 buffer
        sem_post(&full);              // 增加可取的數量
        sleep(1);
    }
}

void* consumer(void* arg)
{
    int item;
    while (1)
    {
        sem_wait(&full);            // 等有滿格
        pthread_mutex_lock(&mutex); // 鎖住 buffer

        item = buffer[out];
        printf("Consumer %lu <- item %d (out=%d)\n", pthread_self(), item, out);
        out = (out + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex); // 解鎖 buffer
        sem_post(&empty);             // 增加空位
        sleep(2);
    }
}

int main()
{
    pthread_t p1, p2, c1;

    pthread_mutex_init(&mutex, NULL);
    sem_init(&empty, 0, BUFFER_SIZE); // 一開始全是空位
    sem_init(&full, 0, 0);            // 一開始沒有滿格

    pthread_create(&p1, NULL, producer, NULL);
    pthread_create(&p2, NULL, producer, NULL);
    pthread_create(&c1, NULL, consumer, NULL);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(c1, NULL);

    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
}
