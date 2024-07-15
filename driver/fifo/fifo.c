#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/poll.h>

#define GLOBALMEM_SIZE 0x1000
#define MEM_CLEAR 0x1
#define GLOBALMEM_MAJOR 0
#define DEVICE_NUM      2

static int globalmem_major = GLOBALMEM_MAJOR;

struct globalmem_dev {
    struct cdev cdev;
    unsigned int current_len;
    unsigned char mem[GLOBALMEM_SIZE];
    struct semaphore sem;
    wait_queue_head_t r_wait;
    wait_queue_head_t w_wait;
    struct fasync_struct *async_queue;
};

struct globalmem_dev *globalmem_devp;

static int globalmem_fasync(int fd, struct file *filp, int mode)
{
    struct globalmem_dev *dev = filp->private_data;

    return fasync_helper(fd, filp, mode, &dev->async_queue);
}

int globalmem_release(struct inode *inode, struct file *filp)
{
    struct globalmem_dev *dev = filp->private_data;

    globalmem_fasync(-1, filp, 0);

    return 0;
}

static unsigned int globalmem_poll(struct file *filp, poll_table *wait)
{
    unsigned int mask = 0;
    struct globalmem_dev *dev = filp->private_data;

    down(&dev->sem);
    poll_wait(filp, &dev->r_wait, wait);
    poll_wait(filp, &dev->w_wait, wait);

    if (dev->current_len != 0) {
        mask |= POLLIN | POLLRDNORM;
    }

    if (dev->current_len != GLOBALMEM_SIZE) {
        mask |= POLLOUT | POLLWRNORM;
    }

    up(&dev->sem);
    return mask;
}

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    struct globalmem_dev *dev;
    DECLARE_WAITQUEUE(wait, current);

    dev = filp->private_data;

    down(&dev->sem);
    add_wait_queue(&dev->r_wait, &wait);

    if (dev->current_len == 0) {
        if (filp->f_flags & O_NONBLOCK) {
            ret = -EAGAIN;
            goto out;
        }

        __set_current_state(TASK_INTERRUPTIBLE);
        up(&dev->sem);

        schedule();
        if (signal_pending(current)) {
            ret = -ERESTARTSYS;
            goto out2;
        }

        down(&dev->sem);
    }

    if (count > dev->current_len) {
        count = dev->current_len;
    }

    if (copy_to_user(buf, (void *)(dev->mem), count)) {
        ret = - EFAULT;
        goto out2;
    } else {
        memcpy(dev->mem, dev->mem + count, dev->current_len - count);
        dev->current_len -= count;
        printk(KERN_INFO "read %ld byte(s), current_len: %ud\n", count, dev->current_len);
        wake_up_interruptible(&dev->w_wait);

        if (dev->async_queue) {
            kill_fasync(&dev->async_queue, SIGIO, POLL_IN);    
        }

        ret = count;
    }

out:
    up(&dev->sem);
out2:
    remove_wait_queue(&dev->w_wait, &wait);

    set_current_state(TASK_RUNNING);
    return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    struct globalmem_dev *dev;
    DECLARE_WAITQUEUE(wait, current);

    dev = filp->private_data;

    down(&dev->sem);
    add_wait_queue(&dev->w_wait, &wait);

    if (dev->current_len == GLOBALMEM_SIZE) {
        if (filp->f_flags & O_NONBLOCK) {
            ret = -EAGAIN;
            goto out;
        }

        __set_current_state(TASK_INTERRUPTIBLE);
        up(&dev->sem);

        schedule();
        if (signal_pending(current)) {
            ret = -ERESTARTSYS;
            goto out2;
        }

        down(&dev->sem);
    }

    if (count > GLOBALMEM_SIZE - dev->current_len) {
        count = GLOBALMEM_SIZE - dev->current_len;
    }

    if (copy_from_user((void *)(dev->mem + dev->current_len), buf, count)) {
        ret = -EFAULT;
        goto out;
    } else {
        dev->current_len += count;
        printk(KERN_INFO "written %ld byte(s), current_len: %ud\n", count, dev->current_len);
        wake_up_interruptible(&dev->r_wait);
        ret = count;
    }

out:
    up(&dev->sem);
out2:
    remove_wait_queue(&dev->w_wait, &wait);

    set_current_state(TASK_RUNNING);
    return ret;
}

static int globalmem_open(struct inode *inode, struct file *filp)
{
    struct globalmem_dev *dev;

    dev = container_of(inode->i_cdev, struct globalmem_dev, cdev);
    filp->private_data = dev;

    return 0;
}

static const struct file_operations globalmem_fops = {
    .owner = THIS_MODULE,
    .read = globalmem_read,
    .write = globalmem_write,
    .open = globalmem_open,
    .poll = globalmem_poll,
};

static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
    int err, devno = MKDEV(globalmem_major, index);

    cdev_init(&dev->cdev, &globalmem_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &globalmem_fops;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding globalmem%d", err, index);
}

int globalmem_init(void)
{
    int result;
    dev_t devno = MKDEV(globalmem_major, 0);

    if (globalmem_major) {
        result = register_chrdev_region(devno, 2, "fifo");
    } else {
        result = alloc_chrdev_region(&devno, 0, 2, "fifo");
        globalmem_major = MAJOR(devno);
    }

    if (result < 0)
        return result;

    globalmem_devp = kmalloc(sizeof(struct globalmem_dev) * DEVICE_NUM, GFP_KERNEL);
    if (!globalmem_devp) {
        result = -ENOMEM;
        goto fail_malloc;
    }
    memset(globalmem_devp, 0, sizeof(struct globalmem_dev) * DEVICE_NUM);

    globalmem_setup_cdev(&globalmem_devp[0], 0);
    globalmem_setup_cdev(&globalmem_devp[1], 1);
    sema_init(&globalmem_devp[0].sem, 1);
    sema_init(&globalmem_devp[1].sem, 1);
    init_waitqueue_head(&globalmem_devp[0].r_wait);
    init_waitqueue_head(&globalmem_devp[0].w_wait);
    init_waitqueue_head(&globalmem_devp[1].r_wait);
    init_waitqueue_head(&globalmem_devp[1].w_wait);

    return 0;

fail_malloc:
    unregister_chrdev_region(devno, 1);
    return result;
}

void globalmem_exit(void)
{
    cdev_del(&(globalmem_devp[0].cdev));
    cdev_del(&(globalmem_devp[1].cdev));
    kfree(globalmem_devp);
    unregister_chrdev_region(MKDEV(globalmem_major, 0), 2);
}

module_init(globalmem_init);
module_exit(globalmem_exit);

MODULE_AUTHOR("Huang Liang");
MODULE_LICENSE("GPL");