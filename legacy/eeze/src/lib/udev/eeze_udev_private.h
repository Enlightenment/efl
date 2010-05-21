#ifndef EEZE_UDEV_PRIVATE_H
#define EEZE_UDEV_PRIVATE_H
#include "Eeze_Udev.h"
#ifndef E_EEZE_COLOR_DEFAULT
#define E_EEZE_COLOR_DEFAULT EINA_COLOR_CYAN
#endif
extern int _e_eeze_udev_log_dom;
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

#define DBG(...)   EINA_LOG_DOM_DBG(_e_eeze_udev_log_dom, __VA_ARGS__)
#define INFO(...)    EINA_LOG_DOM_INFO(_e_eeze_udev_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(_e_eeze_udev_log_dom, __VA_ARGS__)
#define ERR(...)   EINA_LOG_DOM_ERR(_e_eeze_udev_log_dom, __VA_ARGS__)

Eina_Bool _walk_parents_test_attr(struct udev_device *device, const char *sysattr, const char* value);
const char *_walk_parents_get_attr(struct udev_device *device, const char *sysattr);
void _get_unlisted_parents(Eina_List *list, struct udev_device *device);

#endif
