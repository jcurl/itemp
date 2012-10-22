#include <linux/module.h>
#include <linux/kernel.h>

static int __init itemp_init(void)
{
  int err = -EBUSY;
  return err;
}

static void __exit itemp_exit(void)
{
  
}

module_init(itemp_init);
module_exit(itemp_exit);

