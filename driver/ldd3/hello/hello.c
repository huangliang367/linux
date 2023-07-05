#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
MODULE_LICENSE("GPL");

static int howmany = 1;
static char *who = "world";
module_param(howmany, int, S_IRUGO);
module_param(who, charp, S_IRUGO);


static int __init hello_init(void)
{
    int i;

    for (i = 0; i < howmany; i++)
        printk(KERN_ALERT "Hello, %s!\n", who);
    return 0;    
}

static void __exit hello_exit(void)
{
    printk(KERN_ALERT "Goodbye, curel world!\n");
}

module_init(hello_init);
module_exit(hello_exit);
