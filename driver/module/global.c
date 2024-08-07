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

#define GLOBALMEM_SIZE 0x1000
#define MEM_CLEAR 0x1
#define GLOBALMEM_MAJOR 0
#define DEVICE_NUM      2

static int globalmem_major = GLOBALMEM_MAJOR;

struct globalmem_dev {
    struct cdev cdev;
    unsigned char mem[GLOBALMEM_SIZE];
    struct semaphore sem;
};

struct globalmem_dev *globalmem_devp;

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    unsigned long p = *ppos;
    int ret = 0;
    struct globalmem_dev *dev;

    dev = filp->private_data;

    if (p > GLOBALMEM_SIZE)
        return count ? -ENXIO : 0;
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    if (copy_to_user(buf, (void *)(dev->mem + p), count)) {
        ret = - EFAULT;
    } else {
        *ppos += count;
        ret = count;
        printk(KERN_INFO "read %ld byte(s) from %ld\n", count, p);
    }

    up(&dev->sem);

    return ret;
}

static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    unsigned long p = *ppos;
    int ret = 0;
    struct globalmem_dev *dev;

    dev = filp->private_data;

    if (p >= GLOBALMEM_SIZE)
        return count ? -ENXIO : 0;
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;

    if (down_interruptible(&dev->sem)) {
        return -ERESTARTSYS;
    }

    if (copy_from_user((void *)(dev->mem + p), buf, count)) {
        ret = -EFAULT;
    } else {
        *ppos += count;
        ret = count;
        printk(KERN_INFO "written %ld byte(s) from %ld\n", count, p);
    }

    up(&dev->sem);

    return ret;
}

static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
{
    return 0;
}

/*
static int globalmem_ioctl(struct inode *inodep, struct file *filp, unsigned int cmd, unsigned long arg)
{
    return 0;
}
*/

static int globalmem_open(struct inode *inode, struct file *filp)
{
    struct globalmem_dev *dev;

    dev = container_of(inode->i_cdev, struct globalmem_dev, cdev);
    filp->private_data = dev;

    return 0;
}

static const struct file_operations globalmem_fops = {
    .owner = THIS_MODULE,
    .llseek = globalmem_llseek,
    .read = globalmem_read,
    .write = globalmem_write,
//    .ioctl = globalmem_ioctl,
    .open = globalmem_open,
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
        result = register_chrdev_region(devno, 2, "globalmem");
    } else {
        result = alloc_chrdev_region(&devno, 0, 2, "globalmem");
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