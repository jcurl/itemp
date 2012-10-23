// $URL: $
// $Id: $

#include <linux/init.h>

int __cpuinit itemp_device_create(int cpu);
void itemp_device_destroy(int cpu);

extern struct notifier_block __refdata itemp_class_cpu_notifier;
