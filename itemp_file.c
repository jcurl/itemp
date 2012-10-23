// $URL: $
// $Id: $

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>

int itemp_open(struct inode *inode, struct file *file)
{
  return -EIO;
}

loff_t itemp_seek(struct file*file, loff_t offset, int orig)
{
  return -EIO;
}

ssize_t itemp_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
  return -EIO;
}

ssize_t itemp_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
  return -EIO;
}

long itemp_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{
  return -EIO;
}
