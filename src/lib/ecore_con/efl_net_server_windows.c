#define EFL_NET_SERVER_WINDOWS_PROTECTED 1
#define EFL_NET_SERVER_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

/*
 * See https://msdn.microsoft.com/en-us/library/windows/desktop/aa365601(v=vs.85).aspx
 * Named Pipe Server Using Completion Routines
 *
 * See https://msdn.microsoft.com/en-us/library/windows/desktop/aa365603(v=vs.85).aspx
 * Named Pipe Server Using Overlapped I/O
 *
 * Each instance (PIPEINST) is an Efl_Net_Socket_Windows. Instead of
 * pre-creating all possible instances and having all of them to
 * accept connections (ConnectNamedPipe()), we create a single one
 * (Efl_Net_Server_Windows_Data::next_client), once it's connected we
 * announce the client and if it's used, a new "next_client" is
 * started, otherwise if the announced client is not used, then it's
 * disconnected and reused with a new ConnectNamedPipe().
 */

#define MY_CLASS EFL_NET_SERVER_WINDOWS_CLASS

typedef struct _Efl_Net_Server_Windows_Data
{
   Eo *next_client;
   Eina_List *pending_clients;

   Eina_Stringshare *address; /* includes prefix: \\.\pipe\, returned without it */
   Eina_Future *pending_announcer_job;
   unsigned int clients_count;
   unsigned int clients_limit;
   Eina_Bool clients_reject_excess;
   Eina_Bool serving;
   Eina_Bool first;
   Eina_Bool allow_remote;
} Efl_Net_Server_Windows_Data;

static Eina_Error _efl_net_server_windows_client_listen(Eo *o, Efl_Net_Server_Windows_Data *pd);
static Eina_Error _efl_net_server_windows_client_new(Eo *o, Efl_Net_Server_Windows_Data *pd);

static Eina_Error
_efl_net_server_windows_client_listen_success(void *data, Eo *client, DWORD used_size EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Server_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);
   char cstr[256], sstr[256];
   ULONG cpid = 0, spid = 0;
   const char *addr = efl_net_server_address_get(o);

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd, EINVAL);

#if _WIN32_WINNT >= 0x0600
   if (!GetNamedPipeClientProcessId(_efl_net_socket_windows_handle_get(client), &cpid))
     {
        char *msg = evil_last_error_get();
        WRN("server=%p (%s) could not GetNamedPipeClientProcessId(client): %s", o, addr, msg);
        free(msg);
     }
   if (!GetNamedPipeServerProcessId(_efl_net_socket_windows_handle_get(client), &spid))
     {
        char *msg = evil_last_error_get();
        WRN("server=%p (%s) could not GetNamedPipeServerProcessId(client): %s", o, addr, msg);
        free(msg);
     }
#endif

   if (cpid)
     snprintf(cstr, sizeof(cstr), "%s:%lu", addr, cpid);
   else
     eina_strlcpy(cstr, addr, sizeof(cstr));

   if (spid)
     snprintf(sstr, sizeof(sstr), "%s:%lu", addr, spid);
   else
     eina_strlcpy(sstr, addr, sizeof(sstr));

   DBG("server=%p received incoming connection at %s<->%s", o, sstr, cstr);

   efl_ref(o); /* will trigger events, which call user which may delete us */

   if ((pd->clients_limit > 0) && (pd->clients_count >= pd->clients_limit))
     {
        if (!pd->clients_reject_excess)
          {
             /* keep queueing, but do not call user */

             pd->pending_clients = eina_list_append(pd->pending_clients, client);
             if (pd->next_client == client)
               pd->next_client = NULL;

             efl_net_socket_address_local_set(client, sstr);
             efl_net_socket_address_remote_set(client, cstr);

             DBG("server=%p queued client %p", o, client);
          }
        else
          {
             DBG("server=%p rejecting client %p", o, client);
             efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_REJECTED, cstr);

             if (pd->next_client != client)
               efl_del(client);
             else
               {
                  HANDLE h = _efl_net_socket_windows_handle_get(client);
                  DisconnectNamedPipe(h);

                  /* reuse existing pipe for a new connection */
                  _efl_net_server_windows_client_listen(o, pd);
               }
          }
     }
   else
     {
        efl_net_socket_address_local_set(client, sstr);
        efl_net_socket_address_remote_set(client, cstr);

        DBG("server=%p announcing client %p", o, client);
        if (pd->next_client == client)
          pd->next_client = NULL;
        efl_net_server_client_announce(o, client);
     }

   if (!pd->next_client)
     _efl_net_server_windows_client_new(o, pd);

   efl_unref(o);
   return 0;
}

static Eina_Error
_efl_net_server_windows_client_listen_failure(void *data, Eo *client EINA_UNUSED, DWORD win32err)
{
   Eo *o = data;
   Eina_Error err = EINVAL;

   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        char *msg = _efl_net_windows_error_msg_get(win32err);
        DBG("server=%p failed to accept connection at %s: error=%s",
            o, efl_net_server_address_get(o), msg);
        free(msg);
     }

   switch (win32err)
     {
      case ERROR_PIPE_CONNECTED: err = 0; break;
      case ERROR_NO_DATA: err = ENOTCONN; break;
      case ERROR_OPERATION_ABORTED: err = ECANCELED; break;
     }

   if (err)
     efl_event_callback_call(o, EFL_NET_SERVER_EVENT_ERROR, &err);

   // TODO: create a new one on failure?

   return err;
}

static Eina_Error
_efl_net_server_windows_client_listen(Eo *o, Efl_Net_Server_Windows_Data *pd)
{
   Efl_Net_Socket_Windows_Operation *op;
   HANDLE h;
   OVERLAPPED *ovl;
   DWORD win32err;
   BOOL r;

   op = _efl_net_socket_windows_operation_new(pd->next_client,
                                              _efl_net_server_windows_client_listen_success,
                                              _efl_net_server_windows_client_listen_failure,
                                              o);

   EINA_SAFETY_ON_NULL_RETURN_VAL(op, EINVAL);

   h = _efl_net_socket_windows_handle_get(pd->next_client);
   ovl = _efl_net_socket_windows_operation_overlapped_get(op);

   r = ConnectNamedPipe(h, ovl);
   win32err = GetLastError();
   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        char *msg = _efl_net_windows_error_msg_get(win32err);
        DBG("server=%p (%s) socket=%p ConnectNamedPipe(%p, %p)=%s",
            o, efl_net_server_address_get(o),
            pd->next_client, h, ovl, msg);
        free(msg);
     }

   if (!r)
     {
        if (win32err == ERROR_IO_PENDING)
          return 0;
        else
          {
             return _efl_net_socket_windows_operation_failed(op, win32err);
          }
     }

   return _efl_net_socket_windows_operation_succeeded(op, 0);
}

static Eina_Error
_efl_net_server_windows_client_new(Eo *o, Efl_Net_Server_Windows_Data *pd)
{
   Eina_Error err;
   HANDLE h;

   h = CreateNamedPipe(pd->address,
                       PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED |
                       (pd->first ? FILE_FLAG_FIRST_PIPE_INSTANCE : 0),
                       PIPE_TYPE_BYTE | PIPE_READMODE_BYTE |
                       (pd->allow_remote ? PIPE_ACCEPT_REMOTE_CLIENTS : PIPE_REJECT_REMOTE_CLIENTS),
                       (pd->clients_limit > 0 ? pd->clients_limit : PIPE_UNLIMITED_INSTANCES),
                       4096, 4096, INFINITE, NULL);
   if (h == INVALID_HANDLE_VALUE)
     {
        DWORD win32err = GetLastError();
        switch (win32err)
          {
           case ERROR_ACCESS_DENIED: return EACCES;
           case ERROR_INVALID_PARAMETER: return EINVAL;
           default:
             {
                char *msg = evil_format_message(win32err);
                ERR("Unexpected win32err=%lu (%s)", win32err, msg);
                return EINVAL;
             }
          }
     }

   pd->next_client = efl_add(EFL_NET_SOCKET_WINDOWS_CLASS, o,
                             efl_io_closer_close_on_destructor_set(efl_added, EINA_TRUE));
   if (!pd->next_client)
     {
        err = ENOMEM;
        goto error_socket;
     }

   err = _efl_net_socket_windows_init(pd->next_client, h);
   if (err) goto error_init;

   pd->first = EINA_FALSE;

   err = _efl_net_server_windows_client_listen(o, pd);
   if (err) return err;

   efl_net_server_serving_set(o, EINA_TRUE);
   return 0;

 error_init:
   efl_del(pd->next_client);
 error_socket:
   CloseHandle(h);
   return err;
}

EOLIAN static void
_efl_net_server_windows_allow_remote_set(Eo *o EINA_UNUSED, Efl_Net_Server_Windows_Data *pd, Eina_Bool allow_remote)
{
   pd->allow_remote = allow_remote;
}

EOLIAN static Eina_Bool
_efl_net_server_windows_allow_remote_get(Eo *o EINA_UNUSED, Efl_Net_Server_Windows_Data *pd)
{
   return pd->allow_remote;
}

EOLIAN static Eo *
_efl_net_server_windows_efl_object_constructor(Eo *o, Efl_Net_Server_Windows_Data *pd)
{
   pd->first = EINA_TRUE;

   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_server_windows_efl_object_destructor(Eo *o, Efl_Net_Server_Windows_Data *pd)
{
   if (pd->next_client)
     {
        efl_del(pd->next_client);
        pd->next_client = NULL;
     }

   while (pd->pending_clients)
     {
        Eo *client = pd->pending_clients->data;
        pd->pending_clients = eina_list_remove_list(pd->pending_clients, pd->pending_clients);
        efl_del(client);
     }

   efl_destructor(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_server_windows_efl_net_server_address_set(Eo *o EINA_UNUSED, Efl_Net_Server_Windows_Data *pd, const char *address)
{
   const char *tmp = eina_stringshare_printf(PIPE_NS "%s", address);
   eina_stringshare_del(pd->address);
   pd->address = tmp;
}

EOLIAN static const char *
_efl_net_server_windows_efl_net_server_address_get(Eo *o EINA_UNUSED, Efl_Net_Server_Windows_Data *pd)
{
   return pd->address + strlen(PIPE_NS);
}

static Eina_Value
_efl_net_server_windows_pending_announce_job(Eo *o, const Eina_Value v)
{
   Efl_Net_Server_Windows_Data *pd = efl_data_scope_get(o, MY_CLASS);
   Eo *client;

   if (!pd->pending_clients) return v;
   if ((pd->clients_limit > 0) && (pd->clients_limit <= pd->clients_count)) return v;

   client = pd->pending_clients->data;
   pd->pending_clients = eina_list_remove_list(pd->pending_clients, pd->pending_clients);

   efl_net_server_client_announce(o, client);
   return v;
}

static void
_efl_net_server_windows_pending_announce_job_schedule(Eo *o, Efl_Net_Server_Windows_Data *pd)
{
   Eo *loop;

   if (pd->pending_announcer_job) return;
   if (!pd->pending_clients) return;
   if ((pd->clients_limit > 0) && (pd->clients_limit <= pd->clients_count)) return;

   loop = efl_loop_get(o);
   if (!loop) return;
   efl_future_Eina_FutureXXX_then(o, efl_loop_Eina_FutureXXX_job(loop),
                                  .success = _efl_net_server_windows_pending_announce_job,
                                  .storage = &pd->pending_announcer_job);
}

EOLIAN static void
_efl_net_server_windows_efl_net_server_clients_count_set(Eo *o, Efl_Net_Server_Windows_Data *pd, unsigned int count)
{
   pd->clients_count = count;

   /* a job to avoid blowing the stack with recursion,
    * do each announcement from main loop
    */
   _efl_net_server_windows_pending_announce_job_schedule(o, pd);
}

EOLIAN static unsigned int
_efl_net_server_windows_efl_net_server_clients_count_get(Eo *o EINA_UNUSED, Efl_Net_Server_Windows_Data *pd)
{
   return pd->clients_count;
}

EOLIAN static void
_efl_net_server_windows_efl_net_server_clients_limit_set(Eo *o EINA_UNUSED, Efl_Net_Server_Windows_Data *pd, unsigned int limit, Eina_Bool reject_excess)
{
   pd->clients_limit = limit;
   pd->clients_reject_excess = reject_excess;

   if ((limit > 0) && (reject_excess))
     {
        while (pd->pending_clients)
          {
             Eina_List *last = eina_list_last(pd->pending_clients);
             Eo *client = eina_list_data_get(last);
             efl_del(client);
             pd->pending_clients = eina_list_remove_list(pd->pending_clients, last);
          }
     }

   _efl_net_server_windows_pending_announce_job_schedule(o, pd);
}

EOLIAN static void
_efl_net_server_windows_efl_net_server_clients_limit_get(Eo *o EINA_UNUSED, Efl_Net_Server_Windows_Data *pd, unsigned int *limit, Eina_Bool *reject_excess)
{
   if (limit) *limit = pd->clients_limit;
   if (reject_excess) *reject_excess = pd->clients_reject_excess;
}

EOLIAN static void
_efl_net_server_windows_efl_net_server_serving_set(Eo *o EINA_UNUSED, Efl_Net_Server_Windows_Data *pd, Eina_Bool serving)
{
   if (pd->serving == serving) return;
   pd->serving = serving;
   if (serving)
     efl_event_callback_call(o, EFL_NET_SERVER_EVENT_SERVING, NULL);
}

EOLIAN static Eina_Bool
_efl_net_server_windows_efl_net_server_serving_get(Eo *o EINA_UNUSED, Efl_Net_Server_Windows_Data *pd)
{
   return pd->serving;
}

EOLIAN static Eina_Error
_efl_net_server_windows_efl_net_server_serve(Eo *o, Efl_Net_Server_Windows_Data *pd, const char *address)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(strchr(address, '/') != NULL, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(strchr(address, '\\') != NULL, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(strlen(PIPE_NS) + strlen(address) >= 256, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(pd->serving, EALREADY);

   efl_net_server_address_set(o, address);

   return _efl_net_server_windows_client_new(o, pd);
}

static void
_efl_net_server_windows_client_event_closed(void *data, const Efl_Event *event)
{
   Eo *server = data;
   Eo *client = event->object;

   efl_event_callback_del(client, EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_windows_client_event_closed, server);
   if (efl_parent_get(client) == server)
     efl_parent_set(client, NULL);

   efl_net_server_clients_count_set(server, efl_net_server_clients_count_get(server) - 1);
}

EOLIAN static Eina_Bool
_efl_net_server_windows_efl_net_server_client_announce(Eo *o, Efl_Net_Server_Windows_Data *pd, Efl_Net_Socket *client)
{
   Eina_Error err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(client, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_GOTO(efl_isa(client, EFL_NET_SOCKET_INTERFACE), wrong_type);
   EINA_SAFETY_ON_FALSE_GOTO(efl_parent_get(client) == o, wrong_parent);
   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_ADD, client);

   if (efl_parent_get(client) != o)
     {
        DBG("client %s was reparented! Ignoring it...",
            efl_net_socket_address_remote_get(client));
        return EINA_TRUE;
     }

   if (efl_ref_get(client) == 1) /* users must take a reference themselves */
     {
        DBG("client %s was not handled, closing it...",
            efl_net_socket_address_remote_get(client));
        if (pd->next_client)
          efl_del(client);
        else
          {
             HANDLE h = _efl_net_socket_windows_handle_get(client);
             DisconnectNamedPipe(h);

             /* reuse existing pipe for a new connection */
             pd->next_client = client;
             _efl_net_server_windows_client_listen(o, pd);
          }
        return EINA_FALSE;
     }
   else if (efl_io_closer_closed_get(client))
     {
        DBG("client %s was closed from 'client,add', delete it...",
            efl_net_socket_address_remote_get(client));
        efl_del(client);
        return EINA_FALSE;
     }

   efl_net_server_clients_count_set(o, efl_net_server_clients_count_get(o) + 1);
   efl_event_callback_add(client, EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_windows_client_event_closed, o);

   err = _efl_net_socket_windows_io_start(client);
   if (err)
     {
        WRN("server=%p client=%p failed to start I/O: %s", o, client, eina_error_msg_get(err));
        if (!efl_io_closer_closed_get(client))
          efl_io_closer_close(client);
     }

   return EINA_TRUE;

 wrong_type:
   ERR("%p client %p (%s) doesn't implement Efl.Net.Socket interface, deleting it.", o, client, efl_class_name_get(efl_class_get(client)));
   efl_io_closer_close(client);
   efl_del(client);
   return EINA_FALSE;

 wrong_parent:
   ERR("%p client %p (%s) parent=%p is not our child, deleting it.", o, client, efl_class_name_get(efl_class_get(client)), efl_parent_get(client));
   efl_io_closer_close(client);
   efl_del(client);
   return EINA_FALSE;

}

#include "efl_net_server_windows.eo.c"
