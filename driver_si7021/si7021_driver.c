#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>


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
    printk(KERN_ERR"si7021_driver: successfully registered\n");
    return i2c_add_driver(&si7021_driver);
}

static void __exit si7021_cleanup(void)
{
    printk(KERN_ERR"si7021_driver removal\n");
    i2c_del_driver(&si7021_driver);
}

module_init(si7021_init);
module_exit(si7021_cleanup);


MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Antoni Pokusinski");
MODULE_DESCRIPTION ("si7021 driver");
