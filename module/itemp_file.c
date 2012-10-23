// $URL: $
// $Id: $

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/msr.h>

#include "itemp.h"
#include "itemp_global.h"

int itemp_open(struct inode *inode, struct file *file)
{
  unsigned int cpu;
  struct cpuinfo_x86 *c;

  cpu = iminor(inode) - itemp_minor;
  if (cpu >= nr_cpu_ids || !cpu_online(cpu)) return -ENXIO;

  c = &cpu_data(cpu);
  if (!cpu_has(c, X86_FEATURE_MSR)) return -EIO;

  return 0;
}

loff_t itemp_seek(struct file*file, loff_t offset, int orig)
{
  return -EINVAL;
}

ssize_t itemp_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
  return -EINVAL;
}

ssize_t itemp_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
  return -EINVAL;
}

long itemp_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{
  int __user *temp = (int __user *)arg;
  u32 l, h;
  int cpu = iminor(file->f_path.dentry->d_inode) - itemp_minor;
  int err = 0;
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
      cputemp = 100 - cputemp;
    } else {
      cputemp = (l >> 16) & 0xFF - cputemp;
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
