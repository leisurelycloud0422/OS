#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/mutex.h>
#include<linux/wait.h>
#include<linux/kthread.h>
#include<linux/delay.h>

#define BUFFER_SIZE 5

staticint buffer[BUFFER_SIZE];
staticint count =0;
staticint front =0;
staticint rear =0;

staticstructmutexlock;
staticwait_queue_head_t not_full;
staticwait_queue_head_t not_empty;

staticstructtask_struct *producer_thread;
staticstructtask_struct *consumer_thread;

/* ================= Producer ================= */
staticintproducer_fn(void *data)
{
int item =1;

while (!kthread_should_stop() && item <=10) {

        mutex_lock(&lock);
while (count == BUFFER_SIZE) {
            mutex_unlock(&lock);
            wait_event_interruptible(not_full, count < BUFFER_SIZE);
            mutex_lock(&lock);
        }

        buffer[rear] = item;
        rear = (rear +1) % BUFFER_SIZE;
        count++;

        pr_info("[Producer] produced %d (count=%d)\n", item, count);

        wake_up(&not_empty);
        mutex_unlock(&lock);

        item++;
        msleep(1000);
    }

return0;
}

/* ================= Consumer ================= */
staticintconsumer_fn(void *data)
{
int item;

while (!kthread_should_stop()) {

        mutex_lock(&lock);
while (count ==0) {
            mutex_unlock(&lock);
            wait_event_interruptible(not_empty, count >0);
            mutex_lock(&lock);
        }

        item = buffer[front];
        front = (front +1) % BUFFER_SIZE;
        count--;

        pr_info("[Consumer] consumed %d (count=%d)\n", item, count);

        wake_up(&not_full);
        mutex_unlock(&lock);

        msleep(2000);
    }

return0;
}

/* ================= Module init ================= */
staticint __initpc_init(void)
{
    pr_info("Producer-Consumer module init\n");

    mutex_init(&lock);
    init_waitqueue_head(&not_full);
    init_waitqueue_head(&not_empty);

    producer_thread = kthread_run(producer_fn,NULL,"pc_producer");
    consumer_thread = kthread_run(consumer_fn,NULL,"pc_consumer");

if (IS_ERR(producer_thread) || IS_ERR(consumer_thread)) {
        pr_err("Failed to create threads\n");
return -ENOMEM;
    }

return0;
}

/* ================= Module exit ================= */
staticvoid __exitpc_exit(void)
{
if (producer_thread)
        kthread_stop(producer_thread);
if (consumer_thread)
        kthread_stop(consumer_thread);

    pr_info("Producer-Consumer module exit\n");
}

module_init(pc_init);
module_exit(pc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Kernel Producer Consumer Demo");
