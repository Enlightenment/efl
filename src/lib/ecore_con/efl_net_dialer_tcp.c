#define EFL_NET_DIALER_TCP_PROTECTED 1
#define EFL_NET_DIALER_PROTECTED 1
#define EFL_NET_SOCKET_FD_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

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
#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#define MY_CLASS EFL_NET_DIALER_TCP_CLASS

typedef struct _Efl_Net_Dialer_Tcp_Data
{
   Eina_Stringshare *address_dial;
   Eina_Stringshare *proxy;
   Eina_Bool connected;
   double timeout_dial;
} Efl_Net_Dialer_Tcp_Data;

EOLIAN static void
_efl_net_dialer_tcp_efl_object_destructor(Eo *o, Efl_Net_Dialer_Tcp_Data *pd)
{
   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address_dial, NULL);
   eina_stringshare_replace(&pd->proxy, NULL);
}

EOLIAN static Eina_Error
_efl_net_dialer_tcp_efl_net_dialer_dial(Eo *o, Efl_Net_Dialer_Tcp_Data *pd EINA_UNUSED, const char *address)
{
   struct sockaddr_storage addr = {};
   char *str, *host, *port;
   int r, fd;
   socklen_t addrlen;
   char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")];

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_net_dialer_connected_get(o), EISCONN);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EBADF);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_loop_fd_get(o) >= 0, EALREADY);

   // TODO: change to getaddrinfo() and move to a thread...
   str = host = strdup(address);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, ENOMEM);

   if (host[0] == '[')
     {
        struct sockaddr_in6 *a = (struct sockaddr_in6 *)&addr;
        /* IPv6 is: [IP]:port */
        host++;
        port = strchr(host, ']');
        if (!port)
          {
             ERR("missing ']' in IPv6 address: %s", address);
             goto invalid_address;
          }
        *port = '\0';
        port++;

        if (port[0] == ':')
          port++;
        else
          port = NULL;
        a->sin6_family = AF_INET6;
        a->sin6_port = htons(port ? atoi(port) : 0);
        r = inet_pton(AF_INET6, host, &(a->sin6_addr));
        addrlen = sizeof(*a);
     }
   else
     {
        struct sockaddr_in *a = (struct sockaddr_in *)&addr;
        port = strchr(host, ':');
        if (port)
          {
             *port = '\0';
             port++;
          }
        a->sin_family = AF_INET;
        a->sin_port = htons(port ? atoi(port) : 0);
        r = inet_pton(AF_INET, host, &(a->sin_addr));
        addrlen = sizeof(*a);
     }

   if (r != 1)
     {
        ERR("could not parse IP '%s' (%s)", host, address);
        goto invalid_address;
     }
   free(str);

   efl_net_socket_fd_family_set(o, addr.ss_family);
   efl_net_dialer_address_dial_set(o, address);

   if (efl_net_ip_port_fmt(buf, sizeof(buf), (struct sockaddr *)&addr))
     {
        efl_net_socket_address_remote_set(o, buf);
        efl_event_callback_call(o, EFL_NET_DIALER_EVENT_RESOLVED, NULL);
     }

   fd = efl_net_socket4(addr.ss_family, SOCK_STREAM, IPPROTO_TCP, efl_net_socket_fd_close_on_exec_get(o));
   if (fd < 0)
     {
        ERR("socket(%d, SOCK_STREAM, IPPROTO_TCP): %s",
            addr.ss_family, strerror(errno));
        return errno;
     }

   r = connect(fd, (struct sockaddr *)&addr, addrlen);
   if (r < 0)
     {
        int errno_bkp = errno;
        ERR("connect(%d, %s): %s", fd, address, strerror(errno));
        close(fd);
        return errno_bkp;
     }

   efl_loop_fd_set(o, fd);
   efl_net_dialer_connected_set(o, EINA_TRUE);
   return 0;

 invalid_address:
   free(str);
   return EINVAL;
}

EOLIAN static void
_efl_net_dialer_tcp_efl_net_dialer_address_dial_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Tcp_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_dial, address);
}

EOLIAN static const char *
_efl_net_dialer_tcp_efl_net_dialer_address_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Tcp_Data *pd)
{
   return pd->address_dial;
}

EOLIAN static void
_efl_net_dialer_tcp_efl_net_dialer_proxy_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Tcp_Data *pd, const char *proxy_url)
{
   // TODO: apply proxy
   eina_stringshare_replace(&pd->proxy, proxy_url);
}

EOLIAN static const char *
_efl_net_dialer_tcp_efl_net_dialer_proxy_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Tcp_Data *pd)
{
   return pd->proxy;
}

EOLIAN static void
_efl_net_dialer_tcp_efl_net_dialer_timeout_dial_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Tcp_Data *pd, double seconds)
{
   // TODO: when using ecore_con_info/threads, set timeout
   pd->timeout_dial = seconds;
}

EOLIAN static double
_efl_net_dialer_tcp_efl_net_dialer_timeout_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Tcp_Data *pd)
{
   return pd->timeout_dial;
}

EOLIAN static void
_efl_net_dialer_tcp_efl_net_dialer_connected_set(Eo *o, Efl_Net_Dialer_Tcp_Data *pd, Eina_Bool connected)
{
   if (pd->connected == connected) return;
   pd->connected = connected;
   if (connected) efl_event_callback_call(o, EFL_NET_DIALER_EVENT_CONNECTED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_dialer_tcp_efl_net_dialer_connected_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Tcp_Data *pd)
{
   return pd->connected;
}

EOLIAN static Eina_Error
_efl_net_dialer_tcp_efl_io_closer_close(Eo *o, Efl_Net_Dialer_Tcp_Data *pd EINA_UNUSED)
{
   efl_net_dialer_connected_set(o, EINA_FALSE);
   return efl_io_closer_close(efl_super(o, MY_CLASS));
}

#include "efl_net_dialer_tcp.eo.c"
