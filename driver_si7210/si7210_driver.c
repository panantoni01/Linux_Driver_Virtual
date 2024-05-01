#include <linux/module.h>

static int __init si7210_init(void)
{
	printk(KERN_INFO "s7210_driver registration\n");
	return 0;
}

static void __exit si7210_cleanup(void)
{
	printk(KERN_INFO "si7210_driver removal\n");
}

module_init(si7210_init);
module_exit(si7210_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Antoni Pokusinski");
MODULE_DESCRIPTION("si7210 driver");
