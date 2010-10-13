#ifndef EEZE_UDEV_PRIVATE_H
#define EEZE_UDEV_PRIVATE_H
#include "Eeze.h"

/* typedefs because I'm lazy */
typedef struct udev _udev;
typedef struct udev_list_entry _udev_list_entry;
typedef struct udev_device _udev_device;
typedef struct udev_enumerate _udev_enumerate;
typedef struct udev_monitor _udev_monitor;

_udev_device *_new_device(const char *syspath);
Eina_Bool _walk_parents_test_attr(_udev_device *device, const char *sysattr, const char* value);
const char *_walk_parents_get_attr(_udev_device *device, const char *sysattr);
Eina_List *_get_unlisted_parents(Eina_List *list, _udev_device *device);
_udev_device *_copy_device(_udev_device *device);

#endif
