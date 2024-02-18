#include <linux/module.h>
#include <linux/init.h>


static int __init my_init(void)
{
    printk(KERN_INFO"Registering the driver!\n");
    return 0;
}
static void __exit my_exit(void)
{
    printk(KERN_INFO"Registering the driver!\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Antoni Pokusinski");
MODULE_DESCRIPTION ("Calc peripheral kernel driver");
