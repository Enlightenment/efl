#ifndef EEZE_NET_H
#define EEZE_NET_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#else
# define EAPI
#endif

#include <Eina.h>
#include <Ecore.h>

/**
 * @file Eeze_Net.h
 * @brief Network manipulation
 *
 * Eeze net functions allow you to gather information about network objects
 *
 * @defgroup Eeze_Net Net
 * Gather information about network devices.
 *
 * @ingroup Eeze
 * @{
 */

typedef struct Eeze_Net Eeze_Net;

typedef enum
{
   EEZE_NET_ADDR_TYPE_IP,
   EEZE_NET_ADDR_TYPE_IP6,
   EEZE_NET_ADDR_TYPE_BROADCAST,
   EEZE_NET_ADDR_TYPE_BROADCAST6,
   EEZE_NET_ADDR_TYPE_NETMASK,
   EEZE_NET_ADDR_TYPE_NETMASK6,
} Eeze_Net_Addr_Type;

#ifdef __cplusplus
extern "C" {
#endif

EAPI Eeze_Net   *eeze_net_new(const char *name);
EAPI void        eeze_net_free(Eeze_Net *net);
EAPI const char *eeze_net_mac_get(Eeze_Net *net);
EAPI int         eeze_net_idx_get(Eeze_Net *net);
EAPI Eina_Bool   eeze_net_scan(Eeze_Net *net);
EAPI const char *eeze_net_addr_get(Eeze_Net *net, Eeze_Net_Addr_Type type);
EAPI const char *eeze_net_attribute_get(Eeze_Net *net, const char *attr);
EAPI const char *eeze_net_syspath_get(Eeze_Net *net);
EAPI Eina_List  *eeze_net_list(void);

#ifdef __cplusplus
}
#endif
/** @} */

#endif
