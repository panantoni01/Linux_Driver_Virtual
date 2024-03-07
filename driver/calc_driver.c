#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <asm/ioctl.h>
#include "calc_driver.h"

#define CALC_MAX_MINORS 3

static int calc_major;


struct calc_device_data {
    struct cdev cdev;
    long var;
    unsigned int curr_op;
};

static int calc_open(struct inode *inode, struct file *file)
{
    struct calc_device_data *calc_data = container_of(inode->i_cdev, struct calc_device_data, cdev);
    file->private_data = calc_data;

    return 0;
}

static ssize_t calc_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    struct calc_device_data *calc_data = (struct calc_device_data *)file->private_data;
    size_t buf_size = count < (sizeof(calc_data->var) - *offset) ? count : (sizeof(calc_data->var) - *offset);
    
    if(copy_to_user(buf, &calc_data->var, sizeof(long)))
        return -EFAULT;

    *offset += buf_size;
    return buf_size;
}

static ssize_t calc_write(struct file *file, const char __user *buf, size_t count, loff_t *offset) 
{
    long num_buf;
    size_t buf_size = count < sizeof(num_buf) ? count : sizeof(num_buf);
    struct calc_device_data *calc_data = (struct calc_device_data *)file->private_data;

    if(copy_from_user(&num_buf, buf, buf_size))
        return -EFAULT;    

    switch (calc_data->curr_op) {
        case ADD:
            calc_data->var += num_buf;
            break;
        case SUB:
            calc_data->var -= num_buf;
            break;
        case MUL:
            calc_data->var *= num_buf;
            break;
        case DIV:
            if (num_buf == 0) {
                printk(KERN_ERR"Div0 attempt!\n");
                return buf_size;
            }
            calc_data->var /= num_buf;
            break;
        default:
            break;
    }

    return buf_size;
}

static long calc_ioctl (struct file *file, unsigned int cmd, unsigned long arg) 
{
    struct calc_device_data *calc_data = (struct calc_device_data *)file->private_data;
    
    switch(cmd) {
        case CALC_IOCTL_RESET:
            calc_data->var = 0;
            break;
        case CALC_IOCTL_CHANGE_OP:
            calc_data->curr_op = (unsigned int)arg;
            if (calc_data->curr_op > DIV)
                return -EINVAL;
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static int calc_release (struct inode *inode, struct file *file)
{
    return 0;
}

const struct file_operations calc_fops = {
    .owner = THIS_MODULE,
    .open = calc_open,
    .read = calc_read,
    .write = calc_write,
    .unlocked_ioctl = calc_ioctl,
    .release = calc_release
};

static unsigned char calc_minors[CALC_MAX_MINORS] = {0};

static int get_calc_minor(void)
{
    unsigned int i;

    for (i = 0; i < CALC_MAX_MINORS; i++)
        if (calc_minors[i] == 0)
            return i;

    return -1;
}

static int calc_driver_probe(struct platform_device *pdev)
{
    struct calc_device_data* data;
    unsigned int minor;
    int ret;

    minor = get_calc_minor();
    if (minor == -1) {
        printk(KERN_ERR "calc_driver: reached max number of devices\n");
        return -EIO;
    }
    calc_minors[minor] = 1;

    data = devm_kzalloc(&pdev->dev, sizeof(struct calc_device_data), GFP_KERNEL);
    if (!data) {
        printk(KERN_ERR "calc_driver: unable to allocate driver data\n");
        return -ENOMEM;
    }

    cdev_init(&data->cdev, &calc_fops);
    ret = cdev_add(&data->cdev, MKDEV(calc_major, minor), 1);
    if (ret) {
        printk(KERN_ERR "calc_driver: cdev_add failed\n");
        goto err_min_ret;
    }

    data->var = 0;
    data->curr_op = ADD;

    platform_set_drvdata(pdev, data);

    printk(KERN_INFO"calc_driver: successful probe of device: %s\n",
                    pdev->name);
    return 0;

err_min_ret:
    calc_minors[minor] = 0;
    return ret;
}

static int calc_driver_remove(struct platform_device *pdev)
{
    struct calc_device_data* data;
    struct cdev* cdev;
    unsigned int minor;

    data = platform_get_drvdata(pdev);
    cdev = &data->cdev;
    minor = MINOR(cdev->dev);
    
    cdev_del(&data->cdev);
    calc_minors[minor] = 0;

    return 0;
}

static const struct of_device_id calc_driver_dt_ids[] = 
{
    { .compatible = "calc-driver" },
    {}
};

static struct platform_driver calc_driver = {
    .driver = {
        .name = "calc_driver",
        .of_match_table = calc_driver_dt_ids,
    },
    .probe = calc_driver_probe,
    .remove = calc_driver_remove,
};

int init_module(void)
{
    int ret;
    dev_t dev;

    ret = alloc_chrdev_region(&dev, 0, CALC_MAX_MINORS, "calc_driver");
    if (ret != 0) {
        printk(KERN_ERR "calc_driver: cannot allocate chrdev region\n");
        return ret;
    }
    calc_major = MAJOR(dev);

    ret = platform_driver_register(&calc_driver);
    if (ret) {
        printk(KERN_ERR "calc_driver: error while registering the driver\n");
        goto err_unreg;
    }

    printk(KERN_INFO "calc_driver: successfully registered\n");
    return 0;

err_unreg:
    unregister_chrdev_region(calc_major, CALC_MAX_MINORS);
    return ret;
}

void cleanup_module()
{
    printk(KERN_INFO "calc_driver removal\n");

    unregister_chrdev_region(calc_major, CALC_MAX_MINORS);
    platform_driver_unregister(&calc_driver);
}

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Antoni Pokusinski");
MODULE_DESCRIPTION ("Simple driver for virtual calc device");
