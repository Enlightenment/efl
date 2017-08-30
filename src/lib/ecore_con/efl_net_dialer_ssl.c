#define EFL_NET_DIALER_SSL_PROTECTED 1
#define EFL_NET_SOCKET_SSL_PROTECTED 1
#define EFL_NET_DIALER_PROTECTED 1
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
#ifdef HAVE_NETINET_SSL_H
# include <netinet/ssl.h>
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

#define MY_CLASS EFL_NET_DIALER_SSL_CLASS

typedef struct _Efl_Net_Dialer_Ssl_Data
{
   Eo *sock;
   Eo *ssl_ctx;
   Eina_Future *connect_timeout;
   Eina_Bool connected;
} Efl_Net_Dialer_Ssl_Data;

static void
_efl_net_dialer_ssl_ready(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *o = event->object;
   efl_net_dialer_connected_set(o, EINA_TRUE);
}

static void
_efl_net_dialer_ssl_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *o = event->object;
   Eina_Error *perr = event->info;
   efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, perr);
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_net_dialer_ssl_cbs,
                           { EFL_NET_SOCKET_SSL_EVENT_SSL_READY, _efl_net_dialer_ssl_ready },
                           { EFL_NET_SOCKET_SSL_EVENT_SSL_ERROR, _efl_net_dialer_ssl_error });

EOLIAN static Eo*
_efl_net_dialer_ssl_efl_object_constructor(Eo *o, Efl_Net_Dialer_Ssl_Data *pd)
{
   o = efl_constructor(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   pd->sock = efl_add(EFL_NET_DIALER_TCP_CLASS, o);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->sock, NULL);

   efl_event_callback_array_add(o, _efl_net_dialer_ssl_cbs(), o);

   return o;
}

EOLIAN static Eo*
_efl_net_dialer_ssl_efl_object_finalize(Eo *o, Efl_Net_Dialer_Ssl_Data *pd)
{
   Eo *a_sock, *a_ctx;

   if (efl_net_socket_ssl_adopted_get(o, &a_sock, &a_ctx))
     {
        efl_del(pd->sock); /* stub TCP we created */
        pd->sock = a_sock;
        pd->ssl_ctx = a_ctx;
     }
   else
     {
        if (!pd->ssl_ctx)
          pd->ssl_ctx = efl_ref(efl_net_ssl_context_default_dialer_get(EFL_NET_SSL_CONTEXT_CLASS));

        efl_net_socket_ssl_adopt(o, pd->sock, pd->ssl_ctx);
     }

   return efl_finalize(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_dialer_ssl_efl_object_destructor(Eo *o, Efl_Net_Dialer_Ssl_Data *pd)
{
   if (efl_io_closer_close_on_destructor_get(o) &&
       (!efl_io_closer_closed_get(o)))
     {
        efl_event_freeze(o);
        efl_io_closer_close(o);
        efl_event_thaw(o);
     }

   if (pd->sock)
     {
        efl_del(pd->sock);
        pd->sock = NULL;
     }

   if (pd->ssl_ctx)
     {
        efl_unref(pd->ssl_ctx);
        pd->ssl_ctx = NULL;
     }

   efl_destructor(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_dialer_ssl_ssl_context_set(Eo *o, Efl_Net_Dialer_Ssl_Data *pd, Eo *ssl_ctx)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(ssl_ctx, EFL_NET_SSL_CONTEXT_CLASS));

   if (pd->ssl_ctx == ssl_ctx) return;
   efl_unref(pd->ssl_ctx);
   pd->ssl_ctx = efl_ref(ssl_ctx);
}

EOLIAN static Eo *
_efl_net_dialer_ssl_ssl_context_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Ssl_Data *pd)
{
   return pd->ssl_ctx;
}

static Eina_Value
_efl_net_dialer_ssl_connect_timeout(Eo *o, const Eina_Value v)
{
   Eina_Error err = ETIMEDOUT;

   efl_ref(o);
   efl_io_reader_eos_set(o, EINA_TRUE);
   efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
   efl_unref(o);
   return v;
}

static void
_timeout_schedule(Eo *o, Efl_Net_Dialer_Ssl_Data *pd, double timeout)
{
   efl_future_Eina_FutureXXX_then(o, efl_loop_Eina_FutureXXX_timeout(efl_loop_get(o), timeout),
                                  .success = _efl_net_dialer_ssl_connect_timeout,
                                  .storage = &pd->connect_timeout);
}

EOLIAN static Eina_Error
_efl_net_dialer_ssl_efl_net_dialer_dial(Eo *o, Efl_Net_Dialer_Ssl_Data *pd, const char *address)
{
   double timeout;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_net_dialer_connected_get(o), EISCONN);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EBADF);

   if (pd->connect_timeout) eina_future_cancel(pd->connect_timeout);

   timeout = efl_net_dialer_timeout_dial_get(pd->sock);
   if (timeout > 0.0) _timeout_schedule(o, pd, timeout);

   return efl_net_dialer_dial(pd->sock, address);
}

EOLIAN static const char *
_efl_net_dialer_ssl_efl_net_dialer_address_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Ssl_Data *pd)
{
   return efl_net_dialer_address_dial_get(pd->sock);
}

EOLIAN static void
_efl_net_dialer_ssl_efl_net_dialer_proxy_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Ssl_Data *pd, const char *proxy_url)
{
   efl_net_dialer_proxy_set(pd->sock, proxy_url);
}

EOLIAN static const char *
_efl_net_dialer_ssl_efl_net_dialer_proxy_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Ssl_Data *pd)
{
   return efl_net_dialer_proxy_get(pd->sock);
}

EOLIAN static void
_efl_net_dialer_ssl_efl_net_dialer_timeout_dial_set(Eo *o, Efl_Net_Dialer_Ssl_Data *pd, double seconds)
{
   efl_net_dialer_timeout_dial_set(pd->sock, seconds);

   if (pd->connect_timeout) eina_future_cancel(pd->connect_timeout);

   if ((seconds > 0.0) && (!pd->connected)) _timeout_schedule(o, pd, seconds);
}

EOLIAN static double
_efl_net_dialer_ssl_efl_net_dialer_timeout_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Ssl_Data *pd)
{
   return efl_net_dialer_timeout_dial_get(pd->sock);
}

EOLIAN static void
_efl_net_dialer_ssl_efl_net_dialer_connected_set(Eo *o, Efl_Net_Dialer_Ssl_Data *pd, Eina_Bool connected)
{
   if (pd->connect_timeout)
     eina_future_cancel(pd->connect_timeout);
   if (pd->connected == connected) return;
   pd->connected = connected;
   if (connected) efl_event_callback_call(o, EFL_NET_DIALER_EVENT_CONNECTED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_dialer_ssl_efl_net_dialer_connected_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Ssl_Data *pd)
{
   return pd->connected;
}

EOLIAN static Eina_Error
_efl_net_dialer_ssl_efl_io_closer_close(Eo *o, Efl_Net_Dialer_Ssl_Data *pd EINA_UNUSED)
{
   efl_net_dialer_connected_set(o, EINA_FALSE);
   return efl_io_closer_close(efl_super(o, MY_CLASS));
}

EOLIAN static Eina_Bool
_efl_net_dialer_ssl_keep_alive_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Ssl_Data *pd, Eina_Bool keep_alive)
{
   return efl_net_socket_tcp_keep_alive_set(pd->sock, keep_alive);
}

EOLIAN static Eina_Bool
_efl_net_dialer_ssl_keep_alive_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Ssl_Data *pd)
{
   return efl_net_socket_tcp_keep_alive_get(pd->sock);
}

EOLIAN static Eina_Bool
_efl_net_dialer_ssl_no_delay_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Ssl_Data *pd, Eina_Bool no_delay)
{
   return efl_net_socket_tcp_no_delay_set(pd->sock, no_delay);
}

EOLIAN static Eina_Bool
_efl_net_dialer_ssl_no_delay_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Ssl_Data *pd)
{
   return efl_net_socket_tcp_no_delay_get(pd->sock);
}

#include "efl_net_dialer_ssl.eo.c"
