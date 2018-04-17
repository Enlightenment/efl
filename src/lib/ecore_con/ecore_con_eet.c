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

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"
#include "Ecore_Con_Eet.h"

#define ECORE_CON_EET_RAW_MAGIC 0xDEAD007
#define ECORE_CON_EET_DATA_KEY "ecore_con_eet_data_key"

typedef struct _Ecore_Con_Eet_Base_Data Ecore_Con_Eet_Base_Data;
typedef struct _Ecore_Con_Eet_Server_Obj_Data Ecore_Con_Eet_Server_Obj_Data;
typedef struct _Ecore_Con_Eet_Client_Obj_Data Ecore_Con_Eet_Client_Obj_Data;
typedef struct _Ecore_Con_Eet_Data     Ecore_Con_Eet_Data;
typedef struct _Ecore_Con_Eet_Raw_Data Ecore_Con_Eet_Raw_Data;
typedef struct _Ecore_Con_Eet_Client   Ecore_Con_Eet_Client;
typedef struct _Ecore_Con_Eet_Server   Ecore_Con_Eet_Server;

struct _Ecore_Con_Eet_Server_Obj_Data
{
   Eina_List *connections;
   Eina_List *client_connect_callbacks;
   Eina_List *client_disconnect_callbacks;

   Ecore_Event_Handler *handler_add;
   Ecore_Event_Handler *handler_del;
   Ecore_Event_Handler *handler_data;
};

struct _Ecore_Con_Eet_Client_Obj_Data
{
   Ecore_Con_Reply *r;
   Eina_List       *server_connect_callbacks;
   Eina_List       *server_disconnect_callbacks;

   Ecore_Event_Handler *handler_add;
   Ecore_Event_Handler *handler_del;
   Ecore_Event_Handler *handler_data;
};

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

struct _Ecore_Con_Eet_Base_Data
{
   Ecore_Con_Server    *server;

   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor *matching;

   Eina_Hash           *data_callbacks;
   Eina_Hash           *raw_data_callbacks;
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
_ecore_con_eet_data_descriptor_setup(Ecore_Con_Eet_Base_Data *ece)
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
   Ecore_Con_Eet_Base_Data *ece_data = efl_data_scope_get(n->ece, ECORE_CON_EET_BASE_CLASS);

   protocol = eet_data_descriptor_decode(ece_data->edd, eet_data, size);
   if (!protocol) return EINA_TRUE;

   cb = eina_hash_find(ece_data->data_callbacks, protocol->type);
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
   Ecore_Con_Eet_Base_Data *ece_data = efl_data_scope_get(n->ece, ECORE_CON_EET_BASE_CLASS);

   if (ecore_con_server_send(ece_data->server, data, size) != (int)size)
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_eet_server_connected(void *data, int type EINA_UNUSED, Ecore_Con_Event_Client_Add *ev)
{
   Ecore_Con_Eet_Client *ecec;
   Eina_List *ll;
   Ecore_Con_Reply *n;
   Ecore_Con_Eet *ece_obj = data;
   Ecore_Con_Eet_Base_Data *base_data = efl_data_scope_get(ece_obj, ECORE_CON_EET_BASE_CLASS);
   Ecore_Con_Eet_Server_Obj_Data *r = efl_data_scope_get(ece_obj, ECORE_CON_EET_SERVER_OBJ_CLASS);

   if (ecore_con_client_server_get(ev->client) != base_data->server)
     return EINA_TRUE;

   n = calloc(1, sizeof (Ecore_Con_Reply));
   if (!n) return EINA_TRUE;

   n->client = ev->client;
   n->ece = ece_obj;
   n->econn = eet_connection_new(_ecore_con_eet_read_cb, _ecore_con_eet_server_write_cb, n);
   ecore_con_client_data_set(n->client, n);

   EINA_LIST_FOREACH(r->client_connect_callbacks, ll, ecec)
     if (!ecec->func((void *)ecec->data, n, n->client))
       {
          eet_connection_close(n->econn, NULL);
          free(n);
          return EINA_TRUE;
       }

   r->connections = eina_list_append(r->connections, n);

   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_eet_server_disconnected(void *data, int type EINA_UNUSED, Ecore_Con_Event_Client_Del *ev)
{
   Ecore_Con_Eet *ece_obj = data;
   Ecore_Con_Eet_Base_Data *base_data = efl_data_scope_get(ece_obj, ECORE_CON_EET_BASE_CLASS);
   Ecore_Con_Eet_Server_Obj_Data *r = efl_data_scope_get(ece_obj, ECORE_CON_EET_SERVER_OBJ_CLASS);
   Ecore_Con_Reply *n;
   Eina_List *l;

   if (ecore_con_client_server_get(ev->client) != base_data->server)
     return EINA_TRUE;

   EINA_LIST_FOREACH(r->connections, l, n)
     if (n->client == ev->client)
       {
          Ecore_Con_Eet_Client *ecec;
          Eina_List *ll;

          EINA_LIST_FOREACH(r->client_disconnect_callbacks, ll, ecec)
            ecec->func((void *)ecec->data, n, n->client);

          eet_connection_close(n->econn, NULL);
          free(n);
          r->connections = eina_list_remove_list(r->connections, l);
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
                  Ecore_Con_Eet_Base_Data *eceb_data = efl_data_scope_get(n->ece,ECORE_CON_EET_BASE_CLASS);

                  protocol = buffer;
                  section = buffer + protocol_length;

                  if (protocol[protocol_length - 1] == '\0' &&
                      section[section_length - 1] == '\0')
                    {
                       size -= protocol_length + section_length;
                       buffer = section + section_length;

                       n->buffer_handler = eina_hash_find(eceb_data->raw_data_callbacks, protocol);
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
   Ecore_Con_Eet *ece_obj = data;
   Ecore_Con_Eet_Base_Data *r = efl_data_scope_get(ece_obj, ECORE_CON_EET_BASE_CLASS);
   Ecore_Con_Reply *n;

   if (ecore_con_client_server_get(ev->client) != r->server)
     return EINA_TRUE;

   n = ecore_con_client_data_get(ev->client);

   efl_ref(ece_obj);
   _ecore_con_eet_data(n, ev->data, ev->size);
   efl_unref(ece_obj);

   return EINA_TRUE;
}

/* Dealing connection to a server */

static Eina_Bool
_ecore_con_eet_client_connected(void *data, int type EINA_UNUSED, Ecore_Con_Event_Server_Add *ev)
{
   Ecore_Con_Eet_Server *eces;
   Ecore_Con_Eet *ece_obj = data;
   Ecore_Con_Eet_Base_Data *base_data = efl_data_scope_get(ece_obj, ECORE_CON_EET_BASE_CLASS);
   Ecore_Con_Eet_Client_Obj_Data *r = efl_data_scope_get(ece_obj, ECORE_CON_EET_CLIENT_OBJ_CLASS);
   Ecore_Con_Reply *n;
   Eina_List *ll;

   /* Client did connect */
   if (base_data->server != ev->server) return EINA_TRUE;
   if (r->r) return EINA_TRUE;

   n = calloc(1, sizeof (Ecore_Con_Reply));
   if (!n) return EINA_TRUE;

   n->client = NULL;
   n->ece = ece_obj;
   n->econn = eet_connection_new(_ecore_con_eet_read_cb, _ecore_con_eet_client_write_cb, n);

   EINA_LIST_FOREACH(r->server_connect_callbacks, ll, eces)
     {
        Ecore_Con_Eet_Base_Data *temp = efl_data_scope_get(n->ece, ECORE_CON_EET_BASE_CLASS);
        if (!eces->func((void *)eces->data, n, temp->server))
          {
             eet_connection_close(n->econn, NULL);
             free(n);
             return EINA_TRUE;
          }
     }

   r->r = n;

   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_eet_client_disconnected(void *data, int type EINA_UNUSED, Ecore_Con_Event_Server_Del *ev)
{
   Ecore_Con_Eet *ece_obj = data;
   Ecore_Con_Eet_Base_Data *base_data = efl_data_scope_get(ece_obj, ECORE_CON_EET_BASE_CLASS);
   Ecore_Con_Eet_Client_Obj_Data *r = efl_data_scope_get(ece_obj, ECORE_CON_EET_CLIENT_OBJ_CLASS);
   Ecore_Con_Eet_Server *eces;
   Eina_List *ll;

   if (base_data->server != ev->server) return EINA_TRUE;
   if (!r->r) return EINA_TRUE;

   /* Client disconnected */
   EINA_LIST_FOREACH(r->server_disconnect_callbacks, ll, eces)
     eces->func((void *)eces->data, r->r, base_data->server);

   eet_connection_close(r->r->econn, NULL);
   free(r->r);
   r->r = NULL;

   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_eet_client_data(void *data, int type EINA_UNUSED, Ecore_Con_Event_Server_Data *ev)
{
   Ecore_Con_Eet *ece_obj = data;
   Ecore_Con_Eet_Base_Data *base_data = efl_data_scope_get(ece_obj, ECORE_CON_EET_BASE_CLASS);
   Ecore_Con_Eet_Client_Obj_Data *r = efl_data_scope_get(ece_obj, ECORE_CON_EET_CLIENT_OBJ_CLASS);

   if (base_data->server != ev->server) return EINA_TRUE;
   if (!r->r) return EINA_TRUE;

   /* Got some data */
   efl_ref(ece_obj);
   _ecore_con_eet_data(r->r, ev->data, ev->size);
   efl_unref(ece_obj);

   return EINA_TRUE;
}

/*************
 * Generated API
 */

EOLIAN static void
_ecore_con_eet_base_data_callback_set(Eo *obj EINA_UNUSED, Ecore_Con_Eet_Base_Data *pd, const char *name, Ecore_Con_Eet_Data_Cb func, const void *data)
{
   Ecore_Con_Eet_Data *eced;

   eced = calloc(1, sizeof (Ecore_Con_Eet_Data));
   if (!eced) return;

   eced->func = func;
   eced->data = data;
   eced->name = eina_stringshare_add(name);

   eina_hash_direct_add(pd->data_callbacks, eced->name, eced);
}

EOLIAN static void
_ecore_con_eet_base_raw_data_callback_set(Eo *obj EINA_UNUSED, Ecore_Con_Eet_Base_Data *pd, const char *name, Ecore_Con_Eet_Raw_Data_Cb func, const void *data)
{
   Ecore_Con_Eet_Raw_Data *ecerd;

   ecerd = calloc(1, sizeof (Ecore_Con_Eet_Raw_Data));
   if (!ecerd) return;

   ecerd->func = func;
   ecerd->data = data;
   ecerd->name = eina_stringshare_add(name);

   eina_hash_direct_add(pd->raw_data_callbacks, ecerd->name, ecerd);
}

EOLIAN static void
_ecore_con_eet_base_data_callback_del(Eo *obj EINA_UNUSED, Ecore_Con_Eet_Base_Data *pd, const char *name)
{
   eina_hash_del(pd->data_callbacks, name, NULL);
}

EOLIAN static void
_ecore_con_eet_base_raw_data_callback_del(Eo *obj, Ecore_Con_Eet_Base_Data *pd, const char *name)
{
   Ecore_Con_Eet_Client_Obj_Data *eced = efl_data_scope_get(obj, ECORE_CON_EET_CLIENT_OBJ_CLASS);

   if (efl_isa(obj, ECORE_CON_EET_CLIENT_OBJ_CLASS) &&
       eced->r->buffer_handler &&
       !strcmp(eced->r->buffer_handler->name, name))
     {
        eced->r->buffer_handler = NULL;
        free(eced->r->buffer);
        eced->r->buffer = (void *)1;
     }
   eina_hash_del(pd->raw_data_callbacks, name, NULL);
}

EOLIAN static void
_ecore_con_eet_base_send(Eo *obj EINA_UNUSED, Ecore_Con_Eet_Base_Data *pd, Ecore_Con_Reply *reply, const char *name, void *value)
{
   Ecore_Con_Eet_Protocol protocol;

   if (!reply) return;

   protocol.type = name;
   protocol.data = value;

   eet_connection_send(reply->econn, pd->edd, &protocol, NULL);
}

EOLIAN static void
_ecore_con_eet_base_raw_send(Eo *obj EINA_UNUSED, Ecore_Con_Eet_Base_Data *pd, Ecore_Con_Reply *reply, const char *protocol_name, const char *section, Eina_Binbuf *section_data)
{
   unsigned int protocol[4];
   unsigned int protocol_length;
   unsigned int section_length;
   unsigned int size;
   unsigned int length = 0;
   const void *value = NULL;
   char *tmp;

   if (!reply) return;
   if (!protocol_name) return;
   if (!section) return;

   if (section_data)
     {
        length = eina_binbuf_length_get(section_data);
        value = eina_binbuf_string_get(section_data);
     }

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
        ecore_con_server_send(pd->server, tmp, size);
        ecore_con_server_send(pd->server, value, length);
     }
}

EOLIAN static void
_ecore_con_eet_base_register(Eo *obj EINA_UNUSED, Ecore_Con_Eet_Base_Data *pd, const char *name, Eet_Data_Descriptor *edd)
{
   EET_DATA_DESCRIPTOR_ADD_MAPPING(pd->matching, name, edd);
}

EOLIAN static Efl_Object *
_ecore_con_eet_server_obj_efl_object_constructor(Eo *obj, Ecore_Con_Eet_Server_Obj_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, ECORE_CON_EET_SERVER_OBJ_CLASS));

   if (!obj) return NULL;

   pd->handler_add = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,
                                             (Ecore_Event_Handler_Cb)_ecore_con_eet_server_connected, obj);
   pd->handler_del = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,
                                             (Ecore_Event_Handler_Cb)_ecore_con_eet_server_disconnected, obj);
   pd->handler_data = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
                                              (Ecore_Event_Handler_Cb)_ecore_con_eet_server_data, obj);

   return obj;
}

EOLIAN static void
_ecore_con_eet_server_obj_efl_object_destructor(Eo *obj, Ecore_Con_Eet_Server_Obj_Data *pd EINA_UNUSED)
{
   Ecore_Con_Reply *n;
   Ecore_Con_Eet_Client *c;

   EINA_LIST_FREE(pd->connections, n)
     {
        _ecore_con_eet_reply_cleanup(n);
        eet_connection_close(n->econn, NULL);
        free(n);
     }
   EINA_LIST_FREE(pd->client_connect_callbacks, c)
     free(c);
   EINA_LIST_FREE(pd->client_disconnect_callbacks, c)
     free(c);

   ecore_event_handler_del(pd->handler_add);
   ecore_event_handler_del(pd->handler_del);
   ecore_event_handler_del(pd->handler_data);

   efl_destructor(efl_super(obj, ECORE_CON_EET_SERVER_OBJ_CLASS));
}

EOLIAN static Efl_Object *
_ecore_con_eet_client_obj_efl_object_constructor(Eo *obj, Ecore_Con_Eet_Client_Obj_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, ECORE_CON_EET_CLIENT_OBJ_CLASS));

   if (!obj) return NULL;

   pd->handler_add = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
                                             (Ecore_Event_Handler_Cb)_ecore_con_eet_client_connected, obj);
   pd->handler_del = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,
                                             (Ecore_Event_Handler_Cb)_ecore_con_eet_client_disconnected, obj);
   pd->handler_data = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
                                              (Ecore_Event_Handler_Cb)_ecore_con_eet_client_data, obj);

   return obj;
}

EOLIAN static void
_ecore_con_eet_client_obj_efl_object_destructor(Eo *obj, Ecore_Con_Eet_Client_Obj_Data *pd EINA_UNUSED)
{
   Ecore_Con_Eet_Server *s;

   if (pd->r)
     {
        _ecore_con_eet_reply_cleanup(pd->r);
        eet_connection_close(pd->r->econn, NULL);
     }
   EINA_LIST_FREE(pd->server_connect_callbacks, s)
     free(s);
   EINA_LIST_FREE(pd->server_disconnect_callbacks, s)
     free(s);

   ecore_event_handler_del(pd->handler_add);
   ecore_event_handler_del(pd->handler_del);
   ecore_event_handler_del(pd->handler_data);

   efl_destructor(efl_super(obj, ECORE_CON_EET_CLIENT_OBJ_CLASS));
}

EOLIAN static Efl_Object *
_ecore_con_eet_base_efl_object_constructor(Eo *obj, Ecore_Con_Eet_Base_Data *pd)
{
   obj = efl_constructor(efl_super(obj, ECORE_CON_EET_BASE_CLASS));

   if (!obj) return NULL;

   pd->data_callbacks = eina_hash_stringshared_new(_ecore_con_eet_data_free);
   pd->raw_data_callbacks = eina_hash_string_superfast_new(_ecore_con_eet_raw_data_free);

   _ecore_con_eet_data_descriptor_setup(pd);

   return obj;
}

EOLIAN static void
_ecore_con_eet_base_efl_object_destructor(Eo *obj, Ecore_Con_Eet_Base_Data *pd)
{
   eet_data_descriptor_free(pd->edd);
   eet_data_descriptor_free(pd->matching);
   eina_hash_free(pd->data_callbacks);
   eina_hash_free(pd->raw_data_callbacks);

   efl_destructor(efl_super(obj, ECORE_CON_EET_BASE_CLASS));
}

EOLIAN static Efl_Object *
_ecore_con_eet_base_efl_object_finalize(Eo *obj, Ecore_Con_Eet_Base_Data *pd)
{
   if (pd->server) return efl_finalize(efl_super(obj, ECORE_CON_EET_BASE_CLASS));

   eet_data_descriptor_free(pd->edd);
   eet_data_descriptor_free(pd->matching);
   eina_hash_free(pd->data_callbacks);
   eina_hash_free(pd->raw_data_callbacks);

   return NULL;
}

EOLIAN static void
_ecore_con_eet_base_server_set(Eo *obj EINA_UNUSED, Ecore_Con_Eet_Base_Data *pd, Ecore_Con_Server *data)
{
   if (!ecore_con_server_check(data))
     return;

   pd->server = data;
}

EOLIAN static Ecore_Con_Server *
_ecore_con_eet_base_server_get(const Eo *obj EINA_UNUSED, Ecore_Con_Eet_Base_Data *pd)
{
   return pd->server;
}

/**************
* Global API *
**************/

EAPI Ecore_Con_Eet *
ecore_con_eet_server_new(Ecore_Con_Server *server)
{
   Ecore_Con_Eet *ece_obj;

   if (!server) return NULL;

   ece_obj = efl_add_ref(ECORE_CON_EET_SERVER_OBJ_CLASS, NULL, ecore_con_eet_base_server_set(efl_added, server));

   return ece_obj;
}

EAPI Ecore_Con_Eet *
ecore_con_eet_client_new(Ecore_Con_Server *server)
{
   Ecore_Con_Eet *ece_obj;

   if (!server) return NULL;

   ece_obj = efl_add_ref(ECORE_CON_EET_CLIENT_OBJ_CLASS, NULL, ecore_con_eet_base_server_set(efl_added, server));

   return ece_obj;
}

EAPI void
ecore_con_eet_server_free(Ecore_Con_Eet *server)
{
   efl_unref(server);
}

EAPI void
ecore_con_eet_register(Ecore_Con_Eet *ece, const char *name, Eet_Data_Descriptor *edd)
{
   ecore_con_eet_base_register(ece, name, edd);
}

EAPI void
ecore_con_eet_data_callback_add(Ecore_Con_Eet *ece, const char *name, Ecore_Con_Eet_Data_Cb func, const void *data)
{
   ecore_con_eet_base_data_callback_set(ece, name, func, data);
}

EAPI void
ecore_con_eet_data_callback_del(Ecore_Con_Eet *ece, const char *name)
{
   ecore_con_eet_base_data_callback_del(ece, name);
}

EAPI void
ecore_con_eet_raw_data_callback_add(Ecore_Con_Eet *ece, const char *name, Ecore_Con_Eet_Raw_Data_Cb func, const void *data)
{
   ecore_con_eet_base_raw_data_callback_set(ece, name, func, data);
}

EAPI void
ecore_con_eet_raw_data_callback_del(Ecore_Con_Eet *ece, const char *name)
{
   ecore_con_eet_base_raw_data_callback_del(ece, name);
}

EAPI void
ecore_con_eet_client_connect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data)
{
   Ecore_Con_Eet_Server_Obj_Data *eces = efl_data_scope_get(ece, ECORE_CON_EET_SERVER_OBJ_CLASS);
   Ecore_Con_Eet_Client *c;

   if (!eces || !func) return;

   c = calloc(1, sizeof (Ecore_Con_Eet_Client));
   if (!c) return;

   c->func = func;
   c->data = data;

   eces->client_connect_callbacks = eina_list_append(eces->client_connect_callbacks, c);
}

EAPI void
ecore_con_eet_client_connect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data)
{
   Ecore_Con_Eet_Server_Obj_Data *eces = efl_data_scope_get(ece, ECORE_CON_EET_SERVER_OBJ_CLASS);
   Ecore_Con_Eet_Client *c;
   Eina_List *l;

   if (!eces || !func) return;

   EINA_LIST_FOREACH(eces->client_connect_callbacks, l, c)
     if (c->func == func && c->data == data)
       {
          eces->client_connect_callbacks = eina_list_remove_list(eces->client_connect_callbacks, l);
          free(c);
          return;
       }
}

EAPI void
ecore_con_eet_client_disconnect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data)
{
   Ecore_Con_Eet_Server_Obj_Data *eces = efl_data_scope_get(ece, ECORE_CON_EET_SERVER_OBJ_CLASS);
   Ecore_Con_Eet_Client *c;

   if (!eces || !func) return;

   c = calloc(1, sizeof (Ecore_Con_Eet_Client));
   if (!c) return;

   c->func = func;
   c->data = data;

   eces->client_disconnect_callbacks = eina_list_append(eces->client_disconnect_callbacks, c);
}

EAPI void
ecore_con_eet_client_disconnect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Client_Cb func, const void *data)
{
   Ecore_Con_Eet_Server_Obj_Data *eced = efl_data_scope_get(ece, ECORE_CON_EET_SERVER_OBJ_CLASS);
   Ecore_Con_Eet_Client *c;
   Eina_List *l;

   if (!eced || !func) return;

   EINA_LIST_FOREACH(eced->client_disconnect_callbacks, l, c)
     if (c->func == func && c->data == data)
       {
          eced->client_disconnect_callbacks = eina_list_remove_list(eced->client_disconnect_callbacks, l);
          free(c);
          return;
       }
}

EAPI void
ecore_con_eet_server_connect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data)
{
   Ecore_Con_Eet_Client_Obj_Data *eced = efl_data_scope_get(ece, ECORE_CON_EET_CLIENT_OBJ_CLASS);
   Ecore_Con_Eet_Server *s;

   if (!eced || !func) return;

   s = calloc(1, sizeof (Ecore_Con_Eet_Server));
   if (!s) return;

   s->func = func;
   s->data = data;

   eced->server_connect_callbacks = eina_list_append(eced->server_connect_callbacks, s);
}

EAPI void
ecore_con_eet_server_connect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data)
{
   Ecore_Con_Eet_Client_Obj_Data *eced = efl_data_scope_get(ece, ECORE_CON_EET_CLIENT_OBJ_CLASS);
   Ecore_Con_Eet_Server *s;
   Eina_List *l;

   if (!eced || !func) return;

   EINA_LIST_FOREACH(eced->server_connect_callbacks, l, s)
     if (s->func == func && s->data == data)
       {
          eced->server_connect_callbacks = eina_list_remove_list(eced->server_connect_callbacks, l);
          free(s);
          return;
       }
}

EAPI void
ecore_con_eet_server_disconnect_callback_add(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data)
{
   Ecore_Con_Eet_Client_Obj_Data *eced = efl_data_scope_get(ece, ECORE_CON_EET_CLIENT_OBJ_CLASS);
   Ecore_Con_Eet_Server *s;

   if (!eced || !func) return;

   s = calloc(1, sizeof (Ecore_Con_Eet_Server));
   if (!s) return;

   s->func = func;
   s->data = data;

   eced->server_disconnect_callbacks = eina_list_append(eced->server_disconnect_callbacks, s);
}

EAPI void
ecore_con_eet_server_disconnect_callback_del(Ecore_Con_Eet *ece, Ecore_Con_Eet_Server_Cb func, const void *data)
{
   Ecore_Con_Eet_Client_Obj_Data *eced = efl_data_scope_get(ece, ECORE_CON_EET_CLIENT_OBJ_CLASS);
   Ecore_Con_Eet_Server *s;
   Eina_List *l;

   if (!eced || !func) return;

   EINA_LIST_FOREACH(eced->server_disconnect_callbacks, l, s)
     if (s->func == func && s->data == data)
       {
          eced->server_disconnect_callbacks = eina_list_remove_list(eced->server_disconnect_callbacks, l);
          free(s);
          return;
       }
}

EAPI void
ecore_con_eet_data_set(Ecore_Con_Eet *ece, const void *data)
{
   efl_key_data_set(ece, ECORE_CON_EET_DATA_KEY, data);
}

EAPI const void *
ecore_con_eet_data_get(Ecore_Con_Eet *ece)
{
   return efl_key_data_get(ece, ECORE_CON_EET_DATA_KEY);
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
   ecore_con_eet_base_send(reply->ece, reply, name, value);
}

EAPI void
ecore_con_eet_raw_send(Ecore_Con_Reply *reply, const char *protocol_name, const char *section, void *value, unsigned int length)
{
   Eina_Binbuf *buf = eina_binbuf_manage_new(value, length, 1);
   ecore_con_eet_base_raw_send(reply->ece, reply, protocol_name, section, buf);
   eina_binbuf_free(buf);
}

#include "ecore_con_eet_base.eo.c"
#include "ecore_con_eet_server_obj.eo.c"
#include "ecore_con_eet_client_obj.eo.c"
