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

#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif

#ifdef HAVE_NET_IF_H
# include <net/if.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif

#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

Eina_List *ecore_con_socks_proxies = NULL;

static Ecore_Con_Socks *
_ecore_con_socks_find(unsigned char version, const char *ip, int port, const char *username)
{
   Eina_List *l;
   Ecore_Con_Socks *ecs;
 
   if (!ecore_con_socks_proxies) return NULL;

   EINA_LIST_FOREACH(ecore_con_socks_proxies, l, ecs)
     {
        if (ecs->version != version) continue;
        if (strcmp(ecs->ip, ip)) continue;
        if ((port != -1) && (port != ecs->port)) continue;
        if (username && strcmp(ecs->username, username)) continue;
        return ecs;
     }
   return NULL;
}

static void
_ecore_con_socks_free(Ecore_Con_Socks *ecs)
{
   ECORE_CON_SOCKS_CAST_ELSE(ecs) return;

   if (_ecore_con_proxy_once == ecs) _ecore_con_proxy_once = NULL;
   if (_ecore_con_proxy_global == ecs) _ecore_con_proxy_global = NULL;
   eina_stringshare_del(ecs->ip);
   eina_stringshare_del(ecs->username);
   free(ecs);
}
/////////////////////////////////////////////////////////////////////////////////////
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
ecore_con_socks_read(Ecore_Con_Server *svr, unsigned char *buf, int num)
{
   const unsigned char *data;
   ECORE_CON_SOCKS_CAST_ELSE(svr->ecs) return;

   if (svr->ecs_state != ECORE_CON_SOCKS_STATE_READ) return;

   if (v4)
     {
        DBG("SOCKS: %d bytes", num);
        if (num < 8)
          {
#ifdef FIONREAD
             if (!svr->ecs_recvbuf)
               svr->ecs_recvbuf = eina_binbuf_manage_new_length(buf, num);
             else
               eina_binbuf_append_length(svr->ecs_recvbuf, buf, num);
             if (!svr->ecs_recvbuf) goto error;
#else
             if (!svr->ecs_recvbuf) svr->ecs_recvbuf = eina_binbuf_new();
             if (!svr->ecs_recvbuf) goto error;
             eina_binbuf_append_length(svr->ecs_recvbuf, buf, num);
#endif
             /* the slowest connection on earth */
             if (eina_binbuf_length_get(svr->ecs_recvbuf) != 8)
               {
#ifdef FIONREAD
                  free(buf);
#endif
                  return;
               }
             data = eina_binbuf_string_get(svr->ecs_recvbuf);
          }
        else if (num > 8) goto error;
        else
          data = buf;

     /* http://ufasoft.com/doc/socks4_protocol.htm */
        if (data[0]) goto error;
        switch (data[1])
          {
           case 90:
             /* success! */
             break;
           case 91:
             ecore_con_event_server_error(svr, "proxy request rejected or failed");
             goto error;
           case 92:
             ecore_con_event_server_error(svr, "proxying SOCKS server could not perform authentication");
             goto error;
           case 93:
             ecore_con_event_server_error(svr, "proxy request authentication rejected");
             goto error;
           default:
             ecore_con_event_server_error(svr, "garbage data from proxy");
             goto error;
          }
        if (svr->ecs->bind)
          {
             unsigned int nport;
             char naddr[IF_NAMESIZE];

             memcpy(&nport, &data[2], 2);
             svr->proxyport = ntohl(nport);

             if (!inet_ntop(AF_INET, &data[4], naddr, sizeof(naddr))) goto error;
             svr->proxyip = eina_stringshare_add(naddr);
             ecore_con_event_proxy_bind(svr);
          }
        svr->ecs_state = ECORE_CON_SOCKS_STATE_DONE;
        INF("PROXY CONNECTED");
        if (svr->ecs_recvbuf) eina_binbuf_free(svr->ecs_recvbuf);
#ifdef FIONREAD
        else free(buf);
#endif
        svr->ecs_recvbuf = NULL;
        svr->ecs_buf_offset = svr->ecs_addrlen = 0;
        memset(svr->ecs_addr, 0, sizeof(svr->ecs_addr));
        if (!svr->ssl_state)
          ecore_con_event_server_add(svr);
        if (svr->ssl_state || (svr->buf && eina_binbuf_length_get(svr->buf)))
          ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);
     }
   return;
error:
#ifdef FIONREAD
   free(buf);
#endif
   _ecore_con_server_kill(svr);
}

Eina_Bool
ecore_con_socks_svr_init(Ecore_Con_Server *svr)
{
   unsigned char *sbuf;
   ECORE_CON_SOCKS_CAST_ELSE(svr->ecs) return EINA_FALSE;

   if (!svr->ip) return EINA_FALSE;
   if (svr->ecs_buf) return EINA_FALSE;
   if (svr->ecs_state != ECORE_CON_SOCKS_STATE_INIT) return EINA_FALSE;
   ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
   if (v4)
     {
        size_t addrlen, buflen, ulen = 1;
        addrlen = svr->ecs->lookup ? strlen(svr->name) + 1: 0;
        if (svr->ecs->username) ulen += strlen(svr->ecs->username);
        buflen = sizeof(char) * (8  + ulen + addrlen);
        sbuf = malloc(buflen);
        if (!sbuf)
          {
             ecore_con_event_server_error(svr, "Memory allocation failure!");
             _ecore_con_server_kill(svr);
             return EINA_FALSE;
          }
        /* http://en.wikipedia.org/wiki/SOCKS */
        sbuf[0] = 4;
        sbuf[1] = v4->bind ? 2 : 1;
        sbuf[2] = svr->port >> 8;
        sbuf[3] = svr->port & 0xff;
        if (addrlen)
          {
             sbuf[4] = sbuf[5] = sbuf[6] = 0;
             sbuf[7] = 1;
          }
        else
          memcpy(sbuf + 4, svr->ecs_addr, 4);
        if (svr->ecs->username)
          memcpy(sbuf + 8, svr->ecs->username, ulen);
        else
          sbuf[8] = 0;
        if (addrlen) memcpy(sbuf + 8 + ulen, svr->name, addrlen);
        
        svr->ecs_buf = eina_binbuf_manage_new_length(sbuf, buflen);
     }
   return EINA_TRUE;
}

void
ecore_con_socks_dns_cb(const char *canonname __UNUSED__, const char *ip, struct sockaddr *addr, int addrlen, Ecore_Con_Server *svr)
{
   svr->ip = eina_stringshare_add(ip);
   svr->ecs_addrlen = addrlen;
   svr->ecs_state++;
   if (addr->sa_family == AF_INET)
     memcpy(svr->ecs_addr, &((struct sockaddr_in *)addr)->sin_addr.s_addr, 4);
#ifdef HAVE_IPV6
   else
     memcpy(svr->ecs_addr, &((struct sockaddr_in6 *)addr)->sin6_addr.s6_addr, addrlen);
#endif
   ecore_con_socks_svr_init(svr);
}

void
ecore_con_socks_init(void)
{
   const char *socks;
   char *u, *h, *p, *l;
   char buf[64];
   int port, lookup = 0;
   Ecore_Con_Socks *ecs;
   unsigned char addr[sizeof(struct in_addr)];

   /* ECORE_CON_SOCKS_V4=user@host:port:[1|0] */
   socks = getenv("ECORE_CON_SOCKS_V4");
   if ((!socks) || (!socks[0]) || (strlen(socks) > 64)) return;
   strncpy(buf, socks, sizeof(buf));
   h = strchr(buf, '@');
   u = NULL;
   /* username */
   if (h && (h - buf > 0)) *h++ = 0, u = buf;
   else h = buf;
   
   /* host ip; I ain't resolvin shit here */
   p = strchr(h, ':');
   if (!p) return;
   *p++ = 0;
   if (!inet_pton(AF_INET, h, addr)) return;
   
   errno = 0;
   port = strtol(p, &l, 10);
   if (errno || (port < 0) || (port > 65535)) return;
   if (l && (l[0] == ':'))
     lookup = (l[1] == '1');
   ecs = ecore_con_socks4_remote_add(h, port, u);
   if (!ecs) return;
   ecore_con_socks4_lookup_set(ecs, lookup);
   ecore_con_socks_apply_always(ecs);
   INF("Added global proxy server %s%s%s:%d - DNS lookup %s",
       u ?: "", u ? "@" : "", h, port, lookup ? "ENABLED" : "DISABLED");
}

/////////////////////////////////////////////////////////////////////////////////////

/**
 * @defgroup Ecore_Con_Socks_Group Ecore Connection SOCKS functions
 * @{
 */

/**
 * Add a SOCKS v4 proxy to the proxy list
 *
 * Use this to create (or return, if previously added) a SOCKS proxy
 * object which can be used by any ecore_con servers.
 * @param ip The ip address of the proxy (NOT DOMAIN NAME. IP ADDRESS.)
 * @param port The port to connect to on the proxy
 * @param username The username to use for the proxy (OPTIONAL)
 * @return An allocated proxy object, or NULL on failure
 * @note This object NEVER needs to be explicitly freed.
 * @since 1.2
 */
EAPI Ecore_Con_Socks *
ecore_con_socks4_remote_add(const char *ip, int port, const char *username)
{
   Ecore_Con_Socks *ecs;

   if ((!ip) || (!ip[0]) || (port < 0) || (port > 65535)) return NULL;

   ecs = _ecore_con_socks_find(4, ip, port, username);
   if (ecs) return ecs;

   ecs = calloc(1, sizeof(Ecore_Con_Socks_v4));
   if (!ecs) return NULL;

   ecs->version = 4;
   ecs->ip = eina_stringshare_add(ip);
   ecs->port = port;
   ecs->username = eina_stringshare_add(username);
   ecore_con_socks_proxies = eina_list_append(ecore_con_socks_proxies, ecs);
   return ecs;
}

/**
 * Set DNS lookup mode on an existing SOCKS v4 proxy
 *
 * According to RFC, SOCKS v4 does not require that a proxy perform
 * its own DNS lookups for addresses. SOCKS v4a specifies the protocol
 * for this. If you want to enable remote DNS lookup and are sure that your
 * proxy supports it, use this function.
 * @param ecs The proxy object
 * @param enable If true, the proxy will perform the dns lookup
 * @note By default, this setting is DISABLED.
 * @since 1.2
 */
EAPI void
ecore_con_socks4_lookup_set(Ecore_Con_Socks *ecs, Eina_Bool enable)
{
   ECORE_CON_SOCKS_CAST_ELSE(ecs) return;
   v4->lookup = !!enable;
}

/**
 * Get DNS lookup mode on an existing SOCKS v4 proxy
 *
 * According to RFC, SOCKS v4 does not require that a proxy perform
 * its own DNS lookups for addresses. SOCKS v4a specifies the protocol
 * for this. This function returns whether lookups are enabled on a proxy object.
 * @param ecs The proxy object
 * @return If true, the proxy will perform the dns lookup
 * @note By default, this setting is DISABLED.
 * @since 1.2
 */
EAPI Eina_Bool
ecore_con_socks4_lookup_get(Ecore_Con_Socks *ecs)
{
   ECORE_CON_SOCKS_CAST_ELSE(ecs) return EINA_FALSE;
   return v4 ? v4->lookup : EINA_FALSE;
}

/**
 * Find a SOCKS v4 proxy in the proxy list
 *
 * Use this to determine if a SOCKS proxy was previously added by checking
 * the proxy list against the parameters given.
 * @param ip The ip address of the proxy (NOT DOMAIN NAME. IP ADDRESS.)
 * @param port The port to connect to on the proxy, or -1 to match the first proxy with @p ip
 * @param username The username used for the proxy (OPTIONAL)
 * @return true only if a proxy exists matching the given params
 * @note This function matches slightly more loosely than ecore_con_socks4_remote_add(), and
 * ecore_con_socks4_remote_add() should be used to return the actual object.
 * @since 1.2
 */
EAPI Eina_Bool
ecore_con_socks4_remote_exists(const char *ip, int port, const char *username)
{
   if ((!ip) || (!ip[0]) || (port < -1) || (port > 65535) || (username && (!username[0])))
     return EINA_FALSE;
   return !!_ecore_con_socks_find(4, ip, port, username);
}

/**
 * Remove a SOCKS v4 proxy from the proxy list and delete it
 *
 * Use this to remove a SOCKS proxy from the proxy list by checking
 * the list against the parameters given. The proxy will then be deleted.
 * @param ip The ip address of the proxy (NOT DOMAIN NAME. IP ADDRESS.)
 * @param port The port to connect to on the proxy, or -1 to match the first proxy with @p ip
 * @param username The username used for the proxy (OPTIONAL)
 * @note This function matches in the same way as ecore_con_socks4_remote_exists().
 * @warning Be aware that deleting a proxy which is being used WILL ruin your life.
 * @since 1.2
 */
EAPI void
ecore_con_socks4_remote_del(const char *ip, int port, const char *username)
{
   Ecore_Con_Socks_v4 *v4;

   if ((!ip) || (!ip[0]) || (port < -1) || (port > 65535) || (username && (!username[0]))) return;
   if (!ecore_con_socks_proxies) return;

   v4 = (Ecore_Con_Socks_v4*)_ecore_con_socks_find(4, ip, port, username);
   if (!v4) return;
   ecore_con_socks_proxies = eina_list_remove(ecore_con_socks_proxies, v4);
   _ecore_con_socks_free((Ecore_Con_Socks*)v4);
}

/**
 * Enable bind mode on a SOCKS proxy
 *
 * Use this function to enable binding a remote port for use with a remote server.
 * For more information, see http://ufasoft.com/doc/socks4_protocol.htm
 * @param ecs The proxy object
 * @param is_bind If true, the connection established will be a port binding
 * @warning Be aware that changing the operation mode of an active proxy may result in undefined behavior
 * @since 1.2
 */
EAPI void
ecore_con_socks_bind_set(Ecore_Con_Socks *ecs, Eina_Bool is_bind)
{
   EINA_SAFETY_ON_NULL_RETURN(ecs);
   ecs->bind = !!is_bind;
}

/**
 * Return bind mode of a SOCKS proxy
 *
 * Use this function to return bind mode of a proxy (binding a remote port for use with a remote server).
 * For more information, see http://ufasoft.com/doc/socks4_protocol.htm
 * @param ecs The proxy object
 * @return If true, the connection established will be a port binding
 * @since 1.2
 */
EAPI Eina_Bool
ecore_con_socks_bind_get(Ecore_Con_Socks *ecs)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ecs, EINA_FALSE);
   return ecs->bind;
}

EAPI unsigned int
ecore_con_socks_version_get(Ecore_Con_Socks *ecs)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ecs, 0);
   return ecs->version;
}

/**
 * Remove a SOCKS v4 proxy from the proxy list and delete it
 *
 * Use this to remove a SOCKS proxy from the proxy list by directly deleting the object given.
 * @param ecs The proxy object to delete
 * @warning Be aware that deleting a proxy which is being used WILL ruin your life.
 * @since 1.2
 */
EAPI void
ecore_con_socks_remote_del(Ecore_Con_Socks *ecs)
{
   EINA_SAFETY_ON_NULL_RETURN(ecs);
   if (!ecore_con_socks_proxies) return;

   ecore_con_socks_proxies = eina_list_remove(ecore_con_socks_proxies, ecs);
   _ecore_con_socks_free(ecs);
}

/**
 * Set a proxy object to be used with the next server created with ecore_con_server_connect()
 *
 * This function sets a proxy for the next ecore_con connection. After the next server is created,
 * the proxy will NEVER be applied again unless explicitly enabled.
 * @param ecs The proxy object
 * @see ecore_con_socks_apply_always()
 * @since 1.2
 */
EAPI void
ecore_con_socks_apply_once(Ecore_Con_Socks *ecs)
{
   _ecore_con_proxy_once = ecs;
}

/**
 * Set a proxy object to be used with all servers created with ecore_con_server_connect()
 *
 * This function sets a proxy for all ecore_con connections. It will always be used.
 * @param ecs The proxy object
 * @see ecore_con_socks_apply_once()
 * @since 1.2
 * @note ecore-con supports setting this through environment variables like so:
 *   ECORE_CON_SOCKS_V4=[user@]server:port:lookup
 * user is the OPTIONAL string that would be passed to the proxy as the username
 * server is the IP_ADDRESS of the proxy server
 * port is the port to connect to on the proxy server
 * lookup is 1 if the proxy should perform all DNS lookups, otherwise 0 or omitted
 */
EAPI void
ecore_con_socks_apply_always(Ecore_Con_Socks *ecs)
{
   _ecore_con_proxy_global = ecs;
}
/** @} */
