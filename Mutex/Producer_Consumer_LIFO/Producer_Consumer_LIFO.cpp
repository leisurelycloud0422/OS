#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#define STACK_SIZE 5

int stack[STACK_SIZE];
int top = 0;   // 指向下一個可放位置（元素數量）

std::mutex mtx;
std::condition_variable not_full;
std::condition_variable not_empty;

// 判斷 stack 是否空或滿
bool isEmpty() { return top == 0; }
bool isFull() { return top == STACK_SIZE; }

// Producer：push
void producer()
{
    int item = 0;
    while (true)
    {
        item++;

        std::unique_lock<std::mutex> lock(mtx);
        not_full.wait(lock, [] { return !isFull(); });

        stack[top++] = item;
        std::cout << "Produced: " << item << " (top=" << top << ")" << std::endl;

        lock.unlock();
        not_empty.notify_one();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Consumer：pop
void consumer()
{
    int item;
    while (true)
    {
        std::unique_lock<std::mutex> lock(mtx);
        not_empty.wait(lock, [] { return !isEmpty(); });

        item = stack[--top];
        std::cout << "Consumed: " << item << " (top=" << top << ")" << std::endl;

        lock.unlock();
        not_full.notify_one();

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main()
{
    std::thread prod(producer);
    std::thread cons(consumer);

    prod.join();
    cons.join();

    return 0;
}



