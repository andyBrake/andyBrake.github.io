#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Hcamal");

int hello_init(void)
{
    printk(KERN_INFO "Hello World\n");
    return 0;
}

void hello_exit(void)
{
    printk(KERN_INFO "Goodbye World\n");
}

module_init(hello_init);
module_exit(hello_exit);