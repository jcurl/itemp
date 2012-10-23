// $URL$
// $Id$

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/cpumask.h>
#include <linux/cpu.h>

#include "itemp_global.h"
#include "itemp_file.h"
#include "itemp_cpu.h"

static const struct file_operations itemp_ops = {
    .owner = THIS_MODULE,
    .llseek = itemp_seek,
    .read = itemp_read,
    .write = itemp_write,
    .open = itemp_open,
    .unlocked_ioctl = itemp_ioctl,
    .compat_ioctl = itemp_ioctl,
};

static char *itemp_devnode(struct device *dev, umode_t *mode)
{
    return kasprintf(GFP_KERNEL, "cpu/%u/temp", MINOR(dev->devt) - itemp_minor);
}

static int __init itemp_init(void)
{
    int result;
    int err;
    int i;

    // Allocate / Register the Major/Minor Char Dev Region
    if (itemp_major) {
	itemp_dev = MKDEV(itemp_major, itemp_minor);
	result = register_chrdev_region(itemp_dev, NR_CPUS, "cpu/temp");
    } else {
	result = alloc_chrdev_region(&itemp_dev, 0, NR_CPUS, "cpu/temp");
	itemp_major = MAJOR(itemp_dev);
	itemp_minor = MINOR(itemp_dev);
    }

    if (result < 0) {
	printk(KERN_WARNING "itemp: can't register chrdev (%d, %d)\n",
	       itemp_major, itemp_minor);
	err = -EBUSY;
	goto exit_err;
    }

    // Char Device Registration
    itemp_cdev = cdev_alloc();
    itemp_cdev->owner = THIS_MODULE;
    itemp_cdev->ops = &itemp_ops;
    result = cdev_add(itemp_cdev, itemp_dev, NR_CPUS);
    if (result) {
	printk(KERN_WARNING "itemp: error %d adding chrdev", result);
	err = -EBUSY;
	goto exit_chrdevrgn;
    }

    // Create the class
    itemp_class = class_create(THIS_MODULE, "itemp");
    if (IS_ERR(itemp_class)) {
	err = PTR_ERR(itemp_class);
	goto exit_chrdev;
    }

    // For each CPU create the class/device
    itemp_class->devnode = itemp_devnode;
    for_each_online_cpu(i) {
	result = itemp_device_create(i);
	if (result) {
	    err = result;
	    goto exit_class;
	}
    }

    // Register procfs for easy reading

    // Register CPU hotplug notifier
    register_hotcpu_notifier(&itemp_class_cpu_notifier);

    // Finally
    return 0;
  
    // Clean up code
exit_class:
    i = 0;
    for_each_online_cpu(i) itemp_device_destroy(i);
    class_destroy(itemp_class);
exit_chrdev:
    cdev_del(itemp_cdev);
exit_chrdevrgn:
    unregister_chrdev_region(itemp_dev, NR_CPUS);
exit_err:
    return err;
}

static void __exit itemp_exit(void)
{
    int i;

    unregister_hotcpu_notifier(&itemp_class_cpu_notifier);
    for_each_online_cpu(i) itemp_device_destroy(i);
    class_destroy(itemp_class);
    cdev_del(itemp_cdev);
    unregister_chrdev_region(itemp_dev, NR_CPUS);
}

module_init(itemp_init);
module_exit(itemp_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jason Curl");
MODULE_DESCRIPTION("Get the temperature of each CPU Core for Intel Processors");
MODULE_VERSION("0:0.1-Quantal");
