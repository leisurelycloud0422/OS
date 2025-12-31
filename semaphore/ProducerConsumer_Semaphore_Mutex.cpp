#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore>
#include <chrono>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int in = 0, out = 0;

std::mutex mtx;                       // 保護臨界區
std::counting_semaphore<BUFFER_SIZE> empty(BUFFER_SIZE); // 可放入的空格數
std::counting_semaphore<BUFFER_SIZE> full(0);            // 可取出的滿格數

void producer(int id)
{
    int item = 0;
    while (true)
    {
        item++;

        empty.acquire();         // 等有空位
        {
            std::lock_guard<std::mutex> lock(mtx);
            buffer[in] = item;
            std::cout << "Producer " << id << " -> item " << item << " (in=" << in << ")\n";
            in = (in + 1) % BUFFER_SIZE;
        }
        full.release();          // 增加可取的數量

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void consumer(int id)
{
    int item;
    while (true)
    {
        full.acquire();          // 等有滿格
        {
            std::lock_guard<std::mutex> lock(mtx);
            item = buffer[out];
            std::cout << "Consumer " << id << " <- item " << item << " (out=" << out << ")\n";
            out = (out + 1) % BUFFER_SIZE;
        }
        empty.release();         // 增加空位

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main()
{
    std::thread p1(producer, 1);
    std::thread p2(producer, 2);
    std::thread c1(consumer, 1);

    p1.join();
    p2.join();
    c1.join();

    return 0;
}
