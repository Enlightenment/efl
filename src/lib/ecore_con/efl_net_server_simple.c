#define EFL_NET_SERVER_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

typedef struct
{
   const Efl_Class *inner_class;
   Eo *inner_server;
} Efl_Net_Server_Simple_Data;

#define MY_CLASS EFL_NET_SERVER_SIMPLE_CLASS

static void
_efl_net_server_simple_client_event_closed(void *data, const Efl_Event *event)
{
   Eo *server = data;
   Eo *client = event->object;

   efl_event_callback_del(client, EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_simple_client_event_closed, server);
   if (efl_parent_get(client) == server)
     efl_parent_set(client, NULL);

   /* do NOT change count as we're using the underlying server's count */
   //efl_net_server_clients_count_set(server, efl_net_server_clients_count_get(server) - 1);
}

static Eina_Bool
_efl_net_server_simple_efl_net_server_client_announce(Eo *o, Efl_Net_Server_Simple_Data *pd EINA_UNUSED, Eo *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_GOTO(efl_isa(client, EFL_NET_SOCKET_SIMPLE_CLASS), wrong_type);
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
   efl_event_callback_add(client, EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_simple_client_event_closed, o);
   return EINA_TRUE;

 wrong_type:
   ERR("%p client %p (%s) doesn't implement Efl.Net.Socket.Simple class, deleting it.", o, client, efl_class_name_get(efl_class_get(client)));
   efl_del(client);
   return EINA_FALSE;

 wrong_parent:
   ERR("%p client %p (%s) parent=%p is not our child, deleting it.", o, client, efl_class_name_get(efl_class_get(client)), efl_parent_get(client));
   efl_del(client);
   return EINA_FALSE;
}

static void
_efl_net_server_simple_inner_server_client_add(void *data, const Efl_Event *event)
{
   Eo *o = data;
   Eo *client_inner = event->info;
   Eo *client_simple;
   const char *addr = efl_net_socket_address_remote_get(client_inner);

   client_simple = efl_add(EFL_NET_SOCKET_SIMPLE_CLASS, o,
                           efl_io_buffered_stream_inner_io_set(efl_added, client_inner));

   if (!client_simple)
     {
        ERR("simple server %p could not wrap client %p '%s'", o, client_inner, addr);
        efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_REJECTED, (void *)addr);
        return;
     }

   efl_net_server_client_announce(o, client_simple);
}

static void
_efl_net_server_simple_inner_server_client_rejected(void *data, const Efl_Event *event)
{
   Eo *o = data;
   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_REJECTED, event->info);
}

static void
_efl_net_server_simple_inner_server_error(void *data, const Efl_Event *event)
{
   Eo *o = data;
   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_ERROR, event->info);
}

static void
_efl_net_server_simple_inner_server_serving(void *data, const Efl_Event *event)
{
   Eo *o = data;
   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_SERVING, event->info);
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_net_server_simple_inner_server_cbs,
                           { EFL_NET_SERVER_EVENT_CLIENT_ADD, _efl_net_server_simple_inner_server_client_add },
                           { EFL_NET_SERVER_EVENT_CLIENT_REJECTED, _efl_net_server_simple_inner_server_client_rejected },
                           { EFL_NET_SERVER_EVENT_ERROR, _efl_net_server_simple_inner_server_error },
                           { EFL_NET_SERVER_EVENT_SERVING, _efl_net_server_simple_inner_server_serving });

EOLIAN static Efl_Object *
_efl_net_server_simple_efl_object_finalize(Eo *o, Efl_Net_Server_Simple_Data *pd)
{
   if (pd->inner_server) goto end;

   if (!pd->inner_class)
     {
        ERR("no valid server was set with efl_net_server_simple_inner_server_set() and no class set with efl_net_server_simple_inner_class_set()!");
        return NULL;
     }
   else
     {
        Eo *server = efl_add(pd->inner_class, o);
        EINA_SAFETY_ON_NULL_RETURN_VAL(server, NULL);

        if (!efl_isa(server, EFL_NET_SERVER_INTERFACE))
          {
             ERR("class %s=%p doesn't implement Efl.Net.Server interface!", efl_class_name_get(pd->inner_class), pd->inner_class);
             efl_del(server);
             return NULL;
          }
        DBG("created new inner server %p (%s)", server, efl_class_name_get(efl_class_get(server)));

        efl_net_server_simple_inner_server_set(o, server);
     }

 end:
   return efl_finalize(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_server_simple_efl_object_invalidate(Eo *o, Efl_Net_Server_Simple_Data *pd)
{
   if (pd->inner_server)
     {
        efl_event_callback_array_del(pd->inner_server, _efl_net_server_simple_inner_server_cbs(), o);

        efl_xunref(pd->inner_server, o);
        pd->inner_server = NULL;
     }

   pd->inner_class = NULL;

   efl_invalidate(efl_super(o, MY_CLASS));
}

EOLIAN static Eina_Error
_efl_net_server_simple_efl_net_server_serve(Eo *o EINA_UNUSED, Efl_Net_Server_Simple_Data *pd, const char *address)
{
   return efl_net_server_serve(pd->inner_server, address);
}

EOLIAN static const char *
_efl_net_server_simple_efl_net_server_address_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Simple_Data *pd)
{
   return efl_net_server_address_get(pd->inner_server);
}

EOLIAN static unsigned int
_efl_net_server_simple_efl_net_server_clients_count_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Simple_Data *pd)
{
   return efl_net_server_clients_count_get(pd->inner_server);
}

EOLIAN static void
_efl_net_server_simple_efl_net_server_clients_limit_set(Eo *o EINA_UNUSED, Efl_Net_Server_Simple_Data *pd, unsigned int limit, Eina_Bool reject_excess)
{
   efl_net_server_clients_limit_set(pd->inner_server, limit, reject_excess);
}

EOLIAN static void
_efl_net_server_simple_efl_net_server_clients_limit_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Simple_Data *pd, unsigned int *limit, Eina_Bool *reject_excess)
{
   efl_net_server_clients_limit_get(pd->inner_server, limit, reject_excess);
}

EOLIAN static Eina_Bool
_efl_net_server_simple_efl_net_server_serving_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Simple_Data *pd)
{
   return efl_net_server_serving_get(pd->inner_server);
}

EOLIAN static void
_efl_net_server_simple_inner_class_set(Eo *o, Efl_Net_Server_Simple_Data *pd, const Efl_Class *klass)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   EINA_SAFETY_ON_NULL_RETURN(klass);
   pd->inner_class = klass;
   DBG("%p inner_class=%p %s", o, klass, efl_class_name_get(klass));
}

EOLIAN static const Efl_Class *
_efl_net_server_simple_inner_class_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Simple_Data *pd)
{
   return pd->inner_class;
}

EOLIAN static void
_efl_net_server_simple_inner_server_set(Eo *o, Efl_Net_Server_Simple_Data *pd, Efl_Object *server)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   EINA_SAFETY_ON_NULL_RETURN(server);
   EINA_SAFETY_ON_TRUE_RETURN(pd->inner_server != NULL);
   EINA_SAFETY_ON_FALSE_RETURN(efl_isa(server, EFL_NET_SERVER_INTERFACE));

   pd->inner_server = efl_xref(server, o);
   efl_event_callback_array_add(server, _efl_net_server_simple_inner_server_cbs(), o);
   DBG("%p inner_server=%p (%s)", o, server, efl_class_name_get(efl_class_get(server)));
}

EOLIAN static Efl_Object *
_efl_net_server_simple_inner_server_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Simple_Data *pd)
{
   return pd->inner_server;
}

#include "efl_net_server_simple.eo.c"
