#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/delay.h>

#define BUFFER_SIZE 5

static int buffer[BUFFER_SIZE];
static int in = 0, out = 0;

static struct mutex buffer_lock;
static struct semaphore empty;
static struct semaphore full;

static struct task_struct *prod1, *prod2, *cons1;

/* Producer thread */
static int producer_fn(void *data)
{
    int item = 0;
    const char *name = data;

    while (!kthread_should_stop()) {
        item++;

        down(&empty);                 // 等空位
        mutex_lock(&buffer_lock);     // 進入臨界區

        buffer[in] = item;
        pr_info("%s produced %d (in=%d)\n", name, item, in);
        in = (in + 1) % BUFFER_SIZE;

        mutex_unlock(&buffer_lock);
        up(&full);                    // 通知 consumer

        msleep(1000);
    }
    return 0;
}

/* Consumer thread */
static int consumer_fn(void *data)
{
    int item;
    const char *name = data;

    while (!kthread_should_stop()) {

        down(&full);                  // 等資料
        mutex_lock(&buffer_lock);

        item = buffer[out];
        pr_info("%s consumed %d (out=%d)\n", name, item, out);
        out = (out + 1) % BUFFER_SIZE;

        mutex_unlock(&buffer_lock);
        up(&empty);                   // 釋放空位

        msleep(2000);
    }
    return 0;
}

static int __init pc_init(void)
{
    pr_info("Producer-Consumer module loaded\n");

    mutex_init(&buffer_lock);
    sema_init(&empty, BUFFER_SIZE);
    sema_init(&full, 0);

    prod1 = kthread_run(producer_fn, "P1", "producer1");
    prod2 = kthread_run(producer_fn, "P2", "producer2");
    cons1 = kthread_run(consumer_fn, "C1", "consumer1");

    return 0;
}

static void __exit pc_exit(void)
{
    pr_info("Stopping threads...\n");

    if (prod1) kthread_stop(prod1);
    if (prod2) kthread_stop(prod2);
    if (cons1) kthread_stop(cons1);

    pr_info("Producer-Consumer module unloaded\n");
}

module_init(pc_init);
module_exit(pc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Producer Consumer using semaphore + mutex");




