// lifo_pc_kmod.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/wait.h>

#define STACK_SIZE 5

static int stack[STACK_SIZE];
static int top = 0;

static struct mutex lock;
static wait_queue_head_t not_full;
static wait_queue_head_t not_empty;

static struct task_struct *producer_thread;
static struct task_struct *consumer_thread;

/* 判斷 stack 是否空或滿 */
static inline int isEmpty(void) { return top == 0; }
static inline int isFull(void) { return top == STACK_SIZE; }

/* Producer thread */
static int producer_fn(void *data)
{
    int item = 0;

    while (!kthread_should_stop())
    {
        item++;

        mutex_lock(&lock);
        while (isFull())
        {
            mutex_unlock(&lock);
            wait_event_interruptible(not_full, !isFull());
            mutex_lock(&lock);
        }

        stack[top++] = item;  // push
        pr_info("Produced: %d (top=%d)\n", item, top);

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

    while (!kthread_should_stop())
    {
        mutex_lock(&lock);
        while (isEmpty())
        {
            mutex_unlock(&lock);
            wait_event_interruptible(not_empty, !isEmpty());
            mutex_lock(&lock);
        }

        item = stack[--top];  // pop
        pr_info("Consumed: %d (top=%d)\n", item, top);

        wake_up(&not_full);
        mutex_unlock(&lock);

        msleep(2000);
    }

    return 0;
}

static int __init lifo_pc_init(void)
{
    pr_info("LIFO Producer-Consumer kernel module init\n");

    mutex_init(&lock);
    init_waitqueue_head(&not_full);
    init_waitqueue_head(&not_empty);

    producer_thread = kthread_run(producer_fn, NULL, "producer_thread");
    if (IS_ERR(producer_thread))
        pr_err("Failed to create producer thread\n");

    consumer_thread = kthread_run(consumer_fn, NULL, "consumer_thread");
    if (IS_ERR(consumer_thread))
        pr_err("Failed to create consumer thread\n");

    return 0;
}

static void __exit lifo_pc_exit(void)
{
    if (producer_thread)
        kthread_stop(producer_thread);
    if (consumer_thread)
        kthread_stop(consumer_thread);

    pr_info("LIFO Producer-Consumer kernel module exit\n");
}

module_init(lifo_pc_init);
module_exit(lifo_pc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BMC Example");
MODULE_DESCRIPTION("LIFO Producer-Consumer kernel module example");


