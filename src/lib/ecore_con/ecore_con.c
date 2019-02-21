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

#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif

#ifdef HAVE_SYSTEMD
# include <systemd/sd-daemon.h>
#endif

#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#ifdef _WIN32
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0 /* noop */
#endif

#ifdef HAVE_SYSTEMD
int sd_fd_index = 0;
int sd_fd_max = 0;
#endif

EWAPI Eina_Error EFL_NET_ERROR_COULDNT_RESOLVE_HOST = 0;

EWAPI Eina_Error EFL_NET_DIALER_ERROR_COULDNT_CONNECT = 0;
EWAPI Eina_Error EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY = 0;
EWAPI Eina_Error EFL_NET_DIALER_ERROR_PROXY_AUTHENTICATION_FAILED = 0;

EWAPI Eina_Error EFL_NET_SOCKET_SSL_ERROR_HANDSHAKE = 0;
EWAPI Eina_Error EFL_NET_SOCKET_SSL_ERROR_CERTIFICATE_VERIFY_FAILED = 0;

static int _ecore_con_init_count = 0;
int _ecore_con_log_dom = -1;

Eina_Bool   _efl_net_proxy_helper_can_do      (void);
int         _efl_net_proxy_helper_url_req_send(const char *url, Ecore_Thread *eth);
char      **_efl_net_proxy_helper_url_wait    (int id);
void        _efl_net_proxy_helper_init        (void);
void        _efl_net_proxy_helper_shutdown    (void);

EAPI int
ecore_con_init(void)
{
   if (++_ecore_con_init_count != 1)
     return _ecore_con_init_count;

#ifdef _WIN32
   if (!evil_init())
     return --_ecore_con_init_count;
#endif

   if (!eina_init())
     goto eina_err;

   _ecore_con_log_dom = eina_log_domain_register
         ("ecore_con", ECORE_CON_DEFAULT_LOG_COLOR);
   if (_ecore_con_log_dom < 0)
     goto ecore_con_log_error;

   if (!ecore_init())
     goto ecore_err;

   _efl_net_proxy_helper_init();

   ecore_con_mempool_init();
   ecore_con_legacy_init();

   EFL_NET_ERROR_COULDNT_RESOLVE_HOST = eina_error_msg_static_register("Couldn't resolve host name");

   EFL_NET_DIALER_ERROR_COULDNT_CONNECT = eina_error_msg_static_register("Couldn't connect to server");
   EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY = eina_error_msg_static_register("Couldn't resolve proxy name");
   EFL_NET_DIALER_ERROR_PROXY_AUTHENTICATION_FAILED = eina_error_msg_static_register("Proxy authentication failed");

   EFL_NET_SOCKET_SSL_ERROR_HANDSHAKE = eina_error_msg_static_register("Failed SSL handshake");
   EFL_NET_SOCKET_SSL_ERROR_CERTIFICATE_VERIFY_FAILED = eina_error_msg_static_register("Failed to verify peer's certificate");

#ifdef HAVE_SYSTEMD
   sd_fd_max = sd_listen_fds(0);
#endif

   eina_log_timing(_ecore_con_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   return _ecore_con_init_count;

ecore_con_log_error:
   EINA_LOG_ERR("Failed to create a log domain for Ecore Con.");
   ecore_shutdown();

 ecore_err:
   eina_shutdown();
 eina_err:
#ifdef _WIN32
   evil_shutdown();
#endif
   return --_ecore_con_init_count;
}

EAPI int
ecore_con_shutdown(void)
{
   /* _ecore_con_init_count should not go below zero. */
   if (_ecore_con_init_count < 1)
     {
        ERR("Ecore_Con Shutdown called without calling Ecore_Con Init.\n");
        return 0;
     }
   if (--_ecore_con_init_count != 0)
     return _ecore_con_init_count;

   _efl_net_proxy_helper_shutdown();

   eina_log_timing(_ecore_con_log_dom,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   ecore_con_legacy_shutdown();

   ecore_shutdown();
#ifdef _WIN32
   evil_shutdown();
#endif

   eina_log_domain_unregister(_ecore_con_log_dom);
   _ecore_con_log_dom = -1;

   eina_shutdown();

   return _ecore_con_init_count;
}

EAPI int
ecore_con_ssl_available_get(void)
{
#if HAVE_GNUTLS
   return 1;
#elif HAVE_OPENSSL
   return 2;
#else
   return 0;
#endif
}


#ifndef _WIN32
Eina_Bool
efl_net_unix_fmt(char *buf, size_t buflen, SOCKET fd, const struct sockaddr_un *addr, socklen_t addrlen)
{
   const char *src = addr->sun_path;
   socklen_t pathlen = addrlen - offsetof(struct sockaddr_un, sun_path);

   if (addr->sun_family != AF_UNIX)
     {
        ERR("unsupported address family: %d", addr->sun_family);
        return EINA_FALSE;
     }

   if (addrlen == offsetof(struct sockaddr_un, sun_path))
     {
        int r = snprintf(buf, buflen, "unnamed:" SOCKET_FMT, fd);
        if (r < 0)
          {
             ERR("snprintf(): %s", eina_error_msg_get(errno));
             return EINA_FALSE;
          }
        else if ((size_t)r > buflen)
          {
             ERR("buflen=%zu is too small, required=%d", buflen, r);
             return EINA_FALSE;
          }
        return EINA_TRUE;
     }

   if (src[0] != '\0')
     {
        if (buflen < pathlen)
          {
             ERR("buflen=%zu is too small, required=%u", buflen, pathlen);
             return EINA_FALSE;
          }
     }
   else
     {
        if (buflen < pathlen + sizeof("abstract:") - 2)
          {
             ERR("buflen=%zu is too small, required=%zu", buflen, pathlen + sizeof("abstract:") - 2);
             return EINA_FALSE;
          }
        memcpy(buf, "abstract:", sizeof("abstract:") - 1);
        buf += sizeof("abstract:") - 1;
        src++;
     }

   memcpy(buf, src, pathlen);
   buf[pathlen] = '\0';
   return EINA_TRUE;
}
#endif

/* The reverse of efl_net_ip_port_fmt().
 *
 * If was parsed, then returns EINA_TRUE, otherwise use getaddrinfo()
 * or efl_net_ip_resolve_async_new().
 */
Eina_Bool
efl_net_ip_port_parse(const char *address, struct sockaddr_storage *storage)
{
   char *str;
   const char *host, *port;
   Eina_Bool ret;

   str = strdup(address);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, EINA_FALSE);
   if (!efl_net_ip_port_split(str, &host, &port))
     {
        ERR("invalid IP:PORT address: %s", address);
        ret = EINA_FALSE;
     }
   else
     ret = efl_net_ip_port_parse_split(host, port, storage);

   free(str);
   return ret;
}

Eina_Bool
efl_net_ip_port_parse_split(const char *host, const char *port, struct sockaddr_storage *storage)
{
   int x;
   char *endptr;
   unsigned long p;

   if (!port) port = "0";

   if (strchr(host, ':')) storage->ss_family = AF_INET6;
   else storage->ss_family = AF_INET;

   errno = 0;
   p = strtoul(port, &endptr, 10);
   if ((errno) || (endptr == port) || (*endptr != '\0')) return EINA_FALSE;
   else if (p > UINT16_MAX)
     {
        ERR("invalid port number %lu (out of range)", p);
        return EINA_FALSE;
     }

   if (storage->ss_family == AF_INET6)
     {
        struct sockaddr_in6 *a = (struct sockaddr_in6 *)storage;
        a->sin6_port = eina_htons(p);
        x = inet_pton(AF_INET6, host, &a->sin6_addr);
     }
   else
     {
        struct sockaddr_in *a = (struct sockaddr_in *)storage;
        a->sin_port = eina_htons(p);
        x = inet_pton(AF_INET, host, &a->sin_addr);
     }

   return x == 1;
}

Eina_Bool
efl_net_ip_port_fmt(char *buf, size_t buflen, const struct sockaddr *addr)
{
   char p[INET6_ADDRSTRLEN];
   const void *mem;
   unsigned short port;
   int r;

   if (addr->sa_family == AF_INET)
     {
        const struct sockaddr_in *a = (const struct sockaddr_in *)addr;
        mem = &a->sin_addr;
        port = eina_ntohs(a->sin_port);
     }
   else if (addr->sa_family == AF_INET6)
     {
        const struct sockaddr_in6 *a = (const struct sockaddr_in6 *)addr;
        mem = &a->sin6_addr;
        port = eina_ntohs(a->sin6_port);
     }
   else
     {
        ERR("unsupported address family: %d", addr->sa_family);
        return EINA_FALSE;
     }

   if (!inet_ntop(addr->sa_family, mem, p, sizeof(p)))
     {
        ERR("inet_ntop(%d, %p, %p, %zd): %s",
            addr->sa_family, mem, p, sizeof(p), eina_error_msg_get(errno));
        return EINA_FALSE;
     }

   if (addr->sa_family == AF_INET)
     r = snprintf(buf, buflen, "%s:%hu", p, port);
   else
     r = snprintf(buf, buflen, "[%s]:%hu", p, port);

   if (r < 0)
     {
        ERR("could not snprintf(): %s", eina_error_msg_get(errno));
        return EINA_FALSE;
     }
   else if ((size_t)r > buflen)
     {
        ERR("buffer is too small: %zu, required %d", buflen, r);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
efl_net_ip_port_split(char *buf, const char **p_host, const char **p_port)
{
   char *host, *port;

   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(p_host, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(p_port, EINA_FALSE);

   host = buf;
   if (host[0] == '[')
     {
        /* IPv6 is: [IP]:port */
        host++;
        port = strchr(host, ']');
        if (!port) return EINA_FALSE;
        *port = '\0';
        port++;

        if (port[0] == ':')
          port++;
        else
          port = NULL;
     }
   else
     {
        port = strchr(host, ':');
        if (port)
          {
             *port = '\0';
             port++;
             if (*port == '\0') port = NULL;
          }
     }

   *p_host = host;
   *p_port = port;
   return EINA_TRUE;
}

#ifdef HAVE_SYSTEMD
Eina_Error
efl_net_ip_socket_activate_check(const char *address, int family, int type, Eina_Bool *listening)
{
   SOCKET fd = SD_LISTEN_FDS_START + sd_fd_index;
   int r;

   if (sd_fd_index >= sd_fd_max) return ENOENT;

   if (family == AF_UNIX)
     {
        char buf[sizeof(struct sockaddr_un)] = "";
        const char *sun_path;
        size_t len;

        if (strncmp(address, "abstract:", strlen("abstract:")) == 0)
          {
             const char *path = address + strlen("abstract:");
             if (strlen(path) + 2 > sizeof(buf))
               {
                  ERR("abstract path is too long: %s", path);
                  return EINVAL;
               }
             buf[0] = '\0';
             memcpy(buf + 1, path, strlen(path) + 1);
             sun_path = buf;
             len = strlen(path) + 2;
          }
        else
          {
             if (strlen(address) + 1 > sizeof(buf))
               {
                  ERR("path is too long: %s", address);
                  return EINVAL;
               }
             sun_path = address;
             len = strlen(address) + 1;
          }

        r = sd_is_socket_unix(fd, type, 0, sun_path, len);
        if (r < 0)
          {
             ERR("socket " SOCKET_FMT " is not of family=%d, type=%d", fd, family, type);
             return EINVAL;
          }
        if (listening) *listening = (r == 1);
        return 0;
     }
   else if ((family == AF_UNSPEC) || (family == AF_INET) || (family == AF_INET6))
     {
        char *str;
        const char *host, *port;
        struct sockaddr_storage sock_addr;
        struct sockaddr_storage want_addr = { .ss_family = family };
        socklen_t addrlen;
        Eina_Error err;
        int x;

        r = sd_is_socket(fd, family, type, (type == SOCK_DGRAM) ? -1 : 0);
        if (r < 0)
          {
             ERR("socket " SOCKET_FMT " is not of family=%d, type=%d", fd, family, type);
             return EINVAL;
          }
        if ((type == SOCK_DGRAM) && (listening)) *listening = EINA_FALSE;
        else if (listening) *listening = (r == 1);

        addrlen = sizeof(sock_addr);
        if (getsockname(fd, (struct sockaddr *)&sock_addr, &addrlen) != 0)
          {
             err = efl_net_socket_error_get();
             ERR("could not query socket=" SOCKET_FMT " name: %s", fd, eina_error_msg_get(err));
             return err;
          }

        str = strdup(address);
        EINA_SAFETY_ON_NULL_RETURN_VAL(str, ENOMEM);
        if (!efl_net_ip_port_split(str, &host, &port))
          {
             ERR("invalid IP:PORT address: %s", address);
             free(str);
             return EINVAL;
          }
        if (!port) port = "0";

        if (efl_net_ip_port_parse_split(host, port, &want_addr))
          {
             Eina_Bool matches;

             if (want_addr.ss_family != sock_addr.ss_family)
               {
                  ERR("socket " SOCKET_FMT " family=%d differs from wanted %d", fd, sock_addr.ss_family, want_addr.ss_family);
                  free(str);
                  return EINVAL;
               }
             else if (want_addr.ss_family == AF_INET6)
               matches = memcmp(&want_addr, &sock_addr, sizeof(struct sockaddr_in6)) == 0;
             else
               matches = memcmp(&want_addr, &sock_addr, sizeof(struct sockaddr_in)) == 0;

             if (!matches)
               {
                  char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";

                  efl_net_ip_port_fmt(buf, sizeof(buf), (struct sockaddr *)&sock_addr);
                  ERR("socket " SOCKET_FMT " address %s differs from wanted %s", fd, buf, address);
                  free(str);
                  return EINVAL;
               }

             free(str);
             return 0;
          }
        else
          {
             /*
              * NOTE: this may block, but users should be using the IP:PORT
              * as numbers, getting into the fast path above.
              *
              * This is best-try to help API to be usable, but may
              * impact the main loop execution for a while. However
              * people doing bind are expected to do so on a local
              * address, usually resolves faster without too many DNS
              * lookups.
              */
             struct addrinfo hints = {
               .ai_socktype = type,
               .ai_family = family,
               .ai_flags = AI_ADDRCONFIG | AI_V4MAPPED,
             };
             struct addrinfo *results, *itr;

             DBG("resolving '%s', it may block main loop! Consider using IP:PORT", address);
             do
               {
                  x = getaddrinfo(host, port, &hints, &results);
               }
             while ((x == EAI_AGAIN) || ((x == EAI_SYSTEM) && (errno == EINTR)));

             if (x != 0)
               {
                  ERR("couldn't resolve host='%s', port='%s': %s",
                      host, port, gai_strerror(x));
                  free(str);
                  return EINVAL;
               }

             err = EINVAL;
             for (itr = results; itr != NULL; itr = itr->ai_next)
               {
                  if (sock_addr.ss_family != itr->ai_family) continue;
                  if (memcmp(itr->ai_addr, &sock_addr, itr->ai_addrlen) == 0)
                    {
                       err = 0;
                       break;
                    }
               }
             freeaddrinfo(results);
             free(str);
             return err;
          }
     }
   else
     {
        if (listening) *listening = EINA_FALSE;
        ERR("unsupported family=%d", family);
        return EINVAL;
     }
}
#endif


static void
_cleanup_close(void *data)
{
   SOCKET *p_fd = data;
   SOCKET fd = *p_fd;
   *p_fd = INVALID_SOCKET;
   if (fd != INVALID_SOCKET) closesocket(fd);
}

SOCKET
efl_net_socket4(int domain, int type, int protocol, Eina_Bool close_on_exec)
{
   SOCKET fd = INVALID_SOCKET;

#ifdef SOCK_CLOEXEC
   if (close_on_exec) type |= SOCK_CLOEXEC;
#endif

   fd = socket(domain, type, protocol);
#if !defined(SOCK_CLOEXEC) && defined(FD_CLOEXEC)
   EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);
   if (fd != INVALID_SOCKET)
     {
        if (close_on_exec)
          {
             if (!eina_file_close_on_exec(fd, EINA_TRUE))
               {
                  int errno_bkp = errno;
                  closesocket(fd);
                  fd = INVALID_SOCKET;
                  errno = errno_bkp;
               }
          }
     }
   EINA_THREAD_CLEANUP_POP(EINA_FALSE); /* we need fd on success */
#else
   DBG("close on exec is not supported on your platform");
   (void)close_on_exec;
#endif

   return fd;
}

typedef struct _Efl_Net_Ip_Resolve_Async_Data
{
   Efl_Net_Ip_Resolve_Async_Cb cb;
   const void *data;
   char *host;
   char *port;
   struct addrinfo *result;
   struct addrinfo *hints;
   int gai_error;
} Efl_Net_Ip_Resolve_Async_Data;

static void
_efl_net_ip_resolve_async_run(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Ip_Resolve_Async_Data *d = data;

   /* allows ecore_thread_cancel() to cancel at some points, see
    * man:pthreads(7).
    *
    * no need to set cleanup functions since the main thread will
    * handle that with _efl_net_ip_resolve_async_cancel().
    */
   eina_thread_cancellable_set(EINA_TRUE, NULL);

   while (EINA_TRUE)
     {
        DBG("resolving host='%s' port='%s'", d->host, d->port);
        d->gai_error = getaddrinfo(d->host, d->port, d->hints, &d->result);
        if (d->gai_error == 0) break;
        if (d->gai_error == EAI_AGAIN) continue;
        if ((d->gai_error == EAI_SYSTEM) && (errno == EINTR)) continue;

        DBG("getaddrinfo(\"%s\", \"%s\") failed: %s", d->host, d->port, gai_strerror(d->gai_error));
        break;
     }

   eina_thread_cancellable_set(EINA_FALSE, NULL);

   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";
        const struct addrinfo *addrinfo;
        for (addrinfo = d->result; addrinfo != NULL; addrinfo = addrinfo->ai_next)
          {
             if (efl_net_ip_port_fmt(buf, sizeof(buf), addrinfo->ai_addr))
               DBG("resolved host='%s' port='%s': %s", d->host, d->port, buf);
          }
     }
}

static void
_efl_net_ip_resolve_async_data_free(Efl_Net_Ip_Resolve_Async_Data *d)
{
   free(d->hints);
   free(d->host);
   free(d->port);
   free(d);
}

static void
_efl_net_ip_resolve_async_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Ip_Resolve_Async_Data *d = data;
   d->cb((void *)d->data, d->host, d->port, d->hints, d->result, d->gai_error);
   _efl_net_ip_resolve_async_data_free(d);
}

static void
_efl_net_ip_resolve_async_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Ip_Resolve_Async_Data *d = data;
   if (d->result) freeaddrinfo(d->result);
   _efl_net_ip_resolve_async_data_free(d);
}

Ecore_Thread *
efl_net_ip_resolve_async_new(const char *host, const char *port, const struct addrinfo *hints, Efl_Net_Ip_Resolve_Async_Cb cb, const void *data)
{
   Efl_Net_Ip_Resolve_Async_Data *d;

   EINA_SAFETY_ON_NULL_RETURN_VAL(host, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(port, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, NULL);

   d = malloc(sizeof(Efl_Net_Ip_Resolve_Async_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(d, NULL);

   d->cb = cb;
   d->data = data;
   d->host = strdup(host);
   EINA_SAFETY_ON_NULL_GOTO(d->host, failed_host);
   d->port = strdup(port);
   EINA_SAFETY_ON_NULL_GOTO(d->port, failed_port);

   if (!hints) d->hints = NULL;
   else
     {
        d->hints = malloc(sizeof(struct addrinfo));
        EINA_SAFETY_ON_NULL_GOTO(d->hints, failed_hints);
        memcpy(d->hints, hints, sizeof(struct addrinfo));
     }

   d->result = NULL;

   return ecore_thread_feedback_run(_efl_net_ip_resolve_async_run,
                                    NULL,
                                    _efl_net_ip_resolve_async_end,
                                    _efl_net_ip_resolve_async_cancel,
                                    d, EINA_TRUE);

 failed_hints:
   free(d->port);
 failed_port:
   free(d->host);
 failed_host:
   free(d);
   return NULL;
}

typedef struct _Efl_Net_Connect_Async_Data
{
   Efl_Net_Connect_Async_Cb cb;
   const void *data;
   socklen_t addrlen;
   Eina_Bool close_on_exec;
   int type;
   int protocol;
   SOCKET sockfd;
   Eina_Error error;
   struct sockaddr addr[];
} Efl_Net_Connect_Async_Data;

static void
_efl_net_connect_async_run(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Connect_Async_Data *d = data;
   char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")
#ifndef _WIN32
            + sizeof(struct sockaddr_un)
#endif
            ] = "";
   int r;

   /* allows ecore_thread_cancel() to cancel at some points, see
    * man:pthreads(7).
    *
    * no need to set cleanup functions since the main thread will
    * handle that with _efl_net_connect_async_cancel().
    */
   eina_thread_cancellable_set(EINA_TRUE, NULL);

   d->error = 0;

   /* always close-on-exec since it's not a point to pass an
    * under construction socket to a child process.
    */
   d->sockfd = efl_net_socket4(d->addr->sa_family, d->type, d->protocol, EINA_TRUE);
   if (d->sockfd == INVALID_SOCKET)
     {
        d->error = efl_net_socket_error_get();
        DBG("socket(%d, %d, %d) failed: %s", d->addr->sa_family, d->type, d->protocol, eina_error_msg_get(d->error));
        return;
     }

   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
#ifndef _WIN32
        if (d->addr->sa_family == AF_UNIX)
          efl_net_unix_fmt(buf, sizeof(buf), d->sockfd, (const struct sockaddr_un *)d->addr, d->addrlen);
        else
#endif
          efl_net_ip_port_fmt(buf, sizeof(buf), d->addr);
     }

   DBG("connecting fd=" SOCKET_FMT " to %s", d->sockfd, buf);

   r = connect(d->sockfd, d->addr, d->addrlen);
   if (r != 0)
     {
        SOCKET fd = d->sockfd;
        d->error = efl_net_socket_error_get();
        d->sockfd = INVALID_SOCKET;
        /* close() is a cancellation point, thus unset sockfd before
         * closing, so the main thread _efl_net_connect_async_cancel()
         * won't close it again.
         */
        closesocket(fd);
        DBG("connect(" SOCKET_FMT ", %s) failed: %s", fd, buf, eina_error_msg_get(d->error));
        return;
     }

   DBG("connected fd=" SOCKET_FMT " to %s", d->sockfd, buf);
}

static void
_efl_net_connect_async_data_free(Efl_Net_Connect_Async_Data *d)
{
   free(d);
}

static void
_efl_net_connect_async_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Connect_Async_Data *d = data;

#ifdef FD_CLOEXEC
   /* if it wasn't a close on exec, release the socket to be passed to child */
   if ((!d->close_on_exec) && (d->sockfd != INVALID_SOCKET))
     {
        if (!eina_file_close_on_exec(d->sockfd, EINA_FALSE))
          {
             d->error = errno;
             closesocket(d->sockfd);
             d->sockfd = INVALID_SOCKET;
          }
     }
#endif
   d->cb((void *)d->data, d->addr, d->addrlen, d->sockfd, d->error);
   _efl_net_connect_async_data_free(d);
}

static void
_efl_net_connect_async_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Connect_Async_Data *d = data;
   if (d->sockfd != INVALID_SOCKET) closesocket(d->sockfd);
   _efl_net_connect_async_data_free(d);
}

Ecore_Thread *
efl_net_connect_async_new(const struct sockaddr *addr, socklen_t addrlen, int type, int protocol, Eina_Bool close_on_exec, Efl_Net_Connect_Async_Cb cb, const void *data)
{
   Efl_Net_Connect_Async_Data *d;

   EINA_SAFETY_ON_NULL_RETURN_VAL(addr, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(addrlen < 1, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, NULL);

   d = malloc(sizeof(Efl_Net_Connect_Async_Data) + addrlen);
   EINA_SAFETY_ON_NULL_RETURN_VAL(d, NULL);

   d->cb = cb;
   d->data = data;
   d->addrlen = addrlen;
   d->close_on_exec = close_on_exec;
   d->type = type;
   d->protocol = protocol;
   memcpy(d->addr, addr, addrlen);

   d->sockfd = INVALID_SOCKET;
   d->error = 0;

   return ecore_thread_run(_efl_net_connect_async_run,
                           _efl_net_connect_async_end,
                           _efl_net_connect_async_cancel,
                           d);
}

static Eina_Bool
_efl_net_ip_no_proxy(const char *host, char * const *no_proxy_strv)
{
   char * const *itr;
   size_t host_len;

   if (!no_proxy_strv)
     return EINA_FALSE;

   host_len = strlen(host);
   for (itr = no_proxy_strv; *itr != NULL; itr++)
     {
        const char *s = *itr;
        size_t slen;

        /* '*' is not a glob/pattern, it matches all */
        if (*s == '*') return EINA_TRUE;

        /* old timers use leading dot to avoid matching partial names
         * due implementation bugs not required anymore
         */
        if (*s == '.') s++;

        slen = strlen(s);
        if (slen == 0) continue;

        if (host_len < slen) continue;
        if (memcmp(host + host_len - slen, s, slen) == 0)
          {
             if (slen == host_len)
               return EINA_TRUE;
             if (host[host_len - slen - 1] == '.')
               return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}

typedef struct _Efl_Net_Ip_Connect_Async_Data
{
   Efl_Net_Connect_Async_Cb cb;
   const void *data;
   char *address;
   char *proxy;
   char *proxy_env;
   char **no_proxy_strv;
   socklen_t addrlen;
   Eina_Bool close_on_exec;
   int type;
   int protocol;
   SOCKET sockfd;
   Eina_Error error;
   union {
      struct sockaddr_in addr4;
      struct sockaddr_in6 addr6;
      struct sockaddr addr;
   };
} Efl_Net_Ip_Connect_Async_Data;

static Eina_Error
_efl_net_ip_connect(const struct addrinfo *addr, SOCKET *sockfd)
{
   SOCKET fd = INVALID_SOCKET;
   volatile Eina_Error ret = 0;

   /* always close-on-exec since it's not a point to pass an
    * under construction socket to a child process.
    */
   fd = efl_net_socket4(addr->ai_family, addr->ai_socktype, addr->ai_protocol, EINA_TRUE);
   if (fd == INVALID_SOCKET) ret = efl_net_socket_error_get();
   else
     {
        char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";
        int r;

        EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);
        if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
          {
             if (efl_net_ip_port_fmt(buf, sizeof(buf), addr->ai_addr))
               DBG("connect fd=" SOCKET_FMT " to %s", fd, buf);
          }

        r = connect(fd, addr->ai_addr, addr->ai_addrlen);
        if (r == 0)
          {
             DBG("connected fd=" SOCKET_FMT " to %s", fd, buf);
             *sockfd = fd;
          }
        else
          {
             ret = efl_net_socket_error_get();
             DBG("couldn't connect fd=" SOCKET_FMT " to %s: %s", fd, buf, eina_error_msg_get(ret));
             closesocket(fd);
          }
        EINA_THREAD_CLEANUP_POP(EINA_FALSE); /* we need sockfd on success */
     }
   return ret;
}

static Eina_Error
_efl_net_ip_resolve_and_connect(const char *host, const char *port, int type, int protocol, SOCKET *sockfd, struct sockaddr *addr, socklen_t *p_addrlen)
{
   struct addrinfo *results = NULL;
   struct addrinfo hints = {
     .ai_socktype = type,
     .ai_protocol = protocol,
     .ai_family = AF_UNSPEC,
     .ai_flags = AI_ADDRCONFIG | AI_V4MAPPED,
   };
   volatile Eina_Error ret = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
   int r;

   if (strchr(host, ':')) hints.ai_family = AF_INET6;

   do
     r = getaddrinfo(host, port, &hints, &results);
   while ((r == EAI_AGAIN) || ((r == EAI_SYSTEM) && (errno == EINTR)));

   if (r != 0)
     {
        DBG("couldn't resolve host='%s', port='%s': %s",
            host, port, gai_strerror(r));
        ret = EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
        *sockfd = INVALID_SOCKET;
     }
   else
     {
        const struct addrinfo *addrinfo;

        EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)freeaddrinfo, results);
        for (addrinfo = results; addrinfo != NULL; addrinfo = addrinfo->ai_next)
          {
             if (addrinfo->ai_socktype != type) continue;
             if (addrinfo->ai_protocol != protocol) continue;
             ret = _efl_net_ip_connect(addrinfo, sockfd);
             if (ret == 0)
               {
                  memcpy(addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
                  *p_addrlen = addrinfo->ai_addrlen;
                  break;
               }
          }
        if (ret != 0)
          {
             if (results)
               {
                  memcpy(addr, results->ai_addr, results->ai_addrlen);
                  *p_addrlen = results->ai_addrlen;
               }
             ret = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
          }
        EINA_THREAD_CLEANUP_POP(EINA_TRUE);
     }
   return ret;
}

static void
_efl_net_ip_connect_async_run_direct(Efl_Net_Ip_Connect_Async_Data *d, const char *host, const char *port)
{
   DBG("direct connection to %s:%s", host, port);
   d->error = _efl_net_ip_resolve_and_connect(host, port, d->type, d->protocol, &d->sockfd, &d->addr, &d->addrlen);
}

static Eina_Bool
_efl_net_ip_port_user_pass_split(char *buf, const char **p_host, const char **p_port, const char **p_user, const char **p_pass)
{
   char *p;

   p = strchr(buf, '@');
   if (!p)
     {
        p = buf;
        *p_user = NULL;
        *p_pass = NULL;
     }
   else
     {
        char *s;
        *p_user = buf;
        *p = '\0';
        p++;

        s = strchr(*p_user, ':');
        if (!s)
          *p_pass = NULL;
        else
          {
             *s = '\0';
             s++;
             *p_pass = s;
          }
     }

   return efl_net_ip_port_split(p, p_host, p_port);
}

typedef enum _Efl_Net_Socks4_Request_Command {
  EFL_NET_SOCKS4_REQUEST_COMMAND_CONNECT = 0x01,
  EFL_NET_SOCKS4_REQUEST_COMMAND_BIND = 0x02
} Efl_Net_Socks4_Request_Command;

typedef struct _Efl_Net_Socks4_Request {
   uint8_t version; /* = 0x4 */
   uint8_t command; /* Efl_Net_Socks4_Request_Command */
   uint16_t port;
   uint8_t ipv4[4];
   char indent[];
} Efl_Net_Socks4_Request;

typedef enum _Efl_Net_Socks4_Reply_Status {
  EFL_NET_SOCKS4_REPLY_STATUS_GRANTED = 0x5a,
  EFL_NET_SOCKS4_REPLY_STATUS_REJECTED = 0x5b,
  EFL_NET_SOCKS4_REPLY_STATUS_FAILED_INDENT = 0x5c,
  EFL_NET_SOCKS4_REPLY_STATUS_FAILED_USER = 0x5d
} Efl_Net_Socks4_Reply_Status;

typedef struct _Efl_Net_Socks4_Reply {
   uint8_t null;
   uint8_t status;
   uint16_t port;
   uint8_t ipv4[4];
} Efl_Net_Socks4_Reply;

static Eina_Bool
_efl_net_ip_connect_async_run_socks4_try(Efl_Net_Ip_Connect_Async_Data *d, const char *proxy_host, const char *proxy_port, const struct addrinfo *addrinfo, Efl_Net_Socks4_Request *request, size_t request_len)
{
   char buf[INET_ADDRSTRLEN + sizeof(":65536")];
   struct sockaddr_in *a = (struct sockaddr_in *)addrinfo->ai_addr;
   struct sockaddr_storage proxy_addr;
   socklen_t proxy_addrlen;
   SOCKET fd;
   Eina_Error err;
   volatile Eina_Bool ret = EINA_FALSE;
   ssize_t s;

   err = _efl_net_ip_resolve_and_connect(proxy_host, proxy_port, SOCK_STREAM, IPPROTO_TCP, &fd, (struct sockaddr *)&proxy_addr, &proxy_addrlen);
   if (err)
     {
        DBG("couldn't connect to socks4://%s:%s: %s", proxy_host, proxy_port, eina_error_msg_get(err));
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        return EINA_TRUE; /* no point in continuing on this error */
     }

   EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);
   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        if (efl_net_ip_port_fmt(buf, sizeof(buf), addrinfo->ai_addr))
          DBG("resolved address='%s' to %s. Connect using fd=" SOCKET_FMT " socks4://%s:%s", d->address, buf, fd, proxy_host, proxy_port);
     }

   request->port = a->sin_port;
   memcpy(request->ipv4, &a->sin_addr, 4);

   s = send(fd, (const char *)request, request_len, MSG_NOSIGNAL);
   if (s != (ssize_t)request_len)
     {
        if (s == SOCKET_ERROR)
          DBG("couldn't request connection to host=%s fd=" SOCKET_FMT " socks4://%s:%s: %s", buf, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
        else
          DBG("couldn't send proxy request: need %zu, did %zd", request_len, s);
     }
   else
     {
        Efl_Net_Socks4_Reply reply;
        s = recv(fd, (char *)&reply, sizeof(reply), MSG_NOSIGNAL);
        if (s != sizeof(reply))
          {
             if (s == SOCKET_ERROR)
               DBG("couldn't recv reply of connection to host=%s fd=" SOCKET_FMT " socks4://%s:%s: %s", buf, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
             else
               DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(reply), s);
          }
        else
          {
             if (reply.status != EFL_NET_SOCKS4_REPLY_STATUS_GRANTED)
               DBG("rejected connection to host=%s fd=" SOCKET_FMT " socks4://%s:%s: reason=%#x", buf, fd, proxy_host, proxy_port, reply.status);
             else
               {
                  memcpy(&d->addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
                  d->addrlen = addrinfo->ai_addrlen;
                  d->sockfd = fd;
                  d->error = 0;
                  ret = EINA_TRUE;
                  DBG("connected to host=%s fd=" SOCKET_FMT " socks4://%s:%s", buf, fd, proxy_host, proxy_port);
               }
          }
     }
   EINA_THREAD_CLEANUP_POP(!ret); /* we need fd on success, on failure just close it */
   return ret;
}

static void
_efl_net_ip_connect_async_run_socks4(Efl_Net_Ip_Connect_Async_Data *d, const char *host, const char *port, const char *proxy)
{
   char *str;
   const char *proxy_user, *proxy_pass, *proxy_host, *proxy_port;
   struct addrinfo *results = NULL;
   struct addrinfo hints = {
     .ai_socktype = d->type,
     .ai_protocol = d->protocol,
     .ai_family = AF_INET,
     .ai_flags = AI_ADDRCONFIG | AI_V4MAPPED,
   };
   int r;

   if (strchr(host, ':'))
     {
        DBG("SOCKSv4 only handles IPv4. Wanted host=%s", host);
        d->error = EAFNOSUPPORT;
        return;
     }

   if ((d->type != SOCK_STREAM) || (d->protocol != IPPROTO_TCP))
     {
        DBG("SOCKSv4 only accepts TCP requests. Wanted type=%#x, protocol=%#x", d->type, d->protocol);
        d->error = EPROTONOSUPPORT;
        return;
     }

   DBG("proxy connection to %s:%s using socks4://%s", host, port, proxy);

   str = strdup(proxy);
   EINA_THREAD_CLEANUP_PUSH(free, str);

   if (!_efl_net_ip_port_user_pass_split(str, &proxy_host, &proxy_port, &proxy_user, &proxy_pass))
     {
        ERR("Invalid proxy string: socks4://%s", proxy);
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }
   if (!proxy_user) proxy_user = "";
   if (!proxy_port) proxy_port = "1080";

   do
     r = getaddrinfo(host, port, &hints, &results);
   while ((r == EAI_AGAIN) || ((r == EAI_SYSTEM) && (errno == EINTR)));
   if (r != 0)
     {
        DBG("couldn't resolve host='%s', port='%s': %s",
            host, port, gai_strerror(r));
        d->error = EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
     }
   else
     {
        const struct addrinfo *addrinfo;
        Efl_Net_Socks4_Request *request;
        size_t request_len;

        d->error = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
        EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)freeaddrinfo, results);

        request_len = sizeof(Efl_Net_Socks4_Request) + strlen(proxy_user) + 1;
        request = malloc(request_len);
        if (request)
          {
             request->version = 0x04;
             request->command = EFL_NET_SOCKS4_REQUEST_COMMAND_CONNECT;
             memcpy(request->indent, proxy_user, strlen(proxy_user) + 1);
             EINA_THREAD_CLEANUP_PUSH(free, request);
             for (addrinfo = results; addrinfo != NULL; addrinfo = addrinfo->ai_next)
               {
                  if (addrinfo->ai_socktype != d->type) continue;
                  if (addrinfo->ai_protocol != d->protocol) continue;
                  if (addrinfo->ai_family != AF_INET) continue;
                  if (_efl_net_ip_connect_async_run_socks4_try(d, proxy_host, proxy_port, addrinfo, request, request_len))
                    break;
               }
             EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(request) */
          }
        EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* freeaddrinfo(results) */
     }
 end:
   EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(str) */
}

static void
_efl_net_ip_connect_async_run_socks4a(Efl_Net_Ip_Connect_Async_Data *d, const char *host, const char *port, const char *proxy)
{
   SOCKET fd = INVALID_SOCKET;
   char *str;
   const char *proxy_user, *proxy_pass, *proxy_host, *proxy_port;
   struct sockaddr_storage proxy_addr;
   socklen_t proxy_addrlen;
   Eina_Error err;
   struct addrinfo *results = NULL;
   struct addrinfo hints = {
     .ai_socktype = d->type,
     .ai_protocol = d->protocol,
     .ai_family = AF_INET,
     .ai_flags = AI_ADDRCONFIG | AI_V4MAPPED,
   };
   int r;

   if (strchr(host, ':'))
     {
        DBG("SOCKSv4 only handles IPv4. Wanted host=%s", host);
        d->error = EAFNOSUPPORT;
        return;
     }

   if ((d->type != SOCK_STREAM) || (d->protocol != IPPROTO_TCP))
     {
        DBG("SOCKSv4 only accepts TCP requests. Wanted type=%#x, protocol=%#x", d->type, d->protocol);
        d->error = EPROTONOSUPPORT;
        return;
     }

   DBG("proxy connection to %s:%s using socks4a://%s", host, port, proxy);

   str = strdup(proxy);
   EINA_THREAD_CLEANUP_PUSH(free, str);

   if (!_efl_net_ip_port_user_pass_split(str, &proxy_host, &proxy_port, &proxy_user, &proxy_pass))
     {
        ERR("Invalid proxy string: socks4a://%s", proxy);
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }
   if (!proxy_user) proxy_user = "";
   if (!proxy_port) proxy_port = "1080";

   err = _efl_net_ip_resolve_and_connect(proxy_host, proxy_port, SOCK_STREAM, IPPROTO_TCP, &fd, (struct sockaddr *)&proxy_addr, &proxy_addrlen);
   if (err)
     {
        DBG("couldn't connect to socks4a://%s: %s", proxy, eina_error_msg_get(err));
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }

   DBG("connected fd=" SOCKET_FMT " to socks4a://%s", fd, proxy);
   EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);

   /* we just resolve the port number here */
   do
     r = getaddrinfo(NULL, port, &hints, &results);
   while ((r == EAI_AGAIN) || ((r == EAI_SYSTEM) && (errno == EINTR)));
   if (r != 0)
     {
        DBG("couldn't resolve port='%s': %s", port, gai_strerror(r));
        d->error = EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
     }
   else
     {
        const struct addrinfo *addrinfo;
        Efl_Net_Socks4_Request *request;
        size_t request_len;

        d->error = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
        EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)freeaddrinfo, results);

        request_len = sizeof(Efl_Net_Socks4_Request) + strlen(proxy_user) + 1 + strlen(host) + 1;
        request = malloc(request_len);
        if (request)
          {
             request->version = 0x04;
             request->command = EFL_NET_SOCKS4_REQUEST_COMMAND_CONNECT;
             memcpy(request->indent, proxy_user, strlen(proxy_user) + 1);
             memcpy(request->indent + strlen(proxy_user) + 1, host, strlen(host) + 1);
             EINA_THREAD_CLEANUP_PUSH(free, request);
             for (addrinfo = results; addrinfo != NULL; addrinfo = addrinfo->ai_next)
               {
                  struct sockaddr_in *a = (struct sockaddr_in *)addrinfo->ai_addr;
                  ssize_t s;

                  if (addrinfo->ai_socktype != d->type) continue;
                  if (addrinfo->ai_protocol != d->protocol) continue;
                  if (addrinfo->ai_family != AF_INET) continue;

                  request->port = a->sin_port;
                  request->ipv4[0] = 0;
                  request->ipv4[1] = 0;
                  request->ipv4[2] = 0;
                  request->ipv4[3] = 255;

                  s = send(fd, (const char *)request, request_len, MSG_NOSIGNAL);
                  if (s != (ssize_t)request_len)
                    {
                       if (s == SOCKET_ERROR)
                         DBG("couldn't send proxy request: %s", eina_error_msg_get(efl_net_socket_error_get()));
                       else
                         DBG("couldn't send proxy request: need %zu, did %zd", request_len, s);
                    }
                  else
                    {
                       Efl_Net_Socks4_Reply reply;
                       s = recv(fd, (char *)&reply, sizeof(reply), MSG_NOSIGNAL);
                       if (s != sizeof(reply))
                         {
                            if (s == SOCKET_ERROR)
                              DBG("couldn't recv proxy reply: %s", eina_error_msg_get(efl_net_socket_error_get()));
                            else
                              DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(reply), s);
                         }
                       else
                         {
                            if (reply.status != EFL_NET_SOCKS4_REPLY_STATUS_GRANTED)
                              DBG("proxy rejected request status=%#x", reply.status);
                            else
                              {
                                 d->addr4.sin_family = AF_INET;
                                 d->addr4.sin_port = a->sin_port;
                                 memcpy(&d->addr4.sin_addr, reply.ipv4, 4);
                                 d->addrlen = sizeof(struct sockaddr_in);
                                 d->sockfd = fd;
                                 d->error = 0;
                              }
                         }
                    }
                  break;
               }
             EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(request) */
          }
        EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* freeaddrinfo(results) */
     }
   EINA_THREAD_CLEANUP_POP(d->sockfd == INVALID_SOCKET); /* we need fd only on success */
 end:
   EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(str) */
}

typedef enum _Efl_Net_Socks5_Auth {
  EFL_NET_SOCKS5_AUTH_NONE = 0x00,
  EFL_NET_SOCKS5_AUTH_GSSAPI = 0x01,
  EFL_NET_SOCKS5_AUTH_USER_PASS = 0x02,
  EFL_NET_SOCKS5_AUTH_FAILED = 0xff
} Efl_Net_Socks5_Auth;

typedef struct _Efl_Net_Socks5_Greeting {
   uint8_t version; /* = 0x5 */
   uint8_t auths_count;
   uint8_t auths[1]; /* series of Efl_Net_Socks5_Auth */
} Efl_Net_Socks5_Greeting;

typedef struct _Efl_Net_Socks5_Greeting_Reply {
   uint8_t version; /* = 0x5 */
   uint8_t auth; /* Efl_Net_Socks5_Auth */
} Efl_Net_Socks5_Greeting_Reply;

typedef enum _Efl_Net_Socks5_Request_Command {
  EFL_NET_SOCKS5_REQUEST_COMMAND_TCP_CONNECT = 0x01,
  EFL_NET_SOCKS5_REQUEST_COMMAND_TCP_BIND = 0x02,
  EFL_NET_SOCKS5_REQUEST_COMMAND_UDP_ASSOCIATE = 0x03
} Efl_Net_Socks5_Request_Command;

typedef enum _Efl_Net_Socks5_Address_Type {
  EFL_NET_SOCKS5_ADDRESS_TYPE_IPV4 = 0x01,
  EFL_NET_SOCKS5_ADDRESS_TYPE_NAME = 0x03,
  EFL_NET_SOCKS5_ADDRESS_TYPE_IPV6 = 0x04
} Efl_Net_Socks5_Address_Type;

typedef struct _Efl_Net_Socks5_Request {
   uint8_t version; /* = 0x5 */
   uint8_t command; /* Efl_Net_Socks5_Command */
   uint8_t reserved;
   uint8_t address_type; /* Efl_Net_Socks5_Address_Type */
   /* follows:
    *  - one of:
    *    - 4 bytes for IPv4
    *    - 16 bytes for IPv6
    *    - 1 byte (size) + N bytes of name
    *  - uint16_t port
    */
} Efl_Net_Socks5_Request;

typedef struct _Efl_Net_Socks5_Address_Ipv4 {
   uint8_t address[4];
   uint16_t port;
} Efl_Net_Socks5_Address_Ipv4;

typedef struct _Efl_Net_Socks5_Address_Ipv6 {
   uint8_t address[16];
   uint16_t port;
} Efl_Net_Socks5_Address_Ipv6;

typedef struct _Efl_Net_Socks5_Request_Ipv4 {
   Efl_Net_Socks5_Request base;
   Efl_Net_Socks5_Address_Ipv4 ipv4;
} Efl_Net_Socks5_Request_Ipv4;

 typedef struct _Efl_Net_Socks5_Request_Ipv6 {
   Efl_Net_Socks5_Request base;
   Efl_Net_Socks5_Address_Ipv6 ipv6;
} Efl_Net_Socks5_Request_Ipv6;

static Efl_Net_Socks5_Request *
efl_net_socks5_request_addr_new(Efl_Net_Socks5_Request_Command command, const struct sockaddr *addr, size_t *p_request_len)
{
   if (addr->sa_family == AF_INET)
     {
        const struct sockaddr_in *a = (const struct sockaddr_in *)addr;
        Efl_Net_Socks5_Request_Ipv4 *request;

        *p_request_len = sizeof(Efl_Net_Socks5_Request_Ipv4);
        request = malloc(*p_request_len);
        EINA_SAFETY_ON_NULL_RETURN_VAL(request, NULL);
        request->base.version = 0x05;
        request->base.command = command;
        request->base.reserved = 0;
        request->base.address_type = EFL_NET_SOCKS5_ADDRESS_TYPE_IPV4;
        memcpy(request->ipv4.address, &a->sin_addr, 4);
        request->ipv4.port = a->sin_port;
        return &request->base;
     }
   else
     {
        const struct sockaddr_in6 *a = (const struct sockaddr_in6 *)addr;
        Efl_Net_Socks5_Request_Ipv6 *request;

        *p_request_len = sizeof(Efl_Net_Socks5_Request_Ipv6);
        request = malloc(*p_request_len);
        EINA_SAFETY_ON_NULL_RETURN_VAL(request, NULL);
        request->base.version = 0x05;
        request->base.command = command;
        request->base.reserved = 0;
        request->base.address_type = EFL_NET_SOCKS5_ADDRESS_TYPE_IPV6;
        memcpy(request->ipv6.address, &a->sin6_addr, 16);
        request->ipv6.port = a->sin6_port;
        return &request->base;
     }
}

/* port must be network endianess */
static Efl_Net_Socks5_Request *
efl_net_socks5_request_name_new(Efl_Net_Socks5_Request_Command command, const char *name, uint16_t port, size_t *p_request_len)
{
   Efl_Net_Socks5_Request *request;
   uint8_t namelen = strlen(name);
   uint8_t *p;

   *p_request_len = sizeof(Efl_Net_Socks5_Request) + 1 + namelen + 2;
   request = malloc(*p_request_len);
   EINA_SAFETY_ON_NULL_RETURN_VAL(request, NULL);
   request->version = 0x05;
   request->command = command;
   request->reserved = 0;
   request->address_type = EFL_NET_SOCKS5_ADDRESS_TYPE_NAME;

   p = (uint8_t *)request + sizeof(Efl_Net_Socks5_Request);
   *p = namelen;
   p++;
   memcpy(p, name, namelen);
   p += namelen;

   memcpy(p, &port, sizeof(port));
   return request;
}

typedef enum _Efl_Net_Socks5_Reply_Status {
  EFL_NET_SOCKS5_REPLY_STATUS_GRANTED = 0x00,
  EFL_NET_SOCKS5_REPLY_STATUS_GENERAL_FAILURE = 0x01,
  EFL_NET_SOCKS5_REPLY_STATUS_REJECTED_BY_RULESET = 0x02,
  EFL_NET_SOCKS5_REPLY_STATUS_NETWORK_UNREACHABLE = 0x03,
  EFL_NET_SOCKS5_REPLY_STATUS_HOST_UNREACHABLE = 0x04,
  EFL_NET_SOCKS5_REPLY_STATUS_CONNECTION_REFUSED = 0x05,
  EFL_NET_SOCKS5_REPLY_STATUS_TTL_EXPIRED = 0x06,
  EFL_NET_SOCKS5_REPLY_STATUS_PROTOCOL_ERROR = 0x07,
  EFL_NET_SOCKS5_REPLY_STATUS_ADDRESS_TYPE_UNSUPORTED = 0x08,
} Efl_Net_Socks5_Reply_Status;

typedef struct _Efl_Net_Socks5_Reply {
   uint8_t version; /* = 0x5 */
   uint8_t status;
   uint8_t null; /* = 0 */
   uint8_t address_type; /* Efl_Net_Socks5_Address_Type */
   /* follows:
    *  - one of:
    *    - 4 bytes for IPv4
    *    - 16 bytes for IPv6
    *    - 1 byte (size) + N bytes name
    *  - uint16_t port
    */
} Efl_Net_Socks5_Reply;

typedef struct _Efl_Net_Socks5_Reply_Ipv4 {
   Efl_Net_Socks5_Reply base;
   Efl_Net_Socks5_Address_Ipv4 ipv4;
} Efl_Net_Socks5_Reply_Ipv4;

 typedef struct _Efl_Net_Socks5_Reply_Ipv6 {
   Efl_Net_Socks5_Reply base;
   Efl_Net_Socks5_Address_Ipv6 ipv6;
} Efl_Net_Socks5_Reply_Ipv6;

static Eina_Bool
_efl_net_ip_connect_async_run_socks5_auth_user_pass(SOCKET fd, const char *user, const char *pass, const char *proxy_protocol, const char *proxy_host, const char *proxy_port)
{
   volatile uint8_t user_len = user ? strlen(user) : 0;
   volatile uint8_t pass_len = pass ? strlen(pass) : 0;
   size_t len = 1 + 1 + user_len + 1 + pass_len;
   char *msg;
   volatile Eina_Bool ret = EINA_FALSE;
   ssize_t s;

   msg = malloc(len);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, EINA_FALSE);
   EINA_THREAD_CLEANUP_PUSH(free, msg);

   msg[0] = 0x01; /* version */
   msg[1] = user_len;
   if (user) memcpy(msg + 1 + 1, user, user_len);
   msg[1 + 1 + user_len] = pass_len;
   if (pass) memcpy(msg + 1 + 1 + user_len + 1, pass, pass_len);

   DBG("authenticate user='%s' pass=%hhu (bytes) to proxy %s://%s:%s", user, pass_len, proxy_protocol, proxy_host, proxy_port);

   s = send(fd, msg, len, MSG_NOSIGNAL);
   if (s != (ssize_t)len)
     {
        if (s == SOCKET_ERROR)
          DBG("couldn't send user-password authentication to fd=" SOCKET_FMT " %s://%s:%s: %s", fd, proxy_protocol, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
        else
          DBG("couldn't send user-password authentication: need %zu, did %zd", len, s);
     }
   else
     {
        uint8_t reply[2];

        s = recv(fd, (char *)&reply, sizeof(reply), MSG_NOSIGNAL);
        if (s != (ssize_t)sizeof(reply))
          {
             if (s == SOCKET_ERROR)
               DBG("couldn't recv user-password authentication reply from fd=" SOCKET_FMT " %s://%s:%s: %s", fd, proxy_protocol, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
             else
               DBG("couldn't recv user-password authentication reply: need %zu, did %zd", len, s);
          }
        else
          {
             if (reply[1] != 0)
               DBG("proxy authentication failed user='%s' pass=%hhu (bytes) to proxy %s://%s:%s: reason=%#x", user, pass_len, proxy_protocol, proxy_host, proxy_port, reply[1]);
             else
               {
                DBG("successfully authenticated user=%s with proxy fd=" SOCKET_FMT " %s://%s:%s", user, fd, proxy_protocol, proxy_host, proxy_port);
                ret = EINA_TRUE;
             }
          }
     }

   EINA_THREAD_CLEANUP_POP(EINA_TRUE);

   return ret;
}

static Eina_Bool
_efl_net_ip_connect_async_run_socks5_try(Efl_Net_Ip_Connect_Async_Data *d, const char *proxy_host, const char *proxy_port, const char *proxy_user, const char *proxy_pass, Efl_Net_Socks5_Request_Command cmd, const struct addrinfo *addrinfo)
{
   char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")];
   Efl_Net_Socks5_Greeting greeting = {
     .version = 0x05,
     .auths_count = 1,
     .auths = { proxy_user ? EFL_NET_SOCKS5_AUTH_USER_PASS : EFL_NET_SOCKS5_AUTH_NONE },
   };
   struct sockaddr_storage proxy_addr;
   socklen_t proxy_addrlen;
   SOCKET fd;
   Eina_Error err;
   volatile Eina_Bool ret = EINA_FALSE;
   ssize_t s;

   err = _efl_net_ip_resolve_and_connect(proxy_host, proxy_port, SOCK_STREAM, IPPROTO_TCP, &fd, (struct sockaddr *)&proxy_addr, &proxy_addrlen);
   if (err)
     {
        DBG("couldn't connect to socks5://%s:%s: %s", proxy_host, proxy_port, eina_error_msg_get(err));
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        return EINA_TRUE; /* no point in continuing on this error */
     }

   EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);
   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        if (efl_net_ip_port_fmt(buf, sizeof(buf), addrinfo->ai_addr))
          DBG("resolved address='%s' to %s. Connect using fd=" SOCKET_FMT " socks5://%s:%s", d->address, buf, fd, proxy_host, proxy_port);
     }

   s = send(fd, (const char *)&greeting, sizeof(greeting), MSG_NOSIGNAL);
   if (s != (ssize_t)sizeof(greeting))
     {
        if (s == SOCKET_ERROR)
          DBG("couldn't request connection to host=%s fd=" SOCKET_FMT " socks5://%s:%s: %s", buf, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
        else
          DBG("couldn't send proxy request: need %zu, did %zd", sizeof(greeting), s);
     }
   else
     {
        Efl_Net_Socks5_Greeting_Reply greeting_reply;
        s = recv(fd, (char *)&greeting_reply, sizeof(greeting_reply), MSG_NOSIGNAL);
        if (s != sizeof(greeting_reply))
          {
             if (s == SOCKET_ERROR)
               DBG("couldn't recv greeting reply of connection to host=%s fd=" SOCKET_FMT " socks5://%s:%s: %s", buf, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
             else
               DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(greeting_reply), s);
          }
        else
          {
             if (greeting_reply.auth != greeting.auths[0])
               DBG("proxy server rejected authentication %#x trying connection to host=%s fd=" SOCKET_FMT " socks5://%s:%s", greeting.auths[0], buf, fd, proxy_host, proxy_port);
             else
               {
                  if ((greeting_reply.auth == EFL_NET_SOCKS5_AUTH_USER_PASS) &&
                      (!_efl_net_ip_connect_async_run_socks5_auth_user_pass(fd, proxy_user, proxy_pass, "socks5", proxy_host, proxy_port)))
                    {
                       d->error = EFL_NET_DIALER_ERROR_PROXY_AUTHENTICATION_FAILED;
                    }
                  else
                    {
                       Efl_Net_Socks5_Request *request;
                       size_t request_len;

                       request = efl_net_socks5_request_addr_new(cmd, addrinfo->ai_addr, &request_len);
                       if (request)
                         {
                            EINA_THREAD_CLEANUP_PUSH(free, request);

                            s = send(fd, (const char *)request, request_len, MSG_NOSIGNAL);
                            if (s != (ssize_t)request_len)
                              {
                                 if (s == SOCKET_ERROR)
                                   DBG("couldn't request connection to host=%s fd=" SOCKET_FMT " socks5://%s:%s: %s", buf, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
                                 else
                                   DBG("couldn't send proxy request: need %zu, did %zd", request_len, s);
                              }
                            else if (addrinfo->ai_family == AF_INET)
                              {
                                 Efl_Net_Socks5_Reply_Ipv4 reply;
                                 s = recv(fd, (char *)&reply, sizeof(reply), MSG_NOSIGNAL);
                                 if (s != sizeof(reply))
                                   {
                                      if (s == SOCKET_ERROR)
                                        DBG("couldn't recv reply of connection to host=%s fd=" SOCKET_FMT " socks5://%s:%s: %s", buf, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
                                      else
                                        DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(reply), s);
                                   }
                                 else
                                   {
                                      if (reply.base.status != EFL_NET_SOCKS5_REPLY_STATUS_GRANTED)
                                        DBG("rejected IPv4 connection to host=%s fd=" SOCKET_FMT " socks5://%s:%s: reason=%#x", buf, fd, proxy_host, proxy_port, reply.base.status);
                                      else
                                        {
                                           memcpy(&d->addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
                                           d->addrlen = addrinfo->ai_addrlen;
                                           d->sockfd = fd;
                                           d->error = 0;
                                           ret = EINA_TRUE;
                                           DBG("connected IPv4 to host=%s fd=" SOCKET_FMT " socks5://%s:%s", buf, fd, proxy_host, proxy_port);
                                        }
                                   }
                              }
                            else if (addrinfo->ai_family == AF_INET6)
                              {
                                 Efl_Net_Socks5_Reply_Ipv6 reply;
                                 s = recv(fd, (char *)&reply, sizeof(reply), MSG_NOSIGNAL);
                                 if (s != sizeof(reply))
                                   {
                                      if (s == SOCKET_ERROR)
                                        DBG("couldn't recv reply of connection to host=%s fd=" SOCKET_FMT " socks5://%s:%s: %s", buf, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
                                      else
                                        DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(reply), s);
                                   }
                                 else
                                   {
                                      if (reply.base.status != EFL_NET_SOCKS5_REPLY_STATUS_GRANTED)
                                        DBG("rejected IPv6 connection to host=%s fd=" SOCKET_FMT " socks5://%s:%s: reason=%#x", buf, fd, proxy_host, proxy_port, reply.base.status);
                                      else
                                        {
                                           memcpy(&d->addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
                                           d->addrlen = addrinfo->ai_addrlen;
                                           d->sockfd = fd;
                                           d->error = 0;
                                           ret = EINA_TRUE;
                                           DBG("connected IPv6 to host=%s fd=" SOCKET_FMT " socks5://%s:%s", buf, fd, proxy_host, proxy_port);
                                        }
                                   }
                              }
                            EINA_THREAD_CLEANUP_POP(EINA_TRUE);
                         }
                    }
               }
          }
     }
   EINA_THREAD_CLEANUP_POP(!ret); /* we need fd on success, on failure just close it */
   return ret;
}

static void
_efl_net_ip_connect_async_run_socks5(Efl_Net_Ip_Connect_Async_Data *d, const char *host, const char *port, const char *proxy)
{
   char *str;
   const char *proxy_user, *proxy_pass, *proxy_host, *proxy_port;
   struct addrinfo *results = NULL;
   struct addrinfo hints = {
     .ai_socktype = d->type,
     .ai_protocol = d->protocol,
     .ai_family = AF_UNSPEC,
     .ai_flags = AI_ADDRCONFIG | AI_V4MAPPED,
   };
   Efl_Net_Socks5_Request_Command cmd;
   int r;

   if ((d->type == SOCK_STREAM) && (d->protocol == IPPROTO_TCP))
     cmd = EFL_NET_SOCKS5_REQUEST_COMMAND_TCP_CONNECT;
   else
     {
        DBG("EFL SOCKSv5 only accepts TCP requests at this moment. Wanted type=%#x, protocol=%#x", d->type, d->protocol);
        d->error = EPROTONOSUPPORT;
        return;
     }

   if (strchr(host, ':')) hints.ai_family = AF_INET6;

   DBG("proxy connection to %s:%s using socks5://%s", host, port, proxy);

   str = strdup(proxy);
   EINA_THREAD_CLEANUP_PUSH(free, str);

   if (!_efl_net_ip_port_user_pass_split(str, &proxy_host, &proxy_port, &proxy_user, &proxy_pass))
     {
        ERR("Invalid proxy string: socks5://%s", proxy);
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }
   if (!proxy_port) proxy_port = "1080";

   do
     r = getaddrinfo(host, port, &hints, &results);
   while ((r == EAI_AGAIN) || ((r == EAI_SYSTEM) && (errno == EINTR)));
   if (r != 0)
     {
        DBG("couldn't resolve host='%s', port='%s': %s",
            host, port, gai_strerror(r));
        d->error = EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
     }
   else
     {
        const struct addrinfo *addrinfo;

        d->error = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
        EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)freeaddrinfo, results);
        for (addrinfo = results; addrinfo != NULL; addrinfo = addrinfo->ai_next)
          {
             if (addrinfo->ai_socktype != d->type) continue;
             if (addrinfo->ai_protocol != d->protocol) continue;
             if (_efl_net_ip_connect_async_run_socks5_try(d, proxy_host, proxy_port, proxy_user, proxy_pass, cmd, addrinfo))
               break;
          }
        EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* freeaddrinfo(results) */
     }
 end:
   EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(str) */
}

static void
_efl_net_ip_connect_async_run_socks5h(Efl_Net_Ip_Connect_Async_Data *d, const char *host, const char *port, const char *proxy)
{
   SOCKET fd = INVALID_SOCKET;
   char *str;
   const char *proxy_user, *proxy_pass, *proxy_host, *proxy_port;
   struct sockaddr_storage proxy_addr;
   socklen_t proxy_addrlen;
   Eina_Error err;
   Efl_Net_Socks5_Greeting greeting = {
     .version = 0x05,
     .auths_count = 1,
   };
   Efl_Net_Socks5_Request_Command cmd;
   ssize_t s;
   int r;

   if ((d->type == SOCK_STREAM) && (d->protocol == IPPROTO_TCP))
     cmd = EFL_NET_SOCKS5_REQUEST_COMMAND_TCP_CONNECT;
   else
     {
        DBG("EFL SOCKSv5 only accepts TCP requests at this moment. Wanted type=%#x, protocol=%#x", d->type, d->protocol);
        d->error = EPROTONOSUPPORT;
        return;
     }

   DBG("proxy connection to %s:%s using socks5h://%s", host, port, proxy);

   str = strdup(proxy);
   EINA_THREAD_CLEANUP_PUSH(free, str);

   if (!_efl_net_ip_port_user_pass_split(str, &proxy_host, &proxy_port, &proxy_user, &proxy_pass))
     {
        ERR("Invalid proxy string: socks5h://%s", proxy);
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }
   if (!proxy_port) proxy_port = "1080";

   greeting.auths[0] = proxy_user ? EFL_NET_SOCKS5_AUTH_USER_PASS : EFL_NET_SOCKS5_AUTH_NONE;

   err = _efl_net_ip_resolve_and_connect(proxy_host, proxy_port, SOCK_STREAM, IPPROTO_TCP, &fd, (struct sockaddr *)&proxy_addr, &proxy_addrlen);
   if (err)
     {
        DBG("couldn't connect to socks5h://%s: %s", proxy, eina_error_msg_get(err));
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }

   DBG("connected fd=" SOCKET_FMT " to socks5h://%s", fd, proxy);
   EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);

   s = send(fd, (const char *)&greeting, sizeof(greeting), MSG_NOSIGNAL);
   if (s != (ssize_t)sizeof(greeting))
     {
        if (s == SOCKET_ERROR)
          DBG("couldn't request connection to host=%s:%s fd=" SOCKET_FMT " socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
        else
          DBG("couldn't send proxy request: need %zu, did %zd", sizeof(greeting), s);
     }
   else
     {
        Efl_Net_Socks5_Greeting_Reply greeting_reply;
        s = recv(fd, (char *)&greeting_reply, sizeof(greeting_reply), MSG_NOSIGNAL);
        if (s != sizeof(greeting_reply))
          {
             if (s == SOCKET_ERROR)
               DBG("couldn't recv greeting reply of connection to host=%s:%s fd=" SOCKET_FMT " socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
             else
               DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(greeting_reply), s);
          }
        else
          {
             if (greeting_reply.auth != greeting.auths[0])
               DBG("proxy server rejected authentication %#x trying connection to host=%s:%s fd=" SOCKET_FMT " socks5h://%s:%s", greeting.auths[0], host, port, fd, proxy_host, proxy_port);
             else
               {
                  if ((greeting_reply.auth == EFL_NET_SOCKS5_AUTH_USER_PASS) &&
                      (!_efl_net_ip_connect_async_run_socks5_auth_user_pass(fd, proxy_user, proxy_pass, "socks5h", proxy_host, proxy_port)))
                    {
                       d->error = EFL_NET_DIALER_ERROR_PROXY_AUTHENTICATION_FAILED;
                    }
                  else
                    {
                       struct addrinfo *results = NULL;
                       struct addrinfo hints = {
                         .ai_socktype = d->type,
                         .ai_protocol = d->protocol,
                         .ai_family = AF_UNSPEC,
                         .ai_flags = AI_ADDRCONFIG | AI_V4MAPPED,
                       };

                       if (strchr(host, ':')) hints.ai_family = AF_INET6;

                       /* we just resolve the port number here */
                       do
                         r = getaddrinfo(NULL, port, &hints, &results);
                       while ((r == EAI_AGAIN) || ((r == EAI_SYSTEM) && (errno == EINTR)));
                       if (r != 0)
                         {
                            DBG("couldn't resolve port='%s': %s", port, gai_strerror(r));
                            d->error = EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
                         }
                       else
                         {
                            const struct addrinfo *addrinfo;

                            d->error = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
                            EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)freeaddrinfo, results);
                            for (addrinfo = results; addrinfo != NULL; addrinfo = addrinfo->ai_next)
                              {
                                 Efl_Net_Socks5_Request *request;
                                 size_t request_len;
                                 uint16_t port_num;

                                 if (addrinfo->ai_socktype != d->type) continue;
                                 if (addrinfo->ai_protocol != d->protocol) continue;

                                 if (addrinfo->ai_family == AF_INET)
                                   port_num = ((const struct sockaddr_in *)addrinfo->ai_addr)->sin_port;
                                 else
                                   port_num = ((const struct sockaddr_in6 *)addrinfo->ai_addr)->sin6_port;

                                 request = efl_net_socks5_request_name_new(cmd, host, port_num, &request_len);
                                 if (request)
                                   {
                                      EINA_THREAD_CLEANUP_PUSH(free, request);

                                      s = send(fd, (const char *)request, request_len, MSG_NOSIGNAL);
                                      if (s != (ssize_t)request_len)
                                        {
                                           if (s == SOCKET_ERROR)
                                             DBG("couldn't request connection to host=%s:%s fd=" SOCKET_FMT " socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
                                           else
                                             DBG("couldn't send proxy request: need %zu, did %zd", request_len, s);
                                        }
                                      else
                                        {
                                           Efl_Net_Socks5_Reply reply;

                                           s = recv(fd, (char *)&reply, sizeof(reply), MSG_NOSIGNAL);
                                           if (s != sizeof(reply))
                                             {
                                                if (s == SOCKET_ERROR)
                                                  DBG("couldn't recv reply of connection to host=%s:%s fd=" SOCKET_FMT " socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
                                                else
                                                  DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(reply), s);
                                             }
                                           else
                                             {
                                                if (reply.status != EFL_NET_SOCKS5_REPLY_STATUS_GRANTED)
                                                  DBG("rejected connection to host=%s:%s fd=" SOCKET_FMT " socks5h://%s:%s: reason=%#x", host, port, fd, proxy_host, proxy_port, reply.status);
                                                else if (reply.address_type == EFL_NET_SOCKS5_ADDRESS_TYPE_IPV4)
                                                  {
                                                     Efl_Net_Socks5_Address_Ipv4 ipv4;

                                                     s = recv(fd, (char *)&ipv4, sizeof(ipv4), MSG_NOSIGNAL);
                                                     if (s != sizeof(ipv4))
                                                       {
                                                          if (s == SOCKET_ERROR)
                                                            DBG("couldn't recv ipv4 of connection to host=%s:%s fd=" SOCKET_FMT " socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
                                                          else
                                                            DBG("couldn't recv proxy ipv4: need %zu, did %zd", sizeof(ipv4), s);
                                                       }
                                                     else
                                                       {
                                                          d->addr4.sin_family = AF_INET;
                                                          d->addr4.sin_port = ipv4.port;
                                                          memcpy(&d->addr4.sin_addr, ipv4.address, 4);
                                                          d->addrlen = sizeof(struct sockaddr_in);
                                                          d->sockfd = fd;
                                                          d->error = 0;
                                                          DBG("connected IPv4 to host=%s:%s fd=" SOCKET_FMT " socks5h://%s:%s", host, port, fd, proxy_host, proxy_port);
                                                       }
                                                  }
                                                else if (reply.address_type == EFL_NET_SOCKS5_ADDRESS_TYPE_IPV6)
                                                  {
                                                     Efl_Net_Socks5_Address_Ipv6 ipv6;

                                                     s = recv(fd, (char *)&ipv6, sizeof(ipv6), MSG_NOSIGNAL);
                                                     if (s != sizeof(ipv6))
                                                       {
                                                          if (s == SOCKET_ERROR)
                                                            DBG("couldn't recv ipv6 of connection to host=%s:%s fd=" SOCKET_FMT " socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, eina_error_msg_get(efl_net_socket_error_get()));
                                                          else
                                                            DBG("couldn't recv proxy ipv6: need %zu, did %zd", sizeof(ipv6), s);
                                                       }
                                                     else
                                                       {
                                                          d->addr6.sin6_family = AF_INET;
                                                          d->addr6.sin6_port = ipv6.port;
                                                          memcpy(&d->addr6.sin6_addr, ipv6.address, 16);
                                                          d->addrlen = sizeof(struct sockaddr_in);
                                                          d->sockfd = fd;
                                                          d->error = 0;
                                                          DBG("connected IPv6 to host=%s:%s fd=" SOCKET_FMT " socks5h://%s:%s", host, port, fd, proxy_host, proxy_port);
                                                       }
                                                  }
                                                else
                                                  {
                                                     /* most proxy servers will return a failure instead of this, but let's guard and log */
                                                     DBG("couldn't resolve host %s:%s fd=" SOCKET_FMT " socks5h://%s:%s",  host, port, fd, proxy_host, proxy_port);
                                                  }
                                             }
                                        }

                                      EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(request) */
                                      break;
                                   }
                              }
                            EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* freeaddrinfo(results) */
                         }
                    }
               }
          }
     }
   EINA_THREAD_CLEANUP_POP(d->sockfd == INVALID_SOCKET); /* we need fd only on success */
 end:
   EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(str) */
}

static void
_efl_net_ip_connect_async_run(void *data, Ecore_Thread *thread)
{
   Efl_Net_Ip_Connect_Async_Data *d = data;
   const char *host, *port, *proxy;
   char *addrcopy;
   char **proxies = NULL;
   volatile int proxies_idx = 0;
   volatile Eina_Bool is_libproxy = EINA_FALSE;

   addrcopy = strdup(d->address);
   if (!addrcopy)
     {
        d->error = errno;
        return;
     }

   if (!efl_net_ip_port_split(addrcopy, &host, &port))
     {
        d->error = EINVAL;
        free(addrcopy);
        return;
     }
   if (!port) port = "0";
   EINA_THREAD_CLEANUP_PUSH(free, addrcopy);

   proxy = d->proxy;

   if ((!proxy) && _efl_net_proxy_helper_can_do())
     {
        Eina_Stringshare *url;

        url = eina_stringshare_printf("%s://%s:%s", d->protocol == IPPROTO_UDP ? "udp" : "tcp", host, port);
        if (!url)
          {
             ERR("Could not assemble URL");
          }
        else
          {
             proxies = ecore_con_libproxy_proxies_get(url, thread);
             eina_stringshare_del(url);
          }
     }

   EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)ecore_con_libproxy_proxies_free, proxies);
 next_proxy:
   if ((!proxy) && (proxies) && (proxies_idx >= 0))
     {
        proxy = proxies[proxies_idx];
        if (!proxy)
          {
             is_libproxy = EINA_FALSE;
             proxies_idx = -1;
          }
        else
          {
             if (strcmp(proxy, "direct://") == 0)
               {
                  /* give a chance to try envvars */
                  proxy = NULL;
                  is_libproxy = EINA_FALSE;
               }
             else
               {
                  DBG("libproxy said %s for host='%s' port='%s'", proxy, host, port);
                  is_libproxy = EINA_TRUE;
               }
             proxies_idx++;
          }
     }

   if (!proxy)
     {
        proxy = d->proxy_env;
        if (!proxy)
          proxy = "";
        else
          {
             if (_efl_net_ip_no_proxy(host, d->no_proxy_strv))
               proxy = "";
             else
               DBG("using proxy %s from envvar", proxy);
          }
     }

   /* allows ecore_thread_cancel() to cancel at some points, see
    * man:pthreads(7).
    */
   eina_thread_cancellable_set(EINA_TRUE, NULL);

   if (!proxy[0])
     _efl_net_ip_connect_async_run_direct(d, host, port);
   else if (eina_str_has_prefix(proxy, "socks4://"))
     _efl_net_ip_connect_async_run_socks4(d, host, port, proxy + strlen("socks4://"));
   else if (eina_str_has_prefix(proxy, "socks5://"))
     _efl_net_ip_connect_async_run_socks5(d, host, port, proxy + strlen("socks5://"));
   else if (eina_str_has_prefix(proxy, "socks4a://"))
     _efl_net_ip_connect_async_run_socks4a(d, host, port, proxy + strlen("socks4a://"));
   else if (eina_str_has_prefix(proxy, "socks5h://"))
     _efl_net_ip_connect_async_run_socks5h(d, host, port, proxy + strlen("socks5h://"));
   else if (eina_str_has_prefix(proxy, "socks://"))
     {
        _efl_net_ip_connect_async_run_socks5(d, host, port, proxy + strlen("socks://"));
        if (d->error)
          _efl_net_ip_connect_async_run_socks4(d, host, port, proxy + strlen("socks://"));
     }
   else if (!strstr(proxy, "://"))
     {
        _efl_net_ip_connect_async_run_socks5(d, host, port, proxy);
        if (d->error)
          _efl_net_ip_connect_async_run_socks4(d, host, port, proxy);
     }
   else
     {
        if (d->proxy)
          {
             d->error = ENOTSUP;
             ERR("proxy protocol not supported '%s'", proxy);
          }
        else
          {
             if (is_libproxy)
               {
                  DBG("libproxy said %s but it's not supported, try next proxy", proxy);
                  proxy = NULL;
                  goto next_proxy;
               }
             /* maybe bogus envvar, ignore it */
             WRN("proxy protocol not supported '%s', connect directly", proxy);
             _efl_net_ip_connect_async_run_direct(d, host, port);
          }
     }

   if ((d->error) && (!d->proxy) && (proxy[0] != '\0'))
     {
        if (is_libproxy)
          {
             DBG("libproxy said %s but it failed, try next proxy", proxy);
             proxy = NULL;
             goto next_proxy;
          }

        WRN("error using proxy '%s' from environment, try direct connect", proxy);
        _efl_net_ip_connect_async_run_direct(d, host, port);
     }

   eina_thread_cancellable_set(EINA_FALSE, NULL);
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);
}

static void
_efl_net_ip_connect_async_data_free(Efl_Net_Ip_Connect_Async_Data *d)
{
   free(d->address);
   free(d->proxy);
   free(d->proxy_env);
   if (d->no_proxy_strv)
     {
        free(d->no_proxy_strv[0]);
        free(d->no_proxy_strv);
     }
   free(d);
}

static void
_efl_net_ip_connect_async_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Ip_Connect_Async_Data *d = data;

#ifdef FD_CLOEXEC
   /* if it wasn't a close on exec, release the socket to be passed to child */
   if ((!d->close_on_exec) && (d->sockfd != INVALID_SOCKET))
     {
        if (!eina_file_close_on_exec(d->sockfd, EINA_FALSE))
          {
             d->error = errno;
             closesocket(d->sockfd);
             d->sockfd = INVALID_SOCKET;
          }
     }
#endif

   d->cb((void *)d->data, &d->addr, d->addrlen, d->sockfd, d->error);
   _efl_net_ip_connect_async_data_free(d);
}

static void
_efl_net_ip_connect_async_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Ip_Connect_Async_Data *d = data;
   if (d->sockfd != INVALID_SOCKET) closesocket(d->sockfd);
   _efl_net_ip_connect_async_data_free(d);
}

Ecore_Thread *
efl_net_ip_connect_async_new(const char *address, const char *proxy, const char *proxy_env, const char *no_proxy_env, int type, int protocol, Eina_Bool close_on_exec, Efl_Net_Connect_Async_Cb cb, const void *data)
{
   Efl_Net_Ip_Connect_Async_Data *d;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, NULL);

   d = calloc(1, sizeof(Efl_Net_Ip_Connect_Async_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(d, NULL);

   d->address = strdup(address);
   EINA_SAFETY_ON_NULL_GOTO(d->address, error_address);

   if (proxy)
     {
        d->proxy = strdup(proxy);
        EINA_SAFETY_ON_NULL_GOTO(d->proxy, error_proxy);
     }
   if (proxy_env)
     {
        d->proxy_env = strdup(proxy_env);
        EINA_SAFETY_ON_NULL_GOTO(d->proxy_env, error_proxy_env);
     }
   if (no_proxy_env)
     {
        d->no_proxy_strv = eina_str_split(no_proxy_env, ",", 0);
        EINA_SAFETY_ON_NULL_GOTO(d->no_proxy_strv, error_no_proxy_strv);
     }

   d->cb = cb;
   d->data = data;
   d->addrlen = 0;
   d->close_on_exec = close_on_exec;
   d->type = type;
   d->protocol = protocol;

   d->sockfd = INVALID_SOCKET;
   d->error = 0;


   return ecore_thread_run(_efl_net_ip_connect_async_run,
                           _efl_net_ip_connect_async_end,
                           _efl_net_ip_connect_async_cancel,
                           d);

 error_no_proxy_strv:
   free(d->proxy_env);
 error_proxy_env:
   free(d->proxy);
 error_proxy:
   free(d->address);
 error_address:
   free(d);
   return NULL;
}

static Eina_Error
efl_net_multicast_address4_parse(const char *address, struct ip_mreq *mreq)
{
   char *str = NULL;
   const char *iface;
   Eina_Error err = 0;
   int r;

   iface = strchr(address, '@');
   if (!iface) iface = "0.0.0.0";
   else
     {
        str = malloc(iface - address + 1);
        EINA_SAFETY_ON_NULL_RETURN_VAL(str, ENOMEM);
        memcpy(str, address, iface - address);
        str[iface - address] = '\0';
        address = str;
        iface++;
        if (iface[0] == '\0') iface = "0.0.0.0";
     }

   if (address[0] == '\0')
     {
        err = EINVAL;
        goto end;
     }

   r = inet_pton(AF_INET, address, &mreq->imr_multiaddr);
   if (r != 1)
     {
        if (r < 0) err = efl_net_socket_error_get();
        else err = EINVAL;
        goto end;
     }

   r = inet_pton(AF_INET, iface, &mreq->imr_interface);
   if (r != 1)
     {
        if (r < 0) err = efl_net_socket_error_get();
        else err = EINVAL;
        goto end;
     }

 end:
   free(str);
   return err;
}

static Eina_Error
efl_net_multicast_address6_parse(const char *address, struct ipv6_mreq *mreq)
{
   char *str;
   char *endptr;
   const char *iface;
   Eina_Error err = 0;
   unsigned long idx;
   int r;

   str = strdup(address);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, ENOMEM);
   address = str;

   if (address[0] == '[')
     {
        address++;

        endptr = strchr(address, ']');
        if (!endptr)
          {
             err = EINVAL;
             goto end;
          }
        memmove(endptr, endptr + 1, strlen(endptr + 1) + 1);
     }

   iface = strchr(address, '@');
   if (!iface) iface = "0";
   else
     {
        str[iface - address] = '\0';
        iface++;
        if (iface[0] == '\0') iface = "0";
     }

   if (address[0] == '\0')
     {
        err = EINVAL;
        goto end;
     }

   r = inet_pton(AF_INET6, address, &mreq->ipv6mr_multiaddr);
   if (r != 1)
     {
        if (r < 0) err = efl_net_socket_error_get();
        else err = EINVAL;
        goto end;
     }

   errno = 0;
   idx = strtoul(iface, &endptr, 10);
   if (errno)
     {
        err = errno;
        goto end;
     }
   else if ((iface == endptr) || (endptr[0] != '\0'))
     {
        errno = EINVAL;
        goto end;
     }
   else if (idx > UINT32_MAX)
     {
        errno = ERANGE;
        goto end;
     }

   mreq->ipv6mr_interface = idx;

 end:
   free(str);
   return err;
}

Eina_Error
efl_net_multicast_join(SOCKET fd, int family, const char *address)
{
   Eina_Error err;

   if (family == AF_INET)
     {
        struct ip_mreq mreq;

        err = efl_net_multicast_address4_parse(address, &mreq);
        if (err)
          return err;

        if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)) == 0)
          return 0;
     }
   else if (family == AF_INET6)
     {
        struct ipv6_mreq mreq;

        err = efl_net_multicast_address6_parse(address, &mreq);
        if (err)
          return err;

        if (setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (const char *)&mreq, sizeof(mreq)) == 0)
          return 0;
     }
   else
     {
        ERR("invalid socket family=%d", family);
        return EINVAL;
     }

   return efl_net_socket_error_get();
}

Eina_Error
efl_net_multicast_leave(SOCKET fd, int family, const char *address)
{
   Eina_Error err;

   if (family == AF_INET)
     {
        struct ip_mreq mreq;

        err = efl_net_multicast_address4_parse(address, &mreq);
        if (err)
          return err;

        if (setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)) == 0)
          return 0;
     }
   else if (family == AF_INET6)
     {
        struct ipv6_mreq mreq;

        err = efl_net_multicast_address6_parse(address, &mreq);
        if (err)
          return err;

        if (setsockopt(fd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, (const char *)&mreq, sizeof(mreq)) == 0)
          return 0;
     }
   else
     {
        ERR("invalid socket family=%d", family);
        return EINVAL;
     }

   return efl_net_socket_error_get();
}

Eina_Error
efl_net_multicast_ttl_set(SOCKET fd, int family, uint8_t ttl)
{
   int level = (family == AF_INET) ? IPPROTO_IP : IPPROTO_IPV6;
   int opt = (family == AF_INET) ? IP_MULTICAST_TTL : IPV6_MULTICAST_HOPS;
#ifdef _WIN32
   DWORD value = ttl;
#else
   int value = ttl;
#endif

   if (setsockopt(fd, level, opt, (const char *)&value, sizeof(value)) == 0)
     return 0;

   return efl_net_socket_error_get();
}

Eina_Error
efl_net_multicast_ttl_get(SOCKET fd, int family, uint8_t *ttl)
{
   int level = (family == AF_INET) ? IPPROTO_IP : IPPROTO_IPV6;
   int opt = (family == AF_INET) ? IP_MULTICAST_TTL : IPV6_MULTICAST_HOPS;
#ifdef _WIN32
   DWORD value;
   int valuelen = sizeof(value);
#else
   int value;
   socklen_t valuelen = sizeof(value);
#endif

   if (getsockopt(fd, level, opt, (char *)&value, &valuelen) == 0)
     {
        *ttl = value;
        return 0;
     }

   return efl_net_socket_error_get();
}

Eina_Error
efl_net_multicast_loopback_set(SOCKET fd, int family, Eina_Bool loopback)
{
   int level = (family == AF_INET) ? IPPROTO_IP : IPPROTO_IPV6;
   int opt = (family == AF_INET) ? IP_MULTICAST_LOOP : IPV6_MULTICAST_LOOP;
#ifdef _WIN32
   DWORD value = loopback;
#else
   int value = loopback;
#endif

   if (setsockopt(fd, level, opt, (const char *)&value, sizeof(value)) == 0)
     return 0;

   return efl_net_socket_error_get();
}

Eina_Error
efl_net_multicast_loopback_get(SOCKET fd, int family, Eina_Bool *loopback)
{
   int level = (family == AF_INET) ? IPPROTO_IP : IPPROTO_IPV6;
   int opt = (family == AF_INET) ? IP_MULTICAST_LOOP : IPV6_MULTICAST_LOOP;
#ifdef _WIN32
   DWORD value;
   int valuelen = sizeof(value);
#else
   int value;
   socklen_t valuelen = sizeof(value);
#endif

   if (getsockopt(fd, level, opt, (char *)&value, &valuelen) == 0)
     {
        *loopback = !!value;
        return 0;
     }

   return efl_net_socket_error_get();
}

size_t
efl_net_udp_datagram_size_query(SOCKET fd)
{
#ifdef _WIN32
   unsigned long size;
   if (ioctlsocket(fd, FIONREAD, &size) == 0)
     return size;
#else
   int size;
   if (ioctl(fd, FIONREAD, &size) == 0)
     return size;
#endif
   return READBUFSIZ;
}

char **
ecore_con_libproxy_proxies_get(const char *url, Ecore_Thread *eth)
{
   int id =  _efl_net_proxy_helper_url_req_send(url, eth);
   if (id < 0) return NULL;
   return _efl_net_proxy_helper_url_wait(id);
}

void
ecore_con_libproxy_proxies_free(char **proxies)
{
   char **itr;

   if (!proxies) return;

   for (itr = proxies; *itr != NULL; itr++)
     free(*itr);
   free(proxies);
}
