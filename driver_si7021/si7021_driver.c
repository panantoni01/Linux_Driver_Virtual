#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include "si7021_driver.h"

#define SI7021_MAX_MINORS 2

#define SI7021_CMD_RESET 0xFE
#define SI7021_CMD_TEMP_MEASURE 0xE3
#define SI7021_CMD_HUMI_MEASURE 0xE5
#define SI7021_CMD_READ_ID_1 0xFA0F
#define SI7021_CMD_READ_ID_2 0xFCC9


static int si7021_major;
static unsigned char si7021_minors[SI7021_MAX_MINORS] = {0};
static struct class* si7021_class;

struct si7021_data {
    struct cdev cdev;
    unsigned long flags;
#define SI7021_BUSY_BIT_POS 0
    struct i2c_client *client;
};

/* Si7021 has commands that are 1- or 2-byte long */
static int si7021_send_cmd(struct i2c_client* client, u16 cmd, int count) {
    return i2c_master_send(client, (char*)&cmd, count);
}

static int si7021_open(struct inode *inode, struct file *file)
{
    struct si7021_data *si7021_data = container_of(inode->i_cdev, struct si7021_data, cdev);

    if (test_and_set_bit(SI7021_BUSY_BIT_POS, &si7021_data->flags))
		return -EBUSY;

    file->private_data = si7021_data;

    return 0;
}

static ssize_t si7021_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    return 0;
}

static ssize_t si7021_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    return 0;
}

static long si7021_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    long long read_id;
    struct si7021_data* si7021_data = (struct si7021_data*)file->private_data;

    switch(cmd) {
        case SI7021_IOCTL_RESET:
            ret = si7021_send_cmd(si7021_data->client, SI7021_CMD_RESET, sizeof(u8));
            if (ret < 0)
                goto send_err;
            break;
        case SI7021_IOCTL_READ_ID:
            ret = si7021_send_cmd(si7021_data->client, SI7021_CMD_READ_ID_1, sizeof(u16));
            if (ret < 0)
                goto send_err;
            ret = i2c_master_recv(si7021_data->client, (char *)&read_id, 4);
            if (ret < 0)
                goto recv_err;

            ret = si7021_send_cmd(si7021_data->client, SI7021_CMD_READ_ID_2, sizeof(u16));
            if (ret < 0)
                goto send_err;
            ret = i2c_master_recv(si7021_data->client, (char *)&read_id + 4, 4);
            if (ret < 0)
                goto recv_err;

            if (copy_to_user((u64*)arg, &read_id, sizeof(long long)))
                ret = -EFAULT;
            break;
        default:
            ret = -EINVAL;
    }

    return ret;
send_err:
    dev_err(&si7021_data->client->dev, "failed to send data to si7021\n");
    return ret;
recv_err:
    dev_err(&si7021_data->client->dev, "failed to receive data from si7021\n");
    return ret;
}

static int si7021_release (struct inode *inode, struct file *file)
{
    struct si7021_data* si7021_data = file->private_data;

    clear_bit(SI7021_BUSY_BIT_POS, &si7021_data->flags);
    return 0;
}

const struct file_operations si7021_fops = {
    .owner = THIS_MODULE,
    .open = si7021_open,
    .read = si7021_read,
    .write = si7021_write,
    .unlocked_ioctl = si7021_ioctl,
    .release = si7021_release
};

static int get_si7021_minor(void)
{
    unsigned int i;

    for (i = 0; i < SI7021_MAX_MINORS; i++)
        if (si7021_minors[i] == 0)
            return i;

    return -1;
}

static int si7021_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    long ret;
    struct si7021_data* data;
    unsigned int minor;

    minor = get_si7021_minor();
    if (minor == -1) {
        printk(KERN_ERR "si7021_driver: reached max number of devices\n");
        return -EIO;
    }
    si7021_minors[minor] = 1;

    data = devm_kzalloc(&client->dev, sizeof(struct si7021_data), GFP_KERNEL);
    if (!data) {
        printk(KERN_ERR "si7021_driver: unable to allocate driver data\n");
        ret = -ENOMEM;
        goto err_min_ret;
    }

    cdev_init(&data->cdev, &si7021_fops);
    ret = cdev_add(&data->cdev, MKDEV(si7021_major, minor), 1);
    if (ret) {
        printk(KERN_ERR "si7021_driver: cdev_add failed\n");
        goto err_min_ret;
    }

    data->client = client;

    i2c_set_clientdata(client, data);

    if (IS_ERR(device_create(si7021_class, &client->dev,
                             MKDEV(si7021_major, minor), NULL,
                             "si7021-%u", minor)))
        printk(KERN_ERR "si7021_driver: cannot create char device\n");

    printk(KERN_INFO"si7021_driver: successful probe of device: %s\n",
                    client->name);
    return 0;

err_min_ret:
    si7021_minors[minor] = 0;
    return ret;
}

static int si7021_remove(struct i2c_client *client)
{
    struct si7021_data* data;
    struct cdev* cdev;
    unsigned int minor;

    data = i2c_get_clientdata(client);
    cdev = &data->cdev;
    minor = MINOR(cdev->dev);
    
    cdev_del(&data->cdev);
    si7021_minors[minor] = 0;

    device_destroy(si7021_class, MKDEV(si7021_major, minor));

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
