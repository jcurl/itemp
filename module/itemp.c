//////////////////////////////////////////////////////////////////////////////
// File: itemp.c
//
// $URL$
// $Id$
//
// Description
//  A simple kernel module that reads the temperature of an Intel CPU core
//  for each CPU node in the system. 
//
//  This code is to show how to write a simple kernel module. It's heavily
//  inspired by 'msr.c' and 'cpuid.c' in 'arch/x86/' of the kernel sources.
//
//  It provides the following features:
//  * Module parameters (tjmax)
//  * sysfs integration
//    * Allows hotplugging with udev
//  * ioctl() for reading the temperature, based on the device actually
//    opened.
//  * Dynamic allocation of the Major/Minor numbers
//  * Execution of "rdmsr" on a specific CPU. See kernel sources of how
//    this really works. 
//  * CPU hotplugging. Removal/insertion of a CPU results in the device
//    being added or removed. If you have 'udev', then this is automatic.
//
//  Code is based on the design also by LDD3. That is, we haven't simply
//  copied the code from 'msr.c'.
//
// Design
//
// Revision History:
//
//  The revision history contains the date of a change, the author of the
//  change and in the description at a minimum the Problem Report that
//  required the change.
//
//  Date    Author   Description
//  ------- -------- ---------------------------------------------------------
//  24Oct12 jcurl    Initial revision. Merged into a single file
//
// To Do:
// * Provide a procfs interface (although this is outdated and shouldn't be
//   used). Instead we could provide a new entry in the sysfs path space.
//
//////////////////////////////////////////////////////////////////////////////

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/cpumask.h>
#include <linux/cpu.h>
#include <asm/uaccess.h>
#include <asm/msr.h>

#include "itemp.h"

// ---------------------------------------------------------------------------
// Module Parameters
// ---------------------------------------------------------------------------

// Specify to the module tjmax=<value>
//   e.g. insmod tjmax=100
//
// When inserting the module, it uses a default of 100degC. This might be
// correct for your processor, but may not be so. For the Intel CPU T2700, the
// datasheet specifies it is 100degC. For other processors it may be 90, 105,
// etc. This value is only used if your processor doesn't support the MSR
// IA32_TEMPERATURE_TARGET
static int itemp_tjmax = 100;
module_param_named(tjmax, itemp_tjmax, int, S_IRUGO);

// Specify the major device number for use with this module. A value of
// zero indicates that the major device number should be dynamically
// allocated. You should then specify the major number when inserting
// your module.
//   e.g. insmod major=252
//
// Ideally, you shouldn't use this at all, leave it at zero, and use udev
// for creating your nodes.
static int itemp_major = 0;
module_param_named(major, itemp_major, int, S_IRUGO);

static int itemp_minor = 0;
static dev_t itemp_dev;
static struct cdev *itemp_cdev = NULL;
static struct class *itemp_class = NULL;

// ---------------------------------------------------------------------------
// File Operations on /dev/cpu/N/temp
// ---------------------------------------------------------------------------
static char *itemp_devnode(struct device *dev, umode_t *mode)
{
    return kasprintf(GFP_KERNEL, "cpu/%u/temp", MINOR(dev->devt) - itemp_minor);
}

static int itemp_open(struct inode *inode, struct file *file)
{
    unsigned int cpu;
    struct cpuinfo_x86 *c;
    
    cpu = iminor(inode) - itemp_minor;
    if (cpu >= nr_cpu_ids || !cpu_online(cpu)) return -ENXIO;
    
    c = &cpu_data(cpu);
    if (!cpu_has(c, X86_FEATURE_MSR)) return -EIO;
    
    return 0;
}

static loff_t itemp_seek(struct file*file, loff_t offset, int orig)
{
    return -EINVAL;
}

static ssize_t itemp_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    return -EINVAL;
}

static ssize_t itemp_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    return -EINVAL;
}

static long itemp_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{
    int __user *temp = (int __user *)arg;
    u32 l, h;
    int cpu = iminor(file->f_path.dentry->d_inode) - itemp_minor;
    int err;
    int cputemp;
    
    switch (ioc) {
    case X86_GET_CORE_TEMPERATURE:
	if (!(file->f_mode & FMODE_READ)) {
	    err = -EBADF;
	    break;
	}
	
	// IA32_THERM_STATUS
	err = rdmsr_safe_on_cpu(cpu, 0x19C, &l, &h);
	if (err) break;
	cputemp = (l & 0x07F0000) >> 16;
	
	// IA32_TEMPERATURE_TARGET
	err = rdmsr_safe_on_cpu(cpu, 0x1A2, &l, &h);
	if (err) {
	    cputemp = itemp_tjmax - cputemp;
	} else {
	    cputemp = ((l >> 16) & 0xFF) - cputemp;
	}
	err = 0;
	
	if (copy_to_user(temp, &cputemp, sizeof(*temp))) {
	    err = -EFAULT;
	    break;
	}
	break;
    
    default:
	err = -ENOTTY;
	break;
    }

    return err;
}

static const struct file_operations itemp_ops = {
    .owner = THIS_MODULE,
    .llseek = itemp_seek,
    .read = itemp_read,
    .write = itemp_write,
    .open = itemp_open,
    .unlocked_ioctl = itemp_ioctl,
    .compat_ioctl = itemp_ioctl,
};

// ---------------------------------------------------------------------------
// CPU Hotplugging
// ---------------------------------------------------------------------------
static int __cpuinit itemp_device_create(int cpu)
{
    struct device *dev;
    dev = device_create(itemp_class, NULL, MKDEV(itemp_major, cpu + itemp_minor), NULL, "temp%d", cpu);
    return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

static void itemp_device_destroy(int cpu)
{
    device_destroy(itemp_class, MKDEV(itemp_major, cpu + itemp_minor));
}

static int __cpuinit itemp_class_cpu_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
{
    unsigned int cpu = (unsigned long)hcpu;
    int err = 0;

    switch (action) {
    case CPU_UP_PREPARE:
	err = itemp_device_create(cpu);
	break;
    case CPU_UP_CANCELED:
    case CPU_UP_CANCELED_FROZEN:
    case CPU_DEAD:
	itemp_device_destroy(cpu);
	break;
    }
    return notifier_from_errno(err);
}

struct notifier_block __refdata itemp_class_cpu_notifier = {
    .notifier_call = itemp_class_cpu_callback,
};

// ---------------------------------------------------------------------------
// Initialisation and Termination of the module
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Module metadata
// ---------------------------------------------------------------------------
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Jason Curl");
MODULE_DESCRIPTION("Get the temperature of each CPU Core for Intel Processors");
MODULE_VERSION("0:0.1-Quantal");
