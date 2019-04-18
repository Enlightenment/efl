#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#ifdef HAVE_NET_IF_H
# include <net/if.h>
#endif

#ifdef _WIN32
# include <ws2tcpip.h>
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

#define ECORE_CON_SOCKS_VERSION_CHECK(X)             do {    \
       if (!(X) || ((X)->version < 4) || ((X)->version > 5)) \
         return;                                             \
  } while (0)
#define ECORE_CON_SOCKS_VERSION_CHECK_RETURN(X, ret) do {    \
       if (!(X) || ((X)->version < 4) || ((X)->version > 5)) \
         return (ret);                                       \
  } while (0)

static Eina_List *ecore_con_socks_proxies = NULL;

static Ecore_Con_Socks *
_ecore_con_socks_find(unsigned char version, const char *ip, int port, const char *username, size_t ulen, const char *password, size_t plen)
{
   Eina_List *l;
   Ecore_Con_Socks_v5 *ecs;

   if (!ecore_con_socks_proxies) return NULL;

   EINA_LIST_FOREACH(ecore_con_socks_proxies, l, ecs)
     {
        if (ecs->version != version) continue;
        if (strcmp(ecs->ip, ip)) continue;
        if ((port != -1) && (port != ecs->port)) continue;
        if (ulen != ecs->ulen) continue;
        if (username && strcmp(ecs->username, username)) continue;
        if (version == 5)
          {
             if (plen != ecs->plen) continue;
             if (password && strcmp(ecs->password, password)) continue;
          }
        return (Ecore_Con_Socks *)ecs;
     }
   return NULL;
}

static void
_ecore_con_socks_free(Ecore_Con_Socks *ecs)
{
   ECORE_CON_SOCKS_VERSION_CHECK(ecs);

   if (_ecore_con_proxy_once == ecs) _ecore_con_proxy_once = NULL;
   if (_ecore_con_proxy_global == ecs) _ecore_con_proxy_global = NULL;
   eina_stringshare_del(ecs->ip);
   eina_stringshare_del(ecs->username);
   free(ecs);
}

void
ecore_con_socks_shutdown(void)
{
   Ecore_Con_Socks *ecs;
   EINA_LIST_FREE(ecore_con_socks_proxies, ecs)
     _ecore_con_socks_free(ecs);
   _ecore_con_proxy_once = NULL;
   _ecore_con_proxy_global = NULL;
}

void
ecore_con_socks_init(void)
{
   const char *socks = NULL;
   char *h, *p, *l, *u = NULL;
   char buf[512];
   int port, lookup = 0;
   Eina_Bool v5 = EINA_FALSE;
   Ecore_Con_Socks *ecs;
   unsigned char addr[sizeof(struct in_addr)];
#ifdef HAVE_IPV6
   unsigned char addr6[sizeof(struct in6_addr)];
#endif

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        /* ECORE_CON_SOCKS_V4=[user@]host-port:[1|0] */
        socks = getenv("ECORE_CON_SOCKS_V4");
        if (!socks)
          {
             /* ECORE_CON_SOCKS_V5=[user@]host-port:[1|0] */
             socks = getenv("ECORE_CON_SOCKS_V5");
             v5 = EINA_TRUE;
          }
     }
   if ((!socks) || (!socks[0]) || (strlen(socks) + 1 > 512)) return;
   memcpy(buf, socks, strlen(socks) + 1);
   h = strchr(buf, '@');
   /* username */
   if (h && (h - buf > 0)) *h++ = 0, u = buf;
   else h = buf;

   /* host ip; I ain't resolvin shit here */
   p = strchr(h, '-');
   if (!p) return;
   *p++ = 0;
   if (!inet_pton(AF_INET, h, addr))
#ifdef HAVE_IPV6
     {
        if (!v5) return;
        if (!inet_pton(AF_INET6, h, addr6))
          return;
     }
#else
     return;
#endif

   errno = 0;
   port = strtol(p, &l, 10);
   if (errno || (port < 0) || (port > 65535)) return;
   if (l && (l[0] == ':'))
     lookup = (l[1] == '1');
   if (v5)
     ecs = ecore_con_socks5_remote_add(h, port, u, NULL);
   else
     ecs = ecore_con_socks4_remote_add(h, port, u);
   if (!ecs) return;
   ecore_con_socks_lookup_set(ecs, lookup);
   ecore_con_socks_apply_always(ecs);
   INF("Added global proxy server %s%s%s:%d - DNS lookup %s",
       u ? : "", u ? "@" : "", h, port, lookup ? "ENABLED" : "DISABLED");
}

/////////////////////////////////////////////////////////////////////////////////////

/*
 * General Socks API.
 */

EAPI Ecore_Con_Socks *
ecore_con_socks4_remote_add(const char *ip, int port, const char *username)
{
   Ecore_Con_Socks *ecs;
   size_t ulen = 0;

   if ((!ip) || (!ip[0]) || (port < 0) || (port > 65535)) return NULL;

   if (username)
     {
        ulen = strlen(username);
        /* max length for protocol */
        if ((!ulen) || (ulen > 255)) return NULL;
     }
   ecs = _ecore_con_socks_find(4, ip, port, username, ulen, NULL, 0);
   if (ecs) return ecs;

   ecs = calloc(1, sizeof(Ecore_Con_Socks_v4));
   if (!ecs) return NULL;

   ecs->version = 4;
   ecs->ip = eina_stringshare_add(ip);
   ecs->port = port;
   ecs->username = eina_stringshare_add(username);
   ecs->ulen = ulen;
   ecore_con_socks_proxies = eina_list_append(ecore_con_socks_proxies, ecs);
   return ecs;
}

EAPI Eina_Bool
ecore_con_socks4_remote_exists(const char *ip, int port, const char *username)
{
   if ((!ip) || (!ip[0]) || (port < -1) || (port > 65535) || (username && (!username[0])))
     return EINA_FALSE;
   return !!_ecore_con_socks_find(4, ip, port, username, username ? strlen(username) : 0, NULL, 0);
}

EAPI void
ecore_con_socks4_remote_del(const char *ip, int port, const char *username)
{
   Ecore_Con_Socks_v4 *v4;

   if ((!ip) || (!ip[0]) || (port < -1) || (port > 65535) || (username && (!username[0]))) return;
   if (!ecore_con_socks_proxies) return;

   v4 = (Ecore_Con_Socks_v4 *)_ecore_con_socks_find(4, ip, port, username, username ? strlen(username) : 0, NULL, 0);
   if (!v4) return;
   ecore_con_socks_proxies = eina_list_remove(ecore_con_socks_proxies, v4);
   _ecore_con_socks_free((Ecore_Con_Socks *)v4);
}

EAPI Ecore_Con_Socks *
ecore_con_socks5_remote_add(const char *ip, int port, const char *username, const char *password)
{
   Ecore_Con_Socks_v5 *ecs5;
   size_t ulen = 0, plen = 0;

   if ((!ip) || (!ip[0]) || (port < 0) || (port > 65535)) return NULL;

   if (username)
     {
        ulen = strlen(username);
        /* max length for protocol */
        if ((!ulen) || (ulen > 255)) return NULL;
     }
   if (password)
     {
        plen = strlen(password);
        /* max length for protocol */
        if ((!plen) || (plen > 255)) return NULL;
     }
   ecs5 = (Ecore_Con_Socks_v5 *)_ecore_con_socks_find(5, ip, port, username, ulen, password, plen);
   if (ecs5) return (Ecore_Con_Socks *)ecs5;

   ecs5 = calloc(1, sizeof(Ecore_Con_Socks_v5));
   if (!ecs5) return NULL;

   ecs5->version = 5;
   ecs5->ip = eina_stringshare_add(ip);
   ecs5->port = port;
   ecs5->username = eina_stringshare_add(username);
   ecs5->ulen = ulen;
   ecs5->password = eina_stringshare_add(password);
   ecs5->plen = plen;
   ecore_con_socks_proxies = eina_list_append(ecore_con_socks_proxies, ecs5);
   return (Ecore_Con_Socks *)ecs5;
}

EAPI Eina_Bool
ecore_con_socks5_remote_exists(const char *ip, int port, const char *username, const char *password)
{
   if ((!ip) || (!ip[0]) || (port < -1) || (port > 65535) || (username && (!username[0])) || (password && (!password[0])))
     return EINA_FALSE;
   return !!_ecore_con_socks_find(5, ip, port, username, username ? strlen(username) : 0, password, password ? strlen(password) : 0);
}

EAPI void
ecore_con_socks5_remote_del(const char *ip, int port, const char *username, const char *password)
{
   Ecore_Con_Socks_v5 *v5;

   if ((!ip) || (!ip[0]) || (port < -1) || (port > 65535) || (username && (!username[0])) || (password && (!password[0])))
     return;
   if (!ecore_con_socks_proxies) return;

   v5 = (Ecore_Con_Socks_v5 *)_ecore_con_socks_find(5, ip, port, username, username ? strlen(username) : 0, password, password ? strlen(password) : 0);
   if (!v5) return;
   ecore_con_socks_proxies = eina_list_remove(ecore_con_socks_proxies, v5);
   _ecore_con_socks_free((Ecore_Con_Socks *)v5);
}

EAPI void
ecore_con_socks_lookup_set(Ecore_Con_Socks *ecs, Eina_Bool enable)
{
   ECORE_CON_SOCKS_VERSION_CHECK(ecs);
   ecs->lookup = !!enable;
}

EAPI Eina_Bool
ecore_con_socks_lookup_get(Ecore_Con_Socks *ecs)
{
   ECORE_CON_SOCKS_VERSION_CHECK_RETURN(ecs, EINA_FALSE);
   return ecs->lookup;
}

EAPI void
ecore_con_socks_bind_set(Ecore_Con_Socks *ecs, Eina_Bool is_bind)
{
   EINA_SAFETY_ON_NULL_RETURN(ecs);
   ECORE_CON_SOCKS_VERSION_CHECK(ecs);
   ecs->bind = !!is_bind;
}

EAPI Eina_Bool
ecore_con_socks_bind_get(Ecore_Con_Socks *ecs)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ecs, EINA_FALSE);
   ECORE_CON_SOCKS_VERSION_CHECK_RETURN(ecs, EINA_FALSE);
   return ecs->bind;
}

EAPI unsigned int
ecore_con_socks_version_get(Ecore_Con_Socks *ecs)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ecs, 0);
   ECORE_CON_SOCKS_VERSION_CHECK_RETURN(ecs, 0);
   return ecs->version;
}

EAPI void
ecore_con_socks_remote_del(Ecore_Con_Socks *ecs)
{
   EINA_SAFETY_ON_NULL_RETURN(ecs);
   if (!ecore_con_socks_proxies) return;

   ecore_con_socks_proxies = eina_list_remove(ecore_con_socks_proxies, ecs);
   _ecore_con_socks_free(ecs);
}

EAPI void
ecore_con_socks_apply_once(Ecore_Con_Socks *ecs)
{
   _ecore_con_proxy_once = ecs;
}

EAPI void
ecore_con_socks_apply_always(Ecore_Con_Socks *ecs)
{
   _ecore_con_proxy_global = ecs;
}

