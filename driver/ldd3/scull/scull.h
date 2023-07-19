#ifndef _SCULL_H_
#define _SCULL_H_

#include <linux/ioctl.h>

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

#endif /* _SCULL_H_ */
