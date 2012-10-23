// $URL: $
// $Id: $

#include <linux/fs.h>

int itemp_open(struct inode *inode, struct file *file);
loff_t itemp_seek(struct file*file, loff_t offset, int orig);
ssize_t itemp_read(struct file *file, char __user *buf, size_t count, loff_t *ppos);
ssize_t itemp_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
long itemp_ioctl(struct file *file, unsigned int ioc, unsigned long arg);
