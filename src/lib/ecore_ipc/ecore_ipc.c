#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
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

static int _ecore_ipc_log_dom = -1;

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

/* EFL_NET_SERVER_UNIX_CLASS and EFL_NET_DIALER_UNIX_CLASS should be defined at the same time, we're only checking for EFL_NET_SERVER_UNIX_CLASS in shared blocks */

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

static void
ecore_ipc_post_event_server_add(Ecore_Ipc_Server *svr)
{
   Ecore_Ipc_Event_Server_Add *ev;

   if (svr->delete_me) return;

   ev = calloc(1, sizeof(Ecore_Ipc_Event_Server_Add));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   svr->event_count++;
   ev->server = svr;
   ecore_event_add(ECORE_IPC_EVENT_SERVER_ADD, ev,
                   _ecore_ipc_event_server_add_free, NULL);
}

static void
ecore_ipc_post_event_server_del(Ecore_Ipc_Server *svr)
{
   Ecore_Ipc_Event_Server_Del *ev;

   if (svr->delete_me) return;

   ev = calloc(1, sizeof(Ecore_Ipc_Event_Server_Del));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   svr->event_count++;
   ev->server = svr;
   ecore_event_add(ECORE_IPC_EVENT_SERVER_DEL, ev,
                   _ecore_ipc_event_server_del_free, NULL);
}

static void
ecore_ipc_post_event_client_add(Ecore_Ipc_Client *cl)
{
   Ecore_Ipc_Event_Client_Add *ev;

   if (cl->delete_me) return;

   ev = calloc(1, sizeof(Ecore_Ipc_Event_Client_Add));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   cl->event_count++;
   ev->client = cl;
   ecore_event_add(ECORE_IPC_EVENT_CLIENT_ADD, ev,
                   _ecore_ipc_event_client_add_free, NULL);
}

static void
ecore_ipc_post_event_client_del(Ecore_Ipc_Client *cl)
{
   Ecore_Ipc_Event_Client_Del *ev;

   if (cl->delete_me) return;

   ev = calloc(1, sizeof(Ecore_Ipc_Event_Client_Del));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   cl->event_count++;
   ev->client = cl;
   ecore_event_add(ECORE_IPC_EVENT_CLIENT_DEL, ev,
                   _ecore_ipc_event_client_del_free, NULL);
}

static Ecore_Ipc_Client *
ecore_ipc_client_add(Ecore_Ipc_Server *svr)
{
   Ecore_Ipc_Client *cl;

   cl = calloc(1, sizeof(Ecore_Ipc_Client));
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, NULL);
   cl->svr = svr;
   cl->max_buf_size = 32 * 1024;

   ECORE_MAGIC_SET(cl, ECORE_MAGIC_IPC_CLIENT);
   svr->clients = eina_list_append(svr->clients, cl);

   return cl;
}

EAPI int
ecore_ipc_init(void)
{
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
     {
        eina_log_domain_unregister(_ecore_ipc_log_dom);
        _ecore_ipc_log_dom = -1;
        return --_ecore_ipc_init_count;
     }

   ECORE_IPC_EVENT_CLIENT_ADD = ecore_event_type_new();
   ECORE_IPC_EVENT_CLIENT_DEL = ecore_event_type_new();
   ECORE_IPC_EVENT_SERVER_ADD = ecore_event_type_new();
   ECORE_IPC_EVENT_SERVER_DEL = ecore_event_type_new();
   ECORE_IPC_EVENT_CLIENT_DATA = ecore_event_type_new();
   ECORE_IPC_EVENT_SERVER_DATA = ecore_event_type_new();

   return _ecore_ipc_init_count;
}

EAPI int
ecore_ipc_shutdown(void)
{
   if (--_ecore_ipc_init_count != 0)
     return _ecore_ipc_init_count;

   Eina_List *l, *l2;
   Ecore_Ipc_Server *svr;
   EINA_LIST_FOREACH_SAFE(servers, l, l2, svr)
     ecore_ipc_server_del(svr);

   ecore_event_type_flush(ECORE_IPC_EVENT_CLIENT_ADD,
                          ECORE_IPC_EVENT_CLIENT_DEL,
                          ECORE_IPC_EVENT_SERVER_ADD,
                          ECORE_IPC_EVENT_SERVER_DEL,
                          ECORE_IPC_EVENT_CLIENT_DATA,
                          ECORE_IPC_EVENT_SERVER_DATA);

   ecore_con_shutdown();

   eina_log_domain_unregister(_ecore_ipc_log_dom);
   _ecore_ipc_log_dom = -1;

   return _ecore_ipc_init_count;
}

static void
_ecore_ipc_server_del(Ecore_Ipc_Server *svr)
{
   DBG("server %p del", svr);

   if (svr->server)
     {
        efl_del(svr->server);
        svr->server = NULL;
     }
}

static void _ecore_ipc_server_client_add(void *data, const Efl_Event *event);

EFL_CALLBACKS_ARRAY_DEFINE(_ecore_ipc_server_cbs,
                           { EFL_NET_SERVER_EVENT_CLIENT_ADD, _ecore_ipc_server_client_add });

/* FIXME: need to add protocol type parameter */
EAPI Ecore_Ipc_Server *
ecore_ipc_server_add(Ecore_Ipc_Type type, const char *name, int port, const void *data)
{
   Ecore_Ipc_Server *svr;
   Eo *loop = efl_main_loop_get();
   char *address = NULL;
   Eina_Error err;
#ifdef EFL_NET_SERVER_UNIX_CLASS
   Eina_Bool local_system = EINA_FALSE;
   mode_t old_mask = 0, new_mask = 0;
#endif

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   svr = calloc(1, sizeof(Ecore_Ipc_Server));
   EINA_SAFETY_ON_NULL_RETURN_VAL(svr, NULL);

   if (0) { }
#ifdef EFL_NET_SERVER_UNIX_CLASS
   if ((type & ECORE_IPC_TYPE) == ECORE_IPC_LOCAL_USER)
     {
        address = ecore_con_local_path_new(EINA_FALSE, name, port);
        EINA_SAFETY_ON_NULL_GOTO(address, error_server);

        new_mask = S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;

        svr->server = efl_add(EFL_NET_SERVER_UNIX_CLASS, efl_main_loop_get(),
                              efl_net_server_unix_leading_directories_create_set(efl_added, EINA_TRUE, S_IRUSR | S_IWUSR | S_IXUSR));
        EINA_SAFETY_ON_NULL_GOTO(svr->server, error_server);
     }
   else if ((type & ECORE_IPC_TYPE) == ECORE_IPC_LOCAL_SYSTEM)
     {
        address = ecore_con_local_path_new(EINA_TRUE, name, port);
        EINA_SAFETY_ON_NULL_GOTO(address, error_server);

        /* ecore_con didn't create leading directories for LOCAL_SYSTEM */

        new_mask = 0;
        local_system = EINA_TRUE;

        svr->server = efl_add(EFL_NET_SERVER_UNIX_CLASS, efl_main_loop_get());
        EINA_SAFETY_ON_NULL_GOTO(svr->server, error_server);
     }
#endif /* EFL_NET_SERVER_UNIX_CLASS */
#ifdef EFL_NET_SERVER_WINDOWS_CLASS
   if ((type & ECORE_IPC_TYPE) == ECORE_IPC_LOCAL_USER)
     {
        address = ecore_con_local_path_new(EINA_FALSE, name, port);
        EINA_SAFETY_ON_NULL_GOTO(address, error_server);

        // TODO: specify SECURITY_ATTRIBUTES to use or some
        // Efl_Net_Server_Windows API to limit access

        svr->server = efl_add(EFL_NET_SERVER_WINDOWS_CLASS, efl_main_loop_get());
        EINA_SAFETY_ON_NULL_GOTO(svr->server, error_server);
     }
   else if ((type & ECORE_IPC_TYPE) == ECORE_IPC_LOCAL_SYSTEM)
     {
        address = ecore_con_local_path_new(EINA_TRUE, name, port);
        EINA_SAFETY_ON_NULL_GOTO(address, error_server);

        // TODO: specify SECURITY_ATTRIBUTES to use or some
        // Efl_Net_Server_Windows API to limit access

        svr->server = efl_add(EFL_NET_SERVER_WINDOWS_CLASS, efl_main_loop_get());
        EINA_SAFETY_ON_NULL_GOTO(svr->server, error_server);
     }
#endif /* EFL_NET_SERVER_WINDOWS_CLASS */
   else if ((type & ECORE_IPC_TYPE) == ECORE_IPC_REMOTE_SYSTEM)
     {
        char buf[4096];

        if (port <= 0)
          {
             ERR("remote system requires port>=0, got %d", port);
             goto error_server;
          }

        snprintf(buf, sizeof(buf), "%s:%d", name, port);
        address = strdup(buf);
        EINA_SAFETY_ON_NULL_GOTO(address, error_server);

        if ((type & ECORE_IPC_USE_SSL) == ECORE_IPC_USE_SSL)
          {
             svr->server = efl_add(EFL_NET_SERVER_SSL_CLASS, loop);
             EINA_SAFETY_ON_NULL_GOTO(svr->server, error_server);
          }
        else
          {
             svr->server = efl_add(EFL_NET_SERVER_TCP_CLASS, loop);
             EINA_SAFETY_ON_NULL_GOTO(svr->server, error_server);
          }
     }
   else
     {
        ERR("IPC Type must be one of: local_user, local_system or remote_system");
        goto error_server;
     }

   efl_event_callback_array_add(svr->server, _ecore_ipc_server_cbs(), svr);

   if (efl_isa(svr->server, EFL_NET_SERVER_FD_CLASS))
     {
        efl_net_server_fd_reuse_address_set(svr->server, EINA_TRUE);
        efl_net_server_fd_reuse_port_set(svr->server, EINA_TRUE);
     }

   if (efl_isa(svr->server, EFL_NET_SERVER_TCP_CLASS))
     {
        /* old ecore_con did not map ipv4 to ipv6... */
        efl_net_server_ip_ipv6_only_set(svr->server, EINA_TRUE);
     }

#ifdef EFL_NET_SERVER_UNIX_CLASS
   if (efl_isa(svr->server, EFL_NET_SERVER_UNIX_CLASS))
     {
        old_mask = umask(new_mask);
        efl_net_server_unix_leading_directories_create_set(svr->server,
                                                           EINA_TRUE,
                                                           local_system ? 0755 : 0700);
     }
#endif

   err = efl_net_server_serve(svr->server, address);

#ifdef EFL_NET_SERVER_UNIX_CLASS
   if (efl_isa(svr->server, EFL_NET_SERVER_UNIX_CLASS))
     umask(old_mask);
#endif

   if (err)
     {
        WRN("Could not serve %s %s: %s",
            efl_class_name_get(efl_class_get(svr->server)),
            address, eina_error_msg_get(err));
        goto error;
     }
   DBG("will serve %p %s address='%s'",
       svr->server,
       efl_class_name_get(efl_class_get(svr->server)),
       address);

   svr->max_buf_size = 32 * 1024;
   svr->data = (void *)data;
   servers = eina_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_IPC_SERVER);
   free(address);
   return svr;

 error:
   free(address);
   _ecore_ipc_server_del(svr);
   free(svr);
   return NULL; /* server will trigger all cleanup on its own callbacks */

 error_server:
   free(address);
   free(svr);
   return NULL;
}

static Efl_Callback_Array_Item *_ecore_ipc_dialer_cbs(void);

static void
_ecore_ipc_dialer_del(Ecore_Ipc_Server *svr)
{
   DBG("dialer %p del", svr);

   if (svr->dialer.recv_copier)
     {
        efl_del(svr->dialer.recv_copier);
        svr->dialer.recv_copier = NULL;
     }

   if (svr->dialer.send_copier)
     {
        efl_del(svr->dialer.send_copier);
        svr->dialer.send_copier = NULL;
     }

   if (svr->dialer.input)
     {
        efl_del(svr->dialer.input);
        svr->dialer.input = NULL;
     }

   if (svr->dialer.dialer)
     {
        efl_event_callback_array_del(svr->dialer.dialer, _ecore_ipc_dialer_cbs(), svr);
        if (!efl_io_closer_closed_get(svr->dialer.dialer))
          efl_io_closer_close(svr->dialer.dialer);

        efl_del(svr->dialer.dialer);
        svr->dialer.dialer = NULL;
     }
}

static void
_ecore_ipc_dialer_eos(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Ipc_Server *svr = data;

   DBG("dialer %p %p eos", svr, svr->dialer.dialer);

   _ecore_ipc_dialer_del(svr);

   ecore_ipc_post_event_server_del(svr);
}

static void
_ecore_ipc_dialer_error(void *data, const Efl_Event *event)
{
   Ecore_Ipc_Server *svr = data;
   Eina_Error *perr = event->info;

   WRN("dialer %p %p error %s", svr, svr->dialer.dialer, eina_error_msg_get(*perr));

   if (!efl_io_closer_closed_get(svr->dialer.dialer))
     efl_io_closer_close(svr->dialer.dialer); /* triggers EOS */
}

static void
_ecore_ipc_dialer_connected(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Ipc_Server *svr = data;

   DBG("connected to %s %s",
       efl_class_name_get(efl_class_get(svr->dialer.dialer)),
       efl_net_dialer_address_dial_get(svr->dialer.dialer));

   ecore_ipc_post_event_server_add(svr);
}

EFL_CALLBACKS_ARRAY_DEFINE(_ecore_ipc_dialer_cbs,
                           { EFL_IO_READER_EVENT_EOS, _ecore_ipc_dialer_eos },
                           { EFL_NET_DIALER_EVENT_DIALER_ERROR, _ecore_ipc_dialer_error },
                           { EFL_NET_DIALER_EVENT_DIALER_CONNECTED, _ecore_ipc_dialer_connected });

static Eina_Bool ecore_ipc_server_data_process(Ecore_Ipc_Server *svr, void *data, int size, Eina_Bool *stolen);

static void
_ecore_ipc_dialer_copier_data(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Ipc_Server *svr = data;
   Eina_Binbuf *binbuf;
   uint8_t *mem;
   int size;
   Eina_Bool stolen;

   DBG("dialer %p recv_copier %p data", svr, svr->dialer.recv_copier);

   binbuf = efl_io_copier_binbuf_steal(svr->dialer.recv_copier);
   EINA_SAFETY_ON_NULL_RETURN(binbuf);
   size = eina_binbuf_length_get(binbuf);
   mem = eina_binbuf_string_steal(binbuf);
   eina_binbuf_free(binbuf);

   ecore_ipc_server_data_process(svr, mem, size, &stolen);
   if (!stolen) free(mem);
}

static void
_ecore_ipc_dialer_copier_error(void *data, const Efl_Event *event)
{
   Ecore_Ipc_Server *svr = data;
   Eina_Error *perr = event->info;

   WRN("dialer %p %p copier %p error %s", svr, svr->dialer.dialer, event->object, eina_error_msg_get(*perr));

   if (!efl_io_closer_closed_get(svr->dialer.dialer))
     efl_io_closer_close(svr->dialer.dialer);
}

EFL_CALLBACKS_ARRAY_DEFINE(_ecore_ipc_dialer_copier_cbs,
                           { EFL_IO_COPIER_EVENT_ERROR, _ecore_ipc_dialer_copier_error });

/* FIXME: need to add protocol type parameter */
EAPI Ecore_Ipc_Server *
ecore_ipc_server_connect(Ecore_Ipc_Type type, char *name, int port, const void *data)
{
   Ecore_Ipc_Server *svr;
   Eo *loop = efl_main_loop_get();
   char *address = NULL;
   Eina_Error err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);

   svr = calloc(1, sizeof(Ecore_Ipc_Server));
   EINA_SAFETY_ON_NULL_RETURN_VAL(svr, NULL);

   if (0) { }
#ifdef EFL_NET_DIALER_UNIX_CLASS
   if ((type & ECORE_IPC_TYPE) == ECORE_IPC_LOCAL_USER)
     {
        struct stat st;

        address = ecore_con_local_path_new(EINA_FALSE, name, port);
        EINA_SAFETY_ON_NULL_GOTO(address, error_dialer);

        if ((stat(address, &st) != 0)
#ifdef S_ISSOCK
            || (!S_ISSOCK(st.st_mode))
#endif
            )
          {
             DBG("%s is not a socket", address);
             goto error_dialer;
          }

        svr->dialer.dialer = efl_add(EFL_NET_DIALER_UNIX_CLASS, efl_main_loop_get());
        EINA_SAFETY_ON_NULL_GOTO(svr->dialer.dialer, error_dialer);
     }
   else if ((type & ECORE_IPC_TYPE) == ECORE_IPC_LOCAL_SYSTEM)
     {
        address = ecore_con_local_path_new(EINA_TRUE, name, port);
        EINA_SAFETY_ON_NULL_GOTO(address, error_dialer);

        svr->dialer.dialer = efl_add(EFL_NET_DIALER_UNIX_CLASS, efl_main_loop_get());
        EINA_SAFETY_ON_NULL_GOTO(svr->dialer.dialer, error_dialer);
     }
#endif /* EFL_NET_DIALER_UNIX_CLASS */
#ifdef EFL_NET_DIALER_WINDOWS_CLASS
   if ((type & ECORE_IPC_TYPE) == ECORE_IPC_LOCAL_USER)
     {
        address = ecore_con_local_path_new(EINA_FALSE, name, port);
        EINA_SAFETY_ON_NULL_GOTO(address, error_dialer);

        svr->dialer.dialer = efl_add(EFL_NET_DIALER_WINDOWS_CLASS, efl_main_loop_get());
        EINA_SAFETY_ON_NULL_GOTO(svr->dialer.dialer, error_dialer);
     }
   else if ((type & ECORE_IPC_TYPE) == ECORE_IPC_LOCAL_SYSTEM)
     {
        address = ecore_con_local_path_new(EINA_TRUE, name, port);
        EINA_SAFETY_ON_NULL_GOTO(address, error_dialer);

        svr->dialer.dialer = efl_add(EFL_NET_DIALER_WINDOWS_CLASS, efl_main_loop_get());
        EINA_SAFETY_ON_NULL_GOTO(svr->dialer.dialer, error_dialer);
     }
#endif /* EFL_NET_DIALER_WINDOWS_CLASS */
   else if ((type & ECORE_IPC_TYPE) == ECORE_IPC_REMOTE_SYSTEM)
     {
        char buf[4096];

        if (port <= 0)
          {
             ERR("remote system requires port>=0, got %d", port);
             goto error_dialer;
          }

        snprintf(buf, sizeof(buf), "%s:%d", name, port);
        address = strdup(buf);
        EINA_SAFETY_ON_NULL_GOTO(address, error_dialer);

        if ((type & ECORE_IPC_USE_SSL) == ECORE_IPC_USE_SSL)
          {
             svr->dialer.dialer = efl_add(EFL_NET_DIALER_SSL_CLASS, loop);
             EINA_SAFETY_ON_NULL_GOTO(svr->dialer.dialer, error_dialer);
          }
        else
          {
             svr->dialer.dialer = efl_add(EFL_NET_DIALER_TCP_CLASS, loop);
             EINA_SAFETY_ON_NULL_GOTO(svr->dialer.dialer, error_dialer);
          }

        if ((type & ECORE_IPC_NO_PROXY) == ECORE_IPC_NO_PROXY)
          efl_net_dialer_proxy_set(svr->dialer.dialer, "");
     }
   else
     {
        ERR("IPC Type must be one of: local_user, local_system or remote_system");
        goto error_dialer;
     }

   efl_io_closer_close_on_invalidate_set(svr->dialer.dialer, EINA_TRUE);
   efl_event_callback_array_add(svr->dialer.dialer, _ecore_ipc_dialer_cbs(), svr);

   svr->dialer.input = efl_add(EFL_IO_QUEUE_CLASS, loop);
   EINA_SAFETY_ON_NULL_GOTO(svr->dialer.input, error);

   svr->dialer.send_copier = efl_add(EFL_IO_COPIER_CLASS, loop,
                                     efl_io_closer_close_on_invalidate_set(efl_added, EINA_FALSE),
                                     efl_io_copier_source_set(efl_added, svr->dialer.input),
                                     efl_io_copier_destination_set(efl_added, svr->dialer.dialer),
                                     efl_event_callback_array_add(efl_added, _ecore_ipc_dialer_copier_cbs(), svr));
   EINA_SAFETY_ON_NULL_GOTO(svr->dialer.send_copier, error);

   svr->dialer.recv_copier = efl_add(EFL_IO_COPIER_CLASS, loop,
                                     efl_io_closer_close_on_invalidate_set(efl_added, EINA_FALSE),
                                     efl_io_copier_source_set(efl_added, svr->dialer.dialer),
                                     efl_event_callback_array_add(efl_added, _ecore_ipc_dialer_copier_cbs(), svr),
                                     efl_event_callback_add(efl_added, EFL_IO_COPIER_EVENT_DATA, _ecore_ipc_dialer_copier_data, svr));
   EINA_SAFETY_ON_NULL_GOTO(svr->dialer.recv_copier, error);

   err = efl_net_dialer_dial(svr->dialer.dialer, address);
   if (err)
     {
        WRN("Could not reach %s %s: %s",
            efl_class_name_get(efl_class_get(svr->dialer.dialer)),
            address, eina_error_msg_get(err));
        goto error;
     }
   DBG("connecting %p %s address='%s'",
       svr->dialer.dialer,
       efl_class_name_get(efl_class_get(svr->dialer.dialer)),
       address);

   svr->max_buf_size = -1;
   svr->data = (void *)data;
   servers = eina_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_IPC_SERVER);
   free(address);
   return svr;

 error:
   free(address);
   _ecore_ipc_dialer_del(svr);
   free(svr);
   return NULL; /* dialer will trigger all cleanup on its own callbacks */

 error_dialer:
   free(address);
   free(svr);
   return NULL;
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

        if (svr->dialer.dialer) _ecore_ipc_dialer_del(svr);
        if (svr->server) _ecore_ipc_server_del(svr);
        servers = eina_list_remove(servers, svr);

        if (svr->buf) free(svr->buf);
        eina_list_free(svr->dead_clients);
        eina_list_free(svr->clients);
        ECORE_MAGIC_SET(svr, ECORE_MAGIC_NONE);
        DBG("server %p freed", svr);
        free(svr);
     }
   else DBG("server %p has %d events pending, postpone deletion", svr, svr->event_count);
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

   if (svr->dialer.dialer)
     return efl_net_dialer_connected_get(svr->dialer.dialer);
   else if (svr->server) return EINA_TRUE;
   return EINA_FALSE;
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
        v = eina_htonl(v); \
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
        v = eina_htons(v); \
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
   *head = eina_htonl(*head);
   svr->prev.o = msg;

   if (svr->dialer.input)
     {
        Eina_Slice slice;
        Eina_Error err;

        slice.mem = dat;
        slice.len = s;
        err = efl_io_writer_write(svr->dialer.input, &slice, NULL);
        if (err)
          {
             ERR("could not write queue=%p %zd bytes: %s",
                 svr->dialer.input, slice.len, eina_error_msg_get(err));
             return 0;
          }
        if (slice.len < (size_t)s)
          {
             ERR("only wrote %zd of %d bytes to queue %p",
                 slice.len, s, svr->dialer.input);
             return 0;
          }

        if ((data) && (size > 0))
          {
             slice.mem = data;
             slice.len = size;
             err = efl_io_writer_write(svr->dialer.input, &slice, NULL);
             if (err)
               {
                  ERR("could not write queue=%p %zd bytes: %s",
                      svr->dialer.input, slice.len, eina_error_msg_get(err));
                  return 0;
               }
             if (slice.len < (size_t)size)
               {
                  ERR("only wrote %zd of %d bytes to queue %p",
                      slice.len, size, svr->dialer.input);
                  return 0;
               }
          }

        return s + size;
     }
   else if (svr->server)
     {
        ERR("Send data to clients, not the server handle");
        return 0;
     }
   return 0;
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
   if (svr->server)
     {
        efl_net_server_clients_limit_set(svr->server, client_limit, reject_excess_clients);
        return;
     }
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

   if (svr->dialer.dialer)
     {
        if (efl_isa(svr->dialer.dialer, EFL_NET_DIALER_TCP_CLASS) ||
            efl_isa(svr->dialer.dialer, EFL_NET_DIALER_SSL_CLASS))
          return efl_net_dialer_address_dial_get(svr->dialer.dialer);
        /* original IPC just returned IP for remote connections */
        return NULL;
     }
   else if (svr->server)
     {
        if (efl_isa(svr->server, EFL_NET_SERVER_TCP_CLASS) ||
            efl_isa(svr->server, EFL_NET_SERVER_SSL_CLASS))
          return efl_net_server_address_get(svr->server);
        /* original IPC just returned IP for remote connections */
        return NULL;
     }
   return NULL;
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
   if (svr->dialer.input)
     {
        while (!efl_io_closer_closed_get(svr->dialer.dialer) &&
               !efl_net_dialer_connected_get(svr->dialer.dialer))
          ecore_main_loop_iterate();
        while ((efl_io_queue_usage_get(svr->dialer.input) > 0) ||
               (efl_io_copier_pending_size_get(svr->dialer.send_copier) > 0))
          efl_io_copier_flush(svr->dialer.send_copier, EINA_TRUE, EINA_TRUE);
        return;
     }
   else if (svr->server)
     {
        ERR("Flush clients, not the server handle");
        return;
     }
}

#define CLENC(_member) \
   d = _ecore_ipc_dlt_int(msg._member, cl->prev.o._member, &md); \
   if (md >= DLT_SET) \
     { \
        unsigned int v; \
        unsigned char *dd; \
        dd = (unsigned char *)&v; \
        v = d; \
        v = eina_htonl(v); \
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
        v = eina_htons(v); \
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
   int *head, md = 0, d, s;
   unsigned char dat[sizeof(Ecore_Ipc_Msg_Head)];

   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
                         "ecore_ipc_client_send");
        return 0;
     }
   if (cl->socket.socket)
     EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(cl->socket.socket), 0);
   else
     {
        ERR("client %p is not connected", cl);
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
   *head = eina_htonl(*head);
   cl->prev.o = msg;

   if (cl->socket.input)
     {
        Eina_Slice slice;
        Eina_Error err;

        slice.mem = dat;
        slice.len = s;
        err = efl_io_writer_write(cl->socket.input, &slice, NULL);
        if (err)
          {
             ERR("could not write queue=%p %zd bytes: %s",
                 cl->socket.input, slice.len, eina_error_msg_get(err));
             return 0;
          }
        if (slice.len < (size_t)s)
          {
             ERR("only wrote %zd of %d bytes to queue %p",
                 slice.len, s, cl->socket.input);
             return 0;
          }
        if ((data) && (size > 0))
          {
             slice.mem = data;
             slice.len = size;
             err = efl_io_writer_write(cl->socket.input, &slice, NULL);
             if (err)
               {
                  ERR("could not write queue=%p %zd bytes: %s",
                      cl->socket.input, slice.len, eina_error_msg_get(err));
                  return 0;
               }
             if (slice.len < (size_t)size)
               {
                  ERR("only wrote %zd of %d bytes to queue %p",
                      slice.len, size, cl->socket.input);
                  return 0;
               }
          }

        return s + size;
     }
   return 0;
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

static Efl_Callback_Array_Item *_ecore_ipc_socket_cbs(void);

static void
_ecore_ipc_client_socket_del(Ecore_Ipc_Client *cl)
{
   DBG("client %p socket del", cl);

   if (cl->socket.recv_copier)
     {
        efl_del(cl->socket.recv_copier);
        cl->socket.recv_copier = NULL;
     }

   if (cl->socket.send_copier)
     {
        efl_del(cl->socket.send_copier);
        cl->socket.send_copier = NULL;
     }

   if (cl->socket.input)
     {
        efl_del(cl->socket.input);
        cl->socket.input = NULL;
     }

   if (cl->socket.socket)
     {
        efl_event_callback_array_del(cl->socket.socket, _ecore_ipc_socket_cbs(), cl);
        /* do not del() as it's owned by srv->server */
        if (!efl_io_closer_closed_get(cl->socket.socket))
          efl_io_closer_close(cl->socket.socket);
        efl_unref(cl->socket.socket);
        cl->socket.socket = NULL;
        if (!cl->svr) return;
        cl->svr->clients = eina_list_remove(cl->svr->clients, cl);
        cl->svr->dead_clients = eina_list_append(cl->svr->dead_clients, cl);
     }
}

static void
_ecore_ipc_client_socket_eos(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Ipc_Client *cl = data;

   DBG("client %p socket %p eos", cl, cl->socket.socket);

   _ecore_ipc_client_socket_del(cl);

   ecore_ipc_post_event_client_del(cl);
}

EFL_CALLBACKS_ARRAY_DEFINE(_ecore_ipc_socket_cbs,
                           { EFL_IO_READER_EVENT_EOS, _ecore_ipc_client_socket_eos });

static Eina_Bool ecore_ipc_client_data_process(Ecore_Ipc_Client *cl, void *data, int size, Eina_Bool *stolen);

static void
_ecore_ipc_client_socket_copier_data(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Ipc_Client *cl = data;
   Eina_Binbuf *binbuf;
   uint8_t *mem;
   int size;
   Eina_Bool stolen;

   DBG("client %p recv_copier %p data", cl, cl->socket.recv_copier);

   binbuf = efl_io_copier_binbuf_steal(cl->socket.recv_copier);
   EINA_SAFETY_ON_NULL_RETURN(binbuf);
   size = eina_binbuf_length_get(binbuf);
   mem = eina_binbuf_string_steal(binbuf);
   eina_binbuf_free(binbuf);

   ecore_ipc_client_data_process(cl, mem, size, &stolen);
   if (!stolen) free(mem);
}

static void
_ecore_ipc_client_socket_copier_error(void *data, const Efl_Event *event)
{
   Ecore_Ipc_Client *cl = data;
   Eina_Error *perr = event->info;

   WRN("client %p socket %p copier %p error %s", cl, cl->socket.socket, event->object, eina_error_msg_get(*perr));

   if (!efl_io_closer_closed_get(cl->socket.socket))
     efl_io_closer_close(cl->socket.socket);
}

EFL_CALLBACKS_ARRAY_DEFINE(_ecore_ipc_client_socket_copier_cbs,
                           { EFL_IO_COPIER_EVENT_ERROR, _ecore_ipc_client_socket_copier_error });

static void
_ecore_ipc_server_client_add(void *data, const Efl_Event *event)
{
   Ecore_Ipc_Server *svr = data;
   Eo *socket = event->info;
   Ecore_Ipc_Client *cl;
   Eo *loop;

   DBG("server %p %p got new client %p (%s)",
       svr, svr->server,
       event->object, efl_net_socket_address_remote_get(socket));

   cl = ecore_ipc_client_add(svr);
   EINA_SAFETY_ON_NULL_RETURN(cl);

   cl->socket.socket = efl_ref(socket);
   efl_event_callback_array_add(cl->socket.socket, _ecore_ipc_socket_cbs(), cl);

   loop = efl_loop_get(socket);

   cl->socket.input = efl_add(EFL_IO_QUEUE_CLASS, loop);
   EINA_SAFETY_ON_NULL_GOTO(cl->socket.input, error);

   cl->socket.send_copier = efl_add(EFL_IO_COPIER_CLASS, loop,
                                     efl_io_closer_close_on_invalidate_set(efl_added, EINA_FALSE),
                                     efl_io_copier_source_set(efl_added, cl->socket.input),
                                     efl_io_copier_destination_set(efl_added, cl->socket.socket),
                                     efl_event_callback_array_add(efl_added, _ecore_ipc_client_socket_copier_cbs(), cl));
   EINA_SAFETY_ON_NULL_GOTO(cl->socket.send_copier, error);

   cl->socket.recv_copier = efl_add(EFL_IO_COPIER_CLASS, loop,
                                     efl_io_closer_close_on_invalidate_set(efl_added, EINA_FALSE),
                                     efl_io_copier_source_set(efl_added, cl->socket.socket),
                                     efl_event_callback_array_add(efl_added, _ecore_ipc_client_socket_copier_cbs(), cl),
                                     efl_event_callback_add(efl_added, EFL_IO_COPIER_EVENT_DATA, _ecore_ipc_client_socket_copier_data, cl));
   EINA_SAFETY_ON_NULL_GOTO(cl->socket.recv_copier, error);

   ecore_ipc_post_event_client_add(cl);
   return;

 error:
   _ecore_ipc_client_socket_del(cl);
   free(cl);
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
        if (cl->socket.socket) _ecore_ipc_client_socket_del(cl);
        if (ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
          svr->dead_clients = eina_list_remove(svr->dead_clients, cl);
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
   if (cl->socket.socket)
     {
        if (efl_isa(cl->socket.socket, EFL_NET_SOCKET_TCP_CLASS) ||
            efl_isa(cl->socket.socket, EFL_NET_SOCKET_SSL_CLASS))
          return efl_net_socket_address_remote_get(cl->socket.socket);
        /* original IPC just returned IP for remote connections,
         * for unix socket it returned 0.0.0.0
         */
        return "0.0.0.0";
     }
   return NULL;
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
   if (cl->socket.input)
     {
        while (efl_io_queue_usage_get(cl->socket.input) > 0)
          efl_io_copier_flush(cl->socket.send_copier, EINA_TRUE, EINA_TRUE);
        return;
     }
}

EAPI int
ecore_ipc_ssl_available_get(void)
{
   return ecore_con_ssl_available_get();
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
        d = (int)eina_ntohl(v); \
        s += 4; \
     } \
   else if (md >= DLT_ADD16) \
     { \
        unsigned short v; \
        unsigned char *dv; \
        dv = (unsigned char *)&v; \
        dv[0] = *(cl->buf + offset + s + 0); \
        dv[1] = *(cl->buf + offset + s + 1); \
        d = (int)eina_ntohs(v); \
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
ecore_ipc_client_data_process(Ecore_Ipc_Client *cl, void *data, int size, Eina_Bool *stolen)
{
   /* use e->data and e->size to reduce diff to original code */
   struct { void *data; int size; } _e = { data, size }, *e = &_e;
   Ecore_Ipc_Server *svr = ecore_ipc_client_server_get(cl);
   *stolen = EINA_FALSE;
   if (1)
     { /* keep same identation as original code to help verification */
        Ecore_Ipc_Msg_Head msg;
        int offset = 0;
        unsigned char *buf;

        if (!cl->buf)
          {
             cl->buf_size = e->size;
             cl->buf = e->data;
             *stolen = EINA_TRUE;
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
             head = eina_ntohl(head);
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
        d = (int)eina_ntohl(v); \
        s += 4; \
     } \
   else if (md >= DLT_ADD16) \
     { \
        unsigned short v; \
        unsigned char *dv; \
        dv = (unsigned char *)&v; \
        dv[0] = *(svr->buf + offset + s + 0); \
        dv[1] = *(svr->buf + offset + s + 1); \
        d = (int)eina_ntohs(v); \
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
ecore_ipc_server_data_process(Ecore_Ipc_Server *svr, void *data, int size, Eina_Bool *stolen)
{
   /* use e->data and e->size to reduce diff to original code */
   struct { void *data; int size; } _e = { data, size }, *e = &_e;
   *stolen = EINA_FALSE;
   if (1)
     { /* keep same identation as original code to help verification */
        Ecore_Ipc_Msg_Head msg;
        int offset = 0;
        unsigned char *buf = NULL;

        if (!svr->buf)
          {
             svr->buf_size = e->size;
             svr->buf = e->data;
             *stolen = EINA_TRUE;
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
             head = eina_ntohl(head);
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
   Ecore_Ipc_Event_Server_Del *e;

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
