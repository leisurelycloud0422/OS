#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/kthread.h>
#include<linux/mutex.h>
#include<linux/wait.h>
#include<linux/delay.h>

#define BUFFER_SIZE 5

staticint buffer[BUFFER_SIZE];
staticint front =0;
staticint rear  =0;

staticstructmutexlock;
staticwait_queue_head_t not_full;
staticwait_queue_head_t not_empty;

staticstructtask_struct *producer_thread;
staticstructtask_struct *consumer_thread;

/* 判斷空 / 滿（與 user space 相同邏輯） */
staticboolis_empty(void)
{
return front == rear;
}

staticboolis_full(void)
{
return (rear +1) % BUFFER_SIZE == front;
}

/* Producer thread */
staticintproducer_fn(void *data)
{
int item =1;

while (!kthread_should_stop() && item <=10) {
        mutex_lock(&lock);

while (is_full()) {
            mutex_unlock(&lock);
            wait_event(not_full, !is_full());
            mutex_lock(&lock);
        }

        buffer[rear] = item;
        rear = (rear +1) % BUFFER_SIZE;

        pr_info("[Producer] produced %d\n", item);

        mutex_unlock(&lock);
        wake_up(&not_empty);

        item++;
        msleep(1000);
    }

return0;
}

/* Consumer thread */
staticintconsumer_fn(void *data)
{
int item;

while (!kthread_should_stop()) {
        mutex_lock(&lock);

while (is_empty()) {
            mutex_unlock(&lock);
            wait_event(not_empty, !is_empty());
            mutex_lock(&lock);
        }

        item = buffer[front];
        front = (front +1) % BUFFER_SIZE;

        pr_info("[Consumer] consumed %d\n", item);

        mutex_unlock(&lock);
        wake_up(&not_full);

        msleep(2000);
    }

return0;
}

/* module init */
staticint __initpc_init(void)
{
    pr_info("Producer-Consumer kernel module init\n");

    mutex_init(&lock);
    init_waitqueue_head(&not_full);
    init_waitqueue_head(&not_empty);

    producer_thread = kthread_run(producer_fn,NULL,"producer_kthread");
    consumer_thread = kthread_run(consumer_fn,NULL,"consumer_kthread");

return0;
}

/* module exit */
staticvoid __exitpc_exit(void)
{
if (producer_thread)
        kthread_stop(producer_thread);

if (consumer_thread)
        kthread_stop(consumer_thread);

    pr_info("Producer-Consumer kernel module exit\n");
}

module_init(pc_init);
module_exit(pc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BMC learner");
MODULE_DESCRIPTION("Producer Consumer using mutex + wait queue");

