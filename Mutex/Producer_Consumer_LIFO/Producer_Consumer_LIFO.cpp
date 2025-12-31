#include<iostream>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<chrono>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int front =0;// consumer index
int rear  =0;// producer index

std::mutex lock;
std::condition_variable not_full;
std::condition_variable not_empty;

// 判斷是否空或滿
boolisEmpty() {
return front == rear;
}

boolisFull() {
return (rear +1) % BUFFER_SIZE == front;
}

voidproducer() {
for (int i =1; i <=10; i++) {
std::unique_lock<std::mutex> lk(lock);

while (isFull()) {
            not_full.wait(lk);
        }

        buffer[rear] = i;
        rear = (rear +1) % BUFFER_SIZE;
        std::cout <<"Produced: " << i << std::endl;

        not_empty.notify_one();
        lk.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

voidconsumer() {
for (int i =1; i <=10; i++) {
std::unique_lock<std::mutex> lk(lock);

while (isEmpty()) {
            not_empty.wait(lk);
        }

int item = buffer[front];
        front = (front +1) % BUFFER_SIZE;
        std::cout <<"Consumed: " << item << std::endl;

        not_full.notify_one();
        lk.unlock();

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

intmain() {
std::thread prod(producer);
std::thread cons(consumer);

    prod.join();
    cons.join();

return0;
}

