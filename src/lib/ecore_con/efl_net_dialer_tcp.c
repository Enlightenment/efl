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
      Eina_Future *timeout;
   } connect;
   Eina_Stringshare *address_dial;
   Eina_Stringshare *proxy;
   Eina_Bool connected;
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
     {
        efl_event_freeze(o);
        efl_io_closer_close(o);
        efl_event_thaw(o);
     }

   if (pd->connect.thread)
     {
        ecore_thread_cancel(pd->connect.thread);
        pd->connect.thread = NULL;
     }

   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address_dial, NULL);
   eina_stringshare_replace(&pd->proxy, NULL);
}

static Eina_Value
_efl_net_dialer_tcp_connect_timeout(Eo *o, const Eina_Value v)
{
   Efl_Net_Dialer_Tcp_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eina_Error err = ETIMEDOUT;

   if (pd->connect.thread)
     {
        ecore_thread_cancel(pd->connect.thread);
        pd->connect.thread = NULL;
     }

   efl_ref(o);
   efl_io_reader_eos_set(o, EINA_TRUE);
   efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
   efl_unref(o);
   return v;
}

static void
_timeout_schedule(Eo *o, Efl_Net_Dialer_Tcp_Data *pd)
{
   efl_future_Eina_FutureXXX_then(o, efl_loop_Eina_FutureXXX_timeout(efl_loop_get(o), pd->timeout_dial),
                                  .success = _efl_net_dialer_tcp_connect_timeout,
                                  .storage = &pd->connect.timeout);
}

static void
_efl_net_dialer_tcp_connected(void *data, const struct sockaddr *addr, socklen_t addrlen EINA_UNUSED, SOCKET sockfd, Eina_Error err)
{
   Eo *o = data;
   Efl_Net_Dialer_Tcp_Data *pd = efl_data_scope_get(o, MY_CLASS);

   pd->connect.thread = NULL;

   efl_ref(o); /* we're emitting callbacks then continuing the workflow */

   if (err) goto error;

   efl_net_socket_fd_family_set(o, addr->sa_family);
   efl_loop_fd_set(o, sockfd);
   if (efl_net_socket_address_remote_get(o))
     {
        efl_event_callback_call(o, EFL_NET_DIALER_EVENT_RESOLVED, NULL);
        efl_net_dialer_connected_set(o, EINA_TRUE);
     }
   else
     {
        err = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
        efl_loop_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
        closesocket(sockfd);
        goto error;
     }

 error:
   if (err)
     {
        if (addr && addr->sa_family)
          {
             char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";
             if (efl_net_ip_port_fmt(buf, sizeof(buf), addr))
               {
                  efl_net_socket_address_remote_set(o, buf);
                  efl_event_callback_call(o, EFL_NET_DIALER_EVENT_RESOLVED, NULL);
               }
          }
        efl_io_reader_eos_set(o, EINA_TRUE);
        efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
     }

   efl_unref(o);
}

EOLIAN static Eina_Error
_efl_net_dialer_tcp_efl_net_dialer_dial(Eo *o, Efl_Net_Dialer_Tcp_Data *pd EINA_UNUSED, const char *address)
{
   const char *proxy_env = NULL, *no_proxy_env = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_net_dialer_connected_get(o), EISCONN);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EBADF);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_loop_fd_get(o) >= 0, EALREADY);

   if (pd->connect.thread)
     {
        ecore_thread_cancel(pd->connect.thread);
        pd->connect.thread = NULL;
     }

   if (!pd->proxy)
     {
        proxy_env = getenv("socks_proxy");
        if (!proxy_env) proxy_env = getenv("SOCKS_PROXY");
        if (!proxy_env) proxy_env = getenv("all_proxy");
        if (!proxy_env) proxy_env = getenv("ALL_PROXY");

        no_proxy_env = getenv("no_proxy");
        if (!no_proxy_env) no_proxy_env = getenv("NO_PROXY");
     }

   pd->connect.thread = efl_net_ip_connect_async_new(address,
                                                     pd->proxy,
                                                     proxy_env,
                                                     no_proxy_env,
                                                     SOCK_STREAM,
                                                     IPPROTO_TCP,
                                                     efl_io_closer_close_on_exec_get(o),
                                                     _efl_net_dialer_tcp_connected, o);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->connect.thread, EINVAL);

   efl_net_dialer_address_dial_set(o, address);

   if (pd->connect.timeout) eina_future_cancel(pd->connect.timeout);
   if (pd->timeout_dial > 0.0) _timeout_schedule(o, pd);

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
   if (pd->connect.timeout) eina_future_cancel(pd->connect.timeout);
   if ((pd->timeout_dial > 0.0) && (pd->connect.thread)) _timeout_schedule(o, pd);
}

EOLIAN static double
_efl_net_dialer_tcp_efl_net_dialer_timeout_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Tcp_Data *pd)
{
   return pd->timeout_dial;
}

EOLIAN static void
_efl_net_dialer_tcp_efl_net_dialer_connected_set(Eo *o, Efl_Net_Dialer_Tcp_Data *pd, Eina_Bool connected)
{
   if (pd->connect.timeout) eina_future_cancel(pd->connect.timeout);
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
