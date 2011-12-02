#ifndef EEZE_NET_PRIVATE_H
#define EEZE_NET_PRIVATE_H
#include <Eeze.h>
#include "eeze_udev_private.h"

#ifndef EEZE_NET_COLOR_DEFAULT
#define EEZE_NET_COLOR_DEFAULT EINA_COLOR_GREEN
#endif
extern int _eeze_net_log_dom;
#ifdef CRI
#undef CRI
#endif

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

#define CRI(...)   EINA_LOG_DOM_CRIT(_eeze_net_log_dom, __VA_ARGS__)
#define DBG(...)   EINA_LOG_DOM_DBG(_eeze_net_log_dom, __VA_ARGS__)
#define INF(...)    EINA_LOG_DOM_INFO(_eeze_net_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(_eeze_net_log_dom, __VA_ARGS__)
#define ERR(...)   EINA_LOG_DOM_ERR(_eeze_net_log_dom, __VA_ARGS__)

struct Eeze_Net
{
   EINA_REFCOUNT;
   int index;
   _udev_device *device;
   const char *syspath;
   const char *name;

   const char *ip;
   const char *broadip;
   const char *netmask;
#ifdef HAVE_IPV6
   const char *ip6;
   const char *broadip6;
   const char *netmask6;
#endif
};

Eina_Bool eeze_net_init(void);
void eeze_net_shutdown(void);
#endif
