// $URL: $
// $Id: $

#include "itemp_global.h"

// The major number of this device. Keep to zero for dynamically allocated
// major number. The minor numbers are assumed to start at zero.
int itemp_major = 0;
int itemp_minor = 0;
dev_t itemp_dev;

struct cdev *itemp_cdev = NULL;
struct class *itemp_class = NULL;
