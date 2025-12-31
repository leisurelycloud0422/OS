#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int front = 0;  // consumer index
int rear  = 0;  // producer index

std::mutex mtx;
std::condition_variable not_full;
std::condition_variable not_empty;

// 判斷是否空或滿
bool isEmpty() {
    return front == rear;
}

bool isFull() {
    return (rear + 1) % BUFFER_SIZE == front;
}

void producer() {
    for (int i = 1; i <= 10; i++) {
        std::unique_lock<std::mutex> lock(mtx);

        not_full.wait(lock, [] {
            return !isFull();
        });

        buffer[rear] = i;
        rear = (rear + 1) % BUFFER_SIZE;
        std::cout << "Produced: " << i << std::endl;

        not_empty.notify_one();
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void consumer() {
    for (int i = 1; i <= 10; i++) {
        std::unique_lock<std::mutex> lock(mtx);

        not_empty.wait(lock, [] {
            return !isEmpty();
        });

        int item = buffer[front];
        front = (front + 1) % BUFFER_SIZE;
        std::cout << "Consumed: " << item << std::endl;

        not_full.notify_one();
        lock.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main() {
    std::thread prod(producer);
    std::thread cons(consumer);

    prod.join();
    cons.join();

    return 0;
}
