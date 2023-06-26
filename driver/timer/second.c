#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define SECOND_MAJOR 0

static int second_major = SECOND_MAJOR;

struct second {
    struct cdev cdev;
    atomic_t counter;
    struct timer_list s_timer;
};

struct second *second_devp;

static void second_timer_handle(struct timer_list *t)
{
    mod_timer(&second_devp->s_timer, jiffies + HZ);
    atomic_inc(&second_devp->counter);
    printk(KERN_NOTICE "counter jiffies is %ld\n", jiffies);
}

int second_open(struct inode *inode, struct file *filp)
{
    timer_setup(&second_devp->s_timer, second_timer_handle, 0);
    second_devp->s_timer.expires = jiffies + HZ;

    add_timer(&second_devp->s_timer);

    atomic_set(&second_devp->counter, 0);

    return 0;
}

int second_release(struct inode *inode, struct file *filp)
{
    del_timer(&second_devp->s_timer);

    return 0;
}

static ssize_t second_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int counter;

    counter = atomic_read(&second_devp->counter);
    if (put_user(counter, (int *)buf)) {
        return -EFAULT;
    } else {
        return sizeof(unsigned int);
    }
}

static const struct file_operations second_fops = {
    .owner = THIS_MODULE,
    .open = second_open,
    .release = second_release,
    .read = second_read,
};

static void second_setup_cdev(struct second *dev, int index)
{
    int err;
    int devno = MKDEV(second_major, index);

    cdev_init(&dev->cdev, &second_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &second_fops;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding second%d", err, index);
}

int second_init(void)
{
    int ret;
    dev_t devno = MKDEV(second_major, 0);

    if (second_major) {
        ret = register_chrdev_region(devno, 1, "second");
    } else {
        ret = alloc_chrdev_region(&devno, 0, 1, "second");
        second_major = MAJOR(devno);
    }

    if (ret < 0)
        return ret;

    second_devp = kmalloc(sizeof(struct second), GFP_KERNEL);
    if (!second_devp) {
        ret = -ENOMEM;
        goto fail_malloc;
    }

    memset(second_devp, 0, sizeof(struct second));
    second_setup_cdev(second_devp, 0);
    return 0;

fail_malloc:
    unregister_chrdev_region(devno, 1);
    return 0;
}

void second_exit(void)
{
    cdev_del(&second_devp->cdev);
    kfree(second_devp);
    unregister_chrdev_region(MKDEV(second_major, 0), 1);
}

MODULE_AUTHOR("HuangLiang");
MODULE_LICENSE("Dual BSD/GPL");

module_param(second_major, int, S_IRUGO);
module_init(second_init);
module_exit(second_exit);
