#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

#define ECORE_CON_TYPE 0x0f
#define ECORE_CON_SSL  0xf0
#define ECORE_CON_SUPER_SSL  0xf00

/* This file exists solely to provide ABI compatibility */

struct _Ecore_Con_Server
{
   ECORE_MAGIC;
   Eo *dialer;
   Eo *server;
   struct {
      Eina_Future *job;
      Eina_Binbuf *pending_send; /* until job is fulfilled, no dialer exists,
                                  * this binbuf allows immediate
                                  * ecore_con_server_send() in that situation */
      Eo *clients_ctx;
      Eina_List *certs;
      Eina_List *privkeys;
      Eina_List *crls;
      Eina_List *cafiles;
      Eina_Stringshare *verify_name;
      Eina_Bool verify;
      Eina_Bool verify_basic;
      Eina_Bool upgrading;
      Ecore_Con_Type upgrade_type;
   } ssl;
   Eina_List *clients;
   Eina_List *event_count;
   const void *data;
   Eina_Stringshare *name;
   Eina_Stringshare *ip;
   size_t pending_write;
   double start_time;
   double timeout;
   Ecore_Con_Type type;
   int port;
   Eina_Bool want_mcast;
   Eina_Bool is_dialer;
   Eina_Bool connecting;
   Eina_Bool delete_me;
};

struct _Ecore_Con_Client
{
   ECORE_MAGIC;
   Eo *socket;
   Ecore_Con_Server *svr;
   Eina_List *event_count;
   const void *data;
   Eina_Stringshare *ip;
   struct {
      Eina_Future *job;
      Eo *ctx;
      Eina_Bool upgrading;
   } ssl;
   size_t pending_write;
   double start_time;
   int port;
   Eina_Bool delete_me;
};

typedef struct _Ecore_Con_Lookup_Ctx {
   Ecore_Thread *thread;
   Ecore_Con_Dns_Cb cb;
   const void *data;
} Ecore_Con_Lookup_Ctx;

/* allows delete_me to be true */
#define ECORE_CON_SERVER_CHECK_RELAXED_RETURN(svr, ...) \
  do \
    { \
       if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER)) \
         { \
            ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, __FUNCTION__); \
            return __VA_ARGS__; \
         } \
    } \
  while (0)

#define ECORE_CON_SERVER_CHECK_RETURN(svr, ...) \
  do \
    { \
       ECORE_CON_SERVER_CHECK_RELAXED_RETURN(svr, __VA_ARGS__) ; \
       EINA_SAFETY_ON_TRUE_RETURN_VAL(svr->delete_me, __VA_ARGS__); \
    } \
  while (0)

#define ECORE_CON_CLIENT_CHECK_RELAXED_RETURN(cl, ...) \
  do \
    { \
       if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT)) \
         { \
            ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, __FUNCTION__); \
            return __VA_ARGS__; \
         } \
    } \
  while (0)

#define ECORE_CON_CLIENT_CHECK_RETURN(cl, ...) \
  do \
    { \
       ECORE_CON_CLIENT_CHECK_RELAXED_RETURN(cl, __VA_ARGS__) ; \
       EINA_SAFETY_ON_TRUE_RETURN_VAL(cl->delete_me, __VA_ARGS__); \
    } \
  while (0)


/* from ecore_con_alloc.c */
#define GENERIC_ALLOC_FREE_HEADER(TYPE, Type) \
  TYPE *Type##_alloc(void); \
  void Type##_free(TYPE *e);

GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Client_Add, ecore_con_event_client_add);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Client_Del, ecore_con_event_client_del);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Server_Add, ecore_con_event_server_add);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Server_Del, ecore_con_event_server_del);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Server_Write, ecore_con_event_server_write);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Server_Data, ecore_con_event_server_data);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Server_Error, ecore_con_event_server_error);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Server_Upgrade, ecore_con_event_server_upgrade);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Proxy_Bind, ecore_con_event_proxy_bind);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Client_Write, ecore_con_event_client_write);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Client_Data, ecore_con_event_client_data);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Client_Error, ecore_con_event_client_error);
GENERIC_ALLOC_FREE_HEADER(Ecore_Con_Event_Client_Upgrade, ecore_con_event_client_upgrade);
#undef GENERIC_ALLOC_FREE_HEADER

/* shared */
EAPI int ECORE_CON_EVENT_SERVER_DEL = 0;
EAPI int ECORE_CON_EVENT_SERVER_ERROR = 0;
/* ecore_con_server_add() */
EAPI int ECORE_CON_EVENT_CLIENT_ADD = 0;
EAPI int ECORE_CON_EVENT_CLIENT_DEL = 0;
/* ecore_con_server_connect() */
EAPI int ECORE_CON_EVENT_SERVER_ADD = 0;
EAPI int ECORE_CON_EVENT_SERVER_DATA = 0;
EAPI int ECORE_CON_EVENT_SERVER_WRITE = 0;
EAPI int ECORE_CON_EVENT_PROXY_BIND = 0; /* we're not supporting proxy bind, keep only for ABI */
EAPI int ECORE_CON_EVENT_SERVER_UPGRADE = 0;
/* for each client from ECORE_CON_EVENT_CLIENT_ADD */
EAPI int ECORE_CON_EVENT_CLIENT_DATA = 0;
EAPI int ECORE_CON_EVENT_CLIENT_WRITE = 0;
EAPI int ECORE_CON_EVENT_CLIENT_ERROR = 0;
EAPI int ECORE_CON_EVENT_CLIENT_UPGRADE = 0;

static Eina_List *_servers = NULL;
static Eina_List *_ecore_con_lookups = NULL;
static int _ecore_con_event_count = 0;

Ecore_Con_Socks *_ecore_con_proxy_once = NULL;
Ecore_Con_Socks *_ecore_con_proxy_global = NULL;

void
ecore_con_legacy_init(void)
{
   ECORE_CON_EVENT_CLIENT_ADD = ecore_event_type_new();
   ECORE_CON_EVENT_CLIENT_DEL = ecore_event_type_new();
   ECORE_CON_EVENT_SERVER_ADD = ecore_event_type_new();
   ECORE_CON_EVENT_SERVER_DEL = ecore_event_type_new();
   ECORE_CON_EVENT_CLIENT_DATA = ecore_event_type_new();
   ECORE_CON_EVENT_SERVER_DATA = ecore_event_type_new();
   ECORE_CON_EVENT_CLIENT_WRITE = ecore_event_type_new();
   ECORE_CON_EVENT_SERVER_WRITE = ecore_event_type_new();
   ECORE_CON_EVENT_CLIENT_ERROR = ecore_event_type_new();
   ECORE_CON_EVENT_SERVER_ERROR = ecore_event_type_new();
   ECORE_CON_EVENT_PROXY_BIND = ecore_event_type_new();
   ECORE_CON_EVENT_SERVER_UPGRADE = ecore_event_type_new();
   ECORE_CON_EVENT_CLIENT_UPGRADE = ecore_event_type_new();

   eina_magic_string_set(ECORE_MAGIC_CON_SERVER, "Ecore_Con_Server");
   eina_magic_string_set(ECORE_MAGIC_CON_CLIENT, "Ecore_Con_Client");
   eina_magic_string_set(ECORE_MAGIC_CON_URL, "Ecore_Con_Url");

   ecore_con_socks_init();
}

static void _ecore_con_server_free(Ecore_Con_Server *svr);

void
ecore_con_legacy_shutdown(void)
{
   Eina_List *l, *l2;
   Ecore_Con_Server *svr;
   Ecore_Con_Lookup_Ctx *lookup_ctx;

   EINA_LIST_FOREACH_SAFE(_servers, l, l2, svr)
     {
        Ecore_Con_Event_Server_Add *ev;

        if (!svr) continue;
        svr->delete_me = EINA_TRUE;
        INF("svr %p is dead", svr);
        /* some pointer hacks here to prevent double frees if people are being stupid */
        EINA_LIST_FREE(svr->event_count, ev)
          ev->server = NULL;
        _ecore_con_server_free(svr);
     }

   EINA_LIST_FREE(_ecore_con_lookups, lookup_ctx)
     ecore_thread_cancel(lookup_ctx->thread);

   ecore_event_type_flush(ECORE_CON_EVENT_CLIENT_ADD,
                          ECORE_CON_EVENT_CLIENT_DEL,
                          ECORE_CON_EVENT_SERVER_ADD,
                          ECORE_CON_EVENT_SERVER_DEL,
                          ECORE_CON_EVENT_CLIENT_DATA,
                          ECORE_CON_EVENT_SERVER_DATA,
                          ECORE_CON_EVENT_CLIENT_WRITE,
                          ECORE_CON_EVENT_SERVER_WRITE,
                          ECORE_CON_EVENT_CLIENT_ERROR,
                          ECORE_CON_EVENT_SERVER_ERROR,
                          ECORE_CON_EVENT_PROXY_BIND,
                          ECORE_CON_EVENT_SERVER_UPGRADE,
                          ECORE_CON_EVENT_CLIENT_UPGRADE);

   ecore_con_socks_shutdown();
   if (!_ecore_con_event_count) ecore_con_mempool_shutdown();
   else WRN("%d events still exist, leaking ecore_con mempools", _ecore_con_event_count);
}

/**
 * @addtogroup Ecore_Con_Client_Group Ecore Connection Client Functions
 *
 * Functions that operate on Ecore connection client objects, these
 * are announced using ECORE_CON_EVENT_CLIENT_ADD by servers created
 * with ecore_con_server_add().
 *
 * @{
 */

static Efl_Callback_Array_Item *_ecore_con_client_socket_cbs(void);
static Efl_Callback_Array_Item *_ecore_con_client_socket_ssl_cbs(void);

static void
_ecore_con_client_socket_close(Ecore_Con_Client *cl)
{
   if (!cl->socket) return;

   /* socket may remain alive due other references, we don't own it */
   efl_event_callback_array_del(cl->socket, _ecore_con_client_socket_cbs(), cl);

   if (!efl_io_closer_closed_get(cl->socket))
     efl_io_closer_close(cl->socket);
}

static void
_ecore_con_client_free(Ecore_Con_Client *cl)
{
   cl->delete_me = EINA_TRUE;

   if (cl->svr)
     cl->svr->clients = eina_list_remove(cl->svr->clients, cl);

   _ecore_con_client_socket_close(cl);

   if (cl->socket)
     {
        Eo *inner_socket = efl_io_buffered_stream_inner_io_get(cl->socket);
        efl_event_callback_array_del(cl->socket, _ecore_con_client_socket_cbs(), cl);
        if (efl_isa(inner_socket, EFL_NET_SOCKET_SSL_CLASS))
          efl_event_callback_array_del(inner_socket, _ecore_con_client_socket_ssl_cbs(), cl);

        if (efl_parent_get(cl->socket) != cl->svr->server)
          efl_del(cl->socket); /* we own it */
        else
          efl_unref(cl->socket);
        cl->socket = NULL;
     }

   if (cl->ssl.job) eina_future_cancel(cl->ssl.job);

   if (cl->ssl.ctx)
     {
        efl_unref(cl->ssl.ctx);
        cl->ssl.ctx = NULL;
     }

   if (cl->event_count) return;

   cl->data = NULL;
   eina_stringshare_replace(&cl->ip, NULL);

   EINA_MAGIC_SET(cl, ECORE_MAGIC_NONE);
   free(cl);
}

/* BEGIN: post of Ecore_Event for ecore_con_server_connect() ************/

static void
_ecore_con_free_event_client_add(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Add *ev = event;

   if (ev->client)
     {
        Ecore_Con_Client *cl = ev->client;
        cl->event_count = eina_list_remove(cl->event_count, ev);
        if ((!cl->event_count) && (cl->delete_me))
          _ecore_con_client_free(cl);
     }
   ecore_con_event_client_add_free(ev);
   _ecore_con_event_count--;
}

static void
_ecore_con_post_event_client_add(Ecore_Con_Client *cl)
{
   Ecore_Con_Event_Client_Add *ev = ecore_con_event_client_add_alloc();
   EINA_SAFETY_ON_NULL_GOTO(ev, error);

   ev->client = cl;
   cl->event_count = eina_list_append(cl->event_count, ev);

   ecore_event_add(ECORE_CON_EVENT_CLIENT_ADD, ev, _ecore_con_free_event_client_add, NULL);
   _ecore_con_event_count++;
   return;

 error:
   _ecore_con_client_free(cl);
}

static void
_ecore_con_free_event_client_del(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Del *ev = event;

   if (ev->client)
     {
        Ecore_Con_Client *cl = ev->client;
        cl->event_count = eina_list_remove(cl->event_count, ev);
        if (!cl->event_count)
          _ecore_con_client_free(cl);
        else
          cl->delete_me = EINA_TRUE;
     }
   ecore_con_event_client_del_free(ev);
   _ecore_con_event_count--;
}

static Eina_Bool
_ecore_con_post_event_client_del(Ecore_Con_Client *cl)
{
   Ecore_Con_Event_Client_Del *ev = ecore_con_event_client_del_alloc();
   EINA_SAFETY_ON_NULL_GOTO(ev, error);

   ev->client = cl;
   cl->event_count = eina_list_append(cl->event_count, ev);

   /* legacy compatibility: test suite expects NULL pointer for IP */
   eina_stringshare_replace(&cl->ip, NULL);

   ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, ev, _ecore_con_free_event_client_del, NULL);
   _ecore_con_event_count++;
   return EINA_TRUE;

 error:
   _ecore_con_client_free(cl);
   return EINA_FALSE;
}

static void
_ecore_con_free_event_client_data(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Data *ev = event;

   if (ev->client)
     {
        Ecore_Con_Client *cl = ev->client;
        cl->event_count = eina_list_remove(cl->event_count, ev);
        if ((!cl->event_count) && (cl->delete_me))
          _ecore_con_client_free(cl);
     }
   free(ev->data);
   ecore_con_event_client_data_free(ev);
   _ecore_con_event_count--;
}

static void
_ecore_con_post_event_client_data(Ecore_Con_Client *cl, Eina_Rw_Slice slice)
{
   Ecore_Con_Event_Client_Data *ev = ecore_con_event_client_data_alloc();
   EINA_SAFETY_ON_NULL_GOTO(ev, error);

   ev->client = cl;
   ev->data = slice.mem;
   ev->size = slice.len;
   cl->event_count = eina_list_append(cl->event_count, ev);

   ecore_event_add(ECORE_CON_EVENT_CLIENT_DATA, ev, _ecore_con_free_event_client_data, NULL);
   _ecore_con_event_count++;
   return;

 error:
   free(slice.mem);
}

static void
_ecore_con_free_event_client_write(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Write *ev = event;

   if (ev->client)
     {
        Ecore_Con_Client *cl = ev->client;
        cl->event_count = eina_list_remove(cl->event_count, ev);
        if ((!cl->event_count) && (cl->delete_me))
          _ecore_con_client_free(cl);
     }
   ecore_con_event_client_write_free(ev);
   _ecore_con_event_count--;
}

static void
_ecore_con_post_event_client_write(Ecore_Con_Client *cl, size_t size)
{
   Ecore_Con_Event_Client_Write *ev = ecore_con_event_client_write_alloc();
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->client = cl;
   ev->size = size;
   cl->event_count = eina_list_append(cl->event_count, ev);

   ecore_event_add(ECORE_CON_EVENT_CLIENT_WRITE, ev, _ecore_con_free_event_client_write, NULL);
   _ecore_con_event_count++;
}

static void
_ecore_con_free_event_client_error(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Error *ev = event;

   if (ev->client)
     {
        Ecore_Con_Client *cl = ev->client;
        cl->event_count = eina_list_remove(cl->event_count, ev);
        if ((!cl->event_count) && (cl->delete_me))
          _ecore_con_client_free(cl);
     }
   free(ev->error);
   ecore_con_event_client_error_free(ev);
   _ecore_con_event_count--;
}

static Eina_Bool
_ecore_con_post_event_client_error(Ecore_Con_Client *cl, const char *err)
{
   Ecore_Con_Event_Client_Error *ev = ecore_con_event_client_error_alloc();
   EINA_SAFETY_ON_NULL_GOTO(ev, error);

   INF("cl=%p error from %s: %s", cl, efl_net_socket_address_remote_get(cl->socket), err);

   ev->client = cl;
   ev->error = err ? strdup(err) : NULL;
   cl->event_count = eina_list_append(cl->event_count, ev);

   ecore_event_add(ECORE_CON_EVENT_CLIENT_ERROR, ev, _ecore_con_free_event_client_error, NULL);
   _ecore_con_event_count++;
   return EINA_TRUE;

 error:
   _ecore_con_client_free(cl);
   return EINA_FALSE;
}

static void
_ecore_con_free_event_client_upgrade(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Client_Upgrade *ev = event;

   if (ev->client)
     {
        Ecore_Con_Client *cl = ev->client;
        cl->event_count = eina_list_remove(cl->event_count, ev);
        if ((!cl->event_count) && (cl->delete_me))
          _ecore_con_client_free(cl);
     }
   ecore_con_event_client_upgrade_free(ev);
   _ecore_con_event_count--;
}

static void
_ecore_con_post_event_client_upgrade(Ecore_Con_Client *cl)
{
   Ecore_Con_Event_Client_Upgrade *ev = ecore_con_event_client_upgrade_alloc();
   EINA_SAFETY_ON_NULL_GOTO(ev, error);

   ev->client = cl;
   cl->event_count = eina_list_append(cl->event_count, ev);

   ecore_event_add(ECORE_CON_EVENT_CLIENT_UPGRADE, ev, _ecore_con_free_event_client_upgrade, NULL);
   _ecore_con_event_count++;
   return;

 error:
   _ecore_con_client_free(cl);
}

/* END: post of Ecore_Event for ecore_con_server_add() ******************/

static void
_ecore_con_client_socket_progress(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Client *cl = data;
   size_t now = efl_io_buffered_stream_pending_write_get(cl->socket);

   if (cl->delete_me) return;
   if (cl->ssl.upgrading) return;

   if (cl->pending_write == now) return;
   EINA_SAFETY_ON_TRUE_GOTO(cl->pending_write < now, end); /* forgot to update! */

   _ecore_con_post_event_client_write(cl, cl->pending_write - now);

 end:
   cl->pending_write = now;
}

static void
_ecore_con_client_socket_slice_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Client *cl = data;
   Eina_Slice ro_slice;
   Eina_Rw_Slice rw_slice;

   if (cl->delete_me) return;
   if (cl->ssl.upgrading) return;

   ro_slice = efl_io_buffered_stream_slice_get(cl->socket);
   if (ro_slice.len == 0) return;

   rw_slice = eina_slice_dup(ro_slice);
   efl_io_buffered_stream_clear(cl->socket);
   _ecore_con_post_event_client_data(cl, rw_slice);
}

static void
_ecore_con_client_socket_read_finished(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Client *cl = data;

   DBG("client %p socket %p read finished, queue EOS", cl, cl->socket);

   if (!efl_io_closer_closed_get(cl->socket))
     efl_io_buffered_stream_eos_mark(cl->socket);
}

static void
_ecore_con_client_socket_finished(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Client *cl = data;

   DBG("client %p socket %p finished", cl, cl->socket);

   _ecore_con_client_socket_close(cl);

   _ecore_con_post_event_client_del(cl);
}

static void
_ecore_con_client_socket_error(void *data, const Efl_Event *event)
{
   Ecore_Con_Client *cl = data;
   Eina_Error *perr = event->info;

   if (cl->delete_me) return;

   WRN("error client %s: %s", efl_net_socket_address_remote_get(cl->socket), eina_error_msg_get(*perr));

   _ecore_con_client_socket_close(cl);
   _ecore_con_post_event_client_error(cl, eina_error_msg_get(*perr));
}

EFL_CALLBACKS_ARRAY_DEFINE(_ecore_con_client_socket_cbs,
                           { EFL_IO_BUFFERED_STREAM_EVENT_PROGRESS, _ecore_con_client_socket_progress },
                           { EFL_IO_BUFFERED_STREAM_EVENT_SLICE_CHANGED, _ecore_con_client_socket_slice_changed },
                           { EFL_IO_BUFFERED_STREAM_EVENT_READ_FINISHED, _ecore_con_client_socket_read_finished },
                           { EFL_IO_BUFFERED_STREAM_EVENT_FINISHED, _ecore_con_client_socket_finished },
                           { EFL_IO_BUFFERED_STREAM_EVENT_ERROR, _ecore_con_client_socket_error });

static Ecore_Con_Client *
ecore_con_client_add(Ecore_Con_Server *svr, Eo *socket)
{
   Ecore_Con_Client *cl;
   Eo *inner_socket;

   cl = calloc(1, sizeof(Ecore_Con_Client));
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);

   cl->socket = efl_ref(socket);
   cl->svr = svr;
   cl->start_time = ecore_time_get();
   cl->port = -1;

   inner_socket = efl_io_buffered_stream_inner_io_get(socket);
   if (efl_isa(inner_socket, EFL_NET_SOCKET_TCP_CLASS) ||
       efl_isa(inner_socket, EFL_NET_SOCKET_SSL_CLASS) ||
       efl_isa(inner_socket, EFL_NET_SERVER_UDP_CLIENT_CLASS))
     {
        const char *addr = efl_net_socket_address_remote_get(inner_socket);
        if (addr)
          {
             const char *p;
             if (addr[0] != '[') p = strchr(addr, ':');
             else
               {
                  addr++;
                  p = strchr(addr, ']');
               }
             if (p)
               {
                  const char *portstr = p;
                  if (portstr[0] == ']') portstr++;
                  if (portstr[0] == ':') portstr++; /* not else if! */
                  cl->ip = eina_stringshare_add_length(addr, p - addr);
                  cl->port = atoi(portstr);
               }
          }
     }
   else
     {
        /* legacy compatibility */
        cl->ip = eina_stringshare_add("0.0.0.0");
        cl->port = -1;
     }

   efl_event_callback_array_add(cl->socket, _ecore_con_client_socket_cbs(), cl);
   if (efl_isa(inner_socket, EFL_NET_SOCKET_SSL_CLASS))
     efl_event_callback_array_add(inner_socket, _ecore_con_client_socket_ssl_cbs(), cl);

   ECORE_MAGIC_SET(cl, ECORE_MAGIC_CON_CLIENT);
   svr->clients = eina_list_append(svr->clients, cl);
   return cl;
}

EAPI int
ecore_con_client_send(Ecore_Con_Client *cl, const void *data, int size)
{
   Eina_Error err;
   Eina_Slice slice = { .mem = data, .len = size };

   ECORE_CON_CLIENT_CHECK_RETURN(cl, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, 0);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(size < 1, 0);

   EINA_SAFETY_ON_NULL_RETURN_VAL(cl->socket, 0);
   err = efl_io_writer_write(cl->socket, &slice, NULL);
   if (err)
     {
        ERR("cl=%p could not send data=%p, size=%d: %s", cl, data, size, eina_error_msg_get(err));
        return 0;
     }
   cl->pending_write = efl_io_buffered_stream_pending_write_get(cl->socket);

   return slice.len;
}

EAPI Eina_Bool
ecore_con_client_connected_get(const Ecore_Con_Client *cl)
{
   ECORE_CON_CLIENT_CHECK_RETURN(cl, EINA_FALSE);
   return !efl_io_closer_closed_get(cl->socket);
}

EAPI void
ecore_con_client_timeout_set(Ecore_Con_Client *cl, double timeout)
{
   ECORE_CON_CLIENT_CHECK_RETURN(cl);
   efl_io_buffered_stream_timeout_inactivity_set(cl->socket, timeout);
}

EAPI double
ecore_con_client_timeout_get(const Ecore_Con_Client *cl)
{
   ECORE_CON_CLIENT_CHECK_RETURN(cl, -1.0);
   return efl_io_buffered_stream_timeout_inactivity_get(cl->socket);
}

EAPI void *
ecore_con_client_del(Ecore_Con_Client *cl)
{
   const void *data;

   ECORE_CON_CLIENT_CHECK_RELAXED_RETURN(cl, NULL);

   data = cl->data;

   _ecore_con_client_free(cl);
   return (void *)data;
}

EAPI void
ecore_con_client_data_set(Ecore_Con_Client *cl,
                          const void *data)
{
   ECORE_CON_CLIENT_CHECK_RELAXED_RETURN(cl);
   cl->data = data;
}

EAPI void *
ecore_con_client_data_get(Ecore_Con_Client *cl)
{
   ECORE_CON_CLIENT_CHECK_RELAXED_RETURN(cl, NULL);
   return (void *)cl->data;
}

EAPI const char *
ecore_con_client_ip_get(const Ecore_Con_Client *cl)
{
   ECORE_CON_CLIENT_CHECK_RELAXED_RETURN(cl, NULL);
   return cl->ip;
}

EAPI int
ecore_con_client_port_get(const Ecore_Con_Client *cl)
{
   ECORE_CON_CLIENT_CHECK_RELAXED_RETURN(cl, -1);
   return cl->port;
}

EAPI Ecore_Con_Server *
ecore_con_client_server_get(const Ecore_Con_Client *cl)
{
   ECORE_CON_CLIENT_CHECK_RELAXED_RETURN(cl, NULL);
   return cl->svr;
}

EAPI double
ecore_con_client_uptime_get(const Ecore_Con_Client *cl)
{
   ECORE_CON_CLIENT_CHECK_RELAXED_RETURN(cl, 0.0);
   return ecore_time_get() - cl->start_time;
}

EAPI void
ecore_con_client_flush(Ecore_Con_Client *cl)
{
   ECORE_CON_CLIENT_CHECK_RETURN(cl);
   Eo *inner_socket;

   if (!cl->socket) return;

   efl_io_buffered_stream_flush(cl->socket, EINA_FALSE, EINA_TRUE);

   inner_socket = efl_io_buffered_stream_inner_io_get(cl->socket);

   if (!efl_isa(inner_socket, EFL_NET_SOCKET_TCP_CLASS)) return;
   if (!efl_net_socket_tcp_cork_get(inner_socket)) return;

   efl_net_socket_tcp_cork_set(inner_socket, EINA_FALSE);
   efl_net_socket_tcp_cork_set(inner_socket, EINA_TRUE);
}

EAPI int
ecore_con_client_fd_get(const Ecore_Con_Client *cl)
{
   ECORE_CON_CLIENT_CHECK_RETURN(cl, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
   if (cl->socket)
     {
        if (efl_isa(cl->socket, EFL_LOOP_FD_CLASS))
          return efl_loop_fd_get(cl->socket);
        return SOCKET_TO_LOOP_FD(INVALID_SOCKET);
     }
   return SOCKET_TO_LOOP_FD(INVALID_SOCKET);
}

static void
_ecore_con_client_socket_ssl_ready(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Client *cl = data;

   cl->ssl.upgrading = EINA_FALSE;
   INF("cl=%p upgraded to SSL at %s", cl, efl_net_socket_address_remote_get(cl->socket));
   _ecore_con_post_event_client_upgrade(cl);
}

static void
_ecore_con_client_socket_ssl_error(void *data, const Efl_Event *event)
{
   Ecore_Con_Client *cl = data;
   Eina_Error *perr = event->info;

   if (cl->delete_me) return;

   WRN("SSL error client %s: %s", efl_net_socket_address_remote_get(cl->socket), eina_error_msg_get(*perr));

   _ecore_con_client_socket_close(cl);
   _ecore_con_post_event_client_error(cl, eina_error_msg_get(*perr));
}

EFL_CALLBACKS_ARRAY_DEFINE(_ecore_con_client_socket_ssl_cbs,
                           { EFL_NET_SOCKET_SSL_EVENT_SSL_READY, _ecore_con_client_socket_ssl_ready },
                           { EFL_NET_SOCKET_SSL_EVENT_SSL_ERROR, _ecore_con_client_socket_ssl_error });

static Eina_Value
_ecore_con_client_ssl_upgrade_job(void *data, const Eina_Value v,
                                  const Eina_Future *dead EINA_UNUSED)
{
   Ecore_Con_Client *cl = data;
   Eo *loop = ecore_main_loop_get();
   Eo *inner_socket;
   Eo *socket;
   Eo *tcp_socket;

   //Canceled
   if (v.type == EINA_VALUE_TYPE_ERROR) return v;

   tcp_socket = cl->socket;
   cl->socket = NULL; /* take it, will be wrapped */

   inner_socket = efl_add(EFL_NET_SOCKET_SSL_CLASS, loop,
                          efl_net_socket_ssl_adopt(efl_added, tcp_socket, cl->ssl.ctx));
   EINA_SAFETY_ON_NULL_GOTO(inner_socket, error_inner_socket);

   /* do not reparent tcp_socket! it's owned by server */
   efl_unref(tcp_socket); /* inner_socket keeps it */

   socket = efl_add(EFL_NET_SOCKET_SIMPLE_CLASS, loop,
                    efl_io_buffered_stream_inner_io_set(efl_added, inner_socket));
   EINA_SAFETY_ON_NULL_GOTO(socket, error_socket);

   efl_parent_set(inner_socket, socket);

   efl_unref(inner_socket); /* socket keeps it */

   cl->socket = socket;
   efl_io_closer_close_on_exec_set(socket, EINA_TRUE);
   efl_io_closer_close_on_destructor_set(socket, EINA_TRUE);
   efl_event_callback_array_del(tcp_socket, _ecore_con_client_socket_cbs(), cl);
   efl_event_callback_array_add(socket, _ecore_con_client_socket_cbs(), cl);
   efl_event_callback_array_add(inner_socket, _ecore_con_client_socket_ssl_cbs(), cl);

   DBG("socket=%p upgraded to SSL with inner_socket=%p, ssl_ctx=%p, tcp_socket=%p. Start handshake...",
       cl->socket,
       efl_io_buffered_stream_inner_io_get(cl->socket),
       cl->ssl.ctx,
       tcp_socket);
   return v;

 error_socket:
   efl_del(inner_socket);
 error_inner_socket:
   cl->socket = tcp_socket; /* put it back */
   if (_ecore_con_post_event_client_error(cl, "Couldn't finish SSL setup"))
     _ecore_con_post_event_client_del(cl);
   return v;
}

static Eo * _ecore_con_server_ssl_ctx_create(const Ecore_Con_Server *svr);

static void
_ecore_con_server_job_schedule(Ecore_Con_Server *svr, Eo *loop,
                               Eina_Future_Cb cb)
{
   eina_future_then(efl_loop_Eina_FutureXXX_job(loop), cb, svr, &svr->ssl.job);
}

EAPI Eina_Bool
ecore_con_ssl_client_upgrade(Ecore_Con_Client *cl, Ecore_Con_Type compl_type)
{
   Ecore_Con_Server *svr;
   Eo *ssl_ctx;
   double start;

   ECORE_CON_CLIENT_CHECK_RETURN(cl, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((compl_type & ECORE_CON_SSL) == 0, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(cl->ssl.upgrading, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(efl_io_buffered_stream_inner_io_get(cl->socket), EFL_NET_SOCKET_TCP_CLASS), EINA_FALSE);

   svr = cl->svr;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_isa(efl_net_server_simple_inner_server_get(svr->server), EFL_NET_SERVER_SSL_CLASS), EINA_FALSE); /* cannot upgrade if already SSL server! */

   start = ecore_time_get();
   while (efl_io_buffered_stream_pending_write_get(cl->socket) && ((ecore_time_get() - start) <= ecore_animator_frametime_get()))
     ecore_con_client_flush(cl);
   if (efl_io_buffered_stream_pending_write_get(cl->socket))
     {
        ERR("cl=%p still pending send %zd bytes! Flush client before upgrading to SSL!",
            cl, efl_io_buffered_stream_pending_write_get(cl->socket));
        return EINA_FALSE;
     }

   if ((!svr->ssl.upgrade_type) || (!svr->ssl.clients_ctx))
     {
        svr->ssl.upgrade_type = compl_type;
        svr->ssl.clients_ctx = _ecore_con_server_ssl_ctx_create(svr);
        EINA_SAFETY_ON_NULL_RETURN_VAL(svr->ssl.clients_ctx, EINA_FALSE);
     }

   if (svr->ssl.upgrade_type == compl_type)
     ssl_ctx = efl_ref(svr->ssl.clients_ctx);
   else
     ssl_ctx = _ecore_con_server_ssl_ctx_create(svr);

   EINA_SAFETY_ON_NULL_RETURN_VAL(ssl_ctx, EINA_FALSE);

   cl->ssl.upgrading = EINA_TRUE;
   cl->ssl.ctx = ssl_ctx;

   eina_future_then(efl_loop_Eina_FutureXXX_job(efl_loop_get(cl->socket)),
                    _ecore_con_client_ssl_upgrade_job, cl, &cl->ssl.job);

   DBG("cl=%p SSL upgrading from %#x to type=%#x", cl, svr->type, compl_type);

   return EINA_TRUE;
}

/**
 * @}
 */

Eina_Bool
ecore_con_server_check(const Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr, EINA_FALSE);
   return EINA_TRUE;
}

static Ecore_Con_Server *
_ecore_con_server_new(Eina_Bool is_dialer, Ecore_Con_Type type, const char *name, int port, const void *data)
{
   Ecore_Con_Server *svr = calloc(1, sizeof(Ecore_Con_Server));
   EINA_SAFETY_ON_NULL_RETURN_VAL(svr, NULL);
   svr->start_time = ecore_time_get();
   svr->name = eina_stringshare_add(name);
   svr->port = port;
   svr->data = data;
   svr->type = type;
   svr->is_dialer = is_dialer;

   EINA_MAGIC_SET(svr, ECORE_MAGIC_CON_SERVER);
   _servers = eina_list_append(_servers, svr);
   return svr;
}

static void
_ecore_con_server_dialer_close(Ecore_Con_Server *svr)
{
   if (!svr->dialer) return;

   if (!efl_io_closer_closed_get(svr->dialer))
     efl_io_closer_close(svr->dialer);
}

static void
_ecore_con_server_free(Ecore_Con_Server *svr)
{
   const char *str;

   svr->delete_me = EINA_TRUE;
   _servers = eina_list_remove(_servers, svr);

   while (svr->clients)
     ecore_con_client_del(svr->clients->data);

   _ecore_con_server_dialer_close(svr);

   if (svr->dialer)
     {
        efl_del(svr->dialer);
        svr->dialer = NULL;
     }

   if (svr->server)
     {
        efl_del(svr->server);
        svr->server = NULL;
     }

   if (svr->ssl.clients_ctx)
     {
        efl_unref(svr->ssl.clients_ctx);
        svr->ssl.clients_ctx = NULL;
     }

   if (svr->ssl.job) eina_future_cancel(svr->ssl.job);

   if (svr->ssl.pending_send)
     {
        eina_binbuf_free(svr->ssl.pending_send);
        svr->ssl.pending_send = NULL;
     }

   if (svr->event_count) return;

   EINA_LIST_FREE(svr->ssl.certs, str) eina_stringshare_del(str);
   EINA_LIST_FREE(svr->ssl.privkeys, str) eina_stringshare_del(str);
   EINA_LIST_FREE(svr->ssl.crls, str) eina_stringshare_del(str);
   EINA_LIST_FREE(svr->ssl.cafiles, str) eina_stringshare_del(str);
   eina_stringshare_replace(&svr->ssl.verify_name, NULL);

   svr->data = NULL;
   eina_stringshare_replace(&svr->name, NULL);
   eina_stringshare_replace(&svr->ip, NULL);

   EINA_MAGIC_SET(svr, ECORE_MAGIC_NONE);
   free(svr);
}

/* BEGIN: post of shared Ecore_Event  ***********************************/

static void
_ecore_con_free_event_server_del(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Server_Del *ev = event;

   if (ev->server)
     {
        Ecore_Con_Server *svr = ev->server;
        svr->event_count = eina_list_remove(svr->event_count, ev);
        if ((!svr->event_count) && (svr->delete_me))
          _ecore_con_server_free(svr);
     }
   ecore_con_event_server_del_free(ev);
   _ecore_con_event_count--;
}

static Eina_Bool
_ecore_con_post_event_server_del(Ecore_Con_Server *svr)
{
   Ecore_Con_Event_Server_Del *ev = ecore_con_event_server_del_alloc();
   EINA_SAFETY_ON_NULL_GOTO(ev, error);

   if (svr->dialer)
     INF("svr=%p disconnected from %s (%s)", svr, efl_net_dialer_address_dial_get(svr->dialer), efl_net_socket_address_remote_get(svr->dialer));
   else if (svr->server)
     INF("svr=%p stopped serving at %s", svr, efl_net_server_address_get(svr->server));
   else
     INF("svr=%p name=%s, port=%d %s destroyed before SSL was configured", svr, svr->name, svr->port, svr->is_dialer ? "dialer" : "server");

   if (svr->connecting)
     {
        DBG("svr=%p was still connecting to %s (%s), ignore ECORE_CON_EVENT_SERVER_DEL", svr, efl_net_dialer_address_dial_get(svr->dialer), efl_net_socket_address_remote_get(svr->dialer));
        return EINA_TRUE;
     }

   ev->server = svr;
   svr->event_count = eina_list_append(svr->event_count, ev);

   ecore_event_add(ECORE_CON_EVENT_SERVER_DEL, ev, _ecore_con_free_event_server_del, NULL);
   _ecore_con_event_count++;
   return EINA_TRUE;

 error:
   _ecore_con_server_free(svr);
   return EINA_FALSE;
}

static void
_ecore_con_free_event_server_error(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Server_Error *ev = event;

   if (ev->server)
     {
        Ecore_Con_Server *svr = ev->server;
        svr->event_count = eina_list_remove(svr->event_count, ev);
        if ((!svr->event_count) && (svr->delete_me))
          _ecore_con_server_free(svr);
     }
   free(ev->error);
   ecore_con_event_server_error_free(ev);
   _ecore_con_event_count--;
}

static Eina_Bool
_ecore_con_post_event_server_error(Ecore_Con_Server *svr, const char *err)
{
   Ecore_Con_Event_Server_Error *ev = ecore_con_event_server_error_alloc();
   EINA_SAFETY_ON_NULL_GOTO(ev, error);

   if (svr->dialer)
     INF("svr=%p error from %s (%s): %s", svr, efl_net_dialer_address_dial_get(svr->dialer), efl_net_socket_address_remote_get(svr->dialer), err);
   else if (svr->server)
     INF("svr=%p error at %s: %s", svr, efl_net_server_address_get(svr->server), err);
   else
     INF("svr=%p name=%s, port=%d %s error before SSL was configured: %s", svr, svr->name, svr->port, svr->is_dialer ? "dialer" : "server", err);

   ev->server = svr;
   ev->error = err ? strdup(err) : NULL;
   svr->event_count = eina_list_append(svr->event_count, ev);

   ecore_event_add(ECORE_CON_EVENT_SERVER_ERROR, ev, _ecore_con_free_event_server_error, NULL);
   _ecore_con_event_count++;
   return EINA_TRUE;

 error:
   _ecore_con_server_free(svr);
   return EINA_FALSE;
}

/* END: post of shared Ecore_Event  ***********************************/


/* BEGIN: post of Ecore_Event for ecore_con_server_connect() ************/

static void
_ecore_con_free_event_server_add(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Server_Add *ev = event;

   if (ev->server)
     {
        Ecore_Con_Server *svr = ev->server;
        svr->event_count = eina_list_remove(svr->event_count, ev);
        if ((!svr->event_count) && (svr->delete_me))
          _ecore_con_server_free(svr);
     }
   ecore_con_event_server_add_free(ev);
   _ecore_con_event_count--;
}

static void
_ecore_con_post_event_server_add(Ecore_Con_Server *svr)
{
   Ecore_Con_Event_Server_Add *ev = ecore_con_event_server_add_alloc();
   EINA_SAFETY_ON_NULL_GOTO(ev, error);

   ev->server = svr;
   svr->event_count = eina_list_append(svr->event_count, ev);

   ecore_event_add(ECORE_CON_EVENT_SERVER_ADD, ev, _ecore_con_free_event_server_add, NULL);
   _ecore_con_event_count++;
   return;

 error:
   _ecore_con_server_free(svr);
}

static void
_ecore_con_free_event_server_data(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Server_Data *ev = event;

   if (ev->server)
     {
        Ecore_Con_Server *svr = ev->server;
        svr->event_count = eina_list_remove(svr->event_count, ev);
        if ((!svr->event_count) && (svr->delete_me))
          _ecore_con_server_free(svr);
     }
   free(ev->data);
   ecore_con_event_server_data_free(ev);
   _ecore_con_event_count--;
}

static void
_ecore_con_post_event_server_data(Ecore_Con_Server *svr, Eina_Rw_Slice slice)
{
   Ecore_Con_Event_Server_Data *ev = ecore_con_event_server_data_alloc();
   EINA_SAFETY_ON_NULL_GOTO(ev, error);

   ev->server = svr;
   ev->data = slice.mem;
   ev->size = slice.len;
   svr->event_count = eina_list_append(svr->event_count, ev);

   ecore_event_add(ECORE_CON_EVENT_SERVER_DATA, ev, _ecore_con_free_event_server_data, NULL);
   _ecore_con_event_count++;
   return;

 error:
   free(slice.mem);
}

static void
_ecore_con_free_event_server_write(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Server_Write *ev = event;

   if (ev->server)
     {
        Ecore_Con_Server *svr = ev->server;
        svr->event_count = eina_list_remove(svr->event_count, ev);
        if ((!svr->event_count) && (svr->delete_me))
          _ecore_con_server_free(svr);
     }
   ecore_con_event_server_write_free(ev);
   _ecore_con_event_count--;
}

static void
_ecore_con_post_event_server_write(Ecore_Con_Server *svr, size_t size)
{
   Ecore_Con_Event_Server_Write *ev = ecore_con_event_server_write_alloc();
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->server = svr;
   ev->size = size;
   svr->event_count = eina_list_append(svr->event_count, ev);

   ecore_event_add(ECORE_CON_EVENT_SERVER_WRITE, ev, _ecore_con_free_event_server_write, NULL);
   _ecore_con_event_count++;
}

static void
_ecore_con_free_event_server_upgrade(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Server_Upgrade *ev = event;

   if (ev->server)
     {
        Ecore_Con_Server *svr = ev->server;
        svr->event_count = eina_list_remove(svr->event_count, ev);
        if ((!svr->event_count) && (svr->delete_me))
          _ecore_con_server_free(svr);
     }
   ecore_con_event_server_upgrade_free(ev);
   _ecore_con_event_count--;
}

static void
_ecore_con_post_event_server_upgrade(Ecore_Con_Server *svr)
{
   Ecore_Con_Event_Server_Upgrade *ev = ecore_con_event_server_upgrade_alloc();
   EINA_SAFETY_ON_NULL_GOTO(ev, error);

   ev->server = svr;
   svr->event_count = eina_list_append(svr->event_count, ev);

   ecore_event_add(ECORE_CON_EVENT_SERVER_UPGRADE, ev, _ecore_con_free_event_server_upgrade, NULL);
   _ecore_con_event_count++;
   return;

 error:
   _ecore_con_server_free(svr);
}

/* END: post of Ecore_Event for ecore_con_server_connect() **************/

static void
_ecore_con_server_dialer_slice_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Server *svr = data;
   Eina_Slice ro_slice;
   Eina_Rw_Slice rw_slice;

   if (svr->delete_me) return;
   if (svr->ssl.upgrading) return;

   ro_slice = efl_io_buffered_stream_slice_get(svr->dialer);
   if (ro_slice.len == 0) return;

   rw_slice = eina_slice_dup(ro_slice);
   efl_io_buffered_stream_clear(svr->dialer);
   _ecore_con_post_event_server_data(svr, rw_slice);
}

static void
_ecore_con_server_dialer_progress(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Server *svr = data;
   size_t now;

   if (svr->delete_me) return;
   if (svr->ssl.upgrading) return;

   now = efl_io_buffered_stream_pending_write_get(svr->dialer);
   if (svr->pending_write == now) return;
   EINA_SAFETY_ON_TRUE_GOTO(svr->pending_write < now, end); /* forgot to update! */

   _ecore_con_post_event_server_write(svr, svr->pending_write - now);

 end:
   svr->pending_write = now;
}

static void
_ecore_con_server_dialer_read_finished(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Server *svr = data;

   if (svr->delete_me) return;

   if (!efl_io_closer_closed_get(svr->dialer))
     efl_io_buffered_stream_eos_mark(svr->dialer);
}

static void
_ecore_con_server_dialer_finished(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Server *svr = data;

   if (svr->delete_me) return;

   _ecore_con_post_event_server_del(svr);
}

static void
_ecore_con_server_dialer_error(void *data, const Efl_Event *event)
{
   Ecore_Con_Server *svr = data;
   Eina_Error *perr = event->info;

   if (svr->delete_me) return;

   WRN("error reaching server %s: %s", efl_net_dialer_address_dial_get(svr->dialer), eina_error_msg_get(*perr));

   _ecore_con_server_dialer_close(svr);
   _ecore_con_post_event_server_error(svr, eina_error_msg_get(*perr));
}

static void
_ecore_con_server_dialer_resolved(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Server *svr = data;
   Eo *inner_dialer;

   if (svr->delete_me) return;

   inner_dialer = efl_io_buffered_stream_inner_io_get(svr->dialer);
   if (efl_isa(inner_dialer, EFL_NET_DIALER_TCP_CLASS) ||
       efl_isa(inner_dialer, EFL_NET_DIALER_UDP_CLASS) ||
       efl_isa(inner_dialer, EFL_NET_DIALER_SSL_CLASS))
     {
        const char *p, *ip = efl_net_socket_address_remote_get(svr->dialer);
        EINA_SAFETY_ON_NULL_RETURN(ip);

        if (ip[0] != '[') p = strchr(ip, ':');
        else
          {
             ip++;
             p = strchr(ip, ']');
          }
        EINA_SAFETY_ON_NULL_RETURN(p);

        eina_stringshare_del(svr->ip);
        svr->ip = eina_stringshare_add_length(ip, p - ip);
     }
}

static void
_ecore_con_server_dialer_connected(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Server *svr = data;

   if (svr->delete_me) return;
   if (svr->ssl.upgrading)
     {
        svr->ssl.upgrading = EINA_FALSE;
        INF("svr=%p upgraded to SSL at %s (%s)", svr, efl_net_dialer_address_dial_get(svr->dialer), efl_net_socket_address_remote_get(svr->dialer));
        if (svr->ssl.pending_send)
          {
             Eina_Slice slice = eina_binbuf_slice_get(svr->ssl.pending_send);
             ecore_con_server_send(svr, slice.mem, slice.len);
             eina_binbuf_free(svr->ssl.pending_send);
             svr->ssl.pending_send = NULL;
          }
        _ecore_con_post_event_server_upgrade(svr);
        return;
     }

   svr->connecting = EINA_FALSE;
   svr->start_time = ecore_time_get();

   INF("svr=%p connected to %s (%s)", svr, efl_net_dialer_address_dial_get(svr->dialer), efl_net_socket_address_remote_get(svr->dialer));

   _ecore_con_post_event_server_add(svr);
}

EFL_CALLBACKS_ARRAY_DEFINE(_ecore_con_server_dialer_cbs,
                           { EFL_IO_BUFFERED_STREAM_EVENT_PROGRESS, _ecore_con_server_dialer_progress },
                           { EFL_IO_BUFFERED_STREAM_EVENT_SLICE_CHANGED, _ecore_con_server_dialer_slice_changed },
                           { EFL_IO_BUFFERED_STREAM_EVENT_READ_FINISHED, _ecore_con_server_dialer_read_finished },
                           { EFL_IO_BUFFERED_STREAM_EVENT_FINISHED, _ecore_con_server_dialer_finished },
                           { EFL_NET_DIALER_EVENT_ERROR, _ecore_con_server_dialer_error },
                           { EFL_NET_DIALER_EVENT_RESOLVED, _ecore_con_server_dialer_resolved },
                           { EFL_NET_DIALER_EVENT_CONNECTED, _ecore_con_server_dialer_connected });

static void
_ecore_con_server_server_client_add(void *data, const Efl_Event *event)
{
   Ecore_Con_Server *svr = data;
   Ecore_Con_Client *cl;
   Eo *socket = event->info;

   INF("svr=%p address=%s got client %p (%s) address=%s",
       svr, efl_net_server_address_get(svr->server),
       socket,
       efl_class_name_get(efl_io_buffered_stream_inner_io_get(socket)),
       efl_net_socket_address_remote_get(socket));

   cl = ecore_con_client_add(svr, socket);
   EINA_SAFETY_ON_NULL_RETURN(cl);

   _ecore_con_post_event_client_add(cl);
}

static void
_ecore_con_server_server_serving(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Server *svr = data;
   Eo *inner_server;
   const char *address = efl_net_server_address_get(svr->server);
   const char *p;
   char *ip;

   EINA_SAFETY_ON_NULL_RETURN(address);

   INF("svr=%p ready at %s", svr, address);

   if (!svr->want_mcast) return;

   inner_server = efl_net_server_simple_inner_server_get(svr->server);

   if (address[0] != '[') p = strchr(address, ':');
   else
     {
        address++;
        p = strchr(address, ']');
     }
   EINA_SAFETY_ON_NULL_RETURN(p);

   ip = malloc(p - address + 1);
   EINA_SAFETY_ON_NULL_RETURN(ip);

   memcpy(ip, address, p - address);
   ip[p - address] = '\0';

   DBG("svr=%p join multicast group: %s", svr, ip);
   efl_net_server_udp_multicast_join(inner_server, ip);
   free(ip);
}

static void
_ecore_con_server_server_error(void *data, const Efl_Event *event)
{
   Ecore_Con_Server *svr = data;
   Eina_Error *perr = event->info;

   WRN("svr=%p address=%s error: %s",
       svr, efl_net_server_address_get(svr->server),
       eina_error_msg_get(*perr));

   _ecore_con_post_event_server_error(svr, eina_error_msg_get(*perr));
}

EFL_CALLBACKS_ARRAY_DEFINE(_ecore_con_server_server_cbs,
                           { EFL_NET_SERVER_EVENT_CLIENT_ADD, _ecore_con_server_server_client_add },
                           { EFL_NET_SERVER_EVENT_SERVING, _ecore_con_server_server_serving },
                           { EFL_NET_SERVER_EVENT_ERROR, _ecore_con_server_server_error });

/**
 * @addtogroup Ecore_Con_Server_Group Ecore Connection Server Functions
 *
 * Functions that operate on Ecore server objects.
 *
 * @{
 */

static Eina_Bool
_ecore_con_server_server_set(Ecore_Con_Server *svr, Eo *server)
{
   char address[4096] = "";
   Eo *inner_server = efl_net_server_simple_inner_server_get(server);
   Ecore_Con_Type type = svr->type & ECORE_CON_TYPE;
   Eina_Error err;

   svr->server = server;
   efl_event_callback_array_add(server, _ecore_con_server_server_cbs(), svr);

   if (efl_isa(inner_server, EFL_NET_SERVER_TCP_CLASS) ||
       efl_isa(inner_server, EFL_NET_SERVER_UDP_CLASS) ||
       efl_isa(inner_server, EFL_NET_SERVER_SSL_CLASS))
     {
        if (strchr(svr->name, ':'))
          snprintf(address, sizeof(address), "[%s]:%d", svr->name, svr->port);
        else
          snprintf(address, sizeof(address), "%s:%d", svr->name, svr->port);
     }
   else if (type == ECORE_CON_LOCAL_ABSTRACT)
     snprintf(address, sizeof(address), "abstract:%s", svr->name);
   else if ((type == ECORE_CON_LOCAL_USER) ||
            (type == ECORE_CON_LOCAL_SYSTEM))
     {
        char *path = ecore_con_local_path_new(type == ECORE_CON_LOCAL_SYSTEM, svr->name, svr->port);
        if (!path)
          {
             ERR("could not create local path for name='%s', port=%d", svr->name, svr->port);
             return EINA_FALSE;
          }
        else
          {
             eina_strlcpy(address, path, sizeof(address));
             free(path);
          }
     }

   if (efl_isa(inner_server, EFL_NET_SERVER_FD_CLASS))
     {
        efl_net_server_fd_reuse_address_set(inner_server, EINA_TRUE);
        efl_net_server_fd_reuse_port_set(inner_server, EINA_TRUE);
     }
   if (efl_isa(inner_server, EFL_NET_SERVER_TCP_CLASS))
     {
        /* old ecore_con did not map ipv4 to ipv6... */
        efl_net_server_tcp_ipv6_only_set(inner_server, EINA_TRUE);
     }
   else if (efl_isa(inner_server, EFL_NET_SERVER_UDP_CLASS))
     {
        /* old ecore_con did not map ipv4 to ipv6... */
        efl_net_server_udp_ipv6_only_set(inner_server, EINA_TRUE);
        svr->want_mcast = type == ECORE_CON_REMOTE_MCAST;
     }
   else if (efl_isa(inner_server, EFL_NET_SERVER_SSL_CLASS))
     {
        /* old ecore_con did not map ipv4 to ipv6... */
        efl_net_server_ssl_ipv6_only_set(inner_server, EINA_TRUE);
        efl_net_server_ssl_reuse_address_set(inner_server, EINA_TRUE);
        efl_net_server_ssl_reuse_port_set(inner_server, EINA_TRUE);
     }
#ifdef EFL_NET_SERVER_UNIX_CLASS
   else if (efl_isa(inner_server, EFL_NET_SERVER_UNIX_CLASS))
     {
        efl_net_server_unix_unlink_before_bind_set(inner_server, EINA_TRUE);
        efl_net_server_unix_leading_directories_create_set(inner_server, EINA_TRUE, (type == ECORE_CON_LOCAL_SYSTEM) ? 0755 : 0700);
     }
#endif

   if (svr->type & ECORE_CON_SOCKET_ACTIVATE)
     {
        if (efl_isa(inner_server, EFL_NET_SERVER_FD_CLASS))
          efl_net_server_fd_socket_activate(inner_server, address);
        else if (efl_isa(inner_server, EFL_NET_SERVER_SSL_CLASS))
          efl_net_server_ssl_socket_activate(inner_server, address);
        else
          {
             ERR("svr=%p (%s): not able to socket-activate this type!", svr, efl_class_name_get(inner_server));
             goto serve;
          }

        if (efl_net_server_serving_get(inner_server))
          {
             DBG("svr=%p (%s) was socket activated as %s",
                 svr, efl_class_name_get(inner_server), address);
             return EINA_TRUE;
          }
        else
          ERR("svr=%p (%s): not able to socket-activate as %s. Try to serve...", svr, efl_class_name_get(inner_server), address);
     }

 serve:
   err = efl_net_server_serve(svr->server, address);
   if (err)
     {
        WRN("Could not serve at address=%s using class=%s",
            address, efl_class_name_get(inner_server));
        return EINA_FALSE;
     }

   DBG("svr=%p server=%p (%s) address=%s",
       svr, svr->server, efl_class_name_get(inner_server),
       efl_net_server_address_get(svr->server));
   return EINA_TRUE;
}

static Eo *
_ecore_con_server_ssl_ctx_create(const Ecore_Con_Server *svr)
{
   Efl_Net_Ssl_Cipher cipher = EFL_NET_SSL_CIPHER_AUTO;
   Ecore_Con_Type ssl_type = svr->ssl.upgrade_type & ECORE_CON_SSL;

   if (ssl_type & ECORE_CON_USE_MIXED)
     cipher = EFL_NET_SSL_CIPHER_AUTO;
   else if (ssl_type & ECORE_CON_USE_TLS)
     cipher = EFL_NET_SSL_CIPHER_TLSV1;
   else if (ssl_type & ECORE_CON_USE_SSL3)
     cipher = EFL_NET_SSL_CIPHER_SSLV3;
   else if (ssl_type & ECORE_CON_USE_SSL2)
     {
        ERR("SSLv2 is unsupported!");
        return NULL;
     }

   /* legacy compatibility: server never verified peer, only dialer did */

   return efl_add(EFL_NET_SSL_CONTEXT_CLASS, NULL,
                  efl_net_ssl_context_certificates_set(efl_added, eina_list_iterator_new(svr->ssl.certs)),
                  efl_net_ssl_context_private_keys_set(efl_added, eina_list_iterator_new(svr->ssl.privkeys)),
                  efl_net_ssl_context_certificate_revocation_lists_set(efl_added, eina_list_iterator_new(svr->ssl.crls)),
                  efl_net_ssl_context_certificate_authorities_set(efl_added, eina_list_iterator_new(svr->ssl.cafiles)),
                  efl_net_ssl_context_default_paths_load_set(efl_added, EINA_FALSE), /* old API didn't load default paths */
                  efl_net_ssl_context_setup(efl_added, cipher, EINA_FALSE));
}

static Eina_Value
_ecore_con_server_server_ssl_job(void *data, const Eina_Value v,
                                 const Eina_Future *dead EINA_UNUSED)
{
   Ecore_Con_Server *svr = data;
   Eo *loop = ecore_main_loop_get();
   Eo *ssl_ctx;
   Eo *inner_server;
   Eo *server;

   if (v.type == EINA_VALUE_TYPE_ERROR) return v;

   ssl_ctx = _ecore_con_server_ssl_ctx_create(svr);
   EINA_SAFETY_ON_NULL_GOTO(ssl_ctx, error_ssl_ctx);

   inner_server = efl_add(EFL_NET_SERVER_SSL_CLASS, loop,
                          efl_net_server_ssl_context_set(efl_added, ssl_ctx));
   EINA_SAFETY_ON_NULL_GOTO(inner_server, error_inner_server);

   server = efl_add(EFL_NET_SERVER_SIMPLE_CLASS, loop,
                    efl_net_server_simple_inner_server_set(efl_added, inner_server));
   EINA_SAFETY_ON_NULL_GOTO(server, error_server);

   efl_parent_set(inner_server, server);

   efl_unref(ssl_ctx); /* inner_server keeps it */
   efl_unref(inner_server); /* server keeps it */

   if (!_ecore_con_server_server_set(svr, server))
     goto error_serve;

   DBG("server=%p configured with inner_server=%p, ssl_ctx=%p",
       svr->server,
       efl_net_server_simple_inner_server_get(svr->server),
       efl_net_server_ssl_context_get(efl_net_server_simple_inner_server_get(svr->server)));

   return v;

 error_serve:
   if (_ecore_con_post_event_server_error(svr, "Couldn't serve using SSL"))
     _ecore_con_post_event_server_del(svr);
   return v;

 error_server:
   efl_del(inner_server);
 error_inner_server:
   efl_del(ssl_ctx);
 error_ssl_ctx:
   if (_ecore_con_post_event_server_error(svr, "Couldn't finish SSL setup"))
     _ecore_con_post_event_server_del(svr);
   return v;
}

/**
 * @example ecore_con_server_example.c
 * Shows how to write a simple server using the Ecore_Con library
 * using ecore_con_server_add()
 */

EAPI Ecore_Con_Server *
ecore_con_server_add(Ecore_Con_Type compl_type,
                     const char *name,
                     int port,
                     const void *data)
{
   const Efl_Class *cls = NULL;
   Ecore_Con_Server *svr;
   Ecore_Con_Type type;
   Eo *server;
   Eo *loop;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   type = compl_type & ECORE_CON_TYPE;

   loop = ecore_main_loop_get();
   EINA_SAFETY_ON_NULL_RETURN_VAL(loop, NULL);

   svr = _ecore_con_server_new(EINA_FALSE, compl_type, name, port, data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(svr, NULL);

   switch (type)
     {
      case ECORE_CON_LOCAL_USER:
      case ECORE_CON_LOCAL_SYSTEM:
      case ECORE_CON_LOCAL_ABSTRACT:
#ifdef EFL_NET_SERVER_UNIX_CLASS
         cls = EFL_NET_SERVER_UNIX_CLASS;
#elif defined(EFL_NET_SERVER_WINDOWS_CLASS)
         cls = EFL_NET_SERVER_WINDOWS_CLASS;
#else
         ERR("Your platform doesn't support Efl_Net_Server-compatible local communication");
         // TODO: maybe write to a file and use TCP
#endif
         break;

      case ECORE_CON_REMOTE_TCP:
      case ECORE_CON_REMOTE_NODELAY:
      case ECORE_CON_REMOTE_CORK:
         cls = EFL_NET_SERVER_TCP_CLASS;
         break;

      case ECORE_CON_REMOTE_UDP:
      case ECORE_CON_REMOTE_MCAST:
         cls = EFL_NET_SERVER_UDP_CLASS;
         break;

      default:
         ERR("Unsupported type=%#x & %#x = %#x", compl_type, ECORE_CON_TYPE, type);
     }

   EINA_SAFETY_ON_NULL_GOTO(cls, error);

   if (compl_type & ECORE_CON_SSL)
     {
        if (cls != EFL_NET_SERVER_TCP_CLASS)
          ERR("SSL can only be used with TCP types, got %s, forcing TCP", efl_class_name_get(cls));

        /* efl_net_ssl_context must be created prior to the object as
         * it's immutable once created. However the previous
         * Ecore_Con_Server API returned a handle and then configured
         * it, like in runtime, but in practice it's only effective
         * before the server starts.
         *
         * Then do not create the SSL server right away, instead do it
         * from a job, let the user configure SSL before the job
         * finishes.
         *
         * EINA_SAFETY_ON_NULL_RETURN(svr->ssl.job) will alert users
         * trying to configure after the job expires.
         *
         * We can do that since the documentation says:
         *
         * > Call this function on a server object before main loop
         * > has started to enable verification of certificates
         * > against loaded certificates.
         */
        svr->ssl.upgrade_type = compl_type;
        _ecore_con_server_job_schedule(svr, loop, _ecore_con_server_server_ssl_job);
        return svr;
     }

   server = efl_add(EFL_NET_SERVER_SIMPLE_CLASS, loop,
                    efl_net_server_simple_inner_class_set(efl_added, cls));
   EINA_SAFETY_ON_NULL_GOTO(server, error);

   if (!_ecore_con_server_server_set(svr, server))
     goto error;

   return svr;

 error:
   _ecore_con_server_free(svr);
   return NULL;
}

/**
 * @brief convert @a socks to proxy URL string and apply using efl_net_dialer_proxy_set().
 *
 * @param svr the server created by ecore_con_server_connect().
 * @param socks the socks handle created by
 *        ecore_con_socks4_remote_add() or ecore_con_socks5_remote_add().
 *
 * @internal
 */
static void
_ecore_con_server_proxy_apply(Ecore_Con_Server *svr, const Ecore_Con_Socks *socks)
{
   char str[4096], port[sizeof(":65536")] = "";
   const char *protocol;
   const char *user = "";
   const char *userpass_sep = "";
   const char *pass = "";
   const char *auth_sep = "";

   if (socks->version == 4)
     {
        Ecore_Con_Socks_v4 *v4 = (Ecore_Con_Socks_v4 *)socks;

        if (v4->lookup) protocol = "socks4a";
        else protocol = "socks4";

        if (v4->port > 0)
          snprintf(port, sizeof(port), ":%d", v4->port);

        if (v4->username)
          {
             user = v4->username;
             auth_sep = "@";
          }
     }
   else if (socks->version == 5)
     {
        Ecore_Con_Socks_v5 *v5 = (Ecore_Con_Socks_v5 *)socks;

        if (socks->lookup) protocol = "socks5h";
        else protocol = "socks5";

        if (v5->port > 0)
          snprintf(port, sizeof(port), ":%d", v5->port);

        if (v5->username)
          {
             user = v5->username;
             auth_sep = "@";

             if (v5->password)
               {
                  pass = v5->password;
                  userpass_sep = ":";
               }
          }
     }
   else
     {
        ERR("unsupported socks->version=%d", socks->version);
        return;
     }

   snprintf(str, sizeof(str),
            "%s://%s%s%s%s%s%s",
            protocol,
            pass, userpass_sep, user, auth_sep,
            socks->ip, port);
   DBG("using proxy url='%s' for server=%s, port=%d",
       str, svr->name, svr->port);

   efl_net_dialer_proxy_set(svr->dialer, str);

   if (socks->bind)
     ERR("proxy bind is not supported! Ecore_Con_Socks=%p %s", socks, str);
}

static Eina_Bool
_ecore_con_server_dialer_set(Ecore_Con_Server *svr, Eo *dialer)
{
   char address[4096] = "";
   Eo *inner_dialer = efl_io_buffered_stream_inner_io_get(dialer);
   Ecore_Con_Type type = svr->type & ECORE_CON_TYPE;
   Eina_Error err;

   svr->dialer = dialer;
   efl_io_closer_close_on_exec_set(dialer, EINA_TRUE);
   efl_io_closer_close_on_destructor_set(dialer, EINA_TRUE);
   efl_io_buffered_stream_timeout_inactivity_set(dialer, svr->timeout);
   efl_event_callback_array_add(dialer, _ecore_con_server_dialer_cbs(), svr);

   if (efl_isa(inner_dialer, EFL_NET_DIALER_TCP_CLASS))
     {
        efl_net_socket_tcp_no_delay_set(inner_dialer, !!(type & ECORE_CON_REMOTE_NODELAY));
        efl_net_socket_tcp_cork_set(inner_dialer, !!(type & ECORE_CON_REMOTE_CORK));
     }
   else if (efl_isa(inner_dialer, EFL_NET_DIALER_SSL_CLASS))
     {
        efl_net_dialer_ssl_no_delay_set(inner_dialer, !!(type & ECORE_CON_REMOTE_NODELAY));
     }

   if (efl_isa(inner_dialer, EFL_NET_DIALER_TCP_CLASS) ||
       efl_isa(inner_dialer, EFL_NET_DIALER_UDP_CLASS) ||
       efl_isa(inner_dialer, EFL_NET_DIALER_SSL_CLASS))
     {
        if (strchr(svr->name, ':'))
          snprintf(address, sizeof(address), "[%s]:%d", svr->name, svr->port);
        else
          snprintf(address, sizeof(address), "%s:%d", svr->name, svr->port);
     }
   else if (type == ECORE_CON_LOCAL_ABSTRACT)
     snprintf(address, sizeof(address), "abstract:%s", svr->name);
   else if ((type == ECORE_CON_LOCAL_USER) ||
            (type == ECORE_CON_LOCAL_SYSTEM))
     {
        char *path = ecore_con_local_path_new(type == ECORE_CON_LOCAL_SYSTEM, svr->name, svr->port);
#ifdef EFL_NET_DIALER_UNIX_CLASS
        struct stat st;
#endif

        if (!path)
          {
             ERR("could not create local path for name='%s', port=%d", svr->name, svr->port);
             return EINA_FALSE;
          }
        else
          {
             eina_strlcpy(address, path, sizeof(address));
             free(path);
          }

#ifdef EFL_NET_DIALER_UNIX_CLASS
        if ((stat(address, &st) != 0)
#ifdef S_ISSOCK
            || (!S_ISSOCK(st.st_mode))
#endif
            )
          {
             DBG("%s is not a socket", address);
             return EINA_FALSE;
          }
#endif
     }

   if ((svr->type & ECORE_CON_NO_PROXY) == ECORE_CON_NO_PROXY)
     {
        DBG("svr=%p not using any proxy for dialer %p (%s)",
            svr, svr->dialer, efl_class_name_get(inner_dialer));
        efl_net_dialer_proxy_set(svr->dialer, "");
     }
   else if (type > ECORE_CON_LOCAL_ABSTRACT)
     {
        if (_ecore_con_proxy_once)
          {
             _ecore_con_server_proxy_apply(svr, _ecore_con_proxy_once);
             _ecore_con_proxy_once = NULL;
          }
        else if (_ecore_con_proxy_global)
          _ecore_con_server_proxy_apply(svr, _ecore_con_proxy_global);
     }

   svr->connecting = EINA_TRUE;

   err = efl_net_dialer_dial(svr->dialer, address);
   if (err)
     {
        WRN("Could not connect to address=%s using class=%s",
            address, efl_class_name_get(inner_dialer));
        return EINA_FALSE;
     }

   DBG("svr=%p dialer=%p (%s) address=%s",
       svr, svr->dialer, efl_class_name_get(inner_dialer),
       efl_net_dialer_address_dial_get(svr->dialer));
   return EINA_TRUE;
}

static Eina_Value
_ecore_con_server_dialer_ssl_job(void *data, const Eina_Value v,
                                 const Eina_Future *dead EINA_UNUSED)
{
   Ecore_Con_Server *svr = data;
   Eo *loop = ecore_main_loop_get();
   Eo *ssl_ctx;
   Eo *inner_dialer;
   Eo *dialer;
   Efl_Net_Ssl_Cipher cipher = EFL_NET_SSL_CIPHER_AUTO;
   Efl_Net_Ssl_Verify_Mode verify_mode = EFL_NET_SSL_VERIFY_MODE_NONE; /* was the default */
   Ecore_Con_Type ssl_type = svr->ssl.upgrade_type & ECORE_CON_SSL;

   if (ssl_type & ECORE_CON_USE_MIXED)
     cipher = EFL_NET_SSL_CIPHER_AUTO;
   else if (ssl_type & ECORE_CON_USE_TLS)
     cipher = EFL_NET_SSL_CIPHER_TLSV1;
   else if (ssl_type & ECORE_CON_USE_SSL3)
     cipher = EFL_NET_SSL_CIPHER_SSLV3;
   else if (ssl_type & ECORE_CON_USE_SSL2)
     {
        ERR("SSLv2 is unsupported!");
        goto error_ssl_ctx;
     }

   if (svr->ssl.verify)
     verify_mode = EFL_NET_SSL_VERIFY_MODE_REQUIRED;

   ssl_ctx = efl_add(EFL_NET_SSL_CONTEXT_CLASS, NULL,
                     efl_net_ssl_context_certificates_set(efl_added, eina_list_iterator_new(svr->ssl.certs)),
                     efl_net_ssl_context_private_keys_set(efl_added, eina_list_iterator_new(svr->ssl.privkeys)),
                     efl_net_ssl_context_certificate_revocation_lists_set(efl_added, eina_list_iterator_new(svr->ssl.crls)),
                     efl_net_ssl_context_certificate_authorities_set(efl_added, eina_list_iterator_new(svr->ssl.cafiles)),
                     efl_net_ssl_context_verify_mode_set(efl_added, verify_mode),
                     efl_net_ssl_context_hostname_set(efl_added, svr->ssl.verify_name ? svr->ssl.verify_name : svr->name),
                     efl_net_ssl_context_hostname_verify_set(efl_added, svr->ssl.verify_basic),
                     efl_net_ssl_context_default_paths_load_set(efl_added, EINA_FALSE), /* old API didn't load default paths */
                     efl_net_ssl_context_setup(efl_added, cipher, EINA_TRUE));
   EINA_SAFETY_ON_NULL_GOTO(ssl_ctx, error_ssl_ctx);

   inner_dialer = efl_add(EFL_NET_DIALER_SSL_CLASS, loop,
                          efl_net_dialer_ssl_context_set(efl_added, ssl_ctx));
   EINA_SAFETY_ON_NULL_GOTO(inner_dialer, error_inner_dialer);

   dialer = efl_add(EFL_NET_DIALER_SIMPLE_CLASS, loop,
                    efl_io_buffered_stream_inner_io_set(efl_added, inner_dialer));
   EINA_SAFETY_ON_NULL_GOTO(dialer, error_dialer);

   efl_parent_set(inner_dialer, dialer);

   efl_unref(ssl_ctx); /* inner_dialer keeps it */
   efl_unref(inner_dialer); /* dialer keeps it */

   if (!_ecore_con_server_dialer_set(svr, dialer))
     goto error_dial;

   DBG("dialer=%p configured with inner_dialer=%p, ssl_ctx=%p",
       svr->dialer,
       efl_io_buffered_stream_inner_io_get(svr->dialer),
       efl_net_dialer_ssl_context_get(efl_io_buffered_stream_inner_io_get(svr->dialer)));

   if (svr->ssl.pending_send)
     {
        /* if ecore_con_server_send() was called before the job completed
         * then we queued data there, flush to dialer.
         * See https://phab.enlightenment.org/T5339
         */
        Eina_Slice slice = eina_binbuf_slice_get(svr->ssl.pending_send);
        ecore_con_server_send(svr, slice.mem, slice.len);
        eina_binbuf_free(svr->ssl.pending_send);
        svr->ssl.pending_send = NULL;
     }

   return v;

 error_dial:
   if (_ecore_con_post_event_server_error(svr, "Couldn't dial using SSL"))
     _ecore_con_post_event_server_del(svr);
   return v;

 error_dialer:
   efl_del(inner_dialer);
 error_inner_dialer:
   efl_del(ssl_ctx);
 error_ssl_ctx:
   if (_ecore_con_post_event_server_error(svr, "Couldn't finish SSL setup"))
     _ecore_con_post_event_server_del(svr);
   return v;
}

static Eina_Value
_ecore_con_server_dialer_ssl_upgrade_job(void *data, const Eina_Value v,
                                         const Eina_Future *dead EINA_UNUSED)
{
   Ecore_Con_Server *svr = data;
   Eo *loop = ecore_main_loop_get();
   Eo *ssl_ctx;
   Eo *inner_dialer;
   Eo *dialer;
   Eo *tcp_dialer;
   Efl_Net_Ssl_Cipher cipher = EFL_NET_SSL_CIPHER_AUTO;
   Efl_Net_Ssl_Verify_Mode verify_mode = EFL_NET_SSL_VERIFY_MODE_NONE; /* was the default */
   Ecore_Con_Type ssl_type = svr->ssl.upgrade_type & ECORE_CON_SSL;

   //Canceled
   if (v.type == EINA_VALUE_TYPE_ERROR) return v;

   if (ssl_type & ECORE_CON_USE_MIXED)
     cipher = EFL_NET_SSL_CIPHER_AUTO;
   else if (ssl_type & ECORE_CON_USE_TLS)
     cipher = EFL_NET_SSL_CIPHER_TLSV1;
   else if (ssl_type & ECORE_CON_USE_SSL3)
     cipher = EFL_NET_SSL_CIPHER_SSLV3;
   else if (ssl_type & ECORE_CON_USE_SSL2)
     {
        ERR("SSLv2 is unsupported!");
        goto error_ssl_ctx;
     }

   if (svr->ssl.verify)
     verify_mode = EFL_NET_SSL_VERIFY_MODE_REQUIRED;

   ssl_ctx = efl_add(EFL_NET_SSL_CONTEXT_CLASS, NULL,
                     efl_net_ssl_context_certificates_set(efl_added, eina_list_iterator_new(svr->ssl.certs)),
                     efl_net_ssl_context_private_keys_set(efl_added, eina_list_iterator_new(svr->ssl.privkeys)),
                     efl_net_ssl_context_certificate_revocation_lists_set(efl_added, eina_list_iterator_new(svr->ssl.crls)),
                     efl_net_ssl_context_certificate_authorities_set(efl_added, eina_list_iterator_new(svr->ssl.cafiles)),
                     efl_net_ssl_context_verify_mode_set(efl_added, verify_mode),
                     efl_net_ssl_context_hostname_set(efl_added, svr->ssl.verify_name ? svr->ssl.verify_name : svr->name),
                     efl_net_ssl_context_hostname_verify_set(efl_added, svr->ssl.verify_basic),
                     efl_net_ssl_context_default_paths_load_set(efl_added, EINA_FALSE), /* old API didn't load default paths */
                     efl_net_ssl_context_setup(efl_added, cipher, EINA_TRUE));
   EINA_SAFETY_ON_NULL_GOTO(ssl_ctx, error_ssl_ctx);

   tcp_dialer = svr->dialer;
   svr->dialer = NULL; /* take it, will be wrapped */

   inner_dialer = efl_add(EFL_NET_DIALER_SSL_CLASS, loop,
                          efl_net_socket_ssl_adopt(efl_added, tcp_dialer, ssl_ctx));
   EINA_SAFETY_ON_NULL_GOTO(inner_dialer, error_inner_dialer);

   efl_parent_set(tcp_dialer, inner_dialer);

   dialer = efl_add(EFL_NET_DIALER_SIMPLE_CLASS, loop,
                    efl_io_buffered_stream_inner_io_set(efl_added, inner_dialer));
   EINA_SAFETY_ON_NULL_GOTO(dialer, error_dialer);

   efl_parent_set(inner_dialer, dialer);

   efl_unref(ssl_ctx); /* inner_dialer keeps it */
   efl_unref(inner_dialer); /* dialer keeps it */

   svr->dialer = dialer;
   efl_io_closer_close_on_exec_set(dialer, EINA_TRUE);
   efl_io_closer_close_on_destructor_set(dialer, EINA_TRUE);
   efl_event_callback_array_del(tcp_dialer, _ecore_con_server_dialer_cbs(), svr);
   efl_event_callback_array_add(dialer, _ecore_con_server_dialer_cbs(), svr);

   DBG("dialer=%p upgraded to SSL with inner_dialer=%p, ssl_ctx=%p, tcp_dialer=%p. Start handshake...",
       svr->dialer,
       efl_io_buffered_stream_inner_io_get(svr->dialer),
       efl_net_dialer_ssl_context_get(efl_io_buffered_stream_inner_io_get(svr->dialer)),
       tcp_dialer);
   return v;

 error_dialer:
   efl_del(inner_dialer);
 error_inner_dialer:
   svr->dialer = tcp_dialer; /* put it back */
   efl_del(ssl_ctx);
 error_ssl_ctx:
   if (_ecore_con_post_event_server_error(svr, "Couldn't finish SSL setup"))
     _ecore_con_post_event_server_del(svr);
   return v;
}

/**
 * @example ecore_con_client_example.c
 *
 * Shows how to write a simple client (dialer that connects to the
 * example server using ecore_con_server_connect().
 */

EAPI Ecore_Con_Server *
ecore_con_server_connect(Ecore_Con_Type compl_type,
                         const char *name,
                         int port,
                         const void *data)
{
   const Efl_Class *cls = NULL;
   Ecore_Con_Server *svr;
   Ecore_Con_Type type;
   Eo *dialer;
   Eo *loop;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   type = compl_type & ECORE_CON_TYPE;

   loop = ecore_main_loop_get();
   EINA_SAFETY_ON_NULL_RETURN_VAL(loop, NULL);

   svr = _ecore_con_server_new(EINA_TRUE, compl_type, name, port, data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(svr, NULL);

   switch (type)
     {
      case ECORE_CON_LOCAL_USER:
      case ECORE_CON_LOCAL_SYSTEM:
      case ECORE_CON_LOCAL_ABSTRACT:
#ifdef EFL_NET_DIALER_UNIX_CLASS
         cls = EFL_NET_DIALER_UNIX_CLASS;
#elif defined(EFL_NET_DIALER_WINDOWS_CLASS)
         cls = EFL_NET_DIALER_WINDOWS_CLASS;
#else
         ERR("Your platform doesn't support Efl_Net_Dialer-compatible local communication");
         // TODO: maybe write to a file and use TCP
#endif
         break;

      case ECORE_CON_REMOTE_TCP:
      case ECORE_CON_REMOTE_NODELAY:
      case ECORE_CON_REMOTE_CORK:
         cls = EFL_NET_DIALER_TCP_CLASS;
         break;

      case ECORE_CON_REMOTE_UDP:
      case ECORE_CON_REMOTE_BROADCAST:
         cls = EFL_NET_DIALER_UDP_CLASS;
         break;

      default:
         ERR("Unsupported type=%#x & %#x = %#x", compl_type, ECORE_CON_TYPE, type);
     }
   EINA_SAFETY_ON_NULL_GOTO(cls, error);

   if (compl_type & ECORE_CON_SSL)
     {
        if (cls != EFL_NET_DIALER_TCP_CLASS)
          ERR("SSL can only be used with TCP types, got %s, forcing TCP", efl_class_name_get(cls));

        /* efl_net_ssl_context must be created prior to the object as
         * it's immutable once created. However the previous
         * Ecore_Con_Server API returned a handle and then configured
         * it, like in runtime, but in practice it's only effective
         * before the connection happens.
         *
         * Then do not create the SSL dialer right away, instead do it
         * from a job, let the user configure SSL before the job
         * finishes.
         *
         * EINA_SAFETY_ON_NULL_RETURN(svr->ssl.job) will alert users
         * trying to configure after the job expires.
         *
         * We can do that since the documentation says:
         *
         * > Call this function on a server object before main loop
         * > has started to enable verification of certificates
         * > against loaded certificates.
         */
        _ecore_con_server_job_schedule(svr, loop, _ecore_con_server_dialer_ssl_job);
        return svr;
     }

   dialer = efl_add(EFL_NET_DIALER_SIMPLE_CLASS, loop,
                    efl_net_dialer_simple_inner_class_set(efl_added, cls));
   EINA_SAFETY_ON_NULL_GOTO(dialer, error);

   if (!_ecore_con_server_dialer_set(svr, dialer))
     goto error;

   return svr;

 error:
   _ecore_con_server_free(svr);
   return NULL;
}

EAPI const char *
ecore_con_server_name_get(const Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RELAXED_RETURN(svr, NULL);
   return svr->name;
}

EAPI void
ecore_con_server_client_limit_set(Ecore_Con_Server *svr,
                                  int client_limit,
                                  char reject_excess_clients)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr);
   if (!svr->server) return;
   efl_net_server_clients_limit_set(svr->server, client_limit, reject_excess_clients);
}

EAPI const Eina_List *
ecore_con_server_clients_get(const Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr, NULL);
   return svr->clients;
}

EAPI void
ecore_con_server_timeout_set(Ecore_Con_Server *svr, double timeout)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr);

   svr->timeout = timeout; /* used for new clients */

   if (!svr->dialer) return;
   efl_io_buffered_stream_timeout_inactivity_set(svr->dialer, timeout);
}

EAPI double
ecore_con_server_timeout_get(const Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr, -1.0);
   return svr->timeout;
}

EAPI void *
ecore_con_server_del(Ecore_Con_Server *svr)
{
   const void *data;

   ECORE_CON_SERVER_CHECK_RELAXED_RETURN(svr, NULL);

   data = svr->data;

   _ecore_con_server_free(svr);
   return (void *)data;
}

EAPI void *
ecore_con_server_data_get(Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RELAXED_RETURN(svr, NULL);
   return (void *)svr->data;
}

EAPI void *
ecore_con_server_data_set(Ecore_Con_Server *svr,
                          void *data)
{
   const void *old;

   ECORE_CON_SERVER_CHECK_RELAXED_RETURN(svr, NULL);
   old = svr->data;
   svr->data = data;
   return (void *)old;
}

EAPI Eina_Bool
ecore_con_server_connected_get(const Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr, EINA_FALSE);
   if (svr->is_dialer)
     {
        if (svr->dialer)
          return efl_net_dialer_connected_get(svr->dialer);
        return EINA_FALSE; /* still setting up SSL */
     }
   return EINA_TRUE;
}

EAPI int
ecore_con_server_port_get(const Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RELAXED_RETURN(svr, -1);
   return svr->port;
}

EAPI int
ecore_con_server_send(Ecore_Con_Server *svr, const void *data, int size)
{
   Eina_Error err;
   Eina_Slice slice = { .mem = data, .len = size };

   ECORE_CON_SERVER_CHECK_RETURN(svr, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, 0);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(size < 1, 0);

   /* while upgrading or no dialer (due SSL dialer being created on
    * the next mainloop iteration from a job as described from
    * ecore_con_server_connect()), queue pending data and send as soon
    * as the dialer is assigned.
    *
    * This allows immediate usage of ecore_con_server_send() after
    * ecore_con_server_connect() as the old API did and needed by
    * https://phab.enlightenment.org/T5339
    */
   if ((svr->ssl.upgrading) || (!svr->dialer))
     {
        Eina_Bool r;
        if (!svr->ssl.pending_send)
          {
             svr->ssl.pending_send = eina_binbuf_new();
             EINA_SAFETY_ON_NULL_RETURN_VAL(svr->ssl.pending_send, 0);
          }
        r = eina_binbuf_append_length(svr->ssl.pending_send, data, size);
        EINA_SAFETY_ON_FALSE_RETURN_VAL(r, 0);
        return size;
     }

   err = efl_io_writer_write(svr->dialer, &slice, NULL);
   if (err)
     {
        ERR("svr=%p could not send data=%p, size=%d: %s",
            svr, data, size, eina_error_msg_get(err));
        return 0;
     }
   svr->pending_write = efl_io_buffered_stream_pending_write_get(svr->dialer);

   return slice.len;
}

EAPI const char *
ecore_con_server_ip_get(const Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RELAXED_RETURN(svr, NULL);
   return svr->ip;
}

EAPI double
ecore_con_server_uptime_get(const Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RELAXED_RETURN(svr, 0.0);
   return ecore_time_get() - svr->start_time;
}

EAPI void
ecore_con_server_flush(Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr);
   Eo *inner_dialer;

   if (!svr->dialer) return;

   while (!efl_io_closer_closed_get(svr->dialer) &&
          !efl_net_dialer_connected_get(svr->dialer))
     ecore_main_loop_iterate();

   efl_io_buffered_stream_flush(svr->dialer, EINA_FALSE, EINA_TRUE);

   inner_dialer = efl_io_buffered_stream_inner_io_get(svr->dialer);

   if (!efl_isa(inner_dialer, EFL_NET_SOCKET_TCP_CLASS)) return;
   if (!efl_net_socket_tcp_cork_get(inner_dialer)) return;

   efl_net_socket_tcp_cork_set(inner_dialer, EINA_FALSE);
   efl_net_socket_tcp_cork_set(inner_dialer, EINA_TRUE);
}

EAPI int
ecore_con_server_fd_get(const Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
   if (svr->dialer)
     {
        Eo *inner_dialer = efl_io_buffered_stream_inner_io_get(svr->dialer);
        if (efl_isa(inner_dialer, EFL_LOOP_FD_CLASS))
          return efl_loop_fd_get(inner_dialer);
        return SOCKET_TO_LOOP_FD(INVALID_SOCKET);
     }
   if (svr->server)
     {
        Eo *inner_server = efl_net_server_simple_inner_server_get(svr->server);
        if (efl_isa(inner_server, EFL_LOOP_FD_CLASS))
          return efl_loop_fd_get(inner_server);
        return SOCKET_TO_LOOP_FD(INVALID_SOCKET);
     }
   return SOCKET_TO_LOOP_FD(INVALID_SOCKET);
}

EAPI Eina_Bool
ecore_con_ssl_server_cert_add(Ecore_Con_Server *svr, const char *cert)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cert, EINA_FALSE);

   if (!svr->server) /* SSL adds a job to allow setup */
     EINA_SAFETY_ON_NULL_RETURN_VAL(svr->ssl.job, EINA_FALSE);
   else
     EINA_SAFETY_ON_TRUE_RETURN_VAL(svr->ssl.clients_ctx != NULL, EINA_FALSE);

   svr->ssl.certs = eina_list_append(svr->ssl.certs, eina_stringshare_add(cert));
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_con_ssl_server_privkey_add(Ecore_Con_Server *svr, const char *privkey)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(privkey, EINA_FALSE);

   if (!svr->server) /* SSL adds a job to allow setup */
     EINA_SAFETY_ON_NULL_RETURN_VAL(svr->ssl.job, EINA_FALSE);
   else
     EINA_SAFETY_ON_TRUE_RETURN_VAL(svr->ssl.clients_ctx != NULL, EINA_FALSE);

   svr->ssl.privkeys = eina_list_append(svr->ssl.privkeys, eina_stringshare_add(privkey));
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_con_ssl_server_crl_add(Ecore_Con_Server *svr, const char *crl)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(crl, EINA_FALSE);

   if (!svr->server) /* SSL adds a job to allow setup */
     EINA_SAFETY_ON_NULL_RETURN_VAL(svr->ssl.job, EINA_FALSE);
   else
     EINA_SAFETY_ON_TRUE_RETURN_VAL(svr->ssl.clients_ctx != NULL, EINA_FALSE);

   svr->ssl.crls = eina_list_append(svr->ssl.crls, eina_stringshare_add(crl));
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_con_ssl_server_cafile_add(Ecore_Con_Server *svr, const char *cafile)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cafile, EINA_FALSE);

   if (!svr->server) /* SSL adds a job to allow setup */
     EINA_SAFETY_ON_NULL_RETURN_VAL(svr->ssl.job, EINA_FALSE);
   else
     EINA_SAFETY_ON_TRUE_RETURN_VAL(svr->ssl.clients_ctx != NULL, EINA_FALSE);

   svr->ssl.cafiles = eina_list_append(svr->ssl.cafiles, eina_stringshare_add(cafile));
   return EINA_TRUE;
}

EAPI void
ecore_con_ssl_server_verify(Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr);

   if (!svr->server) /* SSL adds a job to allow setup */
     EINA_SAFETY_ON_NULL_RETURN(svr->ssl.job);
   else
     EINA_SAFETY_ON_TRUE_RETURN(svr->ssl.clients_ctx != NULL);

   if (!svr->is_dialer)
     {
        /* legacy compatible + print a warning */
        WRN("svr=%p created with ecore_con_server_add()", svr);
        return;
     }

   svr->ssl.verify = EINA_TRUE;
}

EAPI void
ecore_con_ssl_server_verify_basic(Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr);
   EINA_SAFETY_ON_NULL_RETURN(svr->ssl.job);

   if (!svr->server) /* SSL adds a job to allow setup */
     EINA_SAFETY_ON_NULL_RETURN(svr->ssl.job);
   else
     EINA_SAFETY_ON_TRUE_RETURN(svr->ssl.clients_ctx != NULL);

   if (!svr->is_dialer)
     {
        /* legacy compatible + print a warning */
        WRN("svr=%p created with ecore_con_server_add()", svr);
        return;
     }

   svr->ssl.verify_basic = EINA_TRUE;
}

EAPI void
ecore_con_ssl_server_verify_name_set(Ecore_Con_Server *svr, const char *name)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr);
   EINA_SAFETY_ON_NULL_RETURN(name);

   if (!svr->server) /* SSL adds a job to allow setup */
     EINA_SAFETY_ON_NULL_RETURN(svr->ssl.job);
   else
     EINA_SAFETY_ON_TRUE_RETURN(svr->ssl.clients_ctx != NULL);

   eina_stringshare_replace(&svr->ssl.verify_name, name);
}

EAPI const char *
ecore_con_ssl_server_verify_name_get(Ecore_Con_Server *svr)
{
   ECORE_CON_SERVER_CHECK_RETURN(svr, NULL);
   return svr->ssl.verify_name ? svr->ssl.verify_name : svr->name;
}

EAPI Eina_Bool
ecore_con_ssl_server_upgrade(Ecore_Con_Server *svr, Ecore_Con_Type compl_type)
{
   double start;

   ECORE_CON_SERVER_CHECK_RETURN(svr, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(svr->dialer, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(svr->server != NULL, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(svr->ssl.upgrading, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((compl_type & ECORE_CON_SSL) == 0, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(efl_io_buffered_stream_inner_io_get(svr->dialer), EFL_NET_DIALER_TCP_CLASS), EINA_FALSE);

   start = ecore_time_get();
   while (efl_io_buffered_stream_pending_write_get(svr->dialer) && ((ecore_time_get() - start) <= ecore_animator_frametime_get()))
     ecore_con_server_flush(svr);
   if (efl_io_buffered_stream_pending_write_get(svr->dialer))
     {
        ERR("svr=%p still pending send %zd bytes! Flush server before upgrading to SSL!",
            svr, efl_io_buffered_stream_pending_write_get(svr->dialer));
        return EINA_FALSE;
     }

   svr->ssl.upgrading = EINA_TRUE;
   svr->ssl.upgrade_type = compl_type;

   _ecore_con_server_job_schedule(svr, efl_loop_get(svr->dialer),
                                  _ecore_con_server_dialer_ssl_upgrade_job);

   DBG("svr=%p SSL upgrading from %#x to type=%#x", svr, svr->type, compl_type);

   return EINA_TRUE;
}

/**
 * @}
 */

static void
_ecore_con_lookup_done_cb(void *data, const char *host, const char *port EINA_UNUSED, const struct addrinfo *hints EINA_UNUSED, struct addrinfo *result, int gai_error)
{
   Ecore_Con_Lookup_Ctx *ctx = data;

   ctx->thread = NULL;

   if (gai_error)
     WRN("Failed to lookup host='%s': %s", host, gai_strerror(gai_error));
   else if (result)
     {
        char ip[INET6_ADDRSTRLEN];
        const void *mem;

        if (result->ai_family == AF_INET)
          {
             const struct sockaddr_in *a = (const struct sockaddr_in *)result->ai_addr;
             mem = &a->sin_addr;
          }
        else if (result->ai_family == AF_INET6)
          {
             const struct sockaddr_in6 *a = (const struct sockaddr_in6 *)result->ai_addr;
             mem = &a->sin6_addr;
          }
        else
          {
             ERR("unexpected result->ai_family=%d", result->ai_family);
             goto end;
          }

        if (!inet_ntop(result->ai_family, mem, ip, sizeof(ip)))
          {
             ERR("could not convert IP to string: %s", strerror(errno));
             goto end;
          }
        ctx->cb(result->ai_canonname, ip, result->ai_addr, result->ai_addrlen, (void *)ctx->data);
     }

 end:
   freeaddrinfo(result);
   _ecore_con_lookups = eina_list_remove(_ecore_con_lookups, ctx);
   free(ctx);
}

/*
 * NOTE: this function has numerous problems:
 *  - not able to specify family (IPv4 or IPv6 or both).
 *  - callback reports a single result
 *  - doesn't return a handle to cancel (likely to access memory after free)
 *  - doesn't report errors
 */
EAPI Eina_Bool
ecore_con_lookup(const char *name, Ecore_Con_Dns_Cb done_cb, const void *data)
{
   Ecore_Con_Lookup_Ctx *ctx;
   struct addrinfo hints = {
     .ai_family = AF_UNSPEC,
     .ai_flags = AI_ADDRCONFIG | AI_V4MAPPED | AI_CANONNAME,
   };

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(done_cb, EINA_FALSE);

   ctx = malloc(sizeof(Ecore_Con_Lookup_Ctx));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, EINA_FALSE);
   ctx->cb = done_cb;
   ctx->data = data;

   ctx->thread = efl_net_ip_resolve_async_new(name, "0", &hints, _ecore_con_lookup_done_cb, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->thread, error);

   _ecore_con_lookups = eina_list_append(_ecore_con_lookups, ctx);
   return EINA_TRUE;

 error:
   free(ctx);
   return EINA_FALSE;
}
