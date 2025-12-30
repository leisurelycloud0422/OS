#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

constexpr int BUFFER_SIZE = 5;
int buffer[BUFFER_SIZE];
int count = 0;   // 當前 buffer 中元素數量
int front = 0;   // 消費者取資料位置
int rear  = 0;   // 生產者放資料位置

std::mutex mtx;
std::condition_variable not_full;
std::condition_variable not_empty;

void producer() {
    for (int i = 1; i <= 10; ++i) {
        std::unique_lock<std::mutex> lock(mtx);

        // buffer 滿了就等待
        not_full.wait(lock, [] {
            return count < BUFFER_SIZE;
        });

        // 放入資料
        buffer[rear] = i;
        rear = (rear + 1) % BUFFER_SIZE;
        ++count;

        std::cout << "Produced: " << i
                  << " (count=" << count << ")\n";

        // 通知消費者
        not_empty.notify_one();

        lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void consumer() {
    for (int i = 1; i <= 10; ++i) {
        std::unique_lock<std::mutex> lock(mtx);

        // buffer 空了就等待
        not_empty.wait(lock, [] {
            return count > 0;
        });

        // 取出資料
        int item = buffer[front];
        front = (front + 1) % BUFFER_SIZE;
        --count;

        std::cout << "Consumed: " << item
                  << " (count=" << count << ")\n";

        // 通知生產者
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
