#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>

#include "scull.h"

int scull_major = SCULL_MAJOR;
int scull_minor = 0;
int scull_nr_devs = SCULL_NR_DEVS;
int scull_quantum = SCULL_QUANTUM;
int scull_qset = SCULL_QSET;

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_quantum, int, S_IRUGO);
module_param(scull_qset, int, S_IRUGO);

MODULE_LICENSE("Dual BSD/GPL");

struct scull_dev *scull_devices;

int scull_trim(struct scull_dev *dev)
{
    struct scull_qset *next, *dptr;
    int qset = dev->qset;
    int i;

    for (dptr = dev->data; dptr; dptr = next) {
        if (dptr->data) {
            for (i = 0; i < qset; i++)
                kfree(dptr->data[i]);
            kfree(dptr->data);
            dptr->data = NULL;
        }

        next = dptr->next;
        kfree(dptr);
    }

    dev->size = 0;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->data = NULL;

    return 0;
}

struct scull_qset *scull_follow(struct scull_dev *dev, int n)
{
    struct scull_qset *qset = dev->data;

    if (!qset) {
        qset = dev->data = kmalloc(sizeof(scull_qset), GFP_KERNEL);
        if (qset == NULL)
            return NULL;
        memset(qset, 0, sizeof(struct scull_qset));
    }

    while (n--) {
        if (!qset->next) {
            qset->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
            if (qset->next == NULL)
                return NULL;
            memset(qset->next, 0, sizeof(struct scull_qset));
        }

        qset = qset->next;
        continue;
    }

    return qset;
}

ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr;
    int quantum = dev->quantum, qset = dev->qset;
    int itemsize = quantum * qset;
    int item, s_pos, q_pos, rest;
    ssize_t retval = 0;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + count > dev->size)
        count = dev->size - *f_pos;

    PDEBUG("read count = %d, f_pos = %d\n", (int)count, (int)*f_pos);
    
    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum; q_pos = rest % quantum;

    dptr = scull_follow(dev, item);
    if (dptr == NULL || !dptr->data || !dptr->data[s_pos])
        goto out;

    if (count > quantum - q_pos)
        count = quantum - q_pos;
    
    if (copy_to_user(buff, dptr->data[s_pos] + q_pos, count)) {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;
    
    PDEBUG("after, f_pos = %d\n", (int)*f_pos);
out:
    up(&dev->sem);
    return retval;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr;
    int quantum = dev->quantum;
    int qset = dev->qset;
    int itemsize = quantum * qset;
    int item, s_pos, q_pos, rest;
    ssize_t retval = -ENOMEM;

    PDEBUG("write count = %d\n", (int)count);
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum; q_pos = rest % quantum;

    dptr = scull_follow(dev, item);
    if (dptr == NULL)
        goto out;
    if (!dptr->data) {
        dptr->data = kmalloc(qset *sizeof(char *), GFP_KERNEL);
        if (!dptr->data)
            goto out;
        memset(dptr->data, 0, qset * sizeof(char *));
    }
    
    if (!dptr->data[s_pos]) {
        dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (!dptr->data[s_pos])
            goto out;
    }

    if (count > quantum - q_pos)
        count = quantum - q_pos;
    
    if (copy_from_user(dptr->data[s_pos] + q_pos, buf, count)) {
        retval = -EFAULT;
        goto out;
    }
    
    *f_pos += count;
    retval = count;

    PDEBUG("write dev->size = %d, *f_pos = %d\n", (int)dev->size, (int)*f_pos);
    if (dev->size < *f_pos)
        dev->size = *f_pos;
out:
    up(&dev->sem);
    return retval;
}

int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;

    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        if (down_interruptible(&dev->sem))
            return -ERESTARTSYS;
        scull_trim(dev);
        up(&dev->sem);
    }

    return 0;
}

int scull_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
    return 0;
}

loff_t scull_llseek(struct file *filp, loff_t off, int whence)
{
    return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
    return 0;
}

struct file_operations scull_ops = {
    .owner = THIS_MODULE,
    .llseek = scull_llseek,
    .read = scull_read,
    .write = scull_write,
//    .ioctl = scull_ioctl,
    .open = scull_open,
    .release = scull_release,
};

static void scull_setup_cdev(struct scull_dev *dev, int index)
{
    int err;
    int devno = MKDEV(scull_major, scull_minor + index);

    cdev_init(&dev->cdev, &scull_ops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &scull_ops;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        PDEBUG("Error %d adding scull%d", err, index);
}

void scull_cleanup_module(void)
{
    int i;
    dev_t devno = MKDEV(scull_major, scull_minor);

    if (scull_devices) {
        for (i = 0; i < scull_nr_devs; i++) {
            scull_trim(scull_devices + i);
            cdev_del(&scull_devices[i].cdev);
        }
        kfree(scull_devices);
    }

    unregister_chrdev_region(devno, scull_nr_devs);
}

int scull_init_module(void)
{
    int result;
    int i;
    dev_t dev = 0;

    if (scull_major) {
        dev = MKDEV(scull_major, scull_minor);
        result = register_chrdev_region(dev, scull_nr_devs, "scull");
    } else {
        result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scull");
        scull_major = MAJOR(dev);
    }

    if (result < 0) {
        PDEBUG("scull: can't get major %d\n", (int)scull_major);
        return result;
    }

    scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev), GFP_KERNEL);
    if (!scull_devices) {
        result = -ENOMEM;
        goto fail;
    }
    memset(scull_devices, 0, scull_nr_devs * sizeof(struct scull_dev));

    for (i = 0; i < scull_nr_devs; i++) {
        scull_devices[i].quantum = scull_quantum;
        scull_devices[i].qset = scull_qset;
        sema_init(&scull_devices[i].sem, 1);
        scull_setup_cdev(&scull_devices[i], i);
    }

    dev = MKDEV(scull_major, scull_minor + scull_nr_devs);
    
    return 0;
fail:
    scull_cleanup_module();
    return result;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);
