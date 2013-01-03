#ifndef EEZE_UDEV_PRIVATE_H
#define EEZE_UDEV_PRIVATE_H
#include <Eeze.h>

#define LIBUDEV_I_KNOW_THE_API_IS_SUBJECT_TO_CHANGE 1
#include <libudev.h>

#ifndef EEZE_UDEV_COLOR_DEFAULT
#define EEZE_UDEV_COLOR_DEFAULT EINA_COLOR_CYAN
#endif
extern int _eeze_udev_log_dom;
#ifdef ERR
#undef ERR
#endif
#ifdef INF
#undef INF
#endif
#ifdef WARN
#undef WARN
#endif
#ifdef DBG
#undef DBG
#endif

#define DBG(...)   EINA_LOG_DOM_DBG(_eeze_udev_log_dom, __VA_ARGS__)
#define INF(...)    EINA_LOG_DOM_INFO(_eeze_udev_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(_eeze_udev_log_dom, __VA_ARGS__)
#define ERR(...)   EINA_LOG_DOM_ERR(_eeze_udev_log_dom, __VA_ARGS__)

/* typedefs because I'm lazy */
typedef struct udev _udev;
typedef struct udev_list_entry _udev_list_entry;
typedef struct udev_device _udev_device;
typedef struct udev_enumerate _udev_enumerate;
typedef struct udev_monitor _udev_monitor;

extern _udev *udev;

_udev_device *_new_device(const char *syspath);
const char *_walk_children_get_attr(const char *syspath, const char *sysattr, const char *subsystem, Eina_Bool property);
Eina_Bool _walk_parents_test_attr(_udev_device *device, const char *sysattr, const char* value);
const char *_walk_parents_get_attr(_udev_device *device, const char *sysattr, Eina_Bool property);
Eina_List *_get_unlisted_parents(Eina_List *list, _udev_device *device);
_udev_device *_copy_device(_udev_device *device);

#endif
