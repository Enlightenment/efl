#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <Eeze_Net.h>

#include "eeze_udev_private.h"
#include "eeze_net_private.h"

static Eina_Hash *eeze_nets = NULL;

Eina_Bool
eeze_net_init(void)
{
   eeze_nets = eina_hash_string_superfast_new(NULL);
   return !!eeze_nets;
}

void
eeze_net_shutdown(void)
{
   eina_hash_free(eeze_nets);
   eeze_nets = NULL;
}

/** @addtogroup net Net
 * @{
 */

/**
 * @brief Create a new net object
 * @param name The name of the underlying device (eth0, br1, etc)
 * @return A newly allocated net object, or NULL on failure
 *
 * This function creates a new net object based on @p name.
 * Only the most minimal lookups are performed at creation in order
 * to save memory.
 */
Eeze_Net *
eeze_net_new(const char *name)
{
   const char *syspath;
   const char *idx;
   _udev_enumerate *en;
   _udev_list_entry *devs, *cur;
   _udev_device *device;
   Eeze_Net *net;

   net = eina_hash_find(eeze_nets, name);
   if (net)
     {
        EINA_REFCOUNT_REF(net);
        return net;
     }

   en = udev_enumerate_new(udev);
   udev_enumerate_add_match_sysname(en, name);
   udev_enumerate_add_match_subsystem(en, "net");
   udev_enumerate_scan_devices(en);
   devs = udev_enumerate_get_list_entry(en);
   udev_list_entry_foreach(cur, devs)
     {
        const char *devname, *test;

        devname = udev_list_entry_get_name(cur);
        test = strrchr(devname, '/');
        if (strcmp(++test, name)) continue;
        device = _new_device(devname);
        syspath = eina_stringshare_add(name);
        break;
     }
   if (!device) return NULL;
   net = calloc(1, sizeof(Eeze_Net));
   if (!net) return NULL;
   EINA_REFCOUNT_INIT(net);
   net->device = device;
   net->syspath = syspath;
   net->name = eina_stringshare_add(name);
   idx = udev_device_get_sysattr_value(net->device, "ifindex");
   net->index = atoi(idx);
   eina_hash_add(eeze_nets, name, net);
   udev_enumerate_unref(en);
   return net;
}

/**
 * @brief Free a net object
 * @param net The object to free
 *
 * Use this function to free a net object.
 * @see eeze_net_new()
 * @see eeze_net_list()
 */
void
eeze_net_free(Eeze_Net *net)
{
   EINA_SAFETY_ON_NULL_RETURN(net);

   EINA_REFCOUNT_UNREF(net)
     {
        udev_device_unref(net->device);
        eina_stringshare_del(net->syspath);
        eina_stringshare_del(net->name);
        eina_stringshare_del(net->ip);
        eina_stringshare_del(net->broadip);
        eina_stringshare_del(net->netmask);
#ifdef HAVE_IPV6
        eina_stringshare_del(net->ip6);
        eina_stringshare_del(net->broadip6);
        eina_stringshare_del(net->netmask6);
#endif
        free(net);
     }
}

/**
 * @brief Get the MAC address of a net object
 * @param net The net object
 * @return The MAC address, NULL on failure
 * Use this function to retrieve the non-stringshared MAC address of @p net.
 */
const char *
eeze_net_mac_get(Eeze_Net *net)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(net, NULL);

   return udev_device_get_sysattr_value(net->device, "address");
}

/**
 * @brief Get the index of a net object
 * @param net The net object
 * @return The ifindex of the object, -1 on failure
 * Use this function to get the hardware index of @p net
 */
int
eeze_net_idx_get(Eeze_Net *net)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(net, -1);
   return net->index;
}

/**
 * @brief Scan an interface to cache its network addresses
 * @param net The net object to scan
 * @return EINA_TRUE on success, EINA_FALSE on failure
 * Use this function to scan and cache the ip address, netmask, and broadcast
 * address for an interface. This function will perform a full scan every time
 * it is called, and IPv6 addresses will be cached if Eeze was compiled with IPv6
 * support was enabled at compile time.
 * @see eeze_net_addr_get()
 */
Eina_Bool
eeze_net_scan(Eeze_Net *net)
{
   char ip[INET_ADDRSTRLEN];
#ifdef HAVE_IPV6
   char ip6[INET6_ADDRSTRLEN];
   struct sockaddr_in6 *sa6;
#endif
   int sock;
   int ioctls[5] = {SIOCGIFADDR, SIOCGIFBRDADDR, SIOCGIFNETMASK}, *i = ioctls;
   struct ifreq ifr;
   struct sockaddr_in *sa;

   EINA_SAFETY_ON_NULL_RETURN_VAL(net, EINA_FALSE);

   /* ensure that we have the right name, mostly for hahas */
   if_indextoname((unsigned int)net->index, ifr.ifr_name);
   ifr.ifr_addr.sa_family = AF_INET;
   sock = socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) return EINA_FALSE;

   if (ioctl(sock, *i++, &ifr) < 0) goto error;
   sa = (struct sockaddr_in*) & (ifr.ifr_addr);
   inet_ntop(AF_INET, (struct in_addr*)&sa->sin_addr, ip, INET_ADDRSTRLEN);
   eina_stringshare_replace_length(&net->ip, ip, INET_ADDRSTRLEN);

   if (ioctl(sock, *i++, &ifr) < 0) goto error;
   sa = (struct sockaddr_in*) & (ifr.ifr_broadaddr);
   inet_ntop(AF_INET, (struct in_addr*)&sa->sin_addr, ip, INET_ADDRSTRLEN);
   eina_stringshare_replace_length(&net->broadip, ip, INET_ADDRSTRLEN);

   if (ioctl(sock, *i++, &ifr) < 0) goto error;
   sa = (struct sockaddr_in*) & (ifr.ifr_netmask);
   inet_ntop(AF_INET, (struct in_addr*)&sa->sin_addr, ip, INET_ADDRSTRLEN);
   eina_stringshare_replace_length(&net->netmask, ip, INET_ADDRSTRLEN);

   close(sock);
#ifdef HAVE_IPV6
   ifr.ifr_addr.sa_family = AF_INET6;
   sock = socket(AF_INET6, SOCK_DGRAM, 0);
   if (sock < 0) return EINA_FALSE;

   if (ioctl(sock, *i++, &ifr) < 0) goto error;
   sa6 = (struct sockaddr_in6*) & (ifr.ifr_addr);
   inet_ntop(AF_INET6, (struct in6_addr*)&sa6->sin6_addr, ip6, INET6_ADDRSTRLEN);
   eina_stringshare_replace_length(&net->ip6, ip6, INET6_ADDRSTRLEN);

   if (ioctl(sock, *i++, &ifr) < 0) goto error;
   sa6 = (struct sockaddr_in6*) & (ifr.ifr_broadaddr);
   inet_ntop(AF_INET6, (struct in6_addr*)&sa6->sin6_addr, ip6, INET6_ADDRSTRLEN);
   eina_stringshare_replace_length(&net->broadip6, ip6, INET6_ADDRSTRLEN);

   if (ioctl(sock, *i++, &ifr) < 0) goto error;
   sa6 = (struct sockaddr_in6*) & (ifr.ifr_netmask);
   inet_ntop(AF_INET6, (struct in6_addr*)&sa6->sin6_addr, ip6, INET6_ADDRSTRLEN);
   eina_stringshare_replace_length(&net->netmask6, ip6, INET6_ADDRSTRLEN);

   close(sock);
#endif

   return EINA_TRUE;
error:
   close(sock);
   return EINA_FALSE;
}

/**
 * @brief Get the address of a net object
 * @param net The net object
 * @param type The type of address to retrieve
 * @return The stringshared address for @p net corresponding to @p type, NULL on failure
 * This function returns a value previously cached.
 * @see eeze_net_scan()
 */
const char *
eeze_net_addr_get(Eeze_Net *net, Eeze_Net_Addr_Type type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(net, NULL);

   switch (type)
     {
      case EEZE_NET_ADDR_TYPE_IP6:
#ifdef HAVE_IPV6
        return net->ip6;
#else
        return NULL;
#endif
      case EEZE_NET_ADDR_TYPE_BROADCAST:
        return net->broadip;
      case EEZE_NET_ADDR_TYPE_BROADCAST6:
#ifdef HAVE_IPV6
        return net->broadip6;
#else
        return NULL;
#endif
      case EEZE_NET_ADDR_TYPE_NETMASK:
        return net->netmask;
      case EEZE_NET_ADDR_TYPE_NETMASK6:
#ifdef HAVE_IPV6
        return net->netmask6;
#else
        return NULL;
#endif
      default:
        break;
     }
   return net->ip;
}

/**
 * @brief Get a system attribute of a net object
 * @param net The net object
 * @param attr The attribute to retrieve
 * @return The non-stringshared value of the attribute, NULL on failure
 * Use this function to perform a udev sysattr lookup on the underlying device of @p net
 */
const char *
eeze_net_attribute_get(Eeze_Net *net, const char *attr)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(net, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(attr, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!attr[0], NULL);

   return udev_device_get_sysattr_value(net->device, attr);
}

/**
 * @brief Get the /sys/ path of a net object
 * @param net The net object
 * @return The stringshared /sys/ path of the interface, NULL on failure
 */
const char *
eeze_net_syspath_get(Eeze_Net *net)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(net, NULL);

   return net->syspath;
}

/**
 * @brief Get a list of all the network interfaces available
 * @return A list of Eeze_Net objects
 * Use this function to get all network interfaces available to the application.
 * This list must be freed by the user.
 */
Eina_List *
eeze_net_list(void)
{
   struct if_nameindex *ifs, *i;
   Eina_List *ret = NULL;
   Eeze_Net *net;

   ifs = if_nameindex();
   for (i = ifs; i && i->if_name && i->if_name[0]; i++)
     {
        net = eeze_net_new(i->if_name);
        if (net) ret = eina_list_append(ret, net);
     }
   
   if_freenameindex(ifs);
   return ret;
}
/** @} */
