#define EFL_NET_DIALER_TCP_PROTECTED 1
#define EFL_NET_DIALER_PROTECTED 1
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
   struct {
      Ecore_Thread *thread;
      struct addrinfo *names;
      struct addrinfo *current;
   } resolve;
   struct {
      Ecore_Thread *thread;
      Eo *timer;
   } connect;
   Eina_Stringshare *address_dial;
   Eina_Stringshare *proxy;
   Eina_Bool connected;
   Eina_Bool closed;
   double timeout_dial;
} Efl_Net_Dialer_Tcp_Data;

EOLIAN static Eo*
_efl_net_dialer_tcp_efl_object_constructor(Eo *o, Efl_Net_Dialer_Tcp_Data *pd EINA_UNUSED)
{
   o = efl_constructor(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   efl_net_dialer_timeout_dial_set(o, 30.0);
   return o;
}

EOLIAN static void
_efl_net_dialer_tcp_efl_object_destructor(Eo *o, Efl_Net_Dialer_Tcp_Data *pd)
{
   if (efl_io_closer_close_on_destructor_get(o) &&
       (!efl_io_closer_closed_get(o)))
     efl_io_closer_close(o);

   if (pd->connect.timer)
     {
        efl_del(pd->connect.timer);
        pd->connect.timer = NULL;
     }

   if (pd->connect.thread)
     {
        ecore_thread_cancel(pd->connect.thread);
        pd->connect.thread = NULL;
     }

   if (pd->resolve.thread)
     {
        ecore_thread_cancel(pd->resolve.thread);
        pd->resolve.thread = NULL;
     }

   pd->resolve.current = NULL;
   if (pd->resolve.names)
     {
        freeaddrinfo(pd->resolve.names);
        pd->resolve.names = NULL;
     }

   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address_dial, NULL);
   eina_stringshare_replace(&pd->proxy, NULL);
}

static void
_efl_net_dialer_tcp_connected(void *data, const struct sockaddr *addr EINA_UNUSED, socklen_t addrlen EINA_UNUSED, int fd, Eina_Error err)
{
   Eo *o = data;
   Efl_Net_Dialer_Tcp_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const struct addrinfo *addrinfo;

   pd->connect.thread = NULL;

   if (!err)
     {
        freeaddrinfo(pd->resolve.names);
        pd->resolve.names = NULL;
        pd->resolve.current = NULL;
        efl_loop_fd_set(o, fd);
        efl_net_dialer_connected_set(o, EINA_TRUE);
        return;
     }

   if (!pd->resolve.current) return;

   pd->resolve.current = pd->resolve.current->ai_next;
   if (!pd->resolve.current)
     {
        freeaddrinfo(pd->resolve.names);
        pd->resolve.names = NULL;
        efl_io_reader_eos_set(o, EINA_TRUE);
        if (err)
          efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
        else
          {
             err = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
             efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
          }
        return;
     }

   addrinfo = pd->resolve.current;
   pd->connect.thread = efl_net_connect_async_new(addrinfo->ai_addr,
                                                  addrinfo->ai_addrlen,
                                                  SOCK_STREAM,
                                                  IPPROTO_TCP,
                                                  efl_io_closer_close_on_exec_get(o),
                                                  _efl_net_dialer_tcp_connected,
                                                  o);
}

static void
_efl_net_dialer_tcp_connect(Eo *o, Efl_Net_Dialer_Tcp_Data *pd)
{
   char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")];
   const struct addrinfo *addrinfo;

   if (pd->closed || !pd->resolve.current) return;

   efl_ref(o); /* we're emitting callbacks then continuing the workflow */

   addrinfo = pd->resolve.current;

   efl_net_socket_fd_family_set(o, addrinfo->ai_family);
   if (efl_net_ip_port_fmt(buf, sizeof(buf), addrinfo->ai_addr))
     {
        efl_net_socket_address_remote_set(o, buf);
        efl_event_callback_call(o, EFL_NET_DIALER_EVENT_RESOLVED, NULL);
     }

   if (pd->closed) goto end; /* maybe closed from resolved event callback */

   if (pd->connect.thread)
     ecore_thread_cancel(pd->connect.thread);

   pd->connect.thread = efl_net_connect_async_new(addrinfo->ai_addr,
                                                  addrinfo->ai_addrlen,
                                                  SOCK_STREAM,
                                                  IPPROTO_TCP,
                                                  efl_io_closer_close_on_exec_get(o),
                                                  _efl_net_dialer_tcp_connected,
                                                  o);
 end:
   efl_unref(o);
}

static void
_efl_net_dialer_tcp_resolved(void *data, const char *host EINA_UNUSED, const char *port EINA_UNUSED, const struct addrinfo *hints EINA_UNUSED, struct addrinfo *result, int gai_error)
{
   Eo *o = data;
   Efl_Net_Dialer_Tcp_Data *pd = efl_data_scope_get(o, MY_CLASS);

   pd->resolve.thread = NULL;

   if (gai_error)
     {
        Eina_Error err = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_HOST;
        efl_io_reader_eos_set(o, EINA_TRUE);
        efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
        if (result) freeaddrinfo(result);
        return;
     }

   if (pd->resolve.names) freeaddrinfo(pd->resolve.names);
   pd->resolve.names = result;
   pd->resolve.current = result;

   _efl_net_dialer_tcp_connect(o, pd);
}

static void
_efl_net_dialer_tcp_connect_timeout(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Dialer_Tcp_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eina_Error err = ETIMEDOUT;

   pd->connect.timer = NULL;

   if (pd->resolve.thread)
     {
        ecore_thread_cancel(pd->resolve.thread);
        pd->resolve.thread = NULL;
     }
   if (pd->resolve.names)
     {
        freeaddrinfo(pd->resolve.names);
        pd->resolve.names = NULL;
     }
   pd->resolve.current = NULL;

   if (pd->connect.thread)
     {
        ecore_thread_cancel(pd->connect.thread);
        pd->connect.thread = NULL;
     }

   efl_ref(o);
   efl_io_reader_eos_set(o, EINA_TRUE);
   efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
   efl_unref(o);
}

EOLIAN static Eina_Error
_efl_net_dialer_tcp_efl_net_dialer_dial(Eo *o, Efl_Net_Dialer_Tcp_Data *pd EINA_UNUSED, const char *address)
{
   const char *host, *port;
   struct addrinfo hints = {
     .ai_socktype = SOCK_STREAM,
     .ai_protocol = IPPROTO_TCP,
   };
   char *str;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_net_dialer_connected_get(o), EISCONN);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EBADF);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_loop_fd_get(o) >= 0, EALREADY);

   str = strdup(address);
   EINA_SAFETY_ON_NULL_RETURN_VAL(str, ENOMEM);

   if (!efl_net_ip_port_split(str, &host, &port))
     {
        ERR("could not parse IP:PORT '%s'", address);
        free(str);
        return EINVAL;
     }
   if (strchr(host, ':')) hints.ai_family = AF_INET6;
   if (!port) port = "0";

   if (pd->connect.thread)
     {
        ecore_thread_cancel(pd->connect.thread);
        pd->connect.thread = NULL;
     }

   if (pd->resolve.names)
     {
        freeaddrinfo(pd->resolve.names);
        pd->resolve.names = NULL;
     }
   pd->resolve.current = NULL;

   if (pd->resolve.thread)
     ecore_thread_cancel(pd->resolve.thread);

   pd->resolve.thread = efl_net_resolve_async_new(host, port, &hints, _efl_net_dialer_tcp_resolved, o);
   free(str);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->resolve.thread, EINVAL);

   efl_net_dialer_address_dial_set(o, address);

   if (pd->connect.timer)
     {
        efl_del(pd->connect.timer);
        pd->connect.timer = NULL;
     }
   if (pd->timeout_dial > 0.0)
     {
        pd->connect.timer = efl_add(EFL_LOOP_TIMER_CLASS, efl_loop_user_loop_get(o),
                                    efl_loop_timer_interval_set(efl_added, pd->timeout_dial),
                                    efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TICK, _efl_net_dialer_tcp_connect_timeout, o));
     }

   return 0;
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
   pd->timeout_dial = seconds;
   if (pd->connect.timer)
     {
        efl_del(pd->connect.timer);
        pd->connect.timer = NULL;
     }
   if (pd->timeout_dial > 0.0)
     {
        pd->connect.timer = efl_add(EFL_LOOP_TIMER_CLASS, efl_loop_user_loop_get(o),
                                    efl_loop_timer_interval_set(efl_added, pd->timeout_dial),
                                    efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TICK, _efl_net_dialer_tcp_connect_timeout, o));
     }
}

EOLIAN static double
_efl_net_dialer_tcp_efl_net_dialer_timeout_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Tcp_Data *pd)
{
   return pd->timeout_dial;
}

EOLIAN static void
_efl_net_dialer_tcp_efl_net_dialer_connected_set(Eo *o, Efl_Net_Dialer_Tcp_Data *pd, Eina_Bool connected)
{
   if (pd->connect.timer)
     {
        efl_del(pd->connect.timer);
        pd->connect.timer = NULL;
     }
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
_efl_net_dialer_tcp_efl_io_closer_close(Eo *o, Efl_Net_Dialer_Tcp_Data *pd)
{
   pd->closed = EINA_TRUE;
   efl_net_dialer_connected_set(o, EINA_FALSE);
   return efl_io_closer_close(efl_super(o, MY_CLASS));
}

#include "efl_net_dialer_tcp.eo.c"
