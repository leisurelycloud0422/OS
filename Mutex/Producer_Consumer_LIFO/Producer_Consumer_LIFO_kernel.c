#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/wait.h>

#define BUFFER_SIZE 5

static int buffer[BUFFER_SIZE];
static int front = 0;
static int rear  = 0;

static struct mutex lock;
static wait_queue_head_t not_full;
static wait_queue_head_t not_empty;

static struct task_struct *producer_task;
static struct task_struct *consumer_task;

/* 判斷是否空或滿 */
static int is_empty(void)
{
    return front == rear;
}

static int is_full(void)
{
    return (rear + 1) % BUFFER_SIZE == front;
}

/* Producer thread */
static int producer_fn(void *data)
{
    int i = 1;

    while (!kthread_should_stop() && i <= 10) {
        mutex_lock(&lock);

        /* buffer 滿就睡 */
        wait_event(not_full, !is_full());

        buffer[rear] = i;
        rear = (rear + 1) % BUFFER_SIZE;

        pr_info("[Producer] Produced: %d\n", i);
        i++;

        wake_up(&not_empty);
        mutex_unlock(&lock);

        msleep(1000);
    }
    return 0;
}

/* Consumer thread */
static int consumer_fn(void *data)
{
    int item;

    while (!kthread_should_stop()) {
        mutex_lock(&lock);

        /* buffer 空就睡 */
        wait_event(not_empty, !is_empty());

        item = buffer[front];
        front = (front + 1) % BUFFER_SIZE;

        pr_info("[Consumer] Consumed: %d\n", item);

        wake_up(&not_full);
        mutex_unlock(&lock);

        msleep(2000);
    }
    return 0;
}

/* Module init */
static int __init pc_init(void)
{
    pr_info("Producer-Consumer kernel module loaded\n");

    mutex_init(&lock);
    init_waitqueue_head(&not_full);
    init_waitqueue_head(&not_empty);

    producer_task = kthread_run(producer_fn, NULL, "pc_producer");
    consumer_task = kthread_run(consumer_fn, NULL, "pc_consumer");

    if (IS_ERR(producer_task) || IS_ERR(consumer_task)) {
        pr_err("Failed to create kthreads\n");
        return -ENOMEM;
    }

    return 0;
}

/* Module exit */
static void __exit pc_exit(void)
{
    if (producer_task)
        kthread_stop(producer_task);
    if (consumer_task)
        kthread_stop(consumer_task);

    pr_info("Producer-Consumer kernel module unloaded\n");
}

module_init(pc_init);
module_exit(pc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Kernel Producer-Consumer Example");

