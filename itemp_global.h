// $URL: $
// $Id: $

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>

extern int itemp_major;
extern int itemp_minor;
extern dev_t itemp_dev;
extern struct cdev *itemp_cdev;
extern struct class *itemp_class;
