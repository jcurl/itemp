// $URL: $
// $Id: $

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cpu.h>

#include "itemp_global.h"
#include "itemp_cpu.h"

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

int __cpuinit itemp_device_create(int cpu)
{
    struct device *dev;
    dev = device_create(itemp_class, NULL, MKDEV(itemp_major, cpu + itemp_minor), NULL, "temp%d", cpu);
    return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

void itemp_device_destroy(int cpu)
{
    device_destroy(itemp_class, MKDEV(itemp_major, cpu + itemp_minor));
}
