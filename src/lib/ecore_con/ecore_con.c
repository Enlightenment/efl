#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_TCP_H
# include <netinet/tcp.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#ifdef HAVE_SYSTEMD
# include <systemd/sd-daemon.h>
#endif

#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

static Eina_Bool   _ecore_con_client_timer(Ecore_Con_Client *cl);
static void        _ecore_con_cl_timer_update(Ecore_Con_Client *cl);
static Eina_Bool   _ecore_con_server_timer(Ecore_Con_Server *svr);
static void        _ecore_con_server_timer_update(Ecore_Con_Server *svr);

static void        _ecore_con_cb_tcp_connect(void *data,
                                             Ecore_Con_Info *info);
static void        _ecore_con_cb_udp_connect(void *data,
                                             Ecore_Con_Info *info);
static void        _ecore_con_cb_tcp_listen(void *data,
                                            Ecore_Con_Info *info);
static void        _ecore_con_cb_udp_listen(void *data,
                                            Ecore_Con_Info *info);

static void        _ecore_con_server_free(Ecore_Con_Server *svr);
static void        _ecore_con_client_free(Ecore_Con_Client *cl);

static void        _ecore_con_cl_read(Ecore_Con_Server *svr);
static Eina_Bool   _ecore_con_svr_tcp_handler(void *data,
                                              Ecore_Fd_Handler *fd_handler);
static Eina_Bool   _ecore_con_cl_handler(void *data,
                                         Ecore_Fd_Handler *fd_handler);
static Eina_Bool   _ecore_con_cl_udp_handler(void *data,
                                             Ecore_Fd_Handler *fd_handler);
static Eina_Bool   _ecore_con_svr_udp_handler(void *data,
                                              Ecore_Fd_Handler *fd_handler);

static void        _ecore_con_svr_cl_read(Ecore_Con_Client *cl);
static Eina_Bool   _ecore_con_svr_cl_handler(void *data,
                                             Ecore_Fd_Handler *fd_handler);

static void        _ecore_con_server_flush(Ecore_Con_Server *svr);
static void        _ecore_con_client_flush(Ecore_Con_Client *obj);

static void        _ecore_con_event_client_add_free(Ecore_Con_Server *svr,
                                                    void *ev);
static void        _ecore_con_event_client_del_free(Ecore_Con_Server *svr,
                                                    void *ev);
static void        _ecore_con_event_client_data_free(Ecore_Con_Server *svr,
                                                     void *ev);
static void        _ecore_con_event_server_add_free(void *data,
                                                    void *ev);
static void        _ecore_con_event_server_del_free(void *data,
                                                    void *ev);
static void        _ecore_con_event_server_data_free(void *data,
                                                     void *ev);
static void        _ecore_con_event_server_error_free(void *data,
                                                      Ecore_Con_Event_Server_Error *e);
static void        _ecore_con_event_client_error_free(Ecore_Con_Server *svr,
                                                      Ecore_Con_Event_Client_Error *e);
static void        _ecore_con_event_server_write_free(void *data,
                                                      Ecore_Con_Event_Server_Write *e);
static void        _ecore_con_event_client_write_free(Ecore_Con_Server *svr,
                                                      Ecore_Con_Event_Client_Write *e);

static void        _ecore_con_lookup_done(void *data,
                                          Ecore_Con_Info *infos);

static const char *_ecore_con_pretty_ip(struct sockaddr *client_addr);

#define EO_CONSTRUCTOR_CHECK_RETURN(obj) do { \
     if (eo_do(obj, eo_finalized_get())) \
       { \
          ERR("This function is only allowed during construction."); \
          return; \
       } \
} while (0)

#ifdef HAVE_SYSTEMD
int sd_fd_index = 0;
int sd_fd_max = 0;
#endif

void
_ecore_con_client_kill(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   if (cl->delete_me)
     DBG("Multi kill request for client %p", cl);
   else
     {
        ecore_con_event_client_del(obj);
        if (cl->buf) return;
     }
   INF("Lost client %s", (cl->ip) ? cl->ip : "");
   if (cl->fd_handler)
     ecore_main_fd_handler_del(cl->fd_handler);

   cl->fd_handler = NULL;
}

void
_ecore_con_server_kill(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (svr->delete_me)
     DBG("Multi kill request for svr %p", svr);
   else
     ecore_con_event_server_del(obj);

   if (svr->fd_handler)
     ecore_main_fd_handler_del(svr->fd_handler);

   svr->fd_handler = NULL;
}

#define _ecore_con_server_kill(svr) do { \
       DBG("KILL %p", (svr));            \
       _ecore_con_server_kill((svr));    \
  } while (0)

#define _ecore_con_client_kill(cl)  do { \
       DBG("KILL %p", (cl));             \
       _ecore_con_client_kill((cl));     \
  } while (0)

EAPI int ECORE_CON_EVENT_CLIENT_ADD = 0;
EAPI int ECORE_CON_EVENT_CLIENT_DEL = 0;
EAPI int ECORE_CON_EVENT_SERVER_ADD = 0;
EAPI int ECORE_CON_EVENT_SERVER_DEL = 0;
EAPI int ECORE_CON_EVENT_CLIENT_DATA = 0;
EAPI int ECORE_CON_EVENT_SERVER_DATA = 0;
EAPI int ECORE_CON_EVENT_CLIENT_WRITE = 0;
EAPI int ECORE_CON_EVENT_SERVER_WRITE = 0;
EAPI int ECORE_CON_EVENT_CLIENT_ERROR = 0;
EAPI int ECORE_CON_EVENT_SERVER_ERROR = 0;
EAPI int ECORE_CON_EVENT_PROXY_BIND = 0;

static Eina_List *servers = NULL;
static int _ecore_con_init_count = 0;
static int _ecore_con_event_count = 0;
int _ecore_con_log_dom = -1;
Ecore_Con_Socks *_ecore_con_proxy_once = NULL;
Ecore_Con_Socks *_ecore_con_proxy_global = NULL;

EAPI int
ecore_con_init(void)
{
   if (++_ecore_con_init_count != 1)
     return _ecore_con_init_count;

#ifdef HAVE_EVIL
   if (!evil_init())
     return --_ecore_con_init_count;
#endif

   if (!ecore_init())
     return --_ecore_con_init_count;

   _ecore_con_log_dom = eina_log_domain_register
       ("ecore_con", ECORE_CON_DEFAULT_LOG_COLOR);
   if (_ecore_con_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for Ecore Con.");
        ecore_shutdown();
        return --_ecore_con_init_count;
     }

   ecore_con_mempool_init();

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

   eina_magic_string_set(ECORE_MAGIC_CON_SERVER, "Ecore_Con_Server");
   eina_magic_string_set(ECORE_MAGIC_CON_CLIENT, "Ecore_Con_Client");
   eina_magic_string_set(ECORE_MAGIC_CON_URL, "Ecore_Con_Url");

   /* TODO Remember return value, if it fails, use gethostbyname() */
   ecore_con_socks_init();
   ecore_con_ssl_init();
   ecore_con_info_init();

#ifdef HAVE_SYSTEMD
   sd_fd_max = sd_listen_fds(0);
#endif

   eina_log_timing(_ecore_con_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   return _ecore_con_init_count;
}

EAPI int
ecore_con_shutdown(void)
{
   Eina_List *l, *l2;
   Ecore_Con_Server *obj;

   if (--_ecore_con_init_count != 0)
     return _ecore_con_init_count;

   eina_log_timing(_ecore_con_log_dom,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   EINA_LIST_FOREACH_SAFE(servers, l, l2, obj)
     {
        Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
        Ecore_Con_Event_Server_Add *ev;

        svr->delete_me = EINA_TRUE;
        INF("svr %p is dead", svr);
        /* some pointer hacks here to prevent double frees if people are being stupid */
        EINA_LIST_FREE(svr->event_count, ev)
          ev->server = NULL;
        _ecore_con_server_free(obj);
     }

   ecore_con_socks_shutdown();
   if (!_ecore_con_event_count) ecore_con_mempool_shutdown();

   ecore_con_info_shutdown();
   ecore_con_ssl_shutdown();
   eina_log_domain_unregister(_ecore_con_log_dom);
   _ecore_con_log_dom = -1;
   ecore_shutdown();
#ifdef HAVE_EVIL
   evil_shutdown();
#endif

   return _ecore_con_init_count;
}

EOLIAN static Eina_Bool
_ecore_con_base_lookup(Eo *kls_obj EINA_UNUSED, void *pd EINA_UNUSED, const char *name, Ecore_Con_Dns_Cb done_cb, const void *data)
{
   Ecore_Con_Server *obj;
   Ecore_Con_Lookup *lk;
   struct addrinfo hints;

   if (!name || !done_cb)
     return EINA_FALSE;

   obj = eo_add(ECORE_CON_CONNECTOR_CLASS, NULL,
         ecore_con_server_obj_connection_type_set(ECORE_CON_REMOTE_TCP),
         ecore_con_server_obj_name_set(name),
         ecore_con_obj_port_set(1025));

   lk = malloc(sizeof (Ecore_Con_Lookup));
   if (!lk)
     {
        return EINA_FALSE;
     }

   lk->done_cb = done_cb;
   lk->data = data;

   ecore_con_server_data_set(obj, lk);

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_CANONNAME;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   if (ecore_con_info_get(obj, _ecore_con_lookup_done, obj,
                          &hints))
     return EINA_TRUE;

   free(lk);
   eo_del(obj);
   return EINA_FALSE;
}

/**
 * @addtogroup Ecore_Con_Server_Group Ecore Connection Server Functions
 *
 * Functions that operate on Ecore server objects.
 *
 * @{
 */

/**
 * @example ecore_con_server_example.c
 * Shows how to write a simple server using the Ecore_Con library.
 */

EAPI Ecore_Con_Server *
ecore_con_server_add(Ecore_Con_Type compl_type,
                     const char *name,
                     int port,
                     const void *data)
{
   Ecore_Con_Server *obj;

   /* local  system socket: FILE:   /tmp/.ecore_service|[name]|[port] */
   /* remote system socket: TCP/IP: [name]:[port] */
   obj = eo_add(ECORE_CON_SERVER_CLASS, NULL,
         ecore_con_server_obj_connection_type_set(compl_type),
         ecore_con_server_obj_name_set(name),
         ecore_con_obj_port_set(port));

   ecore_con_server_data_set(obj, (void *) data);

   return obj;
}

EOLIAN static void
_ecore_con_server_eo_base_constructor(Ecore_Con_Server *obj, Ecore_Con_Server_Data *svr)
{
   eo_do_super(obj, ECORE_CON_SERVER_CLASS, eo_constructor());

   svr->fd = -1;
   svr->reject_excess_clients = EINA_FALSE;
   svr->client_limit = -1;
   svr->clients = NULL;
}

EOLIAN static Eo *
_ecore_con_server_eo_base_finalize(Ecore_Con_Server *obj, Ecore_Con_Server_Data *svr)
{
   Ecore_Con_Type compl_type = svr->type;
   Ecore_Con_Type type;

   eo_do_super(obj, ECORE_CON_SERVER_CLASS, eo_finalize());

   svr->created = EINA_TRUE;
   svr->ppid = getpid();
   svr->start_time = ecore_time_get();
   svr->use_cert = (svr->type & ECORE_CON_SSL & ECORE_CON_LOAD_CERT) == ECORE_CON_LOAD_CERT;

   servers = eina_list_append(servers, obj);

   if (!svr->name || (svr->port < 0))
     goto error;

   if (ecore_con_ssl_server_prepare(obj, compl_type & ECORE_CON_SSL))
     goto error;

   type = compl_type & ECORE_CON_TYPE;

   if ((type == ECORE_CON_LOCAL_USER) ||
       (type == ECORE_CON_LOCAL_SYSTEM) ||
       (type == ECORE_CON_LOCAL_ABSTRACT))
   /* Local */
#ifdef _WIN32
     if (!ecore_con_local_listen(obj))
       goto error;
#else
     if (!ecore_con_local_listen(obj, _ecore_con_svr_tcp_handler, obj))
       goto error;
#endif

   if ((type == ECORE_CON_REMOTE_TCP) ||
       (type == ECORE_CON_REMOTE_NODELAY) ||
       (type == ECORE_CON_REMOTE_CORK))
     {
        /* TCP */
        if (!ecore_con_info_tcp_listen(obj, _ecore_con_cb_tcp_listen,
                                       obj))
          goto error;
     }
   else if ((type == ECORE_CON_REMOTE_MCAST) ||
            (type == ECORE_CON_REMOTE_UDP))
     /* UDP and MCAST */
     if (!ecore_con_info_udp_listen(obj, _ecore_con_cb_udp_listen,
                                    obj))
       goto error;

   return obj;

error:
   if (svr->delete_me) return NULL;
   _ecore_con_server_kill(obj);
   return NULL;
}

EAPI Ecore_Con_Server *
ecore_con_server_connect(Ecore_Con_Type compl_type,
                         const char *name,
                         int port,
                         const void *data)
{
   Ecore_Con_Server *obj;
   /* local  user   socket: FILE:   ~/.ecore/[name]/[port] */
   /* local  system socket: FILE:   /tmp/.ecore_service|[name]|[port] */
   /* remote system socket: TCP/IP: [name]:[port] */
   obj = eo_add(ECORE_CON_CONNECTOR_CLASS, NULL,
         ecore_con_server_obj_connection_type_set(compl_type),
         ecore_con_server_obj_name_set(name),
         ecore_con_obj_port_set(port));

   ecore_con_server_data_set(obj, (void *) data);

   return obj;
}

EOLIAN static Eo *
_ecore_con_connector_eo_base_finalize(Ecore_Con_Server *obj, void *pd EINA_UNUSED)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   Ecore_Con_Type compl_type = svr->type;
   Ecore_Con_Type type;

   /* XXX: We intentionally put SERVER class here and not connector, as we'd
    * like to skip that one. */
   eo_do_super(obj, ECORE_CON_SERVER_CLASS, eo_finalize());

   svr->use_cert = (compl_type & ECORE_CON_SSL & ECORE_CON_LOAD_CERT) == ECORE_CON_LOAD_CERT;
   svr->disable_proxy = (compl_type & ECORE_CON_SUPER_SSL & ECORE_CON_NO_PROXY) == ECORE_CON_NO_PROXY;
   servers = eina_list_append(servers, obj);

   if (!svr->name || !(svr->name[0]))
     goto error;

   type = compl_type & ECORE_CON_TYPE;

   if ((!svr->disable_proxy) && (type > ECORE_CON_LOCAL_ABSTRACT))
     {
        /* never use proxies on local connections */
        if (_ecore_con_proxy_once)
          svr->ecs = _ecore_con_proxy_once;
        else if (_ecore_con_proxy_global)
          svr->ecs = _ecore_con_proxy_global;
        _ecore_con_proxy_once = NULL;
        if (svr->ecs)
          {
             if ((!svr->ecs->lookup) &&
                 (!ecore_con_lookup(svr->name, (Ecore_Con_Dns_Cb)ecore_con_socks_dns_cb, svr)))
               goto error;
             if (svr->ecs->lookup)
               svr->ecs_state = ECORE_CON_PROXY_STATE_RESOLVED;
          }
     }
   EINA_SAFETY_ON_TRUE_GOTO(ecore_con_ssl_server_prepare(obj, compl_type & ECORE_CON_SSL), error);

   EINA_SAFETY_ON_TRUE_GOTO(((type == ECORE_CON_REMOTE_TCP) ||
                             (type == ECORE_CON_REMOTE_NODELAY) ||
                             (type == ECORE_CON_REMOTE_CORK) ||
                             (type == ECORE_CON_REMOTE_UDP) ||
                             (type == ECORE_CON_REMOTE_BROADCAST)) &&
                            (svr->port < 0), error);

   if ((type == ECORE_CON_LOCAL_USER) ||
       (type == ECORE_CON_LOCAL_SYSTEM) ||
       (type == ECORE_CON_LOCAL_ABSTRACT))
   /* Local */
#ifdef _WIN32
     if (!ecore_con_local_connect(obj, _ecore_con_cl_handler)) goto error;
#else
     if (!ecore_con_local_connect(obj, _ecore_con_cl_handler, obj)) goto error;
#endif

   if ((type == ECORE_CON_REMOTE_TCP) ||
       (type == ECORE_CON_REMOTE_NODELAY) ||
       (type == ECORE_CON_REMOTE_CORK))
     {
        /* TCP */
        EINA_SAFETY_ON_FALSE_GOTO(ecore_con_info_tcp_connect(obj, _ecore_con_cb_tcp_connect, obj), error);
     }
   else if ((type == ECORE_CON_REMOTE_UDP) || (type == ECORE_CON_REMOTE_BROADCAST))
     /* UDP and MCAST */
     EINA_SAFETY_ON_FALSE_GOTO(ecore_con_info_udp_connect(obj, _ecore_con_cb_udp_connect, obj), error);

   return obj;

error:
   if (svr->delete_me) return NULL;
   _ecore_con_server_kill(obj);
   return NULL;
}

EAPI void
ecore_con_server_timeout_set(Ecore_Con *obj, double timeout)
{
   eo_do((Ecore_Con *)obj, ecore_con_obj_timeout_set(timeout));
}

EOLIAN static void
_ecore_con_server_ecore_con_base_timeout_set(Eo *obj, Ecore_Con_Server_Data *svr, double timeout)
{
   if (svr->created)
     svr->client_disconnect_time = timeout;
   else
     svr->disconnect_time = timeout;

   _ecore_con_server_timer_update(obj);
}

EAPI double
ecore_con_server_timeout_get(const Ecore_Con *obj)
{
   return eo_do((Ecore_Con *)obj, ecore_con_obj_timeout_get());
}

EOLIAN static double
_ecore_con_server_ecore_con_base_timeout_get(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr)
{
   return svr->created ? svr->client_disconnect_time : svr->disconnect_time;
}

EAPI void *
ecore_con_server_del(Ecore_Con_Server *obj)
{
   if (!obj) return NULL;
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);

   if (svr->delete_me)
     return NULL;

   _ecore_con_server_kill(obj);
   return svr->data;
}

EAPI void *
ecore_con_server_data_get(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr)
      return NULL;

   return svr->data;
}

EAPI void *
ecore_con_server_data_set(Ecore_Con_Server *obj,
                          void *data)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (!svr)
      return NULL;
   void *ret = NULL;

   ret = svr->data;
   svr->data = data;
   return ret;
}

EAPI Eina_Bool
ecore_con_server_connected_get(const Ecore_Con *obj)
{
   return eo_do((Ecore_Con *)obj, ecore_con_obj_connected_get());
}

EOLIAN static Eina_Bool
_ecore_con_server_ecore_con_base_connected_get(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr)
{
   return !svr->connecting;
}

EOLIAN const Eina_List *
_ecore_con_server_clients_get(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr)
{
   return svr->clients;
}

EOLIAN static void
_ecore_con_server_connection_type_set(Eo *obj, Ecore_Con_Server_Data *svr, Ecore_Con_Type type)
{
   EO_CONSTRUCTOR_CHECK_RETURN(obj);

   svr->type = type;
}

EOLIAN static Ecore_Con_Type
_ecore_con_server_connection_type_get(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr)
{
   return svr->type;
}

EOLIAN static void
_ecore_con_server_name_set(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr, const char *name)
{
   EO_CONSTRUCTOR_CHECK_RETURN(obj);

   if (svr->name)
      free(svr->name);

   svr->name = strdup(name);
}

EOLIAN static const char *
_ecore_con_server_name_get(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr)
{
   return svr->name;
}

EAPI int
ecore_con_server_port_get(const Ecore_Con *obj)
{
   return eo_do((Ecore_Con *)obj, ecore_con_obj_port_get());
}

EOLIAN static void
_ecore_con_server_ecore_con_base_port_set(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr, int port)
{
   EO_CONSTRUCTOR_CHECK_RETURN(obj);

   svr->port = port;
}

EOLIAN static int
_ecore_con_server_ecore_con_base_port_get(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr)
{
   return svr->port;
}

EAPI int
ecore_con_server_send(Ecore_Con *obj, const void *data, int size)
{
   return eo_do((Ecore_Con *)obj, ecore_con_obj_send(data, size));
}

EOLIAN static int
_ecore_con_server_ecore_con_base_send(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr, const void *data, int size)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(svr->delete_me, 0);

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, 0);

   EINA_SAFETY_ON_TRUE_RETURN_VAL(size < 1, 0);

   if (svr->fd_handler)
     ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);

   if (!svr->buf)
     {
        svr->buf = eina_binbuf_new();
        EINA_SAFETY_ON_NULL_RETURN_VAL(svr->buf, 0);
#ifdef TCP_CORK
        if ((svr->fd >= 0) && ((svr->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_CORK))
          {
             int state = 1;
             if (setsockopt(svr->fd, IPPROTO_TCP, TCP_CORK, (char *)&state, sizeof(int)) < 0)
               /* realistically this isn't anything serious so we can just log and continue */
               ERR("corking failed! %s", strerror(errno));
          }
#endif
     }
   if (!eina_binbuf_append_length(svr->buf, data, size))
     ERR("eina_binbuf_append_length() failed");

   return size;
}

EOLIAN static void
_ecore_con_server_client_limit_set(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr,
                                  int client_limit,
                                  char reject_excess_clients)
{
   svr->client_limit = client_limit;
   svr->reject_excess_clients = reject_excess_clients;
}

EOLIAN static void
_ecore_con_server_client_limit_get(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr,
                                  int *client_limit,
                                  char *reject_excess_clients)
{
   if (client_limit) *client_limit = svr->client_limit;
   if (reject_excess_clients) *reject_excess_clients = svr->reject_excess_clients;
}

EAPI const char *
ecore_con_server_ip_get(const Ecore_Con *obj)
{
   return eo_do(obj, ecore_con_obj_ip_get());
}

EOLIAN static const char *
_ecore_con_server_ecore_con_base_ip_get(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr)
{
   return svr->ip;
}

EAPI double
ecore_con_server_uptime_get(const Ecore_Con *obj)
{
   return eo_do(obj, ecore_con_obj_uptime_get());
}

EOLIAN static double
_ecore_con_server_ecore_con_base_uptime_get(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr)
{
   return ecore_time_get() - svr->start_time;
}

EAPI void
ecore_con_server_flush(Ecore_Con *obj)
{
   eo_do((Ecore_Con *)obj, ecore_con_obj_flush());
}

EOLIAN static void
_ecore_con_server_ecore_con_base_flush(Eo *obj, Ecore_Con_Server_Data *svr EINA_UNUSED)
{
   _ecore_con_server_flush(obj);
}

/**
 * @}
 */

/**
 * @addtogroup Ecore_Con_Client_Group Ecore Connection Client Functions
 *
 * Functions that operate on Ecore connection client objects.
 *
 * @{
 */

/**
 * @example ecore_con_client_example.c
 * Shows how to write a simple client that connects to the example server.
 */

EAPI int
ecore_con_client_send(Ecore_Con *obj, const void *data, int size)
{
   return eo_do((Ecore_Con *)obj, ecore_con_obj_send(data, size));
}

EOLIAN static int
_ecore_con_client_ecore_con_base_send(Eo *obj EINA_UNUSED, Ecore_Con_Client_Data *cl, const void *data, int size)
{
   Ecore_Con_Server_Data *host_server = NULL;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(cl->delete_me, 0);

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, 0);

   EINA_SAFETY_ON_TRUE_RETURN_VAL(size < 1, 0);

   if (cl->fd_handler)
     ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);

   if (cl->host_server)
      host_server = eo_data_scope_get(cl->host_server, ECORE_CON_CLIENT_CLASS);


   if (cl->host_server && ((host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_UDP))
     {
        int ret;

        ret = (int)sendto(host_server->fd, data, size, 0,
                          (struct sockaddr *)cl->client_addr,
                          cl->client_addr_len);
        return ret;
     }
   else
     {
        if (!cl->buf)
          {
             cl->buf = eina_binbuf_new();
             EINA_SAFETY_ON_NULL_RETURN_VAL(cl->buf, 0);
#ifdef TCP_CORK
             if ((cl->fd >= 0) && (host_server) &&
                 ((host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_CORK))
               {
                  int state = 1;
                  if (setsockopt(cl->fd, IPPROTO_TCP, TCP_CORK, (char *)&state, sizeof(int)) < 0)
                    /* realistically this isn't anything serious so we can just log and continue */
                    ERR("corking failed! %s", strerror(errno));
               }
#endif
          }
        if (!eina_binbuf_append_length(cl->buf, data, size))
          ERR("eina_binbuf_append_length() failed");
     }
   return size;
}

EOLIAN static Ecore_Con_Server *
_ecore_con_client_server_get(Eo *obj EINA_UNUSED, Ecore_Con_Client_Data *cl)
{
   return cl->host_server;
}

EOLIAN static Eina_Bool
_ecore_con_client_ecore_con_base_connected_get(Eo *obj EINA_UNUSED, Ecore_Con_Client_Data *cl)
{
   return !cl->delete_me;
}

EAPI Eina_Bool
ecore_con_client_connected_get(const Ecore_Con *obj)
{
   return eo_do((Ecore_Con *)obj, ecore_con_obj_connected_get());
}

EOLIAN static void
_ecore_con_client_ecore_con_base_timeout_set(Eo *obj, Ecore_Con_Client_Data *cl, double timeout)
{
   cl->disconnect_time = timeout;

   _ecore_con_cl_timer_update(obj);
}

EAPI void
ecore_con_client_timeout_set(Ecore_Con *obj, double timeout)
{
   eo_do((Ecore_Con *)obj, ecore_con_obj_timeout_set(timeout));
}

EOLIAN static double
_ecore_con_client_ecore_con_base_timeout_get(Eo *obj EINA_UNUSED, Ecore_Con_Client_Data *cl)
{
   return cl->disconnect_time;
}

EAPI double
ecore_con_client_timeout_get(const Ecore_Con *obj)
{
   return eo_do((Ecore_Con *)obj, ecore_con_obj_timeout_get());
}

EAPI void *
ecore_con_client_del(Ecore_Con_Client *obj)
{
   if (!obj) return NULL;
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   if (!cl) return NULL;

   _ecore_con_client_kill(obj);
   return cl->data;
}

EAPI void
ecore_con_client_data_set(Ecore_Con_Client *obj,
                          const void *data)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   if (!cl)
      return;

   cl->data = (void *)data;
}

EAPI void *
ecore_con_client_data_get(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   if (!cl)
      return NULL;

   return cl->data;
}

EOLIAN static const char *
_ecore_con_client_ecore_con_base_ip_get(Eo *obj EINA_UNUSED, Ecore_Con_Client_Data *cl)
{
   if (!cl->ip)
     cl->ip = _ecore_con_pretty_ip(cl->client_addr);

   return cl->ip;
}

EAPI const char *
ecore_con_client_ip_get(const Ecore_Con *obj)
{
   return eo_do(obj, ecore_con_obj_ip_get());
}

EOLIAN static int
_ecore_con_client_ecore_con_base_port_get(Eo *obj EINA_UNUSED, Ecore_Con_Client_Data *cl)
{
   if (cl->client_addr->sa_family == AF_INET)
     return ((struct sockaddr_in *)cl->client_addr)->sin_port;
#ifdef HAVE_IPV6
   return ((struct sockaddr_in6 *)cl->client_addr)->sin6_port;
#else
   return -1;
#endif
}

EAPI int
ecore_con_client_port_get(const Ecore_Con *obj)
{
   return eo_do((Ecore_Con *)obj, ecore_con_obj_port_get());
}

EOLIAN static double
_ecore_con_client_ecore_con_base_uptime_get(Eo *obj EINA_UNUSED, Ecore_Con_Client_Data *cl)
{
   return ecore_time_get() - cl->start_time;
}

EAPI double
ecore_con_client_uptime_get(const Ecore_Con *obj)
{
   return eo_do(obj, ecore_con_obj_uptime_get());
}

EOLIAN static void
_ecore_con_client_ecore_con_base_flush(Eo *obj, Ecore_Con_Client_Data *cl EINA_UNUSED)
{
   _ecore_con_client_flush(obj);
}

EAPI void
ecore_con_client_flush(Ecore_Con *obj)
{
   eo_do((Ecore_Con *)obj, ecore_con_obj_flush());
}

EAPI int
ecore_con_server_fd_get(const Ecore_Con *obj)
{
   return eo_do((Ecore_Con *)obj, ecore_con_obj_fd_get());
}

EOLIAN static int
_ecore_con_server_ecore_con_base_fd_get(Eo *obj EINA_UNUSED, Ecore_Con_Server_Data *svr)
{
   if (svr->created) return -1;
   if (svr->delete_me) return -1;
   return ecore_main_fd_handler_fd_get(svr->fd_handler);
}

EOLIAN static int
_ecore_con_client_ecore_con_base_fd_get(Eo *obj EINA_UNUSED, Ecore_Con_Client_Data *cl)
{
   return ecore_main_fd_handler_fd_get(cl->fd_handler);
}

EAPI int
ecore_con_client_fd_get(const Ecore_Con *obj)
{
   return eo_do((Ecore_Con *)obj, ecore_con_obj_fd_get());
}

/**
 * @}
 */

void
ecore_con_event_proxy_bind(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   Ecore_Con_Event_Proxy_Bind *e;
   int ev = ECORE_CON_EVENT_PROXY_BIND;

   e = ecore_con_event_proxy_bind_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   svr->event_count = eina_list_append(svr->event_count, e);
   _ecore_con_server_timer_update(obj);
   e->server = obj;
   e->ip = svr->proxyip;
   e->port = svr->proxyport;
   ecore_event_add(ev, e,
                   _ecore_con_event_server_add_free, NULL);
   _ecore_con_event_count++;
}

void
ecore_con_event_server_add(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   /* we got our server! */
   Ecore_Con_Event_Server_Add *e;
   int ev = ECORE_CON_EVENT_SERVER_ADD;

   e = ecore_con_event_server_add_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   svr->connecting = EINA_FALSE;
   svr->start_time = ecore_time_get();
   svr->event_count = eina_list_append(svr->event_count, e);
   _ecore_con_server_timer_update(obj);
   e->server = obj;
   if (svr->upgrade) ev = ECORE_CON_EVENT_SERVER_UPGRADE;
   ecore_event_add(ev, e,
                   _ecore_con_event_server_add_free, NULL);
   eo_do(obj, eo_event_callback_call(ECORE_CON_BASE_EVENT_CONNECTION_UPGRADED, NULL));
   _ecore_con_event_count++;
}

void
ecore_con_event_server_del(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   Ecore_Con_Event_Server_Del *e;

   svr->delete_me = EINA_TRUE;
   INF("svr %p is dead", svr);
   e = ecore_con_event_server_del_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   svr->event_count = eina_list_append(svr->event_count, e);
   _ecore_con_server_timer_update(obj);
   e->server = obj;
   if (svr->ecs)
     {
        svr->ecs_state = svr->ecs->lookup ? ECORE_CON_PROXY_STATE_RESOLVED : ECORE_CON_PROXY_STATE_DONE;
        eina_stringshare_replace(&svr->proxyip, NULL);
        svr->proxyport = 0;
     }
   ecore_event_add(ECORE_CON_EVENT_SERVER_DEL, e,
                   _ecore_con_event_server_del_free, NULL);
   _ecore_con_event_count++;
}

void
ecore_con_event_server_write(Ecore_Con_Server *obj, int num)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   Ecore_Con_Event_Server_Write *e;

   e = ecore_con_event_server_write_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   INF("Wrote %d bytes", num);
   svr->event_count = eina_list_append(svr->event_count, e);
   e->server = obj;
   e->size = num;
   ecore_event_add(ECORE_CON_EVENT_SERVER_WRITE, e,
                   (Ecore_End_Cb)_ecore_con_event_server_write_free, NULL);
   _ecore_con_event_count++;
}

void
ecore_con_event_server_data(Ecore_Con_Server *obj, unsigned char *buf, int num, Eina_Bool duplicate)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   Ecore_Con_Event_Server_Data *e;

   e = ecore_con_event_server_data_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   svr->event_count = eina_list_append(svr->event_count, e);
   _ecore_con_server_timer_update(obj);
   e->server = obj;
   if (duplicate)
     {
        e->data = malloc(num);
        if (!e->data)
          {
             ERR("server data allocation failure !");
             _ecore_con_event_server_data_free(NULL, e);
             return;
          }
        memcpy(e->data, buf, num);
     }
   else
     e->data = buf;
   e->size = num;
   ecore_event_add(ECORE_CON_EVENT_SERVER_DATA, e,
                   _ecore_con_event_server_data_free, NULL);
     {
        Ecore_Con_Event_Data_Received event_info = { 0 };
        event_info.data = e->data;
        event_info.size = e->size;
        eo_do(obj, eo_event_callback_call(ECORE_CON_BASE_EVENT_DATA_RECEIVED, &event_info));
     }
   _ecore_con_event_count++;
}

void
ecore_con_event_client_add(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   Ecore_Con_Event_Client_Add *e;
   int ev = ECORE_CON_EVENT_CLIENT_ADD;

   e = ecore_con_event_client_add_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);

   cl->event_count = eina_list_append(cl->event_count, e);
   host_server->event_count = eina_list_append(host_server->event_count, e);
   _ecore_con_cl_timer_update(obj);
   cl->start_time = ecore_time_get();
   e->client = obj;
   if (cl->upgrade) ev = ECORE_CON_EVENT_CLIENT_UPGRADE;
   ecore_event_add(ev, e,
                   (Ecore_End_Cb)_ecore_con_event_client_add_free, cl->host_server);
   eo_do(obj, eo_event_callback_call(ECORE_CON_BASE_EVENT_CONNECTION_UPGRADED, NULL));
   _ecore_con_event_count++;
}

void
ecore_con_event_client_del(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   Ecore_Con_Event_Client_Del *e;

   if (!cl) return;
   cl->delete_me = EINA_TRUE;
   INF("cl %p is dead", cl);
   e = ecore_con_event_client_del_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);
   cl->event_count = eina_list_append(cl->event_count, e);

   Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);

   host_server->event_count = eina_list_append(host_server->event_count, e);
   _ecore_con_cl_timer_update(obj);
   e->client = obj;
   ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, e,
                   (Ecore_End_Cb)_ecore_con_event_client_del_free, cl->host_server);
   _ecore_con_event_count++;
}

void
ecore_con_event_client_write(Ecore_Con_Client *obj, int num)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   Ecore_Con_Event_Client_Write *e;

   e = ecore_con_event_client_write_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);

   cl->event_count = eina_list_append(cl->event_count, e);
   host_server->event_count = eina_list_append(host_server->event_count, e);
   e->client = obj;
   e->size = num;
   ecore_event_add(ECORE_CON_EVENT_CLIENT_WRITE, e,
                   (Ecore_End_Cb)_ecore_con_event_client_write_free, cl->host_server);
   _ecore_con_event_count++;
}

void
ecore_con_event_client_data(Ecore_Con_Client *obj, unsigned char *buf, int num, Eina_Bool duplicate)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   Ecore_Con_Event_Client_Data *e;

   e = ecore_con_event_client_data_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);

   cl->event_count = eina_list_append(cl->event_count, e);
   host_server->event_count = eina_list_append(host_server->event_count, e);
   _ecore_con_cl_timer_update(obj);
   e->client = obj;
   if ((duplicate) && (num > 0))
     {
        e->data = malloc(num);
        if (!e->data)
          {
             ERR("client data allocation failure !");
             _ecore_con_event_client_data_free(cl->host_server, e);
             return;
          }
        memcpy(e->data, buf, num);
     }
   else
     e->data = buf;
   e->size = num;
   ecore_event_add(ECORE_CON_EVENT_CLIENT_DATA, e,
                   (Ecore_End_Cb)_ecore_con_event_client_data_free, cl->host_server);
     {
        Ecore_Con_Event_Data_Received event_info = { 0 };
        event_info.data = e->data;
        event_info.size = e->size;
        eo_do(obj, eo_event_callback_call(ECORE_CON_BASE_EVENT_DATA_RECEIVED, &event_info));
     }
   _ecore_con_event_count++;
}

void
ecore_con_server_infos_del(Ecore_Con_Server *obj, void *info)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   svr->infos = eina_list_remove(svr->infos, info);
}

void
_ecore_con_event_server_error(Ecore_Con_Server *obj, char *error, Eina_Bool duplicate)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   Ecore_Con_Event_Server_Error *e;

   e = ecore_con_event_server_error_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->server = obj;
   e->error = duplicate ? strdup(error) : error;
   DBG("%s", error);
   svr->event_count = eina_list_append(svr->event_count, e);
   ecore_event_add(ECORE_CON_EVENT_SERVER_ERROR, e, (Ecore_End_Cb)_ecore_con_event_server_error_free, NULL);
   eo_do(obj, eo_event_callback_call(ECORE_CON_BASE_EVENT_CONNECTION_ERROR, e->error));
   _ecore_con_event_count++;
}

void
ecore_con_event_client_error(Ecore_Con_Client *obj, const char *error)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   Ecore_Con_Event_Client_Error *e;

   e = ecore_con_event_client_error_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);

   e->client = obj;
   e->error = strdup(error);
   DBG("%s", error);
   cl->event_count = eina_list_append(cl->event_count, e);
   host_server->event_count = eina_list_append(host_server->event_count, e);
   ecore_event_add(ECORE_CON_EVENT_CLIENT_ERROR, e, (Ecore_End_Cb)_ecore_con_event_client_error_free, cl->host_server);
   eo_do(obj, eo_event_callback_call(ECORE_CON_BASE_EVENT_CONNECTION_ERROR, e->error));
   _ecore_con_event_count++;
}

static void
_ecore_con_server_free(Ecore_Con_Server *obj)
{
   eo_del(obj);
}

EOLIAN static void
_ecore_con_server_eo_base_destructor(Eo *obj, Ecore_Con_Server_Data *svr)
{
   Ecore_Con_Client *cl_obj;
   double t_start, t;

   if (svr->event_count) goto end;

   while (svr->infos)
     {
        ecore_con_info_data_clear(svr->infos->data);
        svr->infos = eina_list_remove_list(svr->infos, svr->infos);
     }

   t_start = ecore_time_get();
   while (svr->buf && (!svr->delete_me))
     {
        _ecore_con_server_flush(obj);
        t = ecore_time_get();
        if ((t - t_start) > 0.5)
          {
             WRN("ECORE_CON: EEK - stuck in _ecore_con_server_free() trying\n"
                 "  to flush data out from the server, and have been for\n"
                 "  %1.1f seconds. This is taking too long. Aborting flush.",
                 (t - t_start));
             break;
          }
     }

#ifdef _WIN32
   ecore_con_local_win32_server_del(obj);
#endif
   if (svr->event_count) goto end;

   if (svr->buf)
     eina_binbuf_free(svr->buf);

   EINA_LIST_FREE(svr->clients, cl_obj)
     {
        Ecore_Con_Client_Data *cl = eo_data_scope_get(cl_obj, ECORE_CON_CLIENT_CLASS);
        Ecore_Con_Event_Server_Add *ev;

        /* some pointer hacks here to prevent double frees if people are being stupid */
        EINA_LIST_FREE(cl->event_count, ev)
          ev->server = NULL;
        cl->delete_me = EINA_TRUE;
        INF("cl %p is dead", cl);
        _ecore_con_client_free(cl_obj);
     }
   if ((svr->created) && (svr->path) && (svr->ppid == getpid()))
     unlink(svr->path);

   ecore_con_ssl_server_shutdown(obj);
   free(svr->name);

   free(svr->path);

   eina_stringshare_del(svr->ip);
   eina_stringshare_del(svr->verify_name);

   if (svr->ecs_buf) eina_binbuf_free(svr->ecs_buf);
   if (svr->ecs_recvbuf) eina_binbuf_free(svr->ecs_recvbuf);

   if (svr->fd_handler)
     ecore_main_fd_handler_del(svr->fd_handler);

   if (svr->fd >= 0)
     close(svr->fd);

   if (svr->until_deletion)
     ecore_timer_del(svr->until_deletion);

   servers = eina_list_remove(servers, obj);
   svr->data = NULL;

end:
   eo_do_super(obj, ECORE_CON_SERVER_CLASS, eo_destructor());
}

static void
_ecore_con_client_free(Ecore_Con_Client *obj)
{
   eo_del(obj);
}

EOLIAN static void
_ecore_con_client_eo_base_destructor(Eo *obj, Ecore_Con_Client_Data *cl)
{
   double t_start, t;

   if (cl->event_count) return;

   t_start = ecore_time_get();
   while ((cl->buf) && (!cl->delete_me))
     {
        _ecore_con_client_flush(obj);
        t = ecore_time_get();
        if ((t - t_start) > 0.5)
          {
             WRN("EEK - stuck in _ecore_con_client_free() trying\n"
                 "  to flush data out from the client, and have been for\n"
                 "  %1.1f seconds. This is taking too long. Aborting flush.",
                 (t - t_start));
             break;
          }
     }
   Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);

   host_server->clients = eina_list_remove(host_server->clients, obj);
   --host_server->client_count;

#ifdef _WIN32
   ecore_con_local_win32_client_del(obj);
#endif

   if (cl->event_count) return;

   if (cl->buf) eina_binbuf_free(cl->buf);

   if (host_server->type & ECORE_CON_SSL)
     ecore_con_ssl_client_shutdown(obj);

   if (cl->fd_handler)
     ecore_main_fd_handler_del(cl->fd_handler);

   if (cl->fd >= 0)
     close(cl->fd);

   free(cl->client_addr);
   cl->client_addr = NULL;

   if (cl->until_deletion)
     ecore_timer_del(cl->until_deletion);

   eina_stringshare_del(cl->ip);
   cl->data = NULL;

   eo_do_super(obj, ECORE_CON_CLIENT_CLASS, eo_destructor());
}

static Eina_Bool
_ecore_con_server_timer(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   ecore_con_server_del(obj);

   svr->until_deletion = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_con_server_timer_update(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (svr->disconnect_time)
     {
        if (svr->disconnect_time > 0)
          {
             if (svr->until_deletion)
               {
                  ecore_timer_interval_set(svr->until_deletion, svr->disconnect_time);
                  ecore_timer_reset(svr->until_deletion);
               }
             else
               svr->until_deletion = ecore_timer_add(svr->disconnect_time, (Ecore_Task_Cb)_ecore_con_server_timer, obj);
          }
        else if (svr->until_deletion)
          {
             ecore_timer_del(svr->until_deletion);
             svr->until_deletion = NULL;
          }
     }
   else
     {
        if (svr->until_deletion)
          {
             ecore_timer_del(svr->until_deletion);
             svr->until_deletion = NULL;
          }
     }
}

static Eina_Bool
_ecore_con_client_timer(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   ecore_con_client_del(obj);

   cl->until_deletion = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_con_cl_timer_update(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   if (cl->disconnect_time)
     {
        if (cl->disconnect_time > 0)
          {
             if (cl->until_deletion)
               {
                  ecore_timer_interval_set(cl->until_deletion, cl->disconnect_time);
                  ecore_timer_reset(cl->until_deletion);
               }
             else
               cl->until_deletion = ecore_timer_add(cl->disconnect_time, (Ecore_Task_Cb)_ecore_con_client_timer, cl);
          }
        else if (cl->until_deletion)
          {
             ecore_timer_del(cl->until_deletion);
             cl->until_deletion = NULL;
          }
     }
   else
     {
        Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);

        if (host_server->client_disconnect_time > 0)
          {
             if (cl->until_deletion)
               {
                  ecore_timer_interval_set(cl->until_deletion, host_server->client_disconnect_time);
                  ecore_timer_reset(cl->until_deletion);
               }
             else
               cl->until_deletion = ecore_timer_add(host_server->client_disconnect_time, (Ecore_Task_Cb)_ecore_con_client_timer, cl);
          }
        else if (cl->until_deletion)
          {
             ecore_timer_del(cl->until_deletion);
             cl->until_deletion = NULL;
          }
     }
}

static void
_ecore_con_cb_tcp_listen(void *data,
                         Ecore_Con_Info *net_info)
{
   Ecore_Con_Server *obj = data;
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   struct linger lin;
   const char *memerr = NULL;

   errno = 0;
   if (!net_info) /* error message has already been handled */
     {
        svr->delete_me = EINA_TRUE;
        goto error;
     }

#ifdef HAVE_SYSTEMD
   if (svr->type & ECORE_CON_SOCKET_ACTIVATE && sd_fd_index < sd_fd_max)
     {
        if (sd_is_socket_inet(SD_LISTEN_FDS_START + sd_fd_index,
                              net_info->info.ai_family,
                              net_info->info.ai_socktype,
                              1,
                              svr->port) <= 0)
          {
             ERR("Your systemd unit seems to provide fd in the wrong order for Socket activation.");
             goto error;
          }

        svr->fd = SD_LISTEN_FDS_START + sd_fd_index++;

        /* I am wondering if that's really going to work as the bind is already done */
        if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;

        lin.l_onoff = 1;
        lin.l_linger = 0;
        if (setsockopt(svr->fd, SOL_SOCKET, SO_LINGER, (const void *)&lin,
                       sizeof(struct linger)) < 0)
          goto error;

        if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_NODELAY)
          {
             int flag = 1;

             if (setsockopt(svr->fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag,
                            sizeof(int)) < 0)
               {
                  goto error;
               }
          }

        goto fd_ready;
     }
#endif

   svr->fd = socket(net_info->info.ai_family, net_info->info.ai_socktype,
                    net_info->info.ai_protocol);
   if (svr->fd < 0) goto error;
   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;

   lin.l_onoff = 1;
   lin.l_linger = 0;
   if (setsockopt(svr->fd, SOL_SOCKET, SO_LINGER, (const void *)&lin,
                  sizeof(struct linger)) < 0)
     goto error;

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_NODELAY)
     {
        int flag = 1;

        if (setsockopt(svr->fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag,
                       sizeof(int)) < 0)
          {
             goto error;
          }
     }

   if (bind(svr->fd, net_info->info.ai_addr, net_info->info.ai_addrlen) < 0)
     goto error;

   if (listen(svr->fd, 4096) < 0) goto error;

#ifdef HAVE_SYSTEMD
fd_ready:
#endif
   svr->fd_handler = ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ,
                                               _ecore_con_svr_tcp_handler, obj, NULL, NULL);
   if (!svr->fd_handler)
     {
        memerr = "Memory allocation failure";
        goto error;
     }

   return;

error:
   if (errno || memerr) ecore_con_event_server_error(obj, memerr ? : strerror(errno));
   ecore_con_ssl_server_shutdown(obj);
   _ecore_con_server_kill(obj);
}

static void
_ecore_con_cb_udp_listen(void *data,
                         Ecore_Con_Info *net_info)
{
   Ecore_Con_Server *obj = data;
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   Ecore_Con_Type type;
   struct ip_mreq mreq;
#ifdef HAVE_IPV6
   struct ipv6_mreq mreq6;
#endif
   const int on = 1;
   const char *memerr = NULL;

   type = svr->type;
   type &= ECORE_CON_TYPE;

   errno = 0;
   if (!net_info) /* error message has already been handled */
     {
        svr->delete_me = EINA_TRUE;
        goto error;
     }
#ifdef HAVE_SYSTEMD
   if (svr->type & ECORE_CON_SOCKET_ACTIVATE && sd_fd_index < sd_fd_max)
     {
        if (sd_is_socket_inet(SD_LISTEN_FDS_START + sd_fd_index,
                              net_info->info.ai_family,
                              net_info->info.ai_socktype,
                              -1,
                              svr->port) <= 0)
          {
             ERR("Your systemd unit seems to provide fd in the wrong order for Socket activation.");
             goto error;
          }
        svr->fd = SD_LISTEN_FDS_START + sd_fd_index++;

        if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&on, sizeof(on)) != 0)
          goto error;
        if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;

        goto fd_ready;
     }
#endif
   svr->fd = socket(net_info->info.ai_family, net_info->info.ai_socktype,
                    net_info->info.ai_protocol);
   if (svr->fd < 0) goto error;

   if (type == ECORE_CON_REMOTE_MCAST)
     {
        if (net_info->info.ai_family == AF_INET)
          {
             if (!inet_pton(net_info->info.ai_family, net_info->ip,
                            &mreq.imr_multiaddr))
               goto error;

             mreq.imr_interface.s_addr = htonl(INADDR_ANY);
             if (setsockopt(svr->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                            (const void *)&mreq, sizeof(mreq)) != 0)
               goto error;
          }
#ifdef HAVE_IPV6
        else if (net_info->info.ai_family == AF_INET6)
          {
             if (!inet_pton(net_info->info.ai_family, net_info->ip,
                            &mreq6.ipv6mr_multiaddr))
               goto error;
             mreq6.ipv6mr_interface = htonl(INADDR_ANY);
             if (setsockopt(svr->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                            (const void *)&mreq6, sizeof(mreq6)) != 0)
               goto error;
          }
#endif
     }

   if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&on, sizeof(on)) != 0)
     goto error;
   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;

   if (bind(svr->fd, net_info->info.ai_addr, net_info->info.ai_addrlen) < 0)
     goto error;

#ifdef HAVE_SYSTEMD
fd_ready:
#endif
   svr->fd_handler =
     ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ,
                               _ecore_con_svr_udp_handler, obj, NULL, NULL);
   if (!svr->fd_handler)
     {
        memerr = "Memory allocation failure";
        goto error;
     }

   svr->ip = eina_stringshare_add(net_info->ip);

   return;

error:
   if (errno || memerr) ecore_con_event_server_error(obj, memerr ? : strerror(errno));
   ecore_con_ssl_server_shutdown(obj);
   _ecore_con_server_kill(obj);
}

static void
_ecore_con_cb_tcp_connect(void *data,
                          Ecore_Con_Info *net_info)
{
   Ecore_Con_Server *obj = data;
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   int res;
   int curstate = 0;
   const char *memerr = NULL;

   errno = 0;
   if (!net_info) /* error message has already been handled */
     {
        svr->delete_me = EINA_TRUE;
        goto error;
     }

   svr->fd = socket(net_info->info.ai_family, net_info->info.ai_socktype,
                    net_info->info.ai_protocol);
   if (svr->fd < 0) goto error;

   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;

   if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&curstate, sizeof(curstate)) < 0)
     goto error;

   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_NODELAY)
     {
        int flag = 1;

        if (setsockopt(svr->fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)) < 0)
          {
             goto error;
          }
     }

   res = connect(svr->fd, net_info->info.ai_addr, net_info->info.ai_addrlen);
#ifdef _WIN32
   if (res == SOCKET_ERROR)
     {
        if (WSAGetLastError() != WSAEINPROGRESS)
          {
             char *err;
             err = evil_format_message(WSAGetLastError());
             _ecore_con_event_server_error(obj, err, EINA_FALSE);
             ecore_con_ssl_server_shutdown(obj);
             _ecore_con_server_kill(obj);
             return;
          }

#else
   if (res < 0)
     {
        if (errno != EINPROGRESS) goto error;
#endif
        svr->connecting = EINA_TRUE;
        svr->fd_handler =
          ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ | ECORE_FD_WRITE,
                                    _ecore_con_cl_handler, obj, NULL, NULL);
     }
   else
     svr->fd_handler = ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ,
                                                 _ecore_con_cl_handler, obj, NULL, NULL);

   if (svr->type & ECORE_CON_SSL)
     {
        svr->handshaking = EINA_TRUE;
        svr->ssl_state = ECORE_CON_SSL_STATE_INIT;
        DBG("%s ssl handshake", svr->ecs_state ? "Queuing" : "Beginning");
        if ((!svr->ecs_state) && ecore_con_ssl_server_init(obj))
          goto error;
     }

   if (!svr->fd_handler)
     {
        memerr = "Memory allocation failure";
        goto error;
     }

   if ((!svr->ecs) || (svr->ecs->lookup))
     svr->ip = eina_stringshare_add(net_info->ip);

   return;

error:
   ecore_con_event_server_error(obj,
                                memerr ? : errno ? strerror(errno) : "DNS error");
   ecore_con_ssl_server_shutdown(obj);
   _ecore_con_server_kill(obj);
}

static void
_ecore_con_cb_udp_connect(void *data,
                          Ecore_Con_Info *net_info)
{
   Ecore_Con_Server *obj = data;
   int curstate = 0;
   int broadcast = 1;
   const char *memerr = NULL;
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);

   errno = 0;
   if (!net_info) /* error message has already been handled */
     {
        svr->delete_me = EINA_TRUE;
        goto error;
     }

   svr->fd = socket(net_info->info.ai_family, net_info->info.ai_socktype,
                    net_info->info.ai_protocol);
   if (svr->fd < 0) goto error;
   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
   if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_BROADCAST)
     {
        if (setsockopt(svr->fd, SOL_SOCKET, SO_BROADCAST,
                       (const void *)&broadcast,
                       sizeof(broadcast)) < 0)
          {
             goto error;
          }
     }
   if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR,
                  (const void *)&curstate, sizeof(curstate)) < 0)
     goto error;

   if (connect(svr->fd, net_info->info.ai_addr, net_info->info.ai_addrlen) < 0)
     goto error;

   svr->fd_handler = ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ | ECORE_FD_WRITE,
                                               _ecore_con_cl_udp_handler, obj, NULL, NULL);

   if (!svr->fd_handler)
     {
        memerr = "Memory allocation failure";
        goto error;
     }

   if ((!svr->ecs) || (svr->ecs->lookup))
     svr->ip = eina_stringshare_add(net_info->ip);

   return;

error:
   if (errno || memerr) ecore_con_event_server_error(obj, memerr ? : strerror(errno));
   ecore_con_ssl_server_shutdown(obj);
   _ecore_con_server_kill(obj);
}

static Ecore_Con_State
svr_try_connect_plain(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   int res;
   int so_err = 0;
   socklen_t size = sizeof(int);

   res = getsockopt(svr->fd, SOL_SOCKET, SO_ERROR, (void *)&so_err, &size);
#ifdef _WIN32
   if (res == SOCKET_ERROR)
     so_err = WSAGetLastError();

   if ((so_err == WSAEINPROGRESS) && !svr->delete_me)
     return ECORE_CON_INPROGRESS;

#else
   if (res < 0)
     so_err = errno;

   if ((so_err == EINPROGRESS) && !svr->delete_me)
     return ECORE_CON_INPROGRESS;

#endif

   if (so_err)
     {
        /* we lost our server! */
        ecore_con_event_server_error(obj, strerror(so_err));
        ERR("Connection lost: %s", strerror(so_err));
        _ecore_con_server_kill(obj);
        return ECORE_CON_DISCONNECTED;
     }

   if ((!svr->delete_me) && (!svr->handshaking) && svr->connecting)
     {
        if (svr->ecs)
          {
             if (ecore_con_socks_svr_init(obj))
               return ECORE_CON_INPROGRESS;
          }
        else
          ecore_con_event_server_add(obj);
     }

   if (svr->fd_handler)
     {
        if (svr->buf)
          ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
        else
          ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
     }

   if (!svr->delete_me)
     return ECORE_CON_CONNECTED;
   else
     return ECORE_CON_DISCONNECTED;
}

static const char *
_ecore_con_pretty_ip(struct sockaddr *client_addr)
{
#ifndef HAVE_IPV6
   char ipbuf[INET_ADDRSTRLEN + 1];
#else
   char ipbuf[INET6_ADDRSTRLEN + 1];
#endif
   int family = client_addr->sa_family;
   void *src;

   switch (family)
     {
      case AF_INET:
        src = &(((struct sockaddr_in *)client_addr)->sin_addr);
        break;

#ifdef HAVE_IPV6
      case AF_INET6:
        src = &(((struct sockaddr_in6 *)client_addr)->sin6_addr);

        if (IN6_IS_ADDR_V4MAPPED((struct in6_addr *)src))
          {
             family = AF_INET;
             src = (char *)src + 12;
          }
        break;

#endif
      default:
        return eina_stringshare_add("0.0.0.0");
     }

   if (!inet_ntop(family, src, ipbuf, sizeof(ipbuf)))
     return eina_stringshare_add("0.0.0.0");

   ipbuf[sizeof(ipbuf) - 1] = 0;
   return eina_stringshare_add(ipbuf);
}

static Eina_Bool
_ecore_con_svr_tcp_handler(void *data,
                           Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   Ecore_Con_Server *svr_obj = data;
   Ecore_Con_Client *obj = NULL;
   unsigned char client_addr[256];
   unsigned int client_addr_len;
   const char *clerr = NULL;
   Ecore_Con_Server_Data *svr = eo_data_scope_get(svr_obj, ECORE_CON_SERVER_CLASS);

   if (svr->delete_me)
     return ECORE_CALLBACK_RENEW;

   if ((svr->client_limit >= 0) && (!svr->reject_excess_clients) &&
       (svr->client_count >= (unsigned int)svr->client_limit))
     return ECORE_CALLBACK_RENEW;

   /* a new client */

   obj = eo_add(ECORE_CON_CLIENT_CLASS, NULL);
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   if (!cl)
     {
        ecore_con_event_server_error(svr_obj, "Memory allocation failure when attempting to add a new client");
        return ECORE_CALLBACK_RENEW;
     }
   cl->host_server = svr_obj;

   client_addr_len = sizeof(client_addr);
   memset(&client_addr, 0, client_addr_len);
   cl->fd = accept(svr->fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
   if (cl->fd < 0) goto error;
   if ((svr->client_limit >= 0) && (svr->reject_excess_clients) &&
       (svr->client_count >= (unsigned int)svr->client_limit))
     {
        clerr = "Maximum client limit reached";
        goto error;
     }

   if (fcntl(cl->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(cl->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
   cl->fd_handler = ecore_main_fd_handler_add(cl->fd, ECORE_FD_READ,
                                              _ecore_con_svr_cl_handler, obj, NULL, NULL);
   if (!cl->fd_handler) goto error;

   if ((!svr->upgrade) && (svr->type & ECORE_CON_SSL))
     {
        cl->handshaking = EINA_TRUE;
        cl->ssl_state = ECORE_CON_SSL_STATE_INIT;
        if (ecore_con_ssl_client_init(obj))
          goto error;
     }

   cl->client_addr = malloc(client_addr_len);
   if (!cl->client_addr)
     {
        clerr = "Memory allocation failure when attempting to add a new client";
        goto error;
     }
   cl->client_addr_len = client_addr_len;
   memcpy(cl->client_addr, &client_addr, client_addr_len);

   svr->clients = eina_list_append(svr->clients, obj);
   svr->client_count++;

   if ((!cl->delete_me) && (!cl->handshaking))
     ecore_con_event_client_add(obj);

   return ECORE_CALLBACK_RENEW;

error:
   if (cl->fd_handler) ecore_main_fd_handler_del(cl->fd_handler);
   if (cl->fd >= 0) close(cl->fd);
   {
      Ecore_Event *ev;

      EINA_LIST_FREE(cl->event_count, ev)
        {
           svr->event_count = eina_list_remove(svr->event_count, ev);
           ecore_event_del(ev);
        }
   }
   eo_del(obj);
   if (clerr || errno) ecore_con_event_server_error(svr_obj, clerr ? : strerror(errno));
   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_con_cl_read(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   int num = 0;
   Eina_Bool lost_server = EINA_TRUE;
   unsigned char buf[READBUFSIZ];

   DBG("svr=%p", svr);

   /* only possible with non-ssl connections */
   if (svr->connecting && (svr_try_connect_plain(obj) != ECORE_CON_CONNECTED))
     return;

   if (svr->handshaking && (!svr->ecs_state))
     {
        DBG("Continuing ssl handshake");
        if (!ecore_con_ssl_server_init(obj))
          lost_server = EINA_FALSE;
        _ecore_con_server_timer_update(obj);
     }

   if (svr->ecs_state || !(svr->type & ECORE_CON_SSL))
     {
        errno = 0;
        num = read(svr->fd, buf, sizeof(buf));
        /* 0 is not a valid return value for a tcp socket */
        if ((num > 0) || ((num < 0) && (errno == EAGAIN)))
          lost_server = EINA_FALSE;
        else if (num < 0)
          ecore_con_event_server_error(obj, strerror(errno));
     }
   else
     {
        num = ecore_con_ssl_server_read(obj, buf, sizeof(buf));
        /* this is not an actual 0 return, 0 here just means non-fatal error such as EAGAIN */
        if (num >= 0)
          lost_server = EINA_FALSE;
     }

   if ((!svr->delete_me) && (num > 0))
     {
        if (svr->ecs_state)
          ecore_con_socks_read(obj, buf, num);
        else
          ecore_con_event_server_data(obj, buf, num, EINA_TRUE);
     }

   if (lost_server)
     _ecore_con_server_kill(obj);
}

static Eina_Bool
_ecore_con_cl_handler(void *data,
                      Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Server *obj = data;
   Eina_Bool want_read, want_write;
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);

   if (svr->delete_me)
     return ECORE_CALLBACK_RENEW;

   if (svr->delete_me)
     return ECORE_CALLBACK_RENEW;

   want_read = ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ);
   want_write = ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE);

   if ((!svr->ecs_state) && svr->handshaking && (want_read || want_write))
     {
        DBG("Continuing ssl handshake: preparing to %s...", want_read ? "read" : "write");
#ifdef ISCOMFITOR
        if (want_read)
          {
             char buf[READBUFSIZ];
             ssize_t len;
             len = recv(svr->fd, buf, sizeof(buf), MSG_DONTWAIT | MSG_PEEK);
             DBG("%zu bytes in buffer", len);
          }
#endif
        if (ecore_con_ssl_server_init(obj))
          {
             ERR("ssl handshaking failed!");
             svr->handshaking = EINA_FALSE;
          }
        else if (!svr->ssl_state)
          ecore_con_event_server_add(obj);
        return ECORE_CALLBACK_RENEW;
     }
   if (svr->ecs && svr->ecs_state && (svr->ecs_state < ECORE_CON_PROXY_STATE_READ) && (!svr->ecs_buf))
     {
        if (svr->ecs_state < ECORE_CON_PROXY_STATE_INIT)
          {
             INF("PROXY STATE++");
             svr->ecs_state++;
          }
        if (ecore_con_socks_svr_init(obj)) return ECORE_CALLBACK_RENEW;
     }
   if (want_read)
     _ecore_con_cl_read(obj);
   else if (want_write) /* only possible with non-ssl connections */
     {
        if (svr->connecting && (!svr_try_connect_plain(obj)) && (!svr->ecs_state))
          return ECORE_CALLBACK_RENEW;

        _ecore_con_server_flush(obj);
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_con_cl_udp_handler(void *data,
                          Ecore_Fd_Handler *fd_handler)
{
   unsigned char buf[READBUFSIZ];
   int num;
   Ecore_Con_Server *obj = data;
   Eina_Bool want_read, want_write;
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);

   want_read = ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ);
   want_write = ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE);

   if (svr->delete_me || ((!want_read) && (!want_write)))
     return ECORE_CALLBACK_RENEW;

   if (want_write)
     {
        _ecore_con_server_flush(obj);
        return ECORE_CALLBACK_RENEW;
     }

   num = read(svr->fd, buf, READBUFSIZ);

   if ((!svr->delete_me) && (num > 0))
     ecore_con_event_server_data(obj, buf, num, EINA_TRUE);

   if (num < 0 && (errno != EAGAIN) && (errno != EINTR))
     {
        ecore_con_event_server_error(obj, strerror(errno));
        _ecore_con_server_kill(obj);
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_con_svr_udp_handler(void *data,
                           Ecore_Fd_Handler *fd_handler)
{
   unsigned char buf[READBUFSIZ];
   unsigned char client_addr[256];
   socklen_t client_addr_len = sizeof(client_addr);
   int num;
   Ecore_Con_Server *svr_obj = data;
   Ecore_Con_Client *obj = NULL;

   Ecore_Con_Server_Data *svr = eo_data_scope_get(svr_obj, ECORE_CON_SERVER_CLASS);
   if (svr->delete_me)
     return ECORE_CALLBACK_RENEW;

   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     return ECORE_CALLBACK_RENEW;

   if (!ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     return ECORE_CALLBACK_RENEW;

#ifdef _WIN32
   num = fcntl(svr->fd, F_SETFL, O_NONBLOCK);
   if (num >= 0)
     num = recvfrom(svr->fd, (char *)buf, sizeof(buf), 0,
                    (struct sockaddr *)&client_addr,
                    &client_addr_len);

#else
   num = recvfrom(svr->fd, buf, sizeof(buf), MSG_DONTWAIT,
                  (struct sockaddr *)&client_addr,
                  &client_addr_len);
#endif

   if (num < 0 && (errno != EAGAIN) && (errno != EINTR))
     {
        ecore_con_event_server_error(svr_obj, strerror(errno));
        if (!svr->delete_me)
          ecore_con_event_client_del(NULL);
        _ecore_con_server_kill(svr_obj);
        return ECORE_CALLBACK_CANCEL;
     }

/* Create a new client for use in the client data event */
   obj = eo_add(ECORE_CON_CLIENT_CLASS, NULL);
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, ECORE_CALLBACK_RENEW);

   cl->host_server = svr_obj;
   cl->client_addr = malloc(client_addr_len);
   if (!cl->client_addr)
     {
        free(cl);
        return ECORE_CALLBACK_RENEW;
     }
   cl->client_addr_len = client_addr_len;

   memcpy(cl->client_addr, &client_addr, client_addr_len);
   svr->clients = eina_list_append(svr->clients, obj);
   svr->client_count++;

   ecore_con_event_client_add(obj);
   ecore_con_event_client_data(obj, buf, num, EINA_TRUE);

   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_con_svr_cl_read(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   int num = 0;
   Eina_Bool lost_client = EINA_TRUE;
   unsigned char buf[READBUFSIZ];

   DBG("cl=%p", cl);

   if (cl->handshaking)
     {
        /* add an extra handshake attempt just before read, even though
         * read also attempts to handshake, to try to finish sooner
         */
        if (ecore_con_ssl_client_init(obj))
          lost_client = EINA_FALSE;

        _ecore_con_cl_timer_update(obj);
     }

   Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
   if (!(host_server->type & ECORE_CON_SSL) && (!cl->upgrade))
     {
        num = read(cl->fd, buf, sizeof(buf));
        /* 0 is not a valid return value for a tcp socket */
        if ((num > 0) || ((num < 0) && ((errno == EAGAIN) || (errno == EINTR))))
          lost_client = EINA_FALSE;
        else if (num < 0)
          ecore_con_event_client_error(obj, strerror(errno));
     }
   else
     {
        num = ecore_con_ssl_client_read(obj, buf, sizeof(buf));
        /* this is not an actual 0 return, 0 here just means non-fatal error such as EAGAIN */
        if (num >= 0)
          lost_client = EINA_FALSE;
     }

   if ((!cl->delete_me) && (num > 0))
     ecore_con_event_client_data(obj, buf, num, EINA_TRUE);

   if (lost_client) _ecore_con_client_kill(obj);
}

static Eina_Bool
_ecore_con_svr_cl_handler(void *data,
                          Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Client *obj = data;
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);

   if (cl->delete_me)
     return ECORE_CALLBACK_RENEW;

   if (cl->handshaking && ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ | ECORE_FD_WRITE))
     {
        if (ecore_con_ssl_client_init(obj))
          {
             ERR("ssl handshaking failed!");
             _ecore_con_client_kill(obj);
             return ECORE_CALLBACK_RENEW;
          }
        else if (!cl->ssl_state)
          ecore_con_event_client_add(obj);
     }
   else if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     _ecore_con_svr_cl_read(obj);

   else if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     _ecore_con_client_flush(obj);

   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_con_server_flush(Ecore_Con_Server *obj)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   int count;
   size_t num;
   size_t buf_len;
   size_t *buf_offset;
   const unsigned char *buf;
   Eina_Binbuf *buf_p;

   DBG("(svr=%p,buf=%p)", svr, svr->buf);
   if (!svr->fd_handler) return;
#ifdef _WIN32
   if (ecore_con_local_win32_server_flush(obj))
     return;
#endif

   if ((!svr->buf) && (!svr->ecs_buf))
     {
        ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
        return;
     }

   if (svr->buf)
     {
        buf_p = svr->buf;
        buf_offset = &(svr->write_buf_offset);
     }
   else
     {
        buf_p = svr->ecs_buf;
        buf_offset = &(svr->ecs_buf_offset);
     }
   buf = eina_binbuf_string_get(buf_p);
   buf_len = eina_binbuf_length_get(buf_p);
   num = buf_len - *buf_offset;

   /* check whether we need to write anything at all.
    * we must not write zero bytes with SSL_write() since it
    * causes undefined behaviour
    */
   /* we thank Tommy[D] for needing to check negative buffer sizes
    * here because his system is amazing.
    */
   if (num <= 0) return;

   if ((!svr->ecs_state) && svr->handshaking)
     {
        DBG("Continuing ssl handshake");
        if (ecore_con_ssl_server_init(obj))
          _ecore_con_server_kill(obj);
        _ecore_con_server_timer_update(obj);
        return;
     }

   if (svr->ecs_state || (!(svr->type & ECORE_CON_SSL)))
     count = write(svr->fd, buf + *buf_offset, num);
   else
     count = ecore_con_ssl_server_write(obj, buf + *buf_offset, num);

   if (count < 0)
     {
        if ((errno != EAGAIN) && (errno != EINTR))
          {
             ecore_con_event_server_error(obj, strerror(errno));
             _ecore_con_server_kill(obj);
          }
        return;
     }

   if (count && (!svr->ecs_state)) ecore_con_event_server_write(obj, count);

   if (!eina_binbuf_remove(buf_p, 0, count))
     *buf_offset += count;
   else
     {
        *buf_offset = 0;
        buf_len -= count;
     }
   if (*buf_offset >= buf_len)
     {
        *buf_offset = 0;
        eina_binbuf_free(buf_p);

        if (svr->ecs_buf)
          {
             svr->ecs_buf = NULL;
             INF("PROXY STATE++");
             svr->ecs_state++;
          }
        else
          {
             svr->buf = NULL;
#ifdef TCP_CORK
             if ((svr->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_CORK)
               {
                  int state = 0;
                  if (setsockopt(svr->fd, IPPROTO_TCP, TCP_CORK, (char *)&state, sizeof(int)) < 0)
                    /* realistically this isn't anything serious so we can just log and continue */
                    ERR("uncorking failed! %s", strerror(errno));
               }
#endif
          }
        if (svr->fd_handler)
          ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
     }
   else if (((unsigned int)count < num) && svr->fd_handler)
     ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_WRITE);
}

static void
_ecore_con_client_flush(Ecore_Con_Client *obj)
{
   Ecore_Con_Client_Data *cl = eo_data_scope_get(obj, ECORE_CON_CLIENT_CLASS);
   int count = 0;
   size_t num = 0;

   if (!cl->fd_handler) return;
#ifdef _WIN32
   if (ecore_con_local_win32_client_flush(obj))
     return;
#endif

   if (!cl->buf)
     {
        ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ);
        return;
     }

   if (cl->handshaking)
     {
        if (ecore_con_ssl_client_init(obj))
          count = -1;

        _ecore_con_cl_timer_update(obj);
     }

   if (!count)
     {
        if (!cl->buf) return;
        num = eina_binbuf_length_get(cl->buf) - cl->buf_offset;
        if (num <= 0) return;
        Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
        if (!(host_server->type & ECORE_CON_SSL) && (!cl->upgrade))
          count = write(cl->fd, eina_binbuf_string_get(cl->buf) + cl->buf_offset, num);
        else
          count = ecore_con_ssl_client_write(obj, eina_binbuf_string_get(cl->buf) + cl->buf_offset, num);
     }

   if (count < 0)
     {
        if ((errno != EAGAIN) && (errno != EINTR) && (!cl->delete_me))
          {
             ecore_con_event_client_error(obj, strerror(errno));
             _ecore_con_client_kill(obj);
          }

        return;
     }

   if (count) ecore_con_event_client_write(obj, count);
   cl->buf_offset += count, num -= count;
   if (cl->buf_offset >= eina_binbuf_length_get(cl->buf))
     {
        cl->buf_offset = 0;
        eina_binbuf_free(cl->buf);
        cl->buf = NULL;
#ifdef TCP_CORK
        Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
        if ((host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_CORK)
          {
             int state = 0;
             if (setsockopt(cl->fd, IPPROTO_TCP, TCP_CORK, (char *)&state, sizeof(int)) < 0)
               /* realistically this isn't anything serious so we can just log and continue */
               ERR("uncorking failed! %s", strerror(errno));
          }
#endif
        if (cl->fd_handler)
          ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ);
     }
   else if (cl->fd_handler)
     ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_WRITE);
}

static void
_ecore_con_event_client_add_free(Ecore_Con_Server *obj,
                                 void *ev)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   Ecore_Con_Event_Client_Add *e;

   e = ev;
   if (e->client)
     {
        Ecore_Con_Client_Data *cl = eo_data_scope_get(e->client, ECORE_CON_CLIENT_CLASS);
        Eina_Bool svrfreed = EINA_FALSE;

        cl->event_count = eina_list_remove(cl->event_count, e);
        if (cl->host_server)
          {
             Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
             host_server->event_count = eina_list_remove(host_server->event_count, ev);
             if ((!svr->event_count) && (svr->delete_me))
               {
                  _ecore_con_server_free(obj);
                  svrfreed = EINA_TRUE;
               }
          }
        if (!svrfreed)
          {
             if ((!cl->event_count) && (cl->delete_me))
               ecore_con_client_del(e->client);
          }
     }

   ecore_con_event_client_add_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_client_del_free(Ecore_Con_Server *obj,
                                 void *ev)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   Ecore_Con_Event_Client_Del *e;

   e = ev;
   if (e->client)
     {
        Ecore_Con_Client_Data *cl = eo_data_scope_get(e->client, ECORE_CON_CLIENT_CLASS);
        Eina_Bool svrfreed = EINA_FALSE;

        cl->event_count = eina_list_remove(cl->event_count, e);
        if (cl->host_server)
          {
             Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
             host_server->event_count = eina_list_remove(host_server->event_count, ev);
             if ((!svr->event_count) && (svr->delete_me))
               {
                  _ecore_con_server_free(obj);
                  svrfreed = EINA_TRUE;
               }
          }
        if (!svrfreed)
          {
             if (!cl->event_count)
               _ecore_con_client_free(e->client);
          }
     }
   ecore_con_event_client_del_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_client_write_free(Ecore_Con_Server *obj,
                                   Ecore_Con_Event_Client_Write *e)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (e->client)
     {
        Ecore_Con_Client_Data *cl = eo_data_scope_get(e->client, ECORE_CON_CLIENT_CLASS);
        Eina_Bool svrfreed = EINA_FALSE;

        cl->event_count = eina_list_remove(cl->event_count, e);
        if (cl->host_server)
          {
             Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
             host_server->event_count = eina_list_remove(host_server->event_count, e);
             if ((!svr->event_count) && (svr->delete_me))
               {
                  _ecore_con_server_free(obj);
                  svrfreed = EINA_TRUE;
               }
          }
        if (!svrfreed)
          {
             Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
             if (((!cl->event_count) && (cl->delete_me)) ||
                 ((cl->host_server &&
                   ((host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_UDP ||
                    (host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_MCAST))))
               ecore_con_client_del(e->client);
          }
     }
   ecore_con_event_client_write_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_client_data_free(Ecore_Con_Server *obj,
                                  void *ev)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   Ecore_Con_Event_Client_Data *e;

   e = ev;
   if (e->client)
     {
        Ecore_Con_Client_Data *cl = eo_data_scope_get(e->client, ECORE_CON_CLIENT_CLASS);
        Eina_Bool svrfreed = EINA_FALSE;

        cl->event_count = eina_list_remove(cl->event_count, e);
        if (cl->host_server)
          {
             Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
             host_server->event_count = eina_list_remove(host_server->event_count, ev);
          }
        if ((!svr->event_count) && (svr->delete_me))
          {
             _ecore_con_server_free(obj);
             svrfreed = EINA_TRUE;
          }
        if (!svrfreed)
          {
             Ecore_Con_Server_Data *host_server = eo_data_scope_get(cl->host_server, ECORE_CON_SERVER_CLASS);
             if (((!cl->event_count) && (cl->delete_me)) ||
                 ((cl->host_server &&
                   ((host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_UDP ||
                    (host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_MCAST))))
               ecore_con_client_del(e->client);
          }
     }
   free(e->data);
   ecore_con_event_client_data_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_server_add_free(void *data EINA_UNUSED,
                                 void *ev)
{
   Ecore_Con_Event_Server_Add *e;

   e = ev;
   if (e->server)
     {
        Ecore_Con_Server_Data *svr = eo_data_scope_get(e->server, ECORE_CON_SERVER_CLASS);
        svr->event_count = eina_list_remove(svr->event_count, ev);
        if ((!svr->event_count) && (svr->delete_me))
          _ecore_con_server_free(e->server);
     }
   ecore_con_event_server_add_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_server_del_free(void *data EINA_UNUSED,
                                 void *ev)
{
   Ecore_Con_Event_Server_Del *e;

   e = ev;
   if (e->server)
     {
        Ecore_Con_Server_Data *svr = eo_data_scope_get(e->server, ECORE_CON_SERVER_CLASS);
        svr->event_count = eina_list_remove(svr->event_count, ev);
        if (!svr->event_count)
          _ecore_con_server_free(e->server);
     }
   ecore_con_event_server_del_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_server_write_free(void *data EINA_UNUSED,
                                   Ecore_Con_Event_Server_Write *e)
{
   if (e->server)
     {
        Ecore_Con_Server_Data *svr = eo_data_scope_get(e->server, ECORE_CON_SERVER_CLASS);
        svr->event_count = eina_list_remove(svr->event_count, e);
        if ((!svr->event_count) && (svr->delete_me))
          _ecore_con_server_free(e->server);
     }

   ecore_con_event_server_write_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_server_data_free(void *data EINA_UNUSED,
                                  void *ev)
{
   Ecore_Con_Event_Server_Data *e;

   e = ev;
   if (e->server)
     {
        Ecore_Con_Server_Data *svr = eo_data_scope_get(e->server, ECORE_CON_SERVER_CLASS);
        svr->event_count = eina_list_remove(svr->event_count, ev);
        if ((!svr->event_count) && (svr->delete_me))
          _ecore_con_server_free(e->server);
     }

   free(e->data);
   ecore_con_event_server_data_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_server_error_free(void *data EINA_UNUSED, Ecore_Con_Event_Server_Error *e)
{
   if (e->server)
     {
        Ecore_Con_Server_Data *svr = eo_data_scope_get(e->server, ECORE_CON_SERVER_CLASS);
        svr->event_count = eina_list_remove(svr->event_count, e);
        if ((!svr->event_count) && (svr->delete_me))
          _ecore_con_server_free(e->server);
     }
   free(e->error);
   ecore_con_event_server_error_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_client_error_free(Ecore_Con_Server *obj, Ecore_Con_Event_Client_Error *e)
{
   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   if (e->client)
     {
        Ecore_Con_Client_Data *cl = eo_data_scope_get(e->client, ECORE_CON_CLIENT_CLASS);
        Eina_Bool svrfreed = EINA_FALSE;

        if (eina_list_data_find(svr->clients, e->client))
          {
             cl->event_count = eina_list_remove(cl->event_count, e);
             if ((!cl->event_count) && (cl->delete_me))
               {
                  _ecore_con_client_free(e->client);
                  svrfreed = EINA_TRUE;
               }
          }
        svr->event_count = eina_list_remove(svr->event_count, e);
        if (!svrfreed)
          {
             if ((!svr->event_count) && (svr->delete_me))
               _ecore_con_server_free(obj);
          }
     }
   free(e->error);
   ecore_con_event_client_error_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_lookup_done(void *data,
                       Ecore_Con_Info *infos)
{
   Ecore_Con_Server *obj = data;
   Ecore_Con_Lookup *lk;

   Ecore_Con_Server_Data *svr = eo_data_scope_get(obj, ECORE_CON_SERVER_CLASS);
   lk = svr->data;

   if (infos)
     lk->done_cb(infos->info.ai_canonname, infos->ip,
                 infos->info.ai_addr, infos->info.ai_addrlen,
                 (void *)lk->data);
   else
     lk->done_cb(NULL, NULL, NULL, 0, (void *)lk->data);

   free(svr->name);
   free(lk);
   free(svr);
}

#include "ecore_con_base.eo.c"
#include "ecore_con_client.eo.c"
#include "ecore_con_server.eo.c"
#include "ecore_con_connector.eo.c"
