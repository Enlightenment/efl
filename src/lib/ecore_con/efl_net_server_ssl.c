#define EFL_NET_SERVER_SSL_PROTECTED 1
#define EFL_NET_SERVER_FD_PROTECTED 1
#define EFL_NET_SERVER_PROTECTED 1
#define EFL_LOOP_FD_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

#define MY_CLASS EFL_NET_SERVER_SSL_CLASS

typedef struct _Efl_Net_Server_Ssl_Data
{
   Eo *server;
   Eo *ssl_ctx;
} Efl_Net_Server_Ssl_Data;

static void
_efl_net_server_ssl_client_event_closed(void *data, const Efl_Event *event)
{
   Eo *server = data;
   Eo *client = event->object;

   efl_event_callback_del(client, EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_ssl_client_event_closed, server);
   if (efl_parent_get(client) == server)
     efl_parent_set(client, NULL);

   /* do NOT change count as we're using the underlying server's count */
   //efl_net_server_clients_count_set(server, efl_net_server_clients_count_get(server) - 1);
}

static Eina_Bool
_efl_net_server_ssl_efl_net_server_client_announce(Eo *o, Efl_Net_Server_Ssl_Data *pd EINA_UNUSED, Eo *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_GOTO(efl_isa(client, EFL_NET_SOCKET_SSL_CLASS), wrong_type);
   EINA_SAFETY_ON_FALSE_GOTO(efl_parent_get(client) == o, wrong_parent);

   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_ADD, client);

   if (efl_parent_get(client) != o)
     {
        DBG("client %s was reparented! Ignoring it...",
            efl_net_socket_address_remote_get(client));
        return EINA_TRUE;
     }

   if (efl_ref_count(client) == 1) /* users must take a reference themselves */
     {
        DBG("client %s was not handled, closing it...",
            efl_net_socket_address_remote_get(client));
        efl_del(client);
        return EINA_FALSE;
     }
   else if (efl_io_closer_closed_get(client))
     {
        DBG("client %s was closed from 'client,add', delete it...",
            efl_net_socket_address_remote_get(client));
        efl_del(client);
        return EINA_FALSE;
     }

   /* do NOT change count as we're using the underlying server's count */
   //efl_net_server_clients_count_set(o, efl_net_server_clients_count_get(o) + 1);
   efl_event_callback_add(client, EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_ssl_client_event_closed, o);
   return EINA_TRUE;

 wrong_type:
   ERR("%p client %p (%s) doesn't implement Efl.Net.Socket.Ssl class, deleting it.", o, client, efl_class_name_get(efl_class_get(client)));
   efl_del(client);
   return EINA_FALSE;

 wrong_parent:
   ERR("%p client %p (%s) parent=%p is not our child, deleting it.", o, client, efl_class_name_get(efl_class_get(client)), efl_parent_get(client));
   efl_del(client);
   return EINA_FALSE;
}

static void
_efl_net_server_ssl_efl_net_server_fd_client_add(Eo *o, Efl_Net_Server_Ssl_Data *pd, int client_fd)
{
   Eo *client_ssl;
   Eo *client_tcp;
   const char *addr;

   client_tcp = efl_add(EFL_NET_SOCKET_TCP_CLASS, o,
                        efl_io_closer_close_on_exec_set(efl_added, efl_net_server_fd_close_on_exec_get(o)),
                        efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE),
                        efl_loop_fd_set(efl_added, client_fd));
   if (!client_tcp)
     {
        ERR("could not create client object fd=" SOCKET_FMT, (SOCKET)client_fd);
        closesocket(client_fd);
        return;
     }

   addr = efl_net_socket_address_remote_get(client_tcp);
   if (!pd->ssl_ctx)
     {
        ERR("ssl server %p rejecting client %p '%s' since no SSL context was set!", o, client_tcp, addr);
        efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_REJECTED, (void *)addr);
        efl_del(client_tcp);
        return;
     }

   client_ssl = efl_add(EFL_NET_SOCKET_SSL_CLASS, o,
                        efl_net_socket_ssl_adopt(efl_added, client_tcp, pd->ssl_ctx));
   if (!client_ssl)
     {
        ERR("ssl server %p could not wrap client %p '%s' using context=%p", o, client_tcp, addr, pd->ssl_ctx);
        efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_REJECTED, (void *)addr);
        efl_del(client_tcp);
        return;
     }

   efl_net_server_client_announce(o, client_ssl);
}

EOLIAN Efl_Object *
_efl_net_server_ssl_efl_object_constructor(Eo *o, Efl_Net_Server_Ssl_Data *pd)
{
   o = efl_constructor(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   pd->server = efl_add(EFL_NET_SERVER_TCP_CLASS, o);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->server, NULL);
   if (!efl_composite_attach(o, pd->server))
     goto on_error;

   return o;

 on_error:
   ERR("Failed to composite the object\n");
   efl_del(pd->server);
   efl_del(o);
   return NULL;
}

static void
_efl_net_server_ssl_ctx_del(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *o = data;
   Efl_Net_Server_Ssl_Data *pd = efl_data_scope_get(o, MY_CLASS);
   pd->ssl_ctx = NULL;
}

EOLIAN void
_efl_net_server_ssl_efl_object_invalidate(Eo *o, Efl_Net_Server_Ssl_Data *pd)
{
   pd->server = NULL;

   efl_invalidate(efl_super(o, MY_CLASS));
}

EOLIAN void
_efl_net_server_ssl_efl_object_destructor(Eo *o, Efl_Net_Server_Ssl_Data *pd)
{
   if (pd->ssl_ctx)
     {
        efl_event_callback_del(pd->ssl_ctx, EFL_EVENT_DEL, _efl_net_server_ssl_ctx_del, o);
        efl_unref(pd->ssl_ctx);
        pd->ssl_ctx = NULL;
     }

   efl_destructor(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_server_ssl_ssl_context_set(Eo *o EINA_UNUSED, Efl_Net_Server_Ssl_Data *pd, Eo *ssl_ctx)
{
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(ssl_ctx, EFL_NET_SSL_CONTEXT_CLASS));

   if (pd->ssl_ctx == ssl_ctx) return;
   efl_unref(pd->ssl_ctx);
   pd->ssl_ctx = efl_ref(ssl_ctx);
   if (ssl_ctx)
     efl_event_callback_add(ssl_ctx, EFL_EVENT_DEL, _efl_net_server_ssl_ctx_del, o);
}

EOLIAN static Eo *
_efl_net_server_ssl_ssl_context_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Ssl_Data *pd)
{
   return pd->ssl_ctx;
}

#include "efl_net_server_ssl.eo.c"
