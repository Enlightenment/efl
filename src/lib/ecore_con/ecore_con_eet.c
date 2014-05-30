#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#include <Eina.h>

#include "Ecore_Con_Eet.h"

#define ECORE_CON_EET_RAW_MAGIC 0xDEAD007

typedef struct _Ecore_Con_Eet_Data     Ecore_Con_Eet_Data;
typedef struct _Ecore_Con_Eet_Raw_Data Ecore_Con_Eet_Raw_Data;
typedef struct _Ecore_Con_Eet_Client   Ecore_Con_Eet_Client;
typedef struct _Ecore_Con_Eet_Server   Ecore_Con_Eet_Server;

struct _Ecore_Con_Reply
{
   Ecore_Con_Eet          *ece;
   Ecore_Con_Client       *client;

   Eet_Connection         *econn;

   char                   *buffer_section;
   unsigned char          *buffer;
   unsigned int            buffer_length;
   unsigned int            buffer_current;
   Ecore_Con_Eet_Raw_Data *buffer_handler;
};

struct _Ecore_Con_Eet_Data
{
   Ecore_Con_Eet_Data_Cb func;
   const char           *name;
   const void           *data;
};

struct _Ecore_Con_Eet_Raw_Data
{
   Ecore_Con_Eet_Raw_Data_Cb func;
   const char               *name;
   const void               *data;
};

struct _Ecore_Con_Eet_Client
{
   Ecore_Con_Eet_Client_Cb func;
   const void             *data;
};

struct _Ecore_Con_Eet_Server
{
   Ecore_Con_Eet_Server_Cb func;
   const void             *data;
};

struct _Ecore_Con_Eet
{
   Ecore_Con_Server    *server;

   Ecore_Event_Handler *handler_add;
   Ecore_Event_Handler *handler_del;
   Ecore_Event_Handler *handler_data;

   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor *matching;

   Eina_Hash           *data_callbacks;
   Eina_Hash           *raw_data_callbacks;

   union
   {
      struct
      {
         Eina_List *connections;
         Eina_List *client_connect_callbacks;
         Eina_List *client_disconnect_callbacks;
      } server;
      struct
      {
         Ecore_Con_Reply *r;
         Eina_List       *server_connect_callbacks;
         Eina_List       *server_disconnect_callbacks;
      } client;
   } u;

   const void *data;

   Eina_Bool   client : 1;
};

static void
_ecore_con_eet_data_free(void *data)
{
   Ecore_Con_Eet_Data *eced = data;

   eina_stringshare_del(eced->name);
   free(eced);
}

static void
_ecore_con_eet_raw_data_free(void *data)
{
   Ecore_Con_Eet_Raw_Data *eced = data;

   eina_stringshare_del(eced->name);
   free(eced);
}

static void
_ecore_con_eet_reply_cleanup(Ecore_Con_Reply *n)
{
   if (n->buffer_handler) free(n->buffer);
   n->buffer = NULL;
   n->buffer_handler = NULL;
   free(n->buffer_section);
   n->buffer_section = NULL;
}

typedef struct _Ecore_Con_Eet_Protocol Ecore_Con_Eet_Protocol;
struct _Ecore_Con_Eet_Protocol
{
   const char *type;
   void       *data;
};

static const char *
_ecore_con_eet_data_type_get(const void *data, Eina_Bool *unknow EINA_UNUSED)
{
   const Ecore_Con_Eet_Protocol *p = data;

   return p->type;
}

static Eina_Bool
_ecore_con_eet_data_type_set(const char *type, void *data, Eina_Bool unknow EINA_UNUSED)
{
   Ecore_Con_Eet_Protocol *p = data;

   p->type = type;
   return EINA_TRUE;
}

static void
_ecore_con_eet_data_descriptor_setup(Ecore_Con_Eet *ece)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Ecore_Con_Eet_Protocol);
   ece->edd = eet_data_descriptor_stream_new(&eddc);

   eddc.version = EET_DATA_DESCRIPTOR_CLASS_VERSION;
   eddc.func.type_get = _ecore_con_eet_data_type_get;
   eddc.func.type_set = _ecore_con_eet_data_type_set;
   ece->matching = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_VARIANT(ece->edd, Ecore_Con_Eet_Protocol, "data", data, type, ece->matching);
}

/* Dealing with a server listening to connection */
static Eina_Bool
_ecore_con_eet_read_cb(const void *eet_data, size_t size, void *user_data)
{
   Ecore_Con_Reply *n = user_data;
   Ecore_Con_Eet_Protocol *protocol;
   Ecore_Con_Eet_Data *cb;

   protocol = eet_data_descriptor_decode(n->ece->edd, eet_data, size);
   if (!protocol) return EINA_TRUE;

   cb = eina_hash_find(n->ece->data_callbacks, protocol->type);
   if (!cb) return EINA_TRUE;  /* Should I report unknow protocol communication ? */

   cb->func((void *)cb->data, n, cb->name, protocol->data);

   eina_stringshare_del(protocol->type);
   free(protocol);

   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_eet_server_write_cb(const void *data, size_t size, void *user_data)
{
   Ecore_Con_Reply *n = user_data;

   if (ecore_con_client_send(n->client, data, size) != (int)size)
     return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_eet_client_write_cb(const void *data, size_t size, void *user_data)
{
   Ecore_Con_Reply *n = user_data;

   if (ecore_con_server_send(n->ece->server, data, size) != (int)size)
     return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_eet_server_connected(void *data, int type EINA_UNUSED, Ecore_Con_Event_Client_Add *ev)
{
   Ecore_Con_Eet_Client *ecec;
   Eina_List *ll;
   Ecore_Con_Eet *r = data;
   Ecore_Con_Reply *n;

   if (ecore_con_client_server_get(ev->client) != r->server)
     return EINA_TRUE;

   n = calloc(1, sizeof (Ecore_Con_Reply));
   if (!n) return EINA_TRUE;

   n->client = ev->client;
   n->ece = r;
   n->econn = eet_connection_new(_ecore_con_eet_read_cb, _ecore_con_eet_server_write_cb, n);
   ecore_con_client_data_set(n->client, n);

   EINA_LIST_FOREACH(r->u.server.client_connect_callbacks, ll, ecec)
     if (!ecec->func((void *)ecec->data, n, n->client))
       {
          eet_connection_close(n->econn, NULL);
          free(n);
          return EINA_TRUE;
       }

   r->u.server.connections = eina_list_append(r->u.server.connections, n);

   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_eet_server_disconnected(void *data, int type EINA_UNUSED, Ecore_Con_Event_Client_Del *ev)
{
   Ecore_Con_Eet *r = data;
   Ecore_Con_Reply *n;
   Eina_List *l;

   if (ecore_con_client_server_get(ev->client) != r->server)
     return EINA_TRUE;

   EINA_LIST_FOREACH(r->u.server.connections, l, n)
     if (n->client == ev->client)
       {
          Ecore_Con_Eet_Client *ecec;
          Eina_List *ll;

          EINA_LIST_FOREACH(r->u.server.client_disconnect_callbacks, ll, ecec)
            ecec->func((void *)ecec->data, n, n->client);

          eet_connection_close(n->econn, NULL);
          free(n);
          r->u.server.connections = eina_list_remove_list(r->u.server.connections, l);
          return EINA_TRUE;
       }

   return EINA_TRUE;
}

static void
_ecore_con_eet_raw_data_push(Ecore_Con_Reply *n, void *data, int size)
{
   if (n->buffer_handler)
     memcpy(n->buffer + n->buffer_current, data, size);
   n->buffer_current += size;

   if (n->buffer_current == n->buffer_length)
     {
        if (n->buffer_handler)
          n->buffer_handler->func((void *)n->buffer_handler->data, n, n->buffer_handler->name, n->buffer_section, n->buffer, n->buffer_length);
        _ecore_con_eet_reply_cleanup(n);
     }
}

static void
_ecore_con_eet_data(Ecore_Con_Reply *n, void *data, unsigned int size)
{
   /* FIXME: Enforce detection of attack and kill connection on that case */
   if (n->buffer)
     {
        if (n->buffer_current + size > n->buffer_length)
          {
             _ecore_con_eet_reply_cleanup(n);
             return;
          }

        _ecore_con_eet_raw_data_push(n, data, size);
        return;
     }
   else if (eet_connection_empty(n->econn) && size > (int)(4 * sizeof (unsigned int) + 2))
     {
        unsigned int *tmp = data;
        size -= 4 * sizeof (unsigned int);

        if (ntohl(tmp[0]) == ECORE_CON_EET_RAW_MAGIC)
          {
             unsigned int protocol_length = ntohl(tmp[1]);
             unsigned int section_length = ntohl(tmp[2]);
             unsigned int data_length = ntohl(tmp[3]);

             if (protocol_length > 1 && section_length > 1 && protocol_length + section_length <= size && data_length < 10 * 1024 * 1024)
               {
                  char *buffer = (char *)&tmp[4];
                  char *protocol;
                  char *section;

                  protocol = buffer;
                  section = buffer + protocol_length;

                  if (protocol[protocol_length - 1] == '\0' &&
                      section[section_length - 1] == '\0')
                    {
                       size -= protocol_length + section_length;
                       buffer = section + section_length;

                       n->buffer_handler = eina_hash_find(n->ece->raw_data_callbacks, protocol);
                       n->buffer_section = strdup(section);
                       n->buffer_length = data_length;
                       n->buffer_current = 0;
                       if (n->buffer_handler)
                         n->buffer = malloc(sizeof (unsigned char) * data_length);
                       else
                         n->buffer = (void *)1;
                       if (n->buffer)
                         {
                            _ecore_con_eet_raw_data_push(n, buffer, size);
                            return;
                         }
                       _ecore_con_eet_reply_cleanup(n);

                       size += protocol_length + section_length;
                    }
               }
          }

        size += 4 * sizeof (unsigned int);
     }

   eet_connection_received(n->econn, data, size);
}

static Eina_Bool
_ecore_con_eet_server_data(void *data, int type EINA_UNUSED, Ecore_Con_Event_Client_Data *ev)
{
   Ecore_Con_Eet *r = data;
   Ecore_Con_Reply *n;

   if (ecore_con_client_server_get(ev->client) != r->server)
     return EINA_TRUE;

   n = ecore_con_client_data_get(ev->client);

   _ecore_con_eet_data(n, ev->data, ev->size);

   return EINA_TRUE;
}

/* Dealing connection to a server */

static Eina_Bool
_ecore_con_eet_client_connected(void *data, int type EINA_UNUSED, Ecore_Con_Event_Server_Add *ev)
{
   Ecore_Con_Eet_Server *eces;
   Ecore_Con_Eet *r = data;
   Ecore_Con_Reply *n;
   Eina_List *ll;

   /* Client did connect */
   if (r->server != ev->server) return EINA_TRUE;
   if (r->u.client.r) return EINA_TRUE;

   n = calloc(1, sizeof (Ecore_Con_Reply));
   if (!n) return EINA_TRUE;

   n->client = NULL;
   n->ece = r;
   n->econn = eet_connection_new(_ecore_con_eet_read_cb, _ecore_con_eet_client_write_cb, n);

   EINA_LIST_FOREACH(r->u.client.server_connect_callbacks, ll, eces)
     if (!eces->func((void *)eces->data, n, n->ece->server))
       {
          eet_connection_close(n->econn, NULL);
          free(n);
          return EINA_TRUE;
       }

   r->u.client.r = n;

   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_eet_client_disconnected(void *data, int type EINA_UNUSED, Ecore_Con_Event_Server_Del *ev)
{
   Ecore_Con_Eet *r = data;
   Ecore_Con_Eet_Server *eces;
   Eina_List *ll;

   if (r->server != ev->server) return EINA_TRUE;
   if (!r->u.client.r) return EINA_TRUE;

   /* Client disconnected */
   EINA_LIST_FOREACH(r->u.client.server_disconnect_callbacks, ll, eces)
     eces->func((void *)eces->data, r->u.client.r, r->server);

   eet_connection_close(r->u.client.r->econn, NULL);
   free(r->u.client.r);
   r->u.client.r = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_eet_client_data(void *data, int type EINA_UNUSED, Ecore_Con_Event_Server_Data *ev)
{
   Ecore_Con_Eet *r = data;

   if (r->server != ev->server) return EINA_TRUE;
   if (!r->u.client.r) return EINA_TRUE;

   /* Got some data */
   _ecore_con_eet_data(r->u.client.r, ev->data, ev->size);

   return EINA_TRUE;
}

/**************
* Global API *
**************/

EAPI Ecore_Con_Eet *
ecore_con_eet_server_new(Ecore_Con_Server *server)
{
   Ecore_Con_Eet *r;

   if (!server) return NULL;

   r = calloc(1, sizeof (Ecore_Con_Eet));
   if (!r) return NULL;

   r->server = server;
   r->handler_add = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,
                                            (Ecore_Event_Handler_Cb)_ecore_con_eet_server_connected, r);
   r->handler_del = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,
                                            (Ecore_Event_Handler_Cb)_ecore_con_eet_server_disconnected, r);
   r->handler_data = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
                                             (Ecore_Event_Handler_Cb)_ecore_con_eet_server_data, r);
   r->data_callbacks = eina_hash_stringshared_new(_ecore_con_eet_data_free);
   r->raw_data_callbacks = eina_hash_string_superfast_new(_ecore_con_eet_raw_data_free);

   _ecore_con_eet_data_descriptor_setup(r);

   return r;
}

EAPI Ecore_Con_Eet *
ecore_con_eet_client_new(Ecore_Con_Server *server)
{
   Ecore_Con_Eet *r;

   if (!server) return NULL;

   r = calloc(1, sizeof (Ecore_Con_Eet));
   if (!r) return NULL;

   r->client = EINA_TRUE;
   r->server = server;
   r->handler_add = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
                                            (Ecore_Event_Handler_Cb)_ecore_con_eet_client_connected, r);
   r->handler_del = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,
                                            (Ecore_Event_Handler_Cb)_ecore_con_eet_client_disconnected, r);
   r->handler_data = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
                                             (Ecore_Event_Handler_Cb)_ecore_con_eet_client_data, r);
   r->data_callbacks = eina_hash_stringshared_new(_ecore_con_eet_data_free);
   r->raw_data_callbacks = eina_hash_string_superfast_new(_ecore_con_eet_raw_data_free);

   _ecore_con_eet_data_descriptor_setup(r);

   return r;
}

EAPI void
ecore_con_eet_server_free(Ecore_Con_Eet *r)
{
   if (!r) return;

   eet_data_descriptor_free(r->edd);
   eet_data_descriptor_free(r->matching);
   eina_hash_free(r->data_callbacks);
   eina_hash_free(r->raw_data_callbacks);

   if (r->client)
     {
        Ecore_Con_Eet_Server *s;

        if (r->u.client.r)
          {
             _ecore_con_eet_reply_cleanup(r->u.client.r);
             eet_connection_close(r->u.client.r->econn, NULL);
             free(r->u.client.r);
          }
        EINA_LIST_FREE(r->u.client.server_connect_callbacks, s)
          free(s);
        EINA_LIST_FREE(r->u.client.server_disconnect_callbacks, s)
          free(s);
     }
   else
     {
        Ecore_Con_Reply *n;
        Ecore_Con_Eet_Client *c;

        EINA_LIST_FREE(r->u.server.connections, n)
          {
             _ecore_con_eet_reply_cleanup(n);
             eet_connection_close(n->econn, NULL);
             free(n);
          }
        EINA_LIST_FREE(r->u.server.client_connect_callbacks, c)
          free(c);
        EINA_LIST_FREE(r->u.server.client_disconnect_callbacks, c)
          free(c);
     }

   ecore_event_handler_del(r->handler_add);
   ecore_event_handler_del(r->handler_del);
   ecore_event_handler_del(r->handler_data);
   free(r);
}

EAPI void
ecore_con_eet_register(Ecore_Con_Eet *ece, const char *name, Eet_Data_Descriptor *edd)
{
   if (!ece) return;

   EET_DATA_DESCRIPTOR_ADD_MAPPING(ece->matching, name, edd);
}

EAPI void
ecore_con_eet_data_callback_add(Ecore_Con_Eet *ece, const char *name, Ecore_Con_Eet_Data_Cb func, const void *data)
{
   Ecore_Con_Eet_Data *eced;

   if (!ece) return;

   eced = calloc(1, sizeof (Ecore_Con_Eet_Data));
   if (!eced) return;

   eced->func = func;
   eced->data = data;
   eced->name = eina_stringshare_add(name);

   eina_hash_direct_add(ece->data_callbacks, eced->name, eced);
}

EAPI void
ecore_con_eet_data_callback_del(Ecore_Con_Eet *ece, const char *name)
{
   if (!ece) return;
   eina_hash_del(ece->data_callbacks, name, NULL);
}

EAPI void
ecore_con_eet_raw_data_callback_add(Ecore_Con_Eet *ece, const char *name, Ecore_Con_Eet_Raw_Data_Cb func, const void *data)
{
   Ecore_Con_Eet_Raw_Data *eced;

   if (!ece) return;

   eced = calloc(1, sizeof (Ecore_Con_Eet_Raw_Data));
   if (!eced) return;

   eced->func = func;
   eced->data = data;
   eced->name = eina_stringshare_add(name);

   eina_hash_direct_add(ece->raw_data_callbacks, eced->name, eced);
}

EAPI void
ecore_con_eet_raw_data_callback_del(Ecore_Con_Eet *ece, const char *name)
{
   if (!ece) return;

   if (ece->client && ece->u.client.r->buffer_handler && !strcmp(ece->u.client.r->buffer_handler->name, name))
     {
        ece->u.client.r->buffer_handler = NULL;
        free(ece->u.client.r->buffer);
        ece->u.client.r->buffer = (void *)1;
     }
   eina_hash_del(ece->raw_data_callbacks, name, NULL);
}

EAPI void
ecore_con_eet_client_connect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data)
{
   Ecore_Con_Eet_Client *c;

   if (!ece || !func) return;

   c = calloc(1, sizeof (Ecore_Con_Eet_Client));
   if (!c) return;

   c->func = func;
   c->data = data;

   ece->u.server.client_connect_callbacks = eina_list_append(ece->u.server.client_connect_callbacks, c);
}

EAPI void
ecore_con_eet_client_connect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data)
{
   Ecore_Con_Eet_Client *c;
   Eina_List *l;

   if (!ece || !func) return;

   EINA_LIST_FOREACH(ece->u.server.client_connect_callbacks, l, c)
     if (c->func == func && c->data == data)
       {
          ece->u.server.client_connect_callbacks = eina_list_remove_list(ece->u.server.client_connect_callbacks, l);
          free(c);
          return;
       }
}

EAPI void
ecore_con_eet_client_disconnect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data)
{
   Ecore_Con_Eet_Client *c;

   if (!ece || !func) return;

   c = calloc(1, sizeof (Ecore_Con_Eet_Client));
   if (!c) return;

   c->func = func;
   c->data = data;

   ece->u.server.client_disconnect_callbacks = eina_list_append(ece->u.server.client_disconnect_callbacks, c);
}

EAPI void
ecore_con_eet_client_disconnect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data)
{
   Ecore_Con_Eet_Client *c;
   Eina_List *l;

   if (!ece || !func) return;

   EINA_LIST_FOREACH(ece->u.server.client_disconnect_callbacks, l, c)
     if (c->func == func && c->data == data)
       {
          ece->u.server.client_disconnect_callbacks = eina_list_remove_list(ece->u.server.client_disconnect_callbacks,
                                                                            l);
          free(c);
          return;
       }
}

EAPI void
ecore_con_eet_server_connect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data)
{
   Ecore_Con_Eet_Server *s;

   if (!ece || !func) return;

   s = calloc(1, sizeof (Ecore_Con_Eet_Server));
   if (!s) return;

   s->func = func;
   s->data = data;

   ece->u.client.server_connect_callbacks = eina_list_append(ece->u.client.server_connect_callbacks, s);
}

EAPI void
ecore_con_eet_server_connect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data)
{
   Ecore_Con_Eet_Server *s;
   Eina_List *l;

   if (!ece || !func) return;

   EINA_LIST_FOREACH(ece->u.client.server_connect_callbacks, l, s)
     if (s->func == func && s->data == data)
       {
          ece->u.client.server_connect_callbacks = eina_list_remove_list(ece->u.client.server_connect_callbacks, l);
          free(s);
          return;
       }
}

EAPI void
ecore_con_eet_server_disconnect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data)
{
   Ecore_Con_Eet_Server *s;

   if (!ece || !func) return;

   s = calloc(1, sizeof (Ecore_Con_Eet_Server));
   if (!s) return;

   s->func = func;
   s->data = data;

   ece->u.client.server_disconnect_callbacks = eina_list_append(ece->u.client.server_disconnect_callbacks, s);
}

EAPI void
ecore_con_eet_server_disconnect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data)
{
   Ecore_Con_Eet_Server *s;
   Eina_List *l;

   if (!ece || !func) return;

   EINA_LIST_FOREACH(ece->u.client.server_disconnect_callbacks, l, s)
     if (s->func == func && s->data == data)
       {
          ece->u.client.server_disconnect_callbacks = eina_list_remove_list(ece->u.client.server_disconnect_callbacks, l);
          free(s);
          return;
       }
}

EAPI void
ecore_con_eet_data_set(Ecore_Con_Eet *ece, const void *data)
{
   if (!ece) return;

   ece->data = data;
}

EAPI void *
ecore_con_eet_data_get(Ecore_Con_Eet *ece)
{
   if (!ece) return NULL;
   return (void *)ece->data;
}

EAPI Ecore_Con_Eet *
ecore_con_eet_reply(Ecore_Con_Reply *reply)
{
   if (!reply) return NULL;
   return reply->ece;
}

EAPI void
ecore_con_eet_send(Ecore_Con_Reply *reply, const char *name, void *value)
{
   Ecore_Con_Eet_Protocol protocol;

   if (!reply) return;

   protocol.type = name;
   protocol.data = value;

   eet_connection_send(reply->econn, reply->ece->edd, &protocol, NULL);
}

EAPI void
ecore_con_eet_raw_send(Ecore_Con_Reply *reply, const char *protocol_name, const char *section, void *value, unsigned int length)
{
   unsigned int protocol[4];
   unsigned int protocol_length;
   unsigned int section_length;
   unsigned int size;
   char *tmp;

   if (!reply) return;
   if (!protocol_name) return;
   if (!section) return;

   protocol_length = strlen(protocol_name) + 1;
   if (protocol_length == 1) return;
   section_length = strlen(section) + 1;

   protocol[0] = htonl(ECORE_CON_EET_RAW_MAGIC);
   protocol[1] = htonl(protocol_length);
   protocol[2] = htonl(section_length);
   protocol[3] = htonl(length);

   size = sizeof (protocol) + protocol_length + section_length;
   tmp = alloca(size);
   memcpy(tmp, protocol, sizeof (protocol));
   memcpy(tmp + sizeof (protocol), protocol_name, protocol_length);
   memcpy(tmp + sizeof (protocol) + protocol_length, section, section_length);

   if (reply->client)
     {
        ecore_con_client_send(reply->client, tmp, size);
        ecore_con_client_send(reply->client, value, length);
     }
   else
     {
        ecore_con_server_send(reply->ece->server, tmp, size);
        ecore_con_server_send(reply->ece->server, value, length);
     }
}

