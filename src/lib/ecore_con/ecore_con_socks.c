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

#if defined(_WIN32) && !defined(IF_NAMESIZE)
#define IF_NAMESIZE                        16
#endif

/* http://tools.ietf.org/html/rfc1928
          o  X'00' NO AUTHENTICATION REQUIRED
          o  X'01' GSSAPI
          o  X'02' USERNAME/PASSWORD
          o  X'03' to X'7F' IANA ASSIGNED
          o  X'80' to X'FE' RESERVED FOR PRIVATE METHODS
          o  X'FF' NO ACCEPTABLE METHODS
 */
#define ECORE_CON_SOCKS_V5_METHOD_NONE     0
#define ECORE_CON_SOCKS_V5_METHOD_GSSAPI   1
#define ECORE_CON_SOCKS_V5_METHOD_USERPASS 2

static int ECORE_CON_SOCKS_V5_METHODS[] =
{
   ECORE_CON_SOCKS_V5_METHOD_NONE,
//   ECORE_CON_SOCKS_V5_METHOD_GSSAPI, TODO
   ECORE_CON_SOCKS_V5_METHOD_USERPASS
};

#define ECORE_CON_SOCKS_V5_TOTAL_METHODS (sizeof(ECORE_CON_SOCKS_V5_METHODS) / sizeof(int))

#define _ecore_con_server_kill(svr)                  do { \
       DBG("KILL %p", (svr));                             \
       _ecore_con_server_kill((svr));                     \
  } while (0)

#define ECORE_CON_SOCKS_VERSION_CHECK(X)             do {    \
       if (!(X) || ((X)->version < 4) || ((X)->version > 5)) \
         return;                                             \
  } while (0)
#define ECORE_CON_SOCKS_VERSION_CHECK_RETURN(X, ret) do {    \
       if (!(X) || ((X)->version < 4) || ((X)->version > 5)) \
         return (ret);                                       \
  } while (0)

#define ECORE_CON_SOCKS_CAST(X)        \
  Ecore_Con_Socks_v4 * v4 = NULL;      \
  Ecore_Con_Socks_v5 *v5 = NULL;       \
  if ((X) && ((X)->version == 4))      \
    v4 = (Ecore_Con_Socks_v4 *)(X);    \
  else if ((X) && ((X)->version == 5)) \
    v5 = (Ecore_Con_Socks_v5 *)(X);

Eina_List *ecore_con_socks_proxies = NULL;

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

static Eina_Bool
_ecore_con_socks_svr_init_v4(Ecore_Con_Server *obj, Ecore_Con_Socks_v4 *v4)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   size_t addrlen, buflen, ulen = 1;
   unsigned char *sbuf;

   addrlen = v4->lookup ? strlen(svr->name) + 1 : 0;
   if (v4->username) ulen += v4->ulen;
   buflen = sizeof(char) * (8 + ulen + addrlen);
   sbuf = malloc(buflen);
   if (!sbuf)
     {
        ecore_con_event_server_error(obj, "Memory allocation failure!");
        _ecore_con_server_kill(obj);
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
     /* SOCKSv4 only handles IPV4, so addrlen is always 4 */
     memcpy(sbuf + 4, svr->ecs_addr, 4);
   if (v4->username)
     memcpy(sbuf + 8, v4->username, ulen);
   else
     sbuf[8] = 0;
   if (addrlen) memcpy(sbuf + 8 + ulen, svr->name, addrlen);

   svr->ecs_buf = eina_binbuf_manage_new_length(sbuf, buflen);
   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_socks_svr_init_v5(Ecore_Con_Server *obj, Ecore_Con_Socks_v5 *v5)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   size_t buflen;
   unsigned int x;
   unsigned char *sbuf;

   if (v5->username)
     buflen = sizeof(char) * (2 + ECORE_CON_SOCKS_V5_TOTAL_METHODS);
   else
     buflen = 3;
   sbuf = malloc(buflen);
   if (!sbuf)
     {
        ecore_con_event_server_error(obj, "Memory allocation failure!");
        _ecore_con_server_kill(obj);
        return EINA_FALSE;
     }
   /* http://en.wikipedia.org/wiki/SOCKS
    * http://tools.ietf.org/html/rfc1928
    */
   sbuf[0] = 5;
   if (v5->username)
     {
        sbuf[1] = ECORE_CON_SOCKS_V5_TOTAL_METHODS;
        for (x = 2; x < 2 + ECORE_CON_SOCKS_V5_TOTAL_METHODS; x++)
          sbuf[x] = ECORE_CON_SOCKS_V5_METHODS[x - 2];
     }
   else
     {
        sbuf[1] = 1;
        sbuf[2] = ECORE_CON_SOCKS_V5_METHOD_NONE;
     }

   svr->ecs_buf = eina_binbuf_manage_new_length(sbuf, buflen);
   return EINA_TRUE;
}

#define ECORE_CON_SOCKS_READ(EXACT)                                    \
  if (num < EXACT)                                                     \
    {                                                                  \
       if (!svr->ecs_recvbuf) svr->ecs_recvbuf = eina_binbuf_new();    \
       if (!svr->ecs_recvbuf) goto error;                              \
       eina_binbuf_append_length(svr->ecs_recvbuf, buf, num);          \
       /* the slowest connection on earth */                           \
       if (eina_binbuf_length_get(svr->ecs_recvbuf) != EXACT) return;  \
       data = eina_binbuf_string_get(svr->ecs_recvbuf);                \
    }                                                                  \
  else if (num > EXACT)                                                \
    goto error;                                                        \
  else                                                                 \
    data = buf

static void
_ecore_con_socks_read_v4(Ecore_Con_Server *obj, Ecore_Con_Socks_v4 *v4 EINA_UNUSED, const unsigned char *buf, unsigned int num)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   const unsigned char *data;
   DBG("SOCKS: %d bytes", num);
   ECORE_CON_SOCKS_READ(8);

/* http://ufasoft.com/doc/socks4_protocol.htm */
   if (data[0]) goto error;
   switch (data[1])
     {
      case 90:
        /* success! */
        break;

      case 91:
        ecore_con_event_server_error(obj, "proxy request rejected or failed");
        goto error;

      case 92:
        ecore_con_event_server_error(obj, "proxying SOCKS server could not perform authentication");
        goto error;

      case 93:
        ecore_con_event_server_error(obj, "proxy request authentication rejected");
        goto error;

      default:
        ecore_con_event_server_error(obj, "garbage data from proxy");
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
        ecore_con_event_proxy_bind(obj);
     }
   svr->ecs_state = ECORE_CON_PROXY_STATE_DONE;
   INF("PROXY CONNECTED");
   if (svr->ecs_recvbuf) eina_binbuf_free(svr->ecs_recvbuf);
   svr->ecs_recvbuf = NULL;
   svr->ecs_buf_offset = svr->ecs_addrlen = 0;
   memset(svr->ecs_addr, 0, sizeof(svr->ecs_addr));
   if (!svr->ssl_state)
     ecore_con_event_server_add(obj);
   if (svr->ssl_state || (svr->buf && eina_binbuf_length_get(svr->buf)))
     ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);
   return;
error:
   _ecore_con_server_kill(obj);
}

static Eina_Bool
_ecore_con_socks_auth_v5(Ecore_Con_Server *obj, Ecore_Con_Socks_v5 *v5)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   size_t size;
   unsigned char *data;
   switch (v5->method)
     {
      case ECORE_CON_SOCKS_V5_METHOD_NONE:
        svr->ecs_state = ECORE_CON_PROXY_STATE_REQUEST;
        return EINA_TRUE;

      case ECORE_CON_SOCKS_V5_METHOD_GSSAPI:
        return EINA_TRUE;

      case ECORE_CON_SOCKS_V5_METHOD_USERPASS:
        if (!v5->username) return EINA_FALSE;
        if (!v5->password) v5->plen = 1;
        /* http://tools.ietf.org/html/rfc1929 */
        size = sizeof(char) * (3 + v5->ulen + v5->plen);
        data = malloc(size);
        if (!data) break;
        data[0] = 1;
        data[1] = v5->ulen;
        memcpy(&data[2], v5->username, v5->ulen);
        data[1 + v5->ulen] = v5->plen;
        if (v5->password)
          memcpy(&data[2 + v5->ulen], v5->password, v5->plen);
        else
          data[2 + v5->ulen] = 0;
        svr->ecs_buf = eina_binbuf_manage_new_length(data, size);
        return EINA_TRUE;

      default:
        break;
     }
   return EINA_FALSE;
}

static void
_ecore_con_socks_read_v5(Ecore_Con_Server *obj, Ecore_Con_Socks_v5 *v5, const unsigned char *buf, unsigned int num)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   const unsigned char *data;

   DBG("SOCKS: %d bytes", num);
   switch (svr->ecs_state)
     {
      case ECORE_CON_PROXY_STATE_READ:
        ECORE_CON_SOCKS_READ(2);
        /* http://en.wikipedia.org/wiki/SOCKS */
        if (data[0] != 5) goto error;
        if (data[1] == 0xFF)
          {
             ecore_con_event_server_error(obj, "proxy authentication methods rejected");
             goto error;
          }
        v5->method = data[1];
        if (!_ecore_con_socks_auth_v5(obj, v5)) goto error;
        if (svr->ecs_state == ECORE_CON_PROXY_STATE_REQUEST)
          {
             /* run again to skip auth reading */
             _ecore_con_socks_read_v5(obj, v5, NULL, 0);
             return;
          }
        ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
        svr->ecs_state = ECORE_CON_PROXY_STATE_AUTH;
        break;

      case ECORE_CON_PROXY_STATE_AUTH:
        ECORE_CON_SOCKS_READ(2);
        switch (v5->method)
          {
           case ECORE_CON_SOCKS_V5_METHOD_NONE:
             CRI("HOW DID THIS HAPPEN?????????");
             goto error;

           case ECORE_CON_SOCKS_V5_METHOD_GSSAPI:
             /* TODO: this */
             break;

           case ECORE_CON_SOCKS_V5_METHOD_USERPASS:
             if (data[0] != 1)
               {
                  ecore_con_event_server_error(obj, "protocol error");
                  goto error;   /* wrong version */
               }
             if (data[1])
               {
                  ecore_con_event_server_error(obj, "proxy request authentication rejected");
                  goto error;
               }

           default:
             break;
          }

      case ECORE_CON_PROXY_STATE_REQUEST:
      {
         size_t addrlen, buflen;
         unsigned char *sbuf;
         addrlen = v5->lookup ? strlen(svr->name) + 1 : (unsigned int)svr->ecs_addrlen;
         buflen = sizeof(char) * (6 + addrlen);
         sbuf = malloc(buflen);
         if (!sbuf)
           {
              ecore_con_event_server_error(obj, "Memory allocation failure!");
              goto error;
           }
         sbuf[0] = 5;
         sbuf[1] = v5->bind ? 2 : 1;      /* TODO: 0x03 for UDP port association */
         sbuf[2] = 0;
         if (v5->lookup)      /* domain name */
           {
              sbuf[3] = 3;
              sbuf[4] = addrlen - 1;
              memcpy(sbuf + 5, svr->name, addrlen - 1);
           }
         else
           {
              sbuf[3] = (svr->ecs_addrlen == 4) ? 1 : 4;
              memcpy(sbuf + 4, svr->ecs_addr, addrlen);
           }
         sbuf[addrlen + 4] = svr->port >> 8;
         sbuf[addrlen + 5] = svr->port & 0xff;

         svr->ecs_buf = eina_binbuf_manage_new_length(sbuf, buflen);
         ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
         break;
      }

      case ECORE_CON_PROXY_STATE_CONFIRM:
      {
         /* this is ugly because we have to read an exact number of bytes,
          * but we don't know what that number is until we've already read
          * at least 5 bytes to determine the length of the unknown stream.
          * yep.
          */
         size_t to_read, len = svr->ecs_recvbuf ? eina_binbuf_length_get(svr->ecs_recvbuf) : 0;
         if (num + len < 5)
           {
              /* guarantees we get called again */
              ECORE_CON_SOCKS_READ(5);
           }
         if (len >= 5)
           {
              data = eina_binbuf_string_get(svr->ecs_recvbuf);
              data += 3;
           }
         else
           data = buf + 3 - len;
         switch (data[0])
           {
            case 1:
              to_read = 4;
              break;

            case 3:
              to_read = data[1] + 1;
              break;

            case 4:
              to_read = 16;
              /* lazy debugging stub comment */
              break;

            default:
              ecore_con_event_server_error(obj, "protocol error");
              goto error;
           }
         /* at this point, we finally know exactly how much we need to read */
         ECORE_CON_SOCKS_READ(6 + to_read);

         if (data[0] != 5)
           {
              ecore_con_event_server_error(obj, "protocol error");
              goto error;     /* wrong version */
           }
         switch (data[1])
           {
            case 0:
              break;

            case 1:
              ecore_con_event_server_error(obj, "general proxy failure");
              goto error;

            case 2:
              ecore_con_event_server_error(obj, "connection not allowed by ruleset");
              goto error;

            case 3:
              ecore_con_event_server_error(obj, "network unreachable");
              goto error;

            case 4:
              ecore_con_event_server_error(obj, "host unreachable");
              goto error;

            case 5:
              ecore_con_event_server_error(obj, "connection refused by destination host");
              goto error;

            case 6:
              ecore_con_event_server_error(obj, "TTL expired");
              goto error;

            case 7:
              ecore_con_event_server_error(obj, "command not supported / protocol error");
              goto error;

            case 8:
              ecore_con_event_server_error(obj, "address type not supported");

            default:
              goto error;
           }
         if (data[2])
           {
              ecore_con_event_server_error(obj, "protocol error");
              goto error;
           }
         memset(svr->ecs_addr, 0, sizeof(svr->ecs_addr));
         if (!svr->ssl_state)
           ecore_con_event_server_add(obj);
         if (svr->ssl_state || (svr->buf && eina_binbuf_length_get(svr->buf)))
           ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);
         svr->ecs_buf_offset = svr->ecs_addrlen = 0;
         svr->ecs_state = ECORE_CON_PROXY_STATE_DONE;
         INF("PROXY CONNECTED");
         break;
      }

      default:
        break;
     }
   if (svr->ecs_recvbuf) eina_binbuf_free(svr->ecs_recvbuf);
   svr->ecs_recvbuf = NULL;

   return;
error:
   _ecore_con_server_kill(obj);
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
ecore_con_socks_read(Ecore_Con_Server *obj, unsigned char *buf, int num)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   ECORE_CON_SOCKS_VERSION_CHECK(svr->ecs);
   ECORE_CON_SOCKS_CAST(svr->ecs);

   if (svr->ecs_state < ECORE_CON_PROXY_STATE_READ) return;

   if (v4) _ecore_con_socks_read_v4(obj, v4, buf, (unsigned int)num);
   else _ecore_con_socks_read_v5(obj, v5, buf, (unsigned int)num);
}

Eina_Bool
ecore_con_socks_svr_init(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   ECORE_CON_SOCKS_VERSION_CHECK_RETURN(svr->ecs, EINA_FALSE);
   ECORE_CON_SOCKS_CAST(svr->ecs);

   if (!svr->ip) return EINA_FALSE;
   if (svr->ecs_buf) return EINA_FALSE;
   if (svr->ecs_state != ECORE_CON_PROXY_STATE_INIT) return EINA_FALSE;
   ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
   if (v4) return _ecore_con_socks_svr_init_v4(obj, v4);
   return _ecore_con_socks_svr_init_v5(obj, v5);
}

void
ecore_con_socks_dns_cb(const char *canonname EINA_UNUSED, const char *ip, struct sockaddr *addr, int addrlen EINA_UNUSED, Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   svr->ip = eina_stringshare_add(ip);
   svr->ecs_state++;
   if (addr->sa_family == AF_INET)
     {
        memcpy(svr->ecs_addr, &((struct sockaddr_in *)addr)->sin_addr.s_addr, 4);
        svr->ecs_addrlen = 4;
     }
#ifdef HAVE_IPV6
   else
     {
        memcpy(svr->ecs_addr, &((struct sockaddr_in6 *)addr)->sin6_addr.s6_addr, 16);
        svr->ecs_addrlen = 16;
     }
#endif
   ecore_con_socks_svr_init(obj);
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

/**
 * @defgroup Ecore_Con_Socks_Group Ecore Connection SOCKS functions
 * @ingroup Ecore_Con_Group
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
   return !!_ecore_con_socks_find(4, ip, port, username, username ? strlen(username) : 0, NULL, 0);
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

   v4 = (Ecore_Con_Socks_v4 *)_ecore_con_socks_find(4, ip, port, username, username ? strlen(username) : 0, NULL, 0);
   if (!v4) return;
   ecore_con_socks_proxies = eina_list_remove(ecore_con_socks_proxies, v4);
   _ecore_con_socks_free((Ecore_Con_Socks *)v4);
}

/**
 * Add a SOCKS v5 proxy to the proxy list
 *
 * Use this to create (or return, if previously added) a SOCKS proxy
 * object which can be used by any ecore_con servers.
 * @param ip The ip address of the proxy (NOT DOMAIN NAME. IP ADDRESS.)
 * @param port The port to connect to on the proxy
 * @param username The username to use for the proxy (OPTIONAL)
 * @param password The password to use for the proxy (OPTIONAL)
 * @return An allocated proxy object, or NULL on failure
 * @note This object NEVER needs to be explicitly freed.
 * @since 1.2
 */
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

/**
 * Find a SOCKS v5 proxy in the proxy list
 *
 * Use this to determine if a SOCKS proxy was previously added by checking
 * the proxy list against the parameters given.
 * @param ip The ip address of the proxy (NOT DOMAIN NAME. IP ADDRESS.)
 * @param port The port to connect to on the proxy, or -1 to match the first proxy with @p ip
 * @param username The username used for the proxy (OPTIONAL)
 * @param password The password used for the proxy (OPTIONAL)
 * @return true only if a proxy exists matching the given params
 * @note This function matches slightly more loosely than ecore_con_socks5_remote_add(), and
 * ecore_con_socks5_remote_add() should be used to return the actual object.
 * @since 1.2
 */
EAPI Eina_Bool
ecore_con_socks5_remote_exists(const char *ip, int port, const char *username, const char *password)
{
   if ((!ip) || (!ip[0]) || (port < -1) || (port > 65535) || (username && (!username[0])) || (password && (!password[0])))
     return EINA_FALSE;
   return !!_ecore_con_socks_find(5, ip, port, username, username ? strlen(username) : 0, password, password ? strlen(password) : 0);
}

/**
 * Remove a SOCKS v5 proxy from the proxy list and delete it
 *
 * Use this to remove a SOCKS proxy from the proxy list by checking
 * the list against the parameters given. The proxy will then be deleted.
 * @param ip The ip address of the proxy (NOT DOMAIN NAME. IP ADDRESS.)
 * @param port The port to connect to on the proxy, or -1 to match the first proxy with @p ip
 * @param username The username used for the proxy (OPTIONAL)
 * @param password The password used for the proxy (OPTIONAL)
 * @note This function matches in the same way as ecore_con_socks4_remote_exists().
 * @warning Be aware that deleting a proxy which is being used WILL ruin your life.
 * @since 1.2
 */
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

/**
 * Set DNS lookup mode on an existing SOCKS proxy
 *
 * According to RFC, SOCKS v4 does not require that a proxy perform
 * its own DNS lookups for addresses. SOCKS v4a specifies the protocol
 * for this. SOCKS v5 allows DNS lookups.
 * If you want to enable remote DNS lookup and are sure that your
 * proxy supports it, use this function.
 * @param ecs The proxy object
 * @param enable If true, the proxy will perform the dns lookup
 * @note By default, this setting is DISABLED.
 * @since 1.2
 */
EAPI void
ecore_con_socks_lookup_set(Ecore_Con_Socks *ecs, Eina_Bool enable)
{
   ECORE_CON_SOCKS_VERSION_CHECK(ecs);
   ecs->lookup = !!enable;
}

/**
 * Get DNS lookup mode on an existing SOCKS proxy
 *
 * According to RFC, SOCKS v4 does not require that a proxy perform
 * its own DNS lookups for addresses. SOCKS v4a specifies the protocol
 * for this. SOCKS v5 allows DNS lookups.
 * This function returns whether lookups are enabled on a proxy object.
 * @param ecs The proxy object
 * @return If true, the proxy will perform the dns lookup
 * @note By default, this setting is DISABLED.
 * @since 1.2
 */
EAPI Eina_Bool
ecore_con_socks_lookup_get(Ecore_Con_Socks *ecs)
{
   ECORE_CON_SOCKS_VERSION_CHECK_RETURN(ecs, EINA_FALSE);
   return ecs->lookup;
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
   ECORE_CON_SOCKS_VERSION_CHECK(ecs);
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
   ECORE_CON_SOCKS_VERSION_CHECK_RETURN(ecs, EINA_FALSE);
   return ecs->bind;
}

/**
 * Return SOCKS version of a SOCKS proxy
 *
 * Use this function to return the SOCKS protocol version of a proxy
 * @param ecs The proxy object
 * @return 0 on error, else 4/5
 * @since 1.2
 */
EAPI unsigned int
ecore_con_socks_version_get(Ecore_Con_Socks *ecs)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ecs, 0);
   ECORE_CON_SOCKS_VERSION_CHECK_RETURN(ecs, 0);
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
 *   ECORE_CON_SOCKS_V4=[user@]server-port:lookup
 *   ECORE_CON_SOCKS_V5=[user@]server-port:lookup
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
