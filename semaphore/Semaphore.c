#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define THREAD_NUM 5

sem_t sem;  // 計數信號量

void* worker(void* arg) {
    int id = *(int*)arg;

    printf("Thread %d: Waiting to enter critical section...\n", id);

    // P 操作（等待 semaphore）
    sem_wait(&sem);

    // critical section
    printf("Thread %d: Entered critical section!\n", id);
    sleep(2);  // 模擬工作
    printf("Thread %d: Leaving critical section.\n", id);

    // V 操作（釋放 semaphore）
    sem_post(&sem);

    return NULL;
}

int main() {
    pthread_t threads[THREAD_NUM];
    int ids[THREAD_NUM];

    // 初始化 semaphore，初始值為 2 → 最多允許 2 個 thread 同時進入
    sem_init(&sem, 0, 2);

    for (int i = 0; i < THREAD_NUM; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, worker, &ids[i]);
    }

    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&sem);

    return 0;
}
