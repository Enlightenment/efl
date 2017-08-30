#define EFL_NET_DIALER_UNIX_PROTECTED 1
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
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#define MY_CLASS EFL_NET_DIALER_UNIX_CLASS

typedef struct _Efl_Net_Dialer_Unix_Data
{
   struct {
      Ecore_Thread *thread;
      Eina_Future *timeout;
   } connect;
   Eina_Stringshare *address_dial;
   Eina_Bool connected;
   double timeout_dial;
} Efl_Net_Dialer_Unix_Data;

EOLIAN static Eo*
_efl_net_dialer_unix_efl_object_constructor(Eo *o, Efl_Net_Dialer_Unix_Data *pd EINA_UNUSED)
{
   o = efl_constructor(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   efl_net_dialer_timeout_dial_set(o, 30.0);
   return o;
}

EOLIAN static void
_efl_net_dialer_unix_efl_object_destructor(Eo *o, Efl_Net_Dialer_Unix_Data *pd)
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
}

static Eina_Value
_efl_net_dialer_unix_connect_timeout(Eo *o, const Eina_Value v)
{
   Efl_Net_Dialer_Unix_Data *pd = efl_data_scope_get(o, MY_CLASS);
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
_efl_net_dialer_unix_connected(void *data, const struct sockaddr *addr, socklen_t addrlen EINA_UNUSED, SOCKET sockfd, Eina_Error err)
{
   Eo *o = data;
   Efl_Net_Dialer_Unix_Data *pd = efl_data_scope_get(o, MY_CLASS);

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
        efl_io_reader_eos_set(o, EINA_TRUE);
        efl_event_callback_call(o, EFL_NET_DIALER_EVENT_ERROR, &err);
     }

   efl_unref(o);
}

static void
_timeout_schedule(Eo *o, Efl_Net_Dialer_Unix_Data *pd)
{
   efl_future_Eina_FutureXXX_then(o, efl_loop_Eina_FutureXXX_timeout(efl_loop_get(o), pd->timeout_dial),
                                  .success = _efl_net_dialer_unix_connect_timeout,
                                  .storage = &pd->connect.timeout);
}

EOLIAN static Eina_Error
_efl_net_dialer_unix_efl_net_dialer_dial(Eo *o, Efl_Net_Dialer_Unix_Data *pd, const char *address)
{
   struct sockaddr_un addr = { .sun_family = AF_UNIX };
   socklen_t addrlen;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(address[0] == '\0', EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_net_dialer_connected_get(o), EISCONN);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EBADF);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_loop_fd_get(o) >= 0, EALREADY);

   if (pd->connect.thread)
     {
        ecore_thread_cancel(pd->connect.thread);
        pd->connect.thread = NULL;
     }

   if (strncmp(address, "abstract:", strlen("abstract:")) == 0)
     {
        const char *path = address + strlen("abstract:");
        if (strlen(path) + 2 > sizeof(addr.sun_path))
          {
             ERR("abstract path is too long: %s", path);
             return EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
          }
        addr.sun_path[0] = '\0';
        memcpy(addr.sun_path + 1, path, strlen(path) + 1);
        addrlen = strlen(path) + 2 + offsetof(struct sockaddr_un, sun_path);
     }
   else
     {
        const char *path = address;
        if (strlen(path) + 1 > sizeof(addr.sun_path))
          {
             ERR("path is too long: %s", path);
             return EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
          }
        memcpy(addr.sun_path, path, strlen(path) + 1);
        addrlen = strlen(path) + 1 + offsetof(struct sockaddr_un, sun_path);
     }

   pd->connect.thread = efl_net_connect_async_new((const struct sockaddr *)&addr, addrlen, SOCK_STREAM, 0,
                                                  efl_io_closer_close_on_exec_get(o),
                                                  _efl_net_dialer_unix_connected, o);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->connect.thread, EINVAL);

   efl_net_dialer_address_dial_set(o, address);

   if (pd->connect.timeout) eina_future_cancel(pd->connect.timeout);
   if (pd->timeout_dial > 0.0) _timeout_schedule(o, pd);

   return 0;
}

EOLIAN static void
_efl_net_dialer_unix_efl_net_dialer_address_dial_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Unix_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_dial, address);
}

EOLIAN static const char *
_efl_net_dialer_unix_efl_net_dialer_address_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Unix_Data *pd)
{
   return pd->address_dial;
}

EOLIAN static void
_efl_net_dialer_unix_efl_net_dialer_timeout_dial_set(Eo *o, Efl_Net_Dialer_Unix_Data *pd, double seconds)
{
   pd->timeout_dial = seconds;

   if (pd->connect.timeout) eina_future_cancel(pd->connect.timeout);
   if ((pd->timeout_dial > 0.0) && (pd->connect.thread)) _timeout_schedule(o, pd);
}

EOLIAN static double
_efl_net_dialer_unix_efl_net_dialer_timeout_dial_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Unix_Data *pd)
{
   return pd->timeout_dial;
}

EOLIAN static void
_efl_net_dialer_unix_efl_net_dialer_connected_set(Eo *o, Efl_Net_Dialer_Unix_Data *pd, Eina_Bool connected)
{
   if (pd->connect.timeout) eina_future_cancel(pd->connect.timeout);
   if (pd->connected == connected) return;
   pd->connected = connected;
   if (connected) efl_event_callback_call(o, EFL_NET_DIALER_EVENT_CONNECTED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_dialer_unix_efl_net_dialer_connected_get(Eo *o EINA_UNUSED, Efl_Net_Dialer_Unix_Data *pd)
{
   return pd->connected;
}

EOLIAN static Eina_Error
_efl_net_dialer_unix_efl_io_closer_close(Eo *o, Efl_Net_Dialer_Unix_Data *pd EINA_UNUSED)
{
   efl_net_dialer_connected_set(o, EINA_FALSE);
   return efl_io_closer_close(efl_super(o, MY_CLASS));
}

#include "efl_net_dialer_unix.eo.c"
