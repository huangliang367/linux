#ifndef _SCULL_H_
#define _SCULL_H_

#include <linux/ioctl.h>

#undef PEDBUG
#ifdef SCULL_DEBUG
#   ifdef __KERNEL__
#       define PDEBUG(fmt, args...) printk(KERN_DEBUG "scull: " fmt, ## args)
#   else
#       define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#   endif
#else
#   define PDEBUG(fmt, args...)
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...)

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0
#endif

#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS   4
#endif

#ifndef SCULL_QUANTUM
#define SCULL_QUANTUM   4000
#endif

#ifndef SCULL_QSET
#define SCULL_QSET      1000
#endif

struct scull_qset {
    void **data;
    struct scull_qset *next;
};

struct scull_dev {
    struct scull_qset *data;
    int quantum;
    int qset;
    unsigned long size;
    unsigned int access_key;
    struct semaphore sem;
    struct cdev cdev;
};

extern int scull_major;
extern int scull_minor;
extern int scull_quantum;
extern int scull_qset;

#define SCULL_IOC_MAGIC     'k'

#define SCULL_IOCRESET      _IO(SCULL_IOC_MAGIC, 0)
#define SCULL_IOCSQUANTUM   _IOW(SCULL_IOC_MAGIC, 1, int)
#define SCULL_IOCTQUANTUM   _IO(SCULL_IOC_MAGIC, 2)
#define SCULL_IOCGQUANTUM   _IOR(SCULL_IOC_MAGIC, 3, int)
#define SCULL_IOCXQUANTUM   _IOWR(SCULL_IOC_MAGIC, 4, int)

#define SCULL_IOC_MAXNR     14
#endif /* _SCULL_H_ */
