#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define SI7021_MAX_MINORS 2

static int si7021_major;
static struct class* si7021_class;


static int si7021_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    return 0;
}

static int si7021_remove(struct i2c_client *client)
{
	return 0;
}


static const struct of_device_id si7021_dt_ids[] = {
	{ .compatible = "si7021" },
	{ }
};
 MODULE_DEVICE_TABLE(of, si7021_dt_ids); /* https://stackoverflow.com/questions/22901282 */

static struct i2c_driver si7021_driver = {
	.driver = {
		.name = "si7021",
		.of_match_table = si7021_dt_ids,
	},
	.probe = si7021_probe,
	.remove = si7021_remove,
};

static int __init si7021_init(void)
{
    int ret;
    dev_t dev;

    ret = alloc_chrdev_region(&dev, 0, SI7021_MAX_MINORS, "si7021_driver");
    if (ret != 0) {
        printk(KERN_ERR "si7021_driver: cannot allocate chrdev region\n");
        return ret;
    }
    si7021_major = MAJOR(dev);

    si7021_class = class_create(THIS_MODULE, "thermal");
    if (IS_ERR(si7021_class)) {
        printk(KERN_ERR "si7021_driver: cannot create gpio class\n");
        goto err_unreg;
    }

    ret = i2c_add_driver(&si7021_driver);
    if (ret) {
        printk(KERN_ERR "si7021_driver: error while registering the driver\n");
        goto err_cls;
    }

    printk(KERN_INFO "si7021_driver: successfully registered\n");
    return 0;

err_cls:
    class_destroy(si7021_class);
err_unreg:
    unregister_chrdev_region(si7021_major, SI7021_MAX_MINORS);
    return ret;
}

static void __exit si7021_cleanup(void)
{
    printk(KERN_ERR"si7021_driver removal\n");

	unregister_chrdev_region(si7021_major, SI7021_MAX_MINORS);
    i2c_del_driver(&si7021_driver);
	class_destroy(si7021_class);
}

module_init(si7021_init);
module_exit(si7021_cleanup);


MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Antoni Pokusinski");
MODULE_DESCRIPTION ("si7021 driver");
