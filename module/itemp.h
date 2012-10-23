// $URL: $
// $Id: $

#include <linux/types.h>
#include <linux/ioctl.h>

#ifndef KERNEL_MODULE_ITEMP
#define KERNEL_MDOULE_ITEMP

#ifdef __CPLUSPLUS
extern "c" {
#endif

#define X86_GET_CORE_TEMPERATURE _IOR('c', 0xC0, int)

#ifdef __CPLUSPLUS
}
#endif

#endif
