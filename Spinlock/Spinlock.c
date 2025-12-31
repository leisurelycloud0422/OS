#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#define IRQ_NUM 1    // 假設虛擬 IRQ

static spinlock_t lock;     // 保護共享資源
static unsigned long flags; // 保存中斷狀態

static int shared_counter = 0; // 模擬共享資源

/* 模擬中斷服務程序（ISR） */
static irqreturn_t example_isr(int irq, void *dev_id)
{
    spin_lock_irqsave(&lock, flags); // 禁用本 CPU 中斷並上鎖

    shared_counter++;
    pr_info("ISR: shared_counter=%d\n", shared_counter);

    spin_unlock_irqrestore(&lock, flags); // 恢復中斷狀態並解鎖
    return IRQ_HANDLED;
}

/* 模擬定時器來觸發 "中斷" */
static struct timer_list example_timer;

static void timer_callback(struct timer_list *t)
{
    pr_info("Timer callback triggering fake IRQ\n");
    example_isr(IRQ_NUM, NULL);

    mod_timer(&example_timer, jiffies + msecs_to_jiffies(1000)); // 1 秒後再次觸發
}

static int __init spinlock_irq_init(void)
{
    pr_info("Spinlock + IRQ example init\n");

    spin_lock_init(&lock);

    timer_setup(&example_timer, timer_callback, 0);
    mod_timer(&example_timer, jiffies + msecs_to_jiffies(1000));

    return 0;
}

static void __exit spinlock_irq_exit(void)
{
    del_timer_sync(&example_timer);
    pr_info("Spinlock + IRQ example exit\n");
}

module_init(spinlock_irq_init);
module_exit(spinlock_irq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("BMC Example");
MODULE_DESCRIPTION("Spinlock in ISR example for BMC kernel driver");

