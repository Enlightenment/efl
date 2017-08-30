#define EFL_NET_DIALER_UDP_PROTECTED 1
#define EFL_NET_DIALER_PROTECTED 1
#define EFL_NET_SOCKET_UDP_PROTECTED 1
#define EFL_NET_SOCKET_FD_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1
#define EFL_IO_READER_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_UDP_H
# include <netinet/udp.h>
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

#define MY_CLASS EFL_NET_DIALER_UDP_CLASS

typedef struct _Efl_Net_Dialer_Udp_Data
{
   struct {
      Ecore_Thread *thread;
      Eina_Future *timeout;
   } resolver;
   Eina_Stringshare *address_dial;
   Eina_Bool connected;
   Eina_Bool closed;
   double timeout_dial;
} Efl_Net_Dialer_Udp_Data;

EOLIAN static Eo*
_efl_net_dialer_udp_efl_object_constructor(Eo *o, Efl_Net_Dialer_Udp_Data *pd EINA_UNUSED)
{
   o = efl_constructor(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   efl_net_dialer_timeout_dial_set(o, 30.0);
   return o;
}

EOLIAN static void
_efl_net_dialer_udp_efl_object_destructor(Eo *o, Efl_Net_Dialer_Udp_Data *pd)
{
   if (efl_io_closer_close_on_destructor_get(o) &&
       (!efl_io_closer_closed_get(o)))
     {
        efl_event_freeze(o);
        efl_io_closer_close(o);
        efl_event_thaw(o);
     }

   if (pd->resolver.thread)
     {
        ecore_thread_cancel(pd->resolver.thread);
        pd->resolver.thread = NULL;
     }

   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address_dial, NULL);
}

static Eina_Value
_efl_net_dialer_udp_resolver_timeout(Eo *o, const Eina_Value v)
{
   Efl_Net_Dialer_Udp_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eina_Error err = ETIMEDOUT;

   if (pd->resolver.thread)
     {
        ecore_thread_cancel(pd->resolver.thread);
        pd->resolver.thread = NULL;
     }

   efl_ref(o);
   efl_io_reader_eos_set(o, EINA_TRUE);
   efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
   efl_unref(o);
   return v;
}

static void
_timeout_schedule(Eo *o, Efl_Net_Dialer_Udp_Data *pd)
{
   efl_future_Eina_FutureXXX_then(o, efl_loop_Eina_FutureXXX_timeout(efl_loop_get(o), pd->timeout_dial),
                                  .success = _efl_net_dialer_udp_resolver_timeout,
                                  .storage = &pd->resolver.timeout);
}

static Eina_Error
_efl_net_dialer_udp_resolved_bind(Eo *o, Efl_Net_Dialer_Udp_Data *pd EINA_UNUSED, struct addrinfo *addr)
{
   Eina_Error err = 0;
   Eo *remote_address;
   char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")];
   SOCKET fd;
   int family = addr->ai_family;

   efl_net_socket_fd_family_set(o, family);

   fd = efl_net_socket4(family, addr->ai_socktype, addr->ai_protocol,
                        efl_io_closer_close_on_exec_get(o));
   if (fd == INVALID_SOCKET)
     {
        err = efl_net_socket_error_get();
        ERR("socket(%d, %d, %d): %s",
            family, addr->ai_socktype, addr->ai_protocol,
            eina_error_msg_get(err));
        return err;
     }

   if (!efl_net_socket_udp_bind_get(o))
     {
        if (family == AF_INET)
          efl_net_socket_udp_bind_set(o, "0.0.0.0:0");
        else
          efl_net_socket_udp_bind_set(o, "[::]:0");
     }

   efl_loop_fd_set(o, fd); /* will also apply reuse_address et al */

   if (family == AF_INET)
     {
        const struct sockaddr_in *a = (const struct sockaddr_in *)addr->ai_addr;
        uint32_t ipv4 = ntohl(a->sin_addr.s_addr);
        if (ipv4 == INADDR_BROADCAST)
          {
#ifdef _WIN32
             DWORD enable = 1;
#else
             int enable = 1;
#endif
             if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (const char *)&enable, sizeof(enable)) == 0)
               DBG("enabled SO_BROADCAST for socket=" SOCKET_FMT, fd);
             else
               WRN("could not enable SO_BROADCAST for socket=" SOCKET_FMT ": %s", fd, eina_error_msg_get(efl_net_socket_error_get()));
          }
     }
   else if (family == AF_INET6)
     {
        const struct sockaddr_in6 *a = (const struct sockaddr_in6 *)addr->ai_addr;
        if (IN6_IS_ADDR_MULTICAST(&a->sin6_addr))
          {
             struct ipv6_mreq mreq = {
               .ipv6mr_multiaddr = a->sin6_addr,
             };
             if (setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP, (const char *)&mreq, sizeof(mreq)) == 0)
               {
                  efl_net_ip_port_fmt(buf, sizeof(buf), addr->ai_addr);
                  DBG("joined multicast group %s socket=" SOCKET_FMT, buf, fd);
               }
             else
               {
                  err = efl_net_socket_error_get();
                  efl_net_ip_port_fmt(buf, sizeof(buf), addr->ai_addr);
                  ERR("could not join multicast group %s socket=" SOCKET_FMT ": %s", buf, fd, eina_error_msg_get(err));
                  goto error;
               }
          }
     }

   remote_address = efl_net_ip_address_create_sockaddr(EFL_NET_IP_ADDRESS_CLASS, addr->ai_addr);
   if (remote_address)
     {
        efl_net_socket_udp_init(o, remote_address);
        efl_event_callback_call(o, EFL_NET_DIALER_EVENT_RESOLVED, NULL);
        efl_del(remote_address);
     }
   efl_net_dialer_connected_set(o, EINA_TRUE);
   return 0;

 error:
   efl_net_socket_fd_family_set(o, AF_UNSPEC);
   efl_loop_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
   closesocket(fd);
   return err;
}

static void
_efl_net_dialer_udp_resolved(void *data, const char *host EINA_UNUSED, const char *port EINA_UNUSED, const struct addrinfo *hints EINA_UNUSED, struct addrinfo *result, int gai_error)
{
   Eo *o = data;
   Efl_Net_Dialer_Udp_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eina_Error err;
   struct addrinfo *addr;

   pd->resolver.thread = NULL;

   efl_ref(o); /* we're emitting callbacks then continuing the workflow */

   if (gai_error)
     {
        err = EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
        goto end;
     }

   for (addr = result; addr != NULL; addr = addr->ai_next)
     {
        err = _efl_net_dialer_udp_resolved_bind(o, pd, addr);
        if (err == 0) break;
     }

 end:
   if (err)
     {
        if (result)
          {
             char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";
             if (efl_net_ip_port_fmt(buf, sizeof(buf), result->ai_addr))
               {
                  efl_net_socket_address_remote_set(o, buf);
                  efl_event_callback_call(o, EFL_NET_DIALER_EVENT_RESOLVED, NULL);
               }
          }

        efl_io_reader_eos_set(o, EINA_TRUE);
        efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
     }
   freeaddrinfo(result);

   efl_unref(o);
}

EOLIAN static Eina_Error
_efl_net_dialer_udp_efl_net_dialer_dial(Eo *o, Efl_Net_Dialer_Udp_Data *pd EINA_UNUSED, const char *address)
{
   char *str;
   const char *host, *port;
   struct addrinfo hints = {
     .ai_socktype = SOCK_DGRAM,
     .ai_protocol = IPPROTO_UDP,
     .ai_family = AF_UNSPEC,
     .ai_flags = AI_ADDRCONFIG | AI_V4MAPPED,
   };

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_net_dialer_connected_get(o), EISCONN);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EBADF);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_loop_fd_get(o) >= 0, EALREADY);

   if (pd->resolver.thread)
     {
        ecore_thread_cancel(pd->resolver.thread);
        pd->resolver.thread = NULL;
     }

   str = strdup(address);
   if (!efl_net_ip_port_split(str, &host, &port))
     {
        free(str);
        return EINVAL;
     }
   if (!port) port = "0";
   if (strchr(host, ':')) hints.ai_family = AF_INET6;

   pd->resolver.thread = efl_net_ip_resolve_async_new(host, port, &hints,
                                                     _efl_net_dialer_udp_resolved, o);
   free(str);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->resolver.thread, EINVAL);

   efl_net_dialer_address_dial_set(o, address);

   if (pd->resolver.timeout) eina_future_cancel(pd->resolver.timeout);
   if (pd->timeout_dial > 0.0) _timeout_schedule(o, pd);

   return 0;
}

EOLIAN static void
_efl_net_dialer_udp_efl_net_dialer_address_dial_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Udp_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_dial, address);
}

EOLIAN static const char *
_efl_net_dialer_udp_efl_net_dialer_address_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Udp_Data *pd)
{
   return pd->address_dial;
}

EOLIAN static void
_efl_net_dialer_udp_efl_net_dialer_timeout_dial_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Udp_Data *pd, double seconds)
{
   pd->timeout_dial = seconds;
   if (pd->resolver.timeout) eina_future_cancel(pd->resolver.timeout);
   if ((pd->timeout_dial > 0.0) && (pd->resolver.thread)) _timeout_schedule(o, pd);
}

EOLIAN static double
_efl_net_dialer_udp_efl_net_dialer_timeout_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Udp_Data *pd)
{
   return pd->timeout_dial;
}

EOLIAN static void
_efl_net_dialer_udp_efl_net_dialer_connected_set(Eo *o, Efl_Net_Dialer_Udp_Data *pd, Eina_Bool connected)
{
   if (pd->resolver.timeout) eina_future_cancel(pd->resolver.timeout);
   if (pd->connected == connected) return;
   pd->connected = connected;
   if (connected) efl_event_callback_call(o, EFL_NET_DIALER_EVENT_CONNECTED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_dialer_udp_efl_net_dialer_connected_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Udp_Data *pd)
{
   return pd->connected;
}

EOLIAN static Eina_Error
_efl_net_dialer_udp_efl_io_closer_close(Eo *o, Efl_Net_Dialer_Udp_Data *pd)
{
   pd->closed = EINA_TRUE;
   efl_net_dialer_connected_set(o, EINA_FALSE);
   return efl_io_closer_close(efl_super(o, MY_CLASS));
}

#include "efl_net_dialer_udp.eo.c"
