#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int hello_init(void)
{
	printk(KERN_ALERT "Hello world enter\n");
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "Hello world exit\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Huang Liang");
MODULE_DESCRIPTION("Hello world module");
MODULE_ALIAS("a simplest module");
