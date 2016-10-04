#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <sys/types.h>

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif

#include <Ecore.h>
#include <ecore_private.h>
#include <Ecore_Con.h>

#include "Ecore_Ipc.h"
#include "ecore_ipc_private.h"

#define DLT_ZERO   0
#define DLT_ONE    1
#define DLT_SAME   2
#define DLT_SHL    3
#define DLT_SHR    4
#define DLT_ADD8   5
#define DLT_DEL8   6
#define DLT_ADDU8  7
#define DLT_DELU8  8
#define DLT_ADD16  9
#define DLT_DEL16  10
#define DLT_ADDU16 11
#define DLT_DELU16 12
#define DLT_SET    13
#define DLT_R1     14
#define DLT_R2     15

int _ecore_ipc_log_dom = -1;

/****** This swap function are around just for backward compatibility do not remove *******/
EAPI unsigned short
_ecore_ipc_swap_16(unsigned short v)
{
   return eina_swap16(v);
}

EAPI unsigned int
_ecore_ipc_swap_32(unsigned int v)
{
   return eina_swap32(v);
}

EAPI unsigned long long
_ecore_ipc_swap_64(unsigned long long v)
{
   return eina_swap64(v);
}
/***********************/

static int _ecore_ipc_dlt_int(int out, int prev, int *mode);
static int _ecore_ipc_ddlt_int(int in, int prev, int mode);

static int
_ecore_ipc_dlt_int(int out, int prev, int *mode)
{
   int dlt;

   /* 0 byte */
   if (out == 0)
     {
        *mode = DLT_ZERO;
        return 0;
     }
   if (out == (int)0xffffffff)
     {
        *mode = DLT_ONE;
        return 0;
     }
   if (out == prev)
     {
        *mode = DLT_SAME;
        return 0;
     }
   if (out == prev << 1)
     {
        *mode = DLT_SHL;
        return 0;
     }
   if (out == prev >> 1)
     {
        *mode = DLT_SHR;
        return 0;
     }
   /* 1 byte */
   dlt = out - prev;
   if (!(dlt & 0xffffff00))
     {
        *mode = DLT_ADD8;
        return dlt & 0xff;
     }
   dlt = prev - out;
   if (!(dlt & 0xffffff00))
     {
        *mode = DLT_DEL8;
        return dlt & 0xff;
     }
   dlt = out - prev;
   if (!(dlt & 0x00ffffff))
     {
        *mode = DLT_ADDU8;
        return (dlt >> 24) & 0xff;
     }
   dlt = prev - out;
   if (!(dlt & 0x00ffffff))
     {
        *mode = DLT_DELU8;
        return (dlt >> 24) & 0xff;
     }
   /* 2 byte */
   dlt = out - prev;
   if (!(dlt & 0xffff0000))
     {
        *mode = DLT_ADD16;
        return dlt & 0xffff;
     }
   dlt = prev - out;
   if (!(dlt & 0xffff0000))
     {
        *mode = DLT_DEL16;
        return dlt & 0xffff;
     }
   dlt = out - prev;
   if (!(dlt & 0x0000ffff))
     {
        *mode = DLT_ADDU16;
        return (dlt >> 16) & 0xffff;
     }
   dlt = prev - out;
   if (!(dlt & 0x0000ffff))
     {
        *mode = DLT_DELU16;
        return (dlt >> 16) & 0xffff;
     }
   /* 4 byte */
   *mode = DLT_SET;
   return out;
}

static int
_ecore_ipc_ddlt_int(int in, int prev, int mode)
{
   switch (mode)
     {
      case DLT_ZERO:
        return 0;
        break;
      case DLT_ONE:
        return 0xffffffff;
        break;
      case DLT_SAME:
        return prev;
        break;
      case DLT_SHL:
        return prev << 1;
        break;
      case DLT_SHR:
        return prev >> 1;
        break;
      case DLT_ADD8:
        return prev + in;
        break;
      case DLT_DEL8:
        return prev - in;
        break;
      case DLT_ADDU8:
        return prev + (in << 24);
        break;
      case DLT_DELU8:
        return prev - (in << 24);
        break;
      case DLT_ADD16:
        return prev + in;
        break;
      case DLT_DEL16:
        return prev - in;
        break;
      case DLT_ADDU16:
        return prev + (in << 16);
        break;
      case DLT_DELU16:
        return prev - (in << 16);
        break;
      case DLT_SET:
        return in;
        break;
      case DLT_R1:
        return 0;
        break;
      case DLT_R2:
        return 0;
        break;
      default:
        break;
     }
   return 0;
}

static Eina_Bool _ecore_ipc_event_client_add(void *data, int ev_type, void *ev);
static Eina_Bool _ecore_ipc_event_client_del(void *data, int ev_type, void *ev);
static Eina_Bool _ecore_ipc_event_server_add(void *data, int ev_type, void *ev);
static Eina_Bool _ecore_ipc_event_server_del(void *data, int ev_type, void *ev);
static Eina_Bool _ecore_ipc_event_client_data(void *data, int ev_type, void *ev);
static Eina_Bool _ecore_ipc_event_server_data(void *data, int ev_type, void *ev);
static void _ecore_ipc_event_client_add_free(void *data, void *ev);
static void _ecore_ipc_event_client_del_free(void *data, void *ev);
static void _ecore_ipc_event_client_data_free(void *data, void *ev);
static void _ecore_ipc_event_server_add_free(void *data, void *ev);
static void _ecore_ipc_event_server_del_free(void *data, void *ev);
static void _ecore_ipc_event_server_data_free(void *data, void *ev);

EAPI int ECORE_IPC_EVENT_CLIENT_ADD = 0;
EAPI int ECORE_IPC_EVENT_CLIENT_DEL = 0;
EAPI int ECORE_IPC_EVENT_SERVER_ADD = 0;
EAPI int ECORE_IPC_EVENT_SERVER_DEL = 0;
EAPI int ECORE_IPC_EVENT_CLIENT_DATA = 0;
EAPI int ECORE_IPC_EVENT_SERVER_DATA = 0;

static int                  _ecore_ipc_init_count = 0;
static Eina_List           *servers = NULL;
static Ecore_Event_Handler *handler[6];

EAPI int
ecore_ipc_init(void)
{
   int i = 0;

   if (++_ecore_ipc_init_count != 1)
     return _ecore_ipc_init_count;
   _ecore_ipc_log_dom = eina_log_domain_register
     ("ecore_ipc", ECORE_IPC_DEFAULT_LOG_COLOR);
   if(_ecore_ipc_log_dom < 0)
     {
       EINA_LOG_ERR("Impossible to create a log domain for the Ecore IPC module.");
       return --_ecore_ipc_init_count;
     }
   if (!ecore_con_init())
     return --_ecore_ipc_init_count;

   ECORE_IPC_EVENT_CLIENT_ADD = ecore_event_type_new();
   ECORE_IPC_EVENT_CLIENT_DEL = ecore_event_type_new();
   ECORE_IPC_EVENT_SERVER_ADD = ecore_event_type_new();
   ECORE_IPC_EVENT_SERVER_DEL = ecore_event_type_new();
   ECORE_IPC_EVENT_CLIENT_DATA = ecore_event_type_new();
   ECORE_IPC_EVENT_SERVER_DATA = ecore_event_type_new();

   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,
                                          _ecore_ipc_event_client_add, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,
                                          _ecore_ipc_event_client_del, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
                                          _ecore_ipc_event_server_add, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,
                                          _ecore_ipc_event_server_del, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
                                          _ecore_ipc_event_client_data, NULL);
   handler[i] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
                                          _ecore_ipc_event_server_data, NULL);
   return _ecore_ipc_init_count;
}

EAPI int
ecore_ipc_shutdown(void)
{
   int i;

   if (--_ecore_ipc_init_count != 0)
     return _ecore_ipc_init_count;

   Eina_List *l, *l2;
   Ecore_Ipc_Server *svr;
   EINA_LIST_FOREACH_SAFE(servers, l, l2, svr)
     ecore_ipc_server_del(svr);

   for (i = 0; i < 6; i++)
     ecore_event_handler_del(handler[i]);

   ecore_con_shutdown();
   eina_log_domain_unregister(_ecore_ipc_log_dom);
   _ecore_ipc_log_dom = -1;
   return _ecore_ipc_init_count;
}

/* FIXME: need to add protocol type parameter */
EAPI Ecore_Ipc_Server *
ecore_ipc_server_add(Ecore_Ipc_Type compl_type, const char *name, int port, const void *data)
{
   Ecore_Ipc_Server *svr;
   Ecore_Ipc_Type type;
   Ecore_Con_Type extra = 0;

   if (!name) return NULL;

   svr = calloc(1, sizeof(Ecore_Ipc_Server));
   if (!svr) return NULL;
   type = compl_type;
   type &= ~ECORE_IPC_USE_SSL;
   if (compl_type & ECORE_IPC_USE_SSL) extra = ECORE_CON_USE_SSL;
   switch (type)
     {
      case ECORE_IPC_LOCAL_USER:
        svr->server = ecore_con_server_add(ECORE_CON_LOCAL_USER | extra, name, port, svr);
        break;
      case ECORE_IPC_LOCAL_SYSTEM:
        svr->server = ecore_con_server_add(ECORE_CON_LOCAL_SYSTEM | extra, name, port, svr);
        break;
      case ECORE_IPC_REMOTE_SYSTEM:
        svr->server = ecore_con_server_add(ECORE_CON_REMOTE_SYSTEM | extra, name, port, svr);
        break;
      default:
        free(svr);
        return NULL;
     }
   if (!svr->server)
     {
        free(svr);
        return NULL;
     }
   svr->max_buf_size = 32 * 1024;
   svr->data = (void *)data;
   servers = eina_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_IPC_SERVER);
   return svr;
}

/* FIXME: need to add protocol type parameter */
EAPI Ecore_Ipc_Server *
ecore_ipc_server_connect(Ecore_Ipc_Type compl_type, char *name, int port, const void *data)
{
   Ecore_Ipc_Server *svr;
   Ecore_Ipc_Type type;
   Ecore_Con_Type extra = 0;
   int features;

   svr = calloc(1, sizeof(Ecore_Ipc_Server));
   if (!svr) return NULL;
   type = compl_type & ECORE_IPC_TYPE;
   features = compl_type & ECORE_IPC_SSL;
   if ((features & ECORE_IPC_USE_SSL) == ECORE_IPC_USE_SSL)
     extra |= ECORE_CON_USE_SSL;
   if ((features & ECORE_IPC_NO_PROXY) == ECORE_IPC_NO_PROXY)
     extra |= ECORE_CON_NO_PROXY;
   switch (type)
     {
      case ECORE_IPC_LOCAL_USER:
        svr->server = ecore_con_server_connect(ECORE_CON_LOCAL_USER | extra, name, port, svr);
        break;
      case ECORE_IPC_LOCAL_SYSTEM:
        svr->server = ecore_con_server_connect(ECORE_CON_LOCAL_SYSTEM | extra, name, port, svr);
        break;
      case ECORE_IPC_REMOTE_SYSTEM:
        svr->server = ecore_con_server_connect(ECORE_CON_REMOTE_SYSTEM | extra, name, port, svr);
        break;
      default:
        free(svr);
        return NULL;
     }
   if (!svr->server)
     {
        free(svr);
        return NULL;
     }
   svr->max_buf_size = -1;
   svr->data = (void *)data;
   servers = eina_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_IPC_SERVER);
   return svr;
}

EAPI void *
ecore_ipc_server_del(Ecore_Ipc_Server *svr)
{
   void *data;

   if (!svr) return NULL;
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
                         "ecore_ipc_server_del");
        return NULL;
     }
   if (svr->delete_me) return NULL;

   data = svr->data;
   svr->data = NULL;
   svr->delete_me = 1;
   if (svr->event_count == 0)
     {
        Ecore_Ipc_Client *cl;

        EINA_LIST_FREE(svr->clients, cl)
          {
             cl->svr = NULL;
             ecore_ipc_client_del(cl);
          }
        if (svr->server) ecore_con_server_del(svr->server);
        servers = eina_list_remove(servers, svr);

        if (svr->buf) free(svr->buf);
        ECORE_MAGIC_SET(svr, ECORE_MAGIC_NONE);
        free(svr);
     }
   return data;
}

EAPI void *
ecore_ipc_server_data_get(Ecore_Ipc_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
                         "ecore_ipc_server_data_get");
        return NULL;
     }
   return svr->data;
}

EAPI Eina_Bool
ecore_ipc_server_connected_get(Ecore_Ipc_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
                         "ecore_ipc_server_connected_get");
        return EINA_FALSE;
     }
   return ecore_con_server_connected_get(svr->server);
}

EAPI Eina_List *
ecore_ipc_server_clients_get(Ecore_Ipc_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
                         "ecore_ipc_server_clients_get");
        return NULL;
     }
   return svr->clients;
}

#define SVENC(_member) \
   d = _ecore_ipc_dlt_int(msg._member, svr->prev.o._member, &md); \
   if (md >= DLT_SET) \
     { \
        unsigned int v; \
        unsigned char *dd; \
        dd = (unsigned char *)&v; \
        v = d; \
        v = htonl(v); \
        *(dat + s + 0) = dd[0]; \
        *(dat + s + 1) = dd[1]; \
        *(dat + s + 2) = dd[2]; \
        *(dat + s + 3) = dd[3]; \
        s += 4; \
     } \
   else if (md >= DLT_ADD16) \
     { \
        unsigned short v; \
        unsigned char *dd; \
        dd = (unsigned char *)&v; \
        v = d; \
        v = htons(v); \
        *(dat + s + 0) = dd[0]; \
        *(dat + s + 1) = dd[1]; \
        s += 2; \
     } \
   else if (md >= DLT_ADD8) \
     { \
        *(dat + s + 0) = (unsigned char)d; \
        s += 1; \
     }

/* FIXME: this needs to become an ipc message */
EAPI int
ecore_ipc_server_send(Ecore_Ipc_Server *svr, int major, int minor, int ref, int ref_to, int response, const void *data, int size)
{
   Ecore_Ipc_Msg_Head msg;
   int ret;
   int *head, md = 0, d, s;
   unsigned char dat[sizeof(Ecore_Ipc_Msg_Head)];

   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
                         "ecore_ipc_server_send");
        return 0;
     }
   if (size < 0) size = 0;
   msg.major    = major;
   msg.minor    = minor;
   msg.ref      = ref;
   msg.ref_to   = ref_to;
   msg.response = response;
   msg.size     = size;
   head = (int *)dat;
   s = 4;
   SVENC(major);
   *head = md;
   SVENC(minor);
   *head |= md << (4 * 1);
   SVENC(ref);
   *head |= md << (4 * 2);
   SVENC(ref_to);
   *head |= md << (4 * 3);
   SVENC(response);
   *head |= md << (4 * 4);
   SVENC(size);
   *head |= md << (4 * 5);
   *head = htonl(*head);
   svr->prev.o = msg;
   ret = ecore_con_server_send(svr->server, dat, s);
   if (size > 0) ret += ecore_con_server_send(svr->server, data, size);
   return ret;
}

EAPI void
ecore_ipc_server_client_limit_set(Ecore_Ipc_Server *svr, int client_limit, char reject_excess_clients)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
                         "ecore_ipc_server_client_limit_set");
        return;
     }
   ecore_con_server_client_limit_set(svr->server, client_limit, reject_excess_clients);
}

EAPI void
ecore_ipc_server_data_size_max_set(Ecore_Ipc_Server *svr, int size)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
                         "ecore_ipc_server_data_size_max_set");
        return;
     }
   svr->max_buf_size = size;
}

EAPI int
ecore_ipc_server_data_size_max_get(Ecore_Ipc_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
                         "ecore_ipc_server_data_size_max_get");
        return -1;
     }
   return svr->max_buf_size;
}

EAPI const char *
ecore_ipc_server_ip_get(Ecore_Ipc_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
                         "ecore_ipc_server_ip_get");
        return NULL;
     }
   return ecore_con_server_ip_get(svr->server);
}

EAPI void
ecore_ipc_server_flush(Ecore_Ipc_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
                         "ecore_ipc_server_server_flush");
        return;
     }
   ecore_con_server_flush(svr->server);
}

#define CLENC(_member) \
   d = _ecore_ipc_dlt_int(msg._member, cl->prev.o._member, &md); \
   if (md >= DLT_SET) \
     { \
        unsigned int v; \
        unsigned char *dd; \
        dd = (unsigned char *)&v; \
        v = d; \
        v = htonl(v); \
        *(dat + s + 0) = dd[0]; \
        *(dat + s + 1) = dd[1]; \
        *(dat + s + 2) = dd[2]; \
        *(dat + s + 3) = dd[3]; \
        s += 4; \
     } \
   else if (md >= DLT_ADD16) \
     { \
        unsigned short v; \
        unsigned char *dd; \
        dd = (unsigned char *)&v; \
        v = d; \
        v = htons(v); \
        *(dat + s + 0) = dd[0]; \
        *(dat + s + 1) = dd[1]; \
        s += 2; \
     } \
   else if (md >= DLT_ADD8) \
     { \
        *(dat + s) = (unsigned char)d; \
        s += 1; \
     }

/* FIXME: this needs to become an ipc message */
EAPI int
ecore_ipc_client_send(Ecore_Ipc_Client *cl, int major, int minor, int ref, int ref_to, int response, const void *data, int size)
{
   Ecore_Ipc_Msg_Head msg;
   int ret;
   int *head, md = 0, d, s;
   unsigned char dat[sizeof(Ecore_Ipc_Msg_Head)];

   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
                         "ecore_ipc_client_send");
        return 0;
     }
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!cl->client, 0);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(!ecore_con_client_connected_get(cl->client), 0);
   if (size < 0) size = 0;
   msg.major    = major;
   msg.minor    = minor;
   msg.ref      = ref;
   msg.ref_to   = ref_to;
   msg.response = response;
   msg.size     = size;
   head = (int *)dat;
   s = 4;
   CLENC(major);
   *head = md;
   CLENC(minor);
   *head |= md << (4 * 1);
   CLENC(ref);
   *head |= md << (4 * 2);
   CLENC(ref_to);
   *head |= md << (4 * 3);
   CLENC(response);
   *head |= md << (4 * 4);
   CLENC(size);
   *head |= md << (4 * 5);
   *head = htonl(*head);
   cl->prev.o = msg;
   ret = ecore_con_client_send(cl->client, dat, s);
   if (size > 0) ret += ecore_con_client_send(cl->client, data, size);
   return ret;
}

EAPI Ecore_Ipc_Server *
ecore_ipc_client_server_get(Ecore_Ipc_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
                         "ecore_ipc_client_server_get");
        return NULL;
     }
   return cl->svr;
}

EAPI void *
ecore_ipc_client_del(Ecore_Ipc_Client *cl)
{
   void *data;
   Ecore_Ipc_Server *svr;

   if (!cl) return NULL;
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
                         "ecore_ipc_client_del");
        return NULL;
     }
   data = cl->data;
   cl->data = NULL;
   cl->delete_me = 1;
   if (cl->event_count == 0)
     {
        svr = cl->svr;
        if (cl->client) ecore_con_client_del(cl->client);
        if (ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
          svr->clients = eina_list_remove(svr->clients, cl);
        if (cl->buf) free(cl->buf);
        ECORE_MAGIC_SET(cl, ECORE_MAGIC_NONE);
        free(cl);
     }
   return data;
}

EAPI void
ecore_ipc_client_data_set(Ecore_Ipc_Client *cl, const void *data)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
                         "ecore_ipc_client_data_set");
        return;
     }
   cl->data = (void *)data;
}

EAPI void *
ecore_ipc_client_data_get(Ecore_Ipc_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
                         "ecore_ipc_client_data_get");
        return NULL;
     }
   return cl->data;
}

EAPI void
ecore_ipc_client_data_size_max_set(Ecore_Ipc_Client *cl, int size)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
                         "ecore_ipc_client_data_size_max_set");
        return;
     }
   cl->max_buf_size = size;
}

EAPI int
ecore_ipc_client_data_size_max_get(Ecore_Ipc_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
                         "ecore_ipc_client_data_size_max_get");
        return -1;
     }
   return cl->max_buf_size;
}

EAPI const char *
ecore_ipc_client_ip_get(Ecore_Ipc_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
                         "ecore_ipc_client_ip_get");
        return NULL;
     }
   return ecore_con_client_ip_get(cl->client);
}

EAPI void
ecore_ipc_client_flush(Ecore_Ipc_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
                         "ecore_ipc_client_flush");
        return;
     }
   ecore_con_client_flush(cl->client);
}

EAPI int
ecore_ipc_ssl_available_get(void)
{
   return ecore_con_ssl_available_get();
}


static Eina_Bool
_ecore_ipc_event_client_add(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev)
{
   Ecore_Con_Event_Client_Add *e;
   Ecore_Ipc_Server *svr;

   e = ev;
   svr = ecore_con_server_data_get(ecore_con_client_server_get(e->client));
   if (!eina_list_data_find(servers, svr)) return ECORE_CALLBACK_RENEW;
   /* handling code here */
     {
        Ecore_Ipc_Client *cl;

        cl = calloc(1, sizeof(Ecore_Ipc_Client));
        if (!cl) return ECORE_CALLBACK_CANCEL;
        cl->svr = svr;
        ECORE_MAGIC_SET(cl, ECORE_MAGIC_IPC_CLIENT);
        cl->client = e->client;
        cl->max_buf_size = 32 * 1024;
        ecore_con_client_data_set(cl->client, (void *)cl);
        svr->clients = eina_list_append(svr->clients, cl);
        if (!cl->delete_me)
          {
             Ecore_Ipc_Event_Client_Add *e2;

             e2 = calloc(1, sizeof(Ecore_Ipc_Event_Client_Add));
             if (e2)
               {
                  cl->event_count++;
                  e2->client = cl;
                  ecore_event_add(ECORE_IPC_EVENT_CLIENT_ADD, e2,
                                  _ecore_ipc_event_client_add_free, NULL);
               }
          }
     }
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_ecore_ipc_event_client_del(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev)
{
   Ecore_Con_Event_Client_Del *e;
   Ecore_Ipc_Server *svr;

   e = ev;
   if (!e->client) return ECORE_CALLBACK_RENEW;
   svr = ecore_con_server_data_get(ecore_con_client_server_get(e->client));
   if (!eina_list_data_find(servers, svr)) return ECORE_CALLBACK_RENEW;
   /* handling code here */
     {
        Ecore_Ipc_Client *cl;

        cl = ecore_con_client_data_get(e->client);
        cl->client = NULL;
          {
             Ecore_Ipc_Event_Client_Del *e2;

             if (!cl->delete_me)
               {
                  e2 = calloc(1, sizeof(Ecore_Ipc_Event_Client_Del));
                  if (e2)
                    {
                       cl->event_count++;
                       e2->client = cl;
                       ecore_event_add(ECORE_IPC_EVENT_CLIENT_DEL, e2,
                                       _ecore_ipc_event_client_del_free, NULL);
                    }
               }
          }
     }
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_ecore_ipc_event_server_add(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev)
{
   Ecore_Con_Event_Server_Add *e;

   e = ev;
   if (!eina_list_data_find(servers, ecore_con_server_data_get(e->server))) return ECORE_CALLBACK_RENEW;
   /* handling code here */
     {
        Ecore_Ipc_Server *svr;

        svr = ecore_con_server_data_get(e->server);
        if (!svr->delete_me)
          {
             Ecore_Ipc_Event_Server_Add *e2;

             e2 = calloc(1, sizeof(Ecore_Ipc_Event_Server_Add));
             if (e2)
               {
                  svr->event_count++;
                  e2->server = svr;
                  ecore_event_add(ECORE_IPC_EVENT_SERVER_ADD, e2,
                                  _ecore_ipc_event_server_add_free, NULL);
               }
          }
     }
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_ecore_ipc_event_server_del(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev)
{
   Ecore_Con_Event_Server_Del *e;

   e = ev;
   if (!eina_list_data_find(servers, ecore_con_server_data_get(e->server))) return ECORE_CALLBACK_RENEW;
   /* handling code here */
     {
        Ecore_Ipc_Server *svr;

        svr = ecore_con_server_data_get(e->server);
        svr->server = NULL;
        if (!svr->delete_me)
          {
             Ecore_Ipc_Event_Server_Del *e2;

             e2 = calloc(1, sizeof(Ecore_Ipc_Event_Server_Del));
             if (e2)
               {
                  svr->event_count++;
                  e2->server = svr;
                  ecore_event_add(ECORE_IPC_EVENT_SERVER_DEL, e2,
                                  _ecore_ipc_event_server_del_free, NULL);
               }
          }
     }
   return ECORE_CALLBACK_CANCEL;
}

#define CLSZ(_n) \
   md = ((head >> (4 * _n)) & 0xf); \
   if (md >= DLT_SET) s += 4; \
   else if (md >= DLT_ADD16) s += 2; \
   else if (md >= DLT_ADD8) s += 1;

#define CLDEC(_n, _member) \
   md = ((head >> (4 * _n)) & 0xf); \
   if (md >= DLT_SET) \
     { \
        unsigned int v; \
        unsigned char *dv; \
        dv = (unsigned char *)&v; \
        dv[0] = *(cl->buf + offset + s + 0); \
        dv[1] = *(cl->buf + offset + s + 1); \
        dv[2] = *(cl->buf + offset + s + 2); \
        dv[3] = *(cl->buf + offset + s + 3); \
        d = (int)ntohl(v); \
        s += 4; \
     } \
   else if (md >= DLT_ADD16) \
     { \
        unsigned short v; \
        unsigned char *dv; \
        dv = (unsigned char *)&v; \
        dv[0] = *(cl->buf + offset + s + 0); \
        dv[1] = *(cl->buf + offset + s + 1); \
        d = (int)ntohs(v); \
        s += 2; \
     } \
   else if (md >= DLT_ADD8) \
     { \
        unsigned char v; \
        unsigned char *dv; \
        dv = (unsigned char *)&v; \
        dv[0] = *(cl->buf + offset + s + 0); \
        d = (int)v; \
        s += 1; \
     } \
   msg._member = _ecore_ipc_ddlt_int(d, cl->prev.i._member, md);

static Eina_Bool
_ecore_ipc_event_client_data(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev)
{
   Ecore_Con_Event_Client_Data *e;
   Ecore_Ipc_Server *svr;

   e = ev;
   svr = ecore_con_server_data_get(ecore_con_client_server_get(e->client));
   if (!eina_list_data_find(servers, svr)) return ECORE_CALLBACK_RENEW;
   /* handling code here */
     {
        Ecore_Ipc_Client *cl;
        Ecore_Ipc_Msg_Head msg;
        int offset = 0;
        unsigned char *buf;

        cl = ecore_con_client_data_get(e->client);

        if (!cl->buf)
          {
             cl->buf_size = e->size;
             cl->buf = e->data;
             e->data = NULL; /* take it out of the old event */
          }
        else
          {
             buf = realloc(cl->buf, cl->buf_size + e->size);
             if (!buf)
               {
                  free(cl->buf);
                  cl->buf = 0;
                  cl->buf_size  = 0;
                  return ECORE_CALLBACK_CANCEL;
               }
             cl->buf = buf;
             memcpy(cl->buf + cl->buf_size, e->data, e->size);
             cl->buf_size += e->size;
          }
        /* examine header */
        redo:
        if ((cl->buf_size - offset) >= (int)sizeof(int))
          {
             int s, md, d = 0, head;
             unsigned char *dd;

             dd = (unsigned char *)&head;
             dd[0] = *(cl->buf + offset + 0);
             dd[1] = *(cl->buf + offset + 1);
             dd[2] = *(cl->buf + offset + 2);
             dd[3] = *(cl->buf + offset + 3);
             head = ntohl(head);
             dd = (unsigned char *)&d;
             s = 4;
             CLSZ(0);
             CLSZ(1);
             CLSZ(2);
             CLSZ(3);
             CLSZ(4);
             CLSZ(5);
             if ((cl->buf_size - offset) < s)
               {
                  if (offset > 0) goto scroll;
                  return ECORE_CALLBACK_CANCEL;
               }

             s = 4;
             CLDEC(0, major);
             CLDEC(1, minor);
             CLDEC(2, ref);
             CLDEC(3, ref_to);
             CLDEC(4, response);
             CLDEC(5, size);
             if (msg.size < 0) msg.size = 0;
             /* there is enough data in the buffer for a full message */
             if ((cl->buf_size - offset) >= (s + msg.size))
               {
                  Ecore_Ipc_Event_Client_Data *e2;
                  int max, max2;

                  buf = NULL;
                  max = svr->max_buf_size;
                  max2 = cl->max_buf_size;
                  if ((max >= 0) && (max2 >= 0))
                    {
                       if (max2 < max) max = max2;
                    }
                  else
                    {
                       if (max < 0) max = max2;
                    }
                  if ((max < 0) || (msg.size <= max))
                    {
                       Eina_Bool need_free = EINA_FALSE;
                       if (msg.size > 0)
                         {
                            buf = malloc(msg.size);
                            if (!buf) return ECORE_CALLBACK_CANCEL;
                            memcpy(buf, cl->buf + offset + s, msg.size);
                            need_free = EINA_TRUE;
                         }
                       if (!cl->delete_me)
                         {
                            e2 = calloc(1, sizeof(Ecore_Ipc_Event_Client_Data));
                            if (e2)
                              {
                                 cl->event_count++;
                                 e2->client   = cl;
                                 e2->major    = msg.major;
                                 e2->minor    = msg.minor;
                                 e2->ref      = msg.ref;
                                 e2->ref_to   = msg.ref_to;
                                 e2->response = msg.response;
                                 e2->size     = msg.size;
                                 e2->data     = buf;
                                 ecore_event_add(ECORE_IPC_EVENT_CLIENT_DATA, e2,
                                                 _ecore_ipc_event_client_data_free,
                                                 NULL);
                                 need_free = EINA_FALSE;
                              }
                         }
                       if (need_free) free(buf);
                    }
                  cl->prev.i = msg;
                  offset += (s + msg.size);
                  if (cl->buf_size == offset)
                    {
                       free(cl->buf);
                       cl->buf = NULL;
                       cl->buf_size = 0;
                       return ECORE_CALLBACK_CANCEL;
                    }
                  goto redo;
               }
             else goto scroll;
          }
        else
          {
             scroll:
             buf = malloc(cl->buf_size - offset);
             if (!buf)
               {
                  free(cl->buf);
                  cl->buf = NULL;
                  cl->buf_size = 0;
                  return ECORE_CALLBACK_CANCEL;
               }
             memcpy(buf, cl->buf + offset, cl->buf_size - offset);
             free(cl->buf);
             cl->buf = buf;
             cl->buf_size -= offset;
          }
     }
   return ECORE_CALLBACK_CANCEL;
}

#define SVSZ(_n) \
   md = ((head >> (4 * _n)) & 0xf); \
   if (md >= DLT_SET) s += 4; \
   else if (md >= DLT_ADD16) s += 2; \
   else if (md >= DLT_ADD8) s += 1;

#define SVDEC(_n, _member) \
   md = ((head >> (4 * _n)) & 0xf); \
   if (md >= DLT_SET) \
     { \
        unsigned int v; \
        unsigned char *dv; \
        dv = (unsigned char *)&v; \
        dv[0] = *(svr->buf + offset + s + 0); \
        dv[1] = *(svr->buf + offset + s + 1); \
        dv[2] = *(svr->buf + offset + s + 2); \
        dv[3] = *(svr->buf + offset + s + 3); \
        d = (int)ntohl(v); \
        s += 4; \
     } \
   else if (md >= DLT_ADD16) \
     { \
        unsigned short v; \
        unsigned char *dv; \
        dv = (unsigned char *)&v; \
        dv[0] = *(svr->buf + offset + s + 0); \
        dv[1] = *(svr->buf + offset + s + 1); \
        d = (int)ntohs(v); \
        s += 2; \
     } \
   else if (md >= DLT_ADD8) \
     { \
        unsigned char v; \
        unsigned char *dv; \
        dv = (unsigned char *)&v; \
        dv[0] = *(svr->buf + offset + s + 0); \
        d = (int)v; \
        s += 1; \
     } \
   msg._member = _ecore_ipc_ddlt_int(d, svr->prev.i._member, md);

static Eina_Bool
_ecore_ipc_event_server_data(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev)
{
   Ecore_Con_Event_Server_Data *e;

   e = ev;
   if (!eina_list_data_find(servers, ecore_con_server_data_get(e->server))) 
     return ECORE_CALLBACK_RENEW;
   /* handling code here */
     {
        Ecore_Ipc_Server *svr;
        Ecore_Ipc_Msg_Head msg;
        int offset = 0;
        unsigned char *buf = NULL;

        svr = ecore_con_server_data_get(e->server);

        if (!svr->buf)
          {
             svr->buf_size = e->size;
             svr->buf = e->data;
             e->data = NULL; /* take it out of the old event */
          }
        else
          {
             buf = realloc(svr->buf, svr->buf_size + e->size);
             if (!buf)
               {
                  free(svr->buf);
                  svr->buf = 0;
                  svr->buf_size  = 0;
                  return ECORE_CALLBACK_CANCEL;
               }
             svr->buf = buf;
             memcpy(svr->buf + svr->buf_size, e->data, e->size);
             svr->buf_size += e->size;
          }
        /* examine header */
        redo:
        if ((svr->buf_size - offset) >= (int)sizeof(int))
          {
             int s, md, d = 0, head;
             unsigned char *dd;

             dd = (unsigned char *)&head;
             dd[0] = *(svr->buf + offset + 0);
             dd[1] = *(svr->buf + offset + 1);
             dd[2] = *(svr->buf + offset + 2);
             dd[3] = *(svr->buf + offset + 3);
             head = ntohl(head);
             dd = (unsigned char *)&d;
             s = 4;
             SVSZ(0);
             SVSZ(1);
             SVSZ(2);
             SVSZ(3);
             SVSZ(4);
             SVSZ(5);
             if ((svr->buf_size - offset) < s)
               {
                  if (offset > 0) goto scroll;
                  return ECORE_CALLBACK_CANCEL;
               }

             s = 4;
             SVDEC(0, major);
             SVDEC(1, minor);
             SVDEC(2, ref);
             SVDEC(3, ref_to);
             SVDEC(4, response);
             SVDEC(5, size);
             if (msg.size < 0) msg.size = 0;
             /* there is enough data in the buffer for a full message */
             if ((svr->buf_size - offset) >= (s + msg.size))
               {
                  Ecore_Ipc_Event_Server_Data *e2;
                  int max;

                  if (buf != svr->buf) free(buf);
                  buf = NULL;
                  max = svr->max_buf_size;
                  if ((max < 0) || (msg.size <= max))
                    {
                       if (msg.size > 0)
                         {
                            buf = malloc(msg.size);
                            if (!buf) return ECORE_CALLBACK_CANCEL;
                            memcpy(buf, svr->buf + offset + s, msg.size);
                         }
                       if (!svr->delete_me)
                         {
                            e2 = calloc(1, sizeof(Ecore_Ipc_Event_Server_Data));
                            if (e2)
                              {
                                 svr->event_count++;
                                 e2->server   = svr;
                                 e2->major    = msg.major;
                                 e2->minor    = msg.minor;
                                 e2->ref      = msg.ref;
                                 e2->ref_to   = msg.ref_to;
                                 e2->response = msg.response;
                                 e2->size     = msg.size;
                                 e2->data     = buf;
                                 if (buf == svr->buf)
                                   {
                                      svr->buf = NULL;
                                      svr->buf_size = 0;
                                   }
                                 buf = NULL;
                                 ecore_event_add(ECORE_IPC_EVENT_SERVER_DATA, e2,
                                                 _ecore_ipc_event_server_data_free,
                                                 NULL);
                              }
                            else
                              {
                                 free(buf);
                                 buf = NULL;
                              }
                         }
                       else
                         {
                            free(buf);
                            buf = NULL;
                         }
                    }
                  svr->prev.i = msg;
                  offset += (s + msg.size);
                  if ((svr->buf_size == offset) && (svr->buf))
                    {
                       if (svr->buf) free(svr->buf);
                       svr->buf = NULL;
                       svr->buf_size = 0;
                       return ECORE_CALLBACK_CANCEL;
                    }
                  goto redo;
               }
             else goto scroll;
          }
        else
          {
             scroll:
             if (buf != svr->buf) free(buf);
             buf = malloc(svr->buf_size - offset);
             if (!buf)
               {
                  free(svr->buf);
                  svr->buf = NULL;
                  svr->buf_size = 0;
                  return ECORE_CALLBACK_CANCEL;
               }
             memcpy(buf, svr->buf + offset, svr->buf_size - offset);
             free(svr->buf);
             svr->buf = buf;
             svr->buf_size -= offset;
          }
     }
   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_ipc_event_client_add_free(void *data EINA_UNUSED, void *ev)
{
   Ecore_Ipc_Event_Client_Add *e;

   e = ev;
   e->client->event_count--;
   if ((e->client->event_count == 0) && (e->client->delete_me))
     ecore_ipc_client_del(e->client);
   free(e);
}

static void
_ecore_ipc_event_client_del_free(void *data EINA_UNUSED, void *ev)
{
   Ecore_Ipc_Event_Client_Del *e;

   e = ev;
   e->client->event_count--;
   if ((e->client->event_count == 0) && (e->client->delete_me))
     ecore_ipc_client_del(e->client);
   free(e);
}

static void
_ecore_ipc_event_client_data_free(void *data EINA_UNUSED, void *ev)
{
   Ecore_Ipc_Event_Client_Data *e;

   e = ev;
   e->client->event_count--;
   if (e->data) free(e->data);
   if ((e->client->event_count == 0) && (e->client->delete_me))
     ecore_ipc_client_del(e->client);
   free(e);
}

static void
_ecore_ipc_event_server_add_free(void *data EINA_UNUSED, void *ev)
{
   Ecore_Ipc_Event_Server_Add *e;

   e = ev;
   e->server->event_count--;
   if ((e->server->event_count == 0) && (e->server->delete_me))
     ecore_ipc_server_del(e->server);
   free(e);
}

static void
_ecore_ipc_event_server_del_free(void *data EINA_UNUSED, void *ev)
{
   Ecore_Ipc_Event_Server_Add *e;

   e = ev;
   e->server->event_count--;
   if ((e->server->event_count == 0) && (e->server->delete_me))
     ecore_ipc_server_del(e->server);
   free(e);
}

static void
_ecore_ipc_event_server_data_free(void *data EINA_UNUSED, void *ev)
{
   Ecore_Ipc_Event_Server_Data *e;

   e = ev;
   if (e->data) free(e->data);
   e->server->event_count--;
   if ((e->server->event_count == 0) && (e->server->delete_me))
     ecore_ipc_server_del(e->server);
   free(e);
}
