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

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0 /* noop */
#endif

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
     if (efl_finalized_get(obj)) \
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
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
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

EWAPI Eina_Error EFL_NET_DIALER_ERROR_COULDNT_CONNECT = 0;
EWAPI Eina_Error EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY = 0;
EWAPI Eina_Error EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_HOST = 0;
EWAPI Eina_Error EFL_NET_DIALER_ERROR_PROXY_AUTHENTICATION_FAILED = 0;

static Eina_List *servers = NULL;
static int _ecore_con_init_count = 0;
static int _ecore_con_event_count = 0;
int _ecore_con_log_dom = -1;
Ecore_Con_Socks *_ecore_con_proxy_once = NULL;
Ecore_Con_Socks *_ecore_con_proxy_global = NULL;

typedef struct pxProxyFactory_  pxProxyFactory;
typedef struct _Ecore_Con_Libproxy {
   pxProxyFactory *factory;
   char **(*px_proxy_factory_get_proxies)(pxProxyFactory *factory, const char *url);
   void *(*px_proxy_factory_new)(void);
   void (*px_proxy_factory_free)(pxProxyFactory *);
   Eina_Module *mod;
} Ecore_Con_Libproxy;
static Ecore_Con_Libproxy _ecore_con_libproxy;

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
     goto ecore_err;

   _ecore_con_log_dom = eina_log_domain_register
       ("ecore_con", ECORE_CON_DEFAULT_LOG_COLOR);
   if (_ecore_con_log_dom < 0)
     goto ecore_con_log_error;

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

   EFL_NET_DIALER_ERROR_COULDNT_CONNECT = eina_error_msg_static_register("Couldn't connect to server");
   EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY = eina_error_msg_static_register("Couldn't resolve proxy name");
   EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_HOST = eina_error_msg_static_register("Couldn't resolve host name");
   EFL_NET_DIALER_ERROR_PROXY_AUTHENTICATION_FAILED = eina_error_msg_static_register("Proxy authentication failed");

   eina_magic_string_set(ECORE_MAGIC_CON_SERVER, "Ecore_Con_Server");
   eina_magic_string_set(ECORE_MAGIC_CON_CLIENT, "Ecore_Con_Client");
   eina_magic_string_set(ECORE_MAGIC_CON_URL, "Ecore_Con_Url");

   /* TODO Remember return value, if it fails, use gethostbyname() */
   ecore_con_socks_init();
   ecore_con_ssl_init();
   ecore_con_info_init();
   ecore_con_local_init();

#ifdef HAVE_SYSTEMD
   sd_fd_max = sd_listen_fds(0);
#endif

   eina_log_timing(_ecore_con_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   return _ecore_con_init_count;

ecore_con_log_error:
   EINA_LOG_ERR("Failed to create a log domain for Ecore Con.");
   ecore_shutdown();

ecore_err:
#ifdef HAVE_EVIL
   evil_shutdown();
#endif
   return --_ecore_con_init_count;
}

EAPI int
ecore_con_shutdown(void)
{
   Eina_List *l, *l2;
   Ecore_Con_Server *obj;

   /* _ecore_con_init_count should not go below zero. */
   if (_ecore_con_init_count < 1)
     {
        ERR("Ecore_Con Shutdown called without calling Ecore_Con Init.\n");
        return 0;
     }
   if (--_ecore_con_init_count != 0)
     return _ecore_con_init_count;

   if (_ecore_con_libproxy.factory)
     {
        _ecore_con_libproxy.px_proxy_factory_free(_ecore_con_libproxy.factory);
        _ecore_con_libproxy.factory = NULL;
     }
   if (_ecore_con_libproxy.mod)
     {
        eina_module_free(_ecore_con_libproxy.mod);
        _ecore_con_libproxy.mod = NULL;
     }

   eina_log_timing(_ecore_con_log_dom,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   EINA_LIST_FOREACH_SAFE(servers, l, l2, obj)
     {
        Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
        Ecore_Con_Event_Server_Add *ev;

        if (!svr) continue;
        svr->delete_me = EINA_TRUE;
        INF("svr %p is dead", svr);
        /* some pointer hacks here to prevent double frees if people are being stupid */
        EINA_LIST_FREE(svr->event_count, ev)
          ev->server = NULL;
        _ecore_con_server_free(obj);
     }

   ecore_con_socks_shutdown();
   if (!_ecore_con_event_count) ecore_con_mempool_shutdown();

   ecore_con_local_shutdown();
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
_efl_network_lookup(Eo *kls_obj EINA_UNUSED, void *pd EINA_UNUSED, const char *name, Ecore_Con_Dns_Cb done_cb, const void *data)
{
   Ecore_Con_Server *obj;
   Ecore_Con_Lookup *lk;
   struct addrinfo hints;

   if (!name || !done_cb)
     return EINA_FALSE;

   obj = efl_add(EFL_NETWORK_CONNECTOR_CLASS, NULL, efl_network_server_connection_type_set(efl_added, ECORE_CON_REMOTE_TCP), efl_network_server_name_set(efl_added, name), efl_network_port_set(efl_added, 1025));

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
   efl_del(obj);
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
   obj = efl_add(EFL_NETWORK_SERVER_CLASS, NULL, efl_network_server_connection_type_set(efl_added, compl_type), efl_network_server_name_set(efl_added, name), efl_network_port_set(efl_added, port));

   ecore_con_server_data_set(obj, (void *) data);

   return obj;
}

EOLIAN static Eo *
_efl_network_server_efl_object_constructor(Ecore_Con_Server *obj, Efl_Network_Server_Data *svr)
{
   obj = efl_constructor(efl_super(obj, EFL_NETWORK_SERVER_CLASS));

   svr->fd = -1;
   svr->reject_excess_clients = EINA_FALSE;
   svr->client_limit = -1;
   svr->clients = NULL;

   return obj;
}

EOLIAN static Eo *
_efl_network_server_efl_object_finalize(Ecore_Con_Server *obj, Efl_Network_Server_Data *svr)
{
   Ecore_Con_Type compl_type = svr->type;
   Ecore_Con_Type type;

   efl_finalize(efl_super(obj, EFL_NETWORK_SERVER_CLASS));

   svr->created = EINA_TRUE;
   svr->ppid = getpid();
   svr->start_time = ecore_time_get();
   svr->use_cert = (svr->type & ECORE_CON_SSL & ECORE_CON_LOAD_CERT) == ECORE_CON_LOAD_CERT;

   servers = eina_list_append(servers, obj);

   if (!svr->name)
     goto error;

   type = compl_type & ECORE_CON_TYPE;

   EINA_SAFETY_ON_TRUE_GOTO(((type == ECORE_CON_REMOTE_TCP) ||
                             (type == ECORE_CON_REMOTE_NODELAY) ||
                             (type == ECORE_CON_REMOTE_CORK) ||
                             (type == ECORE_CON_REMOTE_UDP) ||
                             (type == ECORE_CON_REMOTE_BROADCAST)) &&
                             (svr->port < 0), error);

   if (ecore_con_ssl_server_prepare(obj, compl_type & ECORE_CON_SSL))
     goto error;

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
   obj = efl_add(EFL_NETWORK_CONNECTOR_CLASS, NULL, efl_network_server_connection_type_set(efl_added, compl_type), efl_network_server_name_set(efl_added, name), efl_network_port_set(efl_added, port));

   ecore_con_server_data_set(obj, (void *) data);

   return obj;
}

EOLIAN static Eo *
_efl_network_connector_efl_object_finalize(Ecore_Con_Server *obj, void *pd EINA_UNUSED)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   Ecore_Con_Type compl_type = svr->type;
   Ecore_Con_Type type;

   /* XXX: We intentionally put SERVER class here and not connector, as we'd
    * like to skip that one. */
   efl_finalize(efl_super(obj, EFL_NETWORK_SERVER_CLASS));

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
   return NULL;
}

EAPI void
ecore_con_server_timeout_set(Ecore_Con *obj, double timeout)
{
   efl_network_timeout_set((Ecore_Con *)obj, timeout);
}

EOLIAN static void
_efl_network_server_efl_network_timeout_set(Eo *obj, Efl_Network_Server_Data *svr, double timeout)
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
   return efl_network_timeout_get((Ecore_Con *)obj);
}

EOLIAN static double
_efl_network_server_efl_network_timeout_get(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr)
{
   return svr->created ? svr->client_disconnect_time : svr->disconnect_time;
}

EAPI void *
ecore_con_server_del(Ecore_Con_Server *obj)
{
   if (!obj) return NULL;
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);

   if (!svr || svr->delete_me)
     return NULL;

#ifdef _WIN32
   WSASendDisconnect(svr->fd, NULL);
#endif
   _ecore_con_server_kill(obj);
   return svr->data;
}

EAPI void *
ecore_con_server_data_get(Ecore_Con_Server *obj)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   if (!svr)
      return NULL;

   return svr->data;
}

EAPI void *
ecore_con_server_data_set(Ecore_Con_Server *obj,
                          void *data)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   void *ret = NULL;

   if (!svr)
      return NULL;

   ret = svr->data;
   svr->data = data;
   return ret;
}

EAPI Eina_Bool
ecore_con_server_connected_get(const Ecore_Con *obj)
{
   return efl_network_connected_get((Ecore_Con *)obj);
}

EOLIAN static Eina_Bool
_efl_network_server_efl_network_connected_get(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr)
{
   return !svr->connecting;
}

EOLIAN static const Eina_List *
_efl_network_server_clients_get(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr)
{
   return svr->clients;
}

EOLIAN static void
_efl_network_server_connection_type_set(Eo *obj, Efl_Network_Server_Data *svr, Ecore_Con_Type type)
{
   EO_CONSTRUCTOR_CHECK_RETURN(obj);

   svr->type = type;
}

EOLIAN static Ecore_Con_Type
_efl_network_server_connection_type_get(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr)
{
   return svr->type;
}

EOLIAN static void
_efl_network_server_name_set(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr, const char *name)
{
   EO_CONSTRUCTOR_CHECK_RETURN(obj);

   if (svr->name)
      free(svr->name);

   svr->name = strdup(name);
}

EOLIAN static const char *
_efl_network_server_name_get(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr)
{
   return svr->name;
}

EAPI int
ecore_con_server_port_get(const Ecore_Con *obj)
{
   return efl_network_port_get((Ecore_Con *)obj);
}

EOLIAN static void
_efl_network_server_efl_network_port_set(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr, int port)
{
   EO_CONSTRUCTOR_CHECK_RETURN(obj);

   svr->port = port;
}

EOLIAN static int
_efl_network_server_efl_network_port_get(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr)
{
   return svr->port;
}

EAPI int
ecore_con_server_send(Ecore_Con *obj, const void *data, int size)
{
   return efl_network_send((Ecore_Con *)obj, data, size);
}

EOLIAN static int
_efl_network_server_efl_network_send(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr, const void *data, int size)
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
_efl_network_server_client_limit_set(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr,
                                  int client_limit,
                                  char reject_excess_clients)
{
   svr->client_limit = client_limit;
   svr->reject_excess_clients = reject_excess_clients;
}

EOLIAN static void
_efl_network_server_client_limit_get(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr,
                                  int *client_limit,
                                  char *reject_excess_clients)
{
   if (client_limit) *client_limit = svr->client_limit;
   if (reject_excess_clients) *reject_excess_clients = svr->reject_excess_clients;
}

EAPI const char *
ecore_con_server_ip_get(const Ecore_Con *obj)
{
   return efl_network_ip_get(obj);
}

EOLIAN static const char *
_efl_network_server_efl_network_ip_get(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr)
{
   return svr->ip;
}

EAPI double
ecore_con_server_uptime_get(const Ecore_Con *obj)
{
   return efl_network_uptime_get(obj);
}

EOLIAN static double
_efl_network_server_efl_network_uptime_get(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr)
{
   return ecore_time_get() - svr->start_time;
}

EAPI void
ecore_con_server_flush(Ecore_Con *obj)
{
   efl_network_flush((Ecore_Con *)obj);
}

EOLIAN static void
_efl_network_server_efl_network_flush(Eo *obj, Efl_Network_Server_Data *svr EINA_UNUSED)
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
   return efl_network_send((Ecore_Con *)obj, data, size);
}

EOLIAN static int
_efl_network_client_efl_network_send(Eo *obj EINA_UNUSED, Efl_Network_Client_Data *cl, const void *data, int size)
{
   Efl_Network_Server_Data *host_server = NULL;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(cl->delete_me, 0);

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, 0);

   EINA_SAFETY_ON_TRUE_RETURN_VAL(size < 1, 0);

   if (cl->fd_handler)
     ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);

   if (cl->host_server)
      host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_CLIENT_CLASS);


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
_efl_network_client_server_get(Eo *obj EINA_UNUSED, Efl_Network_Client_Data *cl)
{
   return cl->host_server;
}

EOLIAN static Eina_Bool
_efl_network_client_efl_network_connected_get(Eo *obj EINA_UNUSED, Efl_Network_Client_Data *cl)
{
   return !cl->delete_me;
}

EAPI Eina_Bool
ecore_con_client_connected_get(const Ecore_Con *obj)
{
   return efl_network_connected_get((Ecore_Con *)obj);
}

EOLIAN static void
_efl_network_client_efl_network_timeout_set(Eo *obj, Efl_Network_Client_Data *cl, double timeout)
{
   cl->disconnect_time = timeout;

   _ecore_con_cl_timer_update(obj);
}

EAPI void
ecore_con_client_timeout_set(Ecore_Con *obj, double timeout)
{
   efl_network_timeout_set((Ecore_Con *)obj, timeout);
}

EOLIAN static double
_efl_network_client_efl_network_timeout_get(Eo *obj EINA_UNUSED, Efl_Network_Client_Data *cl)
{
   return cl->disconnect_time;
}

EAPI double
ecore_con_client_timeout_get(const Ecore_Con *obj)
{
   return efl_network_timeout_get((Ecore_Con *)obj);
}

EAPI void *
ecore_con_client_del(Ecore_Con_Client *obj)
{
   if (!obj) return NULL;
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   if (!cl) return NULL;

#ifdef _WIN32
   WSASendDisconnect(cl->fd, NULL);
#endif

   _ecore_con_client_kill(obj);
   return cl->data;
}

EAPI void
ecore_con_client_data_set(Ecore_Con_Client *obj,
                          const void *data)
{
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   if (!cl)
      return;

   cl->data = (void *)data;
}

EAPI void *
ecore_con_client_data_get(Ecore_Con_Client *obj)
{
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   if (!cl)
      return NULL;

   return cl->data;
}

EOLIAN static const char *
_efl_network_client_efl_network_ip_get(Eo *obj EINA_UNUSED, Efl_Network_Client_Data *cl)
{
   if (!cl->ip)
     cl->ip = _ecore_con_pretty_ip(cl->client_addr);

   return cl->ip;
}

EAPI const char *
ecore_con_client_ip_get(const Ecore_Con *obj)
{
   return efl_network_ip_get(obj);
}

EOLIAN static int
_efl_network_client_efl_network_port_get(Eo *obj EINA_UNUSED, Efl_Network_Client_Data *cl)
{
   Efl_Network_Server_Data *sd = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);

   if (sd->type != ECORE_CON_REMOTE_TCP &&
       sd->type != ECORE_CON_REMOTE_MCAST &&
       sd->type != ECORE_CON_REMOTE_UDP &&
       sd->type != ECORE_CON_REMOTE_BROADCAST &&
       sd->type != ECORE_CON_REMOTE_NODELAY)
     return -1;

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
   return efl_network_port_get((Ecore_Con *)obj);
}

EOLIAN static double
_efl_network_client_efl_network_uptime_get(Eo *obj EINA_UNUSED, Efl_Network_Client_Data *cl)
{
   return ecore_time_get() - cl->start_time;
}

EAPI double
ecore_con_client_uptime_get(const Ecore_Con *obj)
{
   return efl_network_uptime_get(obj);
}

EOLIAN static void
_efl_network_client_efl_network_flush(Eo *obj, Efl_Network_Client_Data *cl EINA_UNUSED)
{
   _ecore_con_client_flush(obj);
}

EAPI void
ecore_con_client_flush(Ecore_Con *obj)
{
   efl_network_flush((Ecore_Con *)obj);
}

EAPI int
ecore_con_server_fd_get(const Ecore_Con *obj)
{
   return efl_network_fd_get((Ecore_Con *)obj);
}

EOLIAN static int
_efl_network_server_efl_network_fd_get(Eo *obj EINA_UNUSED, Efl_Network_Server_Data *svr)
{
   if (svr->created) return -1;
   if (svr->delete_me) return -1;
   return ecore_main_fd_handler_fd_get(svr->fd_handler);
}

EOLIAN static int
_efl_network_client_efl_network_fd_get(Eo *obj EINA_UNUSED, Efl_Network_Client_Data *cl)
{
   return ecore_main_fd_handler_fd_get(cl->fd_handler);
}

EAPI int
ecore_con_client_fd_get(const Ecore_Con *obj)
{
   return efl_network_fd_get((Ecore_Con *)obj);
}

/**
 * @}
 */

void
ecore_con_event_proxy_bind(Ecore_Con_Server *obj)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
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
   efl_event_callback_call(obj, EFL_NETWORK_EVENT_CONNECTION_UPGRADED, NULL);
   _ecore_con_event_count++;
}

void
ecore_con_event_server_del(Ecore_Con_Server *obj)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
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
        Ecore_Con_Event_Data_Received event_info = { NULL, 0 };
        event_info.data = e->data;
        event_info.size = e->size;
        efl_event_callback_call(obj, EFL_NETWORK_EVENT_DATA_RECEIVED, &event_info);
     }
   _ecore_con_event_count++;
}

void
ecore_con_event_client_add(Ecore_Con_Client *obj)
{
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   Ecore_Con_Event_Client_Add *e;
   int ev = ECORE_CON_EVENT_CLIENT_ADD;

   e = ecore_con_event_client_add_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);

   cl->event_count = eina_list_append(cl->event_count, e);
   host_server->event_count = eina_list_append(host_server->event_count, e);
   _ecore_con_cl_timer_update(obj);
   cl->start_time = ecore_time_get();
   e->client = obj;
   if (cl->upgrade) ev = ECORE_CON_EVENT_CLIENT_UPGRADE;
   ecore_event_add(ev, e,
                   (Ecore_End_Cb)_ecore_con_event_client_add_free, cl->host_server);
   efl_event_callback_call(obj, EFL_NETWORK_EVENT_CONNECTION_UPGRADED, NULL);
   _ecore_con_event_count++;
}

void
ecore_con_event_client_del(Ecore_Con_Client *obj)
{
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   Ecore_Con_Event_Client_Del *e;

   if (!cl) return;
   cl->delete_me = EINA_TRUE;
   INF("cl %p is dead", cl);
   e = ecore_con_event_client_del_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);
   cl->event_count = eina_list_append(cl->event_count, e);

   Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);

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
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   Ecore_Con_Event_Client_Write *e;

   e = ecore_con_event_client_write_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);

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
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   Ecore_Con_Event_Client_Data *e;

   e = ecore_con_event_client_data_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);

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
        Ecore_Con_Event_Data_Received event_info = { NULL, 0 };
        event_info.data = e->data;
        event_info.size = e->size;
        efl_event_callback_call(obj, EFL_NETWORK_EVENT_DATA_RECEIVED, &event_info);
     }
   _ecore_con_event_count++;
}

void
ecore_con_server_infos_del(Ecore_Con_Server *obj, void *info)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   svr->infos = eina_list_remove(svr->infos, info);
}

void
_ecore_con_event_server_error(Ecore_Con_Server *obj, char *error, Eina_Bool duplicate)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   Ecore_Con_Event_Server_Error *e;

   e = ecore_con_event_server_error_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->server = obj;
   e->error = duplicate ? strdup(error) : error;
   DBG("%s", error);
   svr->event_count = eina_list_append(svr->event_count, e);
   ecore_event_add(ECORE_CON_EVENT_SERVER_ERROR, e, (Ecore_End_Cb)_ecore_con_event_server_error_free, NULL);
   efl_event_callback_call(obj, EFL_NETWORK_EVENT_CONNECTION_ERROR, e->error);
   _ecore_con_event_count++;
}

void
ecore_con_event_client_error(Ecore_Con_Client *obj, const char *error)
{
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   Ecore_Con_Event_Client_Error *e;

   e = ecore_con_event_client_error_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);

   e->client = obj;
   e->error = strdup(error);
   DBG("%s", error);
   cl->event_count = eina_list_append(cl->event_count, e);
   host_server->event_count = eina_list_append(host_server->event_count, e);
   ecore_event_add(ECORE_CON_EVENT_CLIENT_ERROR, e, (Ecore_End_Cb)_ecore_con_event_client_error_free, cl->host_server);
   efl_event_callback_call(obj, EFL_NETWORK_EVENT_CONNECTION_ERROR, e->error);
   _ecore_con_event_count++;
}

static void
_ecore_con_server_free(Ecore_Con_Server *obj)
{
   efl_del(obj);
}

EOLIAN static void
_efl_network_server_efl_object_destructor(Eo *obj, Efl_Network_Server_Data *svr)
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
        Efl_Network_Client_Data *cl = efl_data_scope_get(cl_obj, EFL_NETWORK_CLIENT_CLASS);
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
   svr->name = NULL;

   free(svr->path);
   svr->path = NULL;

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

   efl_destructor(efl_super(obj, EFL_NETWORK_SERVER_CLASS));
end:
   return;
}

static void
_ecore_con_client_free(Ecore_Con_Client *obj)
{
   efl_del(obj);
}

EOLIAN static void
_efl_network_client_efl_object_destructor(Eo *obj, Efl_Network_Client_Data *cl)
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
   Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);

   if (host_server)
     {
        host_server->clients = eina_list_remove(host_server->clients, obj);
        --host_server->client_count;
     }

#ifdef _WIN32
   ecore_con_local_win32_client_del(obj);
#endif

   if (cl->event_count) return;

   if (cl->buf) eina_binbuf_free(cl->buf);

   if (host_server && (host_server->type & ECORE_CON_SSL))
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

   efl_destructor(efl_super(obj, EFL_NETWORK_CLIENT_CLASS));
}

static Eina_Bool
_ecore_con_server_timer(Ecore_Con_Server *obj)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   ecore_con_server_del(obj);

   svr->until_deletion = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_con_server_timer_update(Ecore_Con_Server *obj)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
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
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   ecore_con_client_del(obj);

   cl->until_deletion = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_con_cl_timer_update(Ecore_Con_Client *obj)
{
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
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
               cl->until_deletion = ecore_timer_add(cl->disconnect_time, (Ecore_Task_Cb)_ecore_con_client_timer, obj);
          }
        else if (cl->until_deletion)
          {
             ecore_timer_del(cl->until_deletion);
             cl->until_deletion = NULL;
          }
     }
   else
     {
        Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);

        if (host_server && host_server->client_disconnect_time > 0)
          {
             if (cl->until_deletion)
               {
                  ecore_timer_interval_set(cl->until_deletion, host_server->client_disconnect_time);
                  ecore_timer_reset(cl->until_deletion);
               }
             else
               cl->until_deletion = ecore_timer_add(host_server->client_disconnect_time, (Ecore_Task_Cb)_ecore_con_client_timer, obj);
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   struct linger lin;
   const char *memerr = NULL;
#ifdef _WIN32
   u_long mode = 1;
#endif

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
#ifdef _WIN32
   if (svr->fd == INVALID_SOCKET) goto error;

   if (ioctlsocket(svr->fd, FIONBIO, &mode)) goto error;
#else
   if (svr->fd < 0) goto error;

   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
#endif

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

   if (bind(svr->fd, net_info->info.ai_addr, net_info->info.ai_addrlen) != 0)
     goto error;

   if (listen(svr->fd, 4096) != 0) goto error;

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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   Ecore_Con_Type type;
   struct ip_mreq mreq;
#ifdef HAVE_IPV6
   struct ipv6_mreq mreq6;
#endif
   const int on = 1;
   const char *memerr = NULL;
#ifdef _WIN32
   u_long mode = 1;
#endif

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
#ifdef _WIN32
   if (svr->fd == INVALID_SOCKET) goto error;

   if (ioctlsocket(svr->fd, FIONBIO, &mode)) goto error;
#else
   if (svr->fd < 0) goto error;

   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
#endif

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
             if (setsockopt(svr->fd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                            (const void *)&mreq6, sizeof(mreq6)) != 0)
               goto error;
          }
#endif
     }

   if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&on, sizeof(on)) != 0)
     goto error;

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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   int res;
   int curstate = 0;
   const char *memerr = NULL;
#ifdef _WIN32
   u_long mode = 1;
#endif

   errno = 0;
   if (!net_info) /* error message has already been handled */
     {
        svr->delete_me = EINA_TRUE;
        goto error;
     }

   svr->fd = socket(net_info->info.ai_family, net_info->info.ai_socktype,
                    net_info->info.ai_protocol);
#ifdef _WIN32
   if (svr->fd == INVALID_SOCKET) goto error;

   if (ioctlsocket(svr->fd, FIONBIO, &mode)) goto error;
#else
   if (svr->fd < 0) goto error;

   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
#endif

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
        if (WSAGetLastError() != WSAEWOULDBLOCK)
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
     {
        ecore_con_event_server_add(obj);
        svr->fd_handler = ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ,
                                                    _ecore_con_cl_handler, obj, NULL, NULL);
     }

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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
#ifdef _WIN32
   u_long mode = 1;
#endif

   errno = 0;
   if (!net_info) /* error message has already been handled */
     {
        svr->delete_me = EINA_TRUE;
        goto error;
     }

   svr->fd = socket(net_info->info.ai_family, net_info->info.ai_socktype,
                    net_info->info.ai_protocol);
#ifdef _WIN32
   if (svr->fd == INVALID_SOCKET) goto error;

   if (ioctlsocket(svr->fd, FIONBIO, &mode)) goto error;
#else
   if (svr->fd < 0) goto error;

   if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
#endif
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
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
        DBG("Connection lost: %s", strerror(so_err));
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(svr_obj, EFL_NETWORK_SERVER_CLASS);
#ifdef _WIN32
   u_long mode = 1;
#endif

   if (svr->delete_me)
     return ECORE_CALLBACK_RENEW;

   if ((svr->client_limit >= 0) && (!svr->reject_excess_clients) &&
       (svr->client_count >= (unsigned int)svr->client_limit))
     return ECORE_CALLBACK_RENEW;

   /* a new client */

   obj = efl_add(EFL_NETWORK_CLIENT_CLASS, NULL);
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   if (!cl)
     {
        ecore_con_event_server_error(svr_obj, "Memory allocation failure when attempting to add a new client");
        return ECORE_CALLBACK_RENEW;
     }
   cl->host_server = svr_obj;

   client_addr_len = sizeof(client_addr);
   memset(&client_addr, 0, client_addr_len);
   cl->fd = accept(svr->fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
#ifdef _WIN32
   if (cl->fd == INVALID_SOCKET) goto error;
#else
   if (cl->fd < 0) goto error;
#endif
   if ((svr->client_limit >= 0) && (svr->reject_excess_clients) &&
       (svr->client_count >= (unsigned int)svr->client_limit))
     {
        clerr = "Maximum client limit reached";
        goto error;
     }

#ifdef _WIN32
   if (ioctlsocket(cl->fd, FIONBIO, &mode)) goto error;
#else
   if (fcntl(cl->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(cl->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
#endif
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
   efl_del(obj);
   if (clerr || errno) ecore_con_event_server_error(svr_obj, clerr ? : strerror(errno));
   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_con_cl_read(Ecore_Con_Server *obj)
{
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
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
        num = recv(svr->fd, (char *)buf, sizeof(buf), 0);

        /* 0 is not a valid return value for a tcp socket */
#ifdef _WIN32
        if ((num > 0) || ((num < 0) && (WSAGetLastError() == WSAEWOULDBLOCK)))
#else
        if ((num > 0) || ((num < 0) && (errno == EAGAIN)))
#endif
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);

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
             len = recv(svr->fd, (char *)buf, sizeof(buf), MSG_DONTWAIT | MSG_PEEK);
             DBG("%zu bytes in buffer", len);
          }
#endif
        if (ecore_con_ssl_server_init(obj))
          {
             DBG("ssl handshaking failed!");
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);

   want_read = ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ);
   want_write = ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE);

   if (svr->delete_me || ((!want_read) && (!want_write)))
     return ECORE_CALLBACK_RENEW;

   if (want_write)
     {
        _ecore_con_server_flush(obj);
        return ECORE_CALLBACK_RENEW;
     }

   num = recv(svr->fd, (char *)buf, READBUFSIZ, 0);

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
#ifdef _WIN32
   u_long mode = 1;
#endif

   Efl_Network_Server_Data *svr = efl_data_scope_get(svr_obj, EFL_NETWORK_SERVER_CLASS);
   if (svr->delete_me)
     return ECORE_CALLBACK_RENEW;

   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     return ECORE_CALLBACK_RENEW;

   if (!ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     return ECORE_CALLBACK_RENEW;

#ifdef _WIN32
   if (!ioctlsocket(svr->fd, FIONBIO, &mode))
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
   obj = efl_add(EFL_NETWORK_CLIENT_CLASS, NULL);
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, ECORE_CALLBACK_RENEW);

   cl->host_server = svr_obj;
   cl->fd = -1;
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
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
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

   Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);
   if (!(host_server->type & ECORE_CON_SSL) && (!cl->upgrade))
     {
        num = recv(cl->fd, (char *)buf, sizeof(buf), 0);

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
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);

   if (cl->delete_me)
     return ECORE_CALLBACK_RENEW;

   if (cl->handshaking && ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ | ECORE_FD_WRITE))
     {
        if (ecore_con_ssl_client_init(obj))
          {
             DBG("ssl handshaking failed!");
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
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
     count = send(svr->fd, (const char *)buf + *buf_offset, num, 0);
   else
     count = ecore_con_ssl_server_write(obj, buf + *buf_offset, num);

#ifdef _WIN32
   if (count == SOCKET_ERROR)
     {
        switch (WSAGetLastError())
          {
           case WSAEINTR:
           case WSAEINPROGRESS:
           case WSAEWOULDBLOCK:
             break;
           default:
             {
                LPTSTR s;

                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                              FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL, WSAGetLastError(),
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                              (LPTSTR)&s, 0, NULL);
                ecore_con_event_server_error(obj, (char *)s);
                free(s);
                _ecore_con_server_kill(obj);
             }
          }
     }
#else
   if (count < 0)
     {
         if ((errno != EAGAIN) && (errno != EINTR))
           {
              ecore_con_event_server_error(obj, strerror(errno));
              _ecore_con_server_kill(obj);
           }
         return;
     }
#endif


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
   Efl_Network_Client_Data *cl = efl_data_scope_get(obj, EFL_NETWORK_CLIENT_CLASS);
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
        Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);
        if (!(host_server->type & ECORE_CON_SSL) && (!cl->upgrade))
          count = send(cl->fd, (char *)eina_binbuf_string_get(cl->buf) + cl->buf_offset, num, 0);
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
        Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   Ecore_Con_Event_Client_Add *e;

   e = ev;
   if (e->client)
     {
        Efl_Network_Client_Data *cl = efl_data_scope_get(e->client, EFL_NETWORK_CLIENT_CLASS);
        Eina_Bool svrfreed = EINA_FALSE;

        if ((svr) && (cl))
          {
             cl->event_count = eina_list_remove(cl->event_count, e);
             if (cl->host_server)
               {
                  Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);
                  if (host_server)
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   Ecore_Con_Event_Client_Del *e;

   e = ev;
   if (e->client)
     {
        Efl_Network_Client_Data *cl = efl_data_scope_get(e->client, EFL_NETWORK_CLIENT_CLASS);
        Eina_Bool svrfreed = EINA_FALSE;

        if ((svr) && (cl))
          {
             cl->event_count = eina_list_remove(cl->event_count, e);
             if (cl->host_server)
               {
                  Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);
                  if (host_server)
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   if (e->client)
     {
        Efl_Network_Client_Data *cl = efl_data_scope_get(e->client, EFL_NETWORK_CLIENT_CLASS);
        Eina_Bool svrfreed = EINA_FALSE;

        if ((svr) && (cl))
          {
             cl->event_count = eina_list_remove(cl->event_count, e);
             if (cl->host_server)
               {
                  Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);
                  if (host_server)
                    host_server->event_count = eina_list_remove(host_server->event_count, e);
                  if ((!svr->event_count) && (svr->delete_me))
                    {
                       _ecore_con_server_free(obj);
                       svrfreed = EINA_TRUE;
                    }
               }
             if (!svrfreed)
               {
                  Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);
                  if (((!cl->event_count) && (cl->delete_me)) ||
                      ((cl->host_server &&
                        ((host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_UDP ||
                            (host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_MCAST))))
                    ecore_con_client_del(e->client);
               }
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   Ecore_Con_Event_Client_Data *e;

   e = ev;
   if (e->client)
     {
        Efl_Network_Client_Data *cl = efl_data_scope_get(e->client, EFL_NETWORK_CLIENT_CLASS);
        Eina_Bool svrfreed = EINA_FALSE;

        if ((svr) && (cl))
          {
             cl->event_count = eina_list_remove(cl->event_count, e);
             if (cl->host_server)
               {
                  Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);
                  if (host_server)
                    host_server->event_count = eina_list_remove(host_server->event_count, ev);
               }
             if ((!svr->event_count) && (svr->delete_me))
               {
                  _ecore_con_server_free(obj);
                  svrfreed = EINA_TRUE;
               }
             if (!svrfreed)
               {
                  Efl_Network_Server_Data *host_server = efl_data_scope_get(cl->host_server, EFL_NETWORK_SERVER_CLASS);
                  if (((!cl->event_count) && (cl->delete_me)) ||
                      ((cl->host_server &&
                        ((host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_UDP ||
                            (host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_MCAST))))
                    ecore_con_client_del(e->client);
               }
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
        Efl_Network_Server_Data *svr = efl_data_scope_get(e->server, EFL_NETWORK_SERVER_CLASS);
        if (svr)
          {
             svr->event_count = eina_list_remove(svr->event_count, ev);
             if ((!svr->event_count) && (svr->delete_me))
               _ecore_con_server_free(e->server);
          }
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
        Efl_Network_Server_Data *svr = efl_data_scope_get(e->server, EFL_NETWORK_SERVER_CLASS);
        if (svr)
          {
             svr->event_count = eina_list_remove(svr->event_count, ev);
             if (!svr->event_count)
               _ecore_con_server_free(e->server);
          }
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
        Efl_Network_Server_Data *svr = efl_data_scope_get(e->server, EFL_NETWORK_SERVER_CLASS);
        if (svr)
          {
             svr->event_count = eina_list_remove(svr->event_count, e);
             if ((!svr->event_count) && (svr->delete_me))
               _ecore_con_server_free(e->server);
          }
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
        Efl_Network_Server_Data *svr = efl_data_scope_get(e->server, EFL_NETWORK_SERVER_CLASS);
        if (svr)
          {
             svr->event_count = eina_list_remove(svr->event_count, ev);
             if ((!svr->event_count) && (svr->delete_me))
               _ecore_con_server_free(e->server);
          }
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
        Efl_Network_Server_Data *svr = efl_data_scope_get(e->server, EFL_NETWORK_SERVER_CLASS);
        if (svr)
          {
             svr->event_count = eina_list_remove(svr->event_count, e);
             if ((!svr->event_count) && (svr->delete_me))
               _ecore_con_server_free(e->server);
          }
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
   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   if (e->client)
     {
        Efl_Network_Client_Data *cl = efl_data_scope_get(e->client, EFL_NETWORK_CLIENT_CLASS);
        Eina_Bool svrfreed = EINA_FALSE;

        if ((svr) && (cl))
          {
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

   Efl_Network_Server_Data *svr = efl_data_scope_get(obj, EFL_NETWORK_SERVER_CLASS);
   if (!svr) return;
   lk = svr->data;

   if (infos)
     lk->done_cb(infos->info.ai_canonname, infos->ip,
                 infos->info.ai_addr, infos->info.ai_addrlen,
                 (void *)lk->data);
   else
     lk->done_cb(NULL, NULL, NULL, 0, (void *)lk->data);

   free(lk);
}

#include "efl_network.eo.c"
#include "efl_network_client.eo.c"
#include "efl_network_server.eo.c"
#include "efl_network_connector.eo.c"

Eina_Bool
efl_net_ip_port_fmt(char *buf, int buflen, const struct sockaddr *addr)
{
   char p[INET6_ADDRSTRLEN];
   const void *mem;
   unsigned short port;
   int r;

   if (addr->sa_family == AF_INET)
     {
        const struct sockaddr_in *a = (const struct sockaddr_in *)addr;
        mem = &a->sin_addr;
        port = ntohs(a->sin_port);
     }
   else if (addr->sa_family == AF_INET6)
     {
        const struct sockaddr_in6 *a = (const struct sockaddr_in6 *)addr;
        mem = &a->sin6_addr;
        port = ntohs(a->sin6_port);
     }
   else
     {
        ERR("unsupported address family: %d", addr->sa_family);
        return EINA_FALSE;
     }

   if (!inet_ntop(addr->sa_family, mem, p, sizeof(p)))
     {
        ERR("inet_ntop(%d, %p, %p, %zd): %s",
            addr->sa_family, mem, p, sizeof(p), strerror(errno));
        return EINA_FALSE;
     }

   if (addr->sa_family == AF_INET)
     r = snprintf(buf, buflen, "%s:%hu", p, port);
   else
     r = snprintf(buf, buflen, "[%s]:%hu", p, port);

   if (r < 0)
     {
        ERR("could not snprintf(): %s", strerror(errno));
        return EINA_FALSE;
     }
   else if (r > buflen)
     {
        ERR("buffer is too small: %d, required %d", buflen, r);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool
efl_net_ip_port_split(char *buf, const char **p_host, const char **p_port)
{
   char *host, *port;

   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(p_host, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(p_port, EINA_FALSE);

   host = buf;
   if (host[0] == '[')
     {
        /* IPv6 is: [IP]:port */
        host++;
        port = strchr(host, ']');
        if (!port) return EINA_FALSE;
        *port = '\0';
        port++;

        if (port[0] == ':')
          port++;
        else
          port = NULL;
     }
   else
     {
        port = strchr(host, ':');
        if (port)
          {
             *port = '\0';
             port++;
             if (*port == '\0') port = NULL;
          }
     }

   *p_host = host;
   *p_port = port;
   return EINA_TRUE;
}

static void
_cleanup_close(void *data)
{
   int *p_fd = data;
   int fd = *p_fd;
   *p_fd = -1;
   if (fd >= 0) close(fd);
}

int
efl_net_socket4(int domain, int type, int protocol, Eina_Bool close_on_exec)
{
   int fd = -1;

#ifdef SOCK_CLOEXEC
   if (close_on_exec) type |= SOCK_CLOEXEC;
#endif

   fd = socket(domain, type, protocol);
#ifndef SOCK_CLOEXEC
   EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);
   if (fd > 0)
     {
        if (close_on_exec)
          {
             if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0)
               {
                  int errno_bkp = errno;
                  ERR("fcntl(%d, F_SETFD, FD_CLOEXEC): %s", fd, strerror(errno));
                  close(fd);
                  fd = -1;
                  errno = errno_bkp;
               }
          }
     }
   EINA_THREAD_CLEANUP_POP(EINA_FALSE); /* we need fd on success */
#endif

   return fd;
}

typedef struct _Efl_Net_Connect_Async_Data
{
   Efl_Net_Connect_Async_Cb cb;
   const void *data;
   socklen_t addrlen;
   Eina_Bool close_on_exec;
   int type;
   int protocol;
   int sockfd;
   Eina_Error error;
   struct sockaddr addr[];
} Efl_Net_Connect_Async_Data;

static void
_efl_net_connect_async_run(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Connect_Async_Data *d = data;
   char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";
   int r;

   /* allows ecore_thread_cancel() to cancel at some points, see
    * man:pthreads(7).
    *
    * no need to set cleanup functions since the main thread will
    * handle that with _efl_net_connect_async_cancel().
    */
   eina_thread_cancellable_set(EINA_TRUE, NULL);

   d->error = 0;

   /* always close-on-exec since it's not a point to pass an
    * under construction socket to a child process.
    */
   d->sockfd = efl_net_socket4(d->addr->sa_family, d->type, d->protocol, EINA_TRUE);
   if (d->sockfd < 0)
     {
        d->error = errno;
        DBG("socket(%d, %d, %d) failed: %s", d->addr->sa_family, d->type, d->protocol, strerror(errno));
        return;
     }

   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     efl_net_ip_port_fmt(buf, sizeof(buf), d->addr);

   if ((d->type == SOCK_DGRAM) &&
       (d->addr->sa_family == AF_INET) &&
       (((const struct sockaddr_in *)d->addr)->sin_addr.s_addr == INADDR_BROADCAST))
     {
        int enable = 1;
        if (setsockopt(d->sockfd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) == 0)
          DBG("enabled SO_BROADCAST for socket=%d", d->sockfd);
        else
          WRN("could not enable SO_BROADCAST for socket=%d: %s", d->sockfd, strerror(errno));
     }

   DBG("connecting fd=%d to %s", d->sockfd, buf);

   r = connect(d->sockfd, d->addr, d->addrlen);
   if (r < 0)
     {
        int fd = d->sockfd;
        d->error = errno;
        d->sockfd = -1;
        /* close() is a cancellation point, thus unset sockfd before
         * closing, so the main thread _efl_net_connect_async_cancel()
         * won't close it again.
         */
        close(fd);
        DBG("connect(%d, %s) failed: %s", fd, buf, strerror(errno));
        return;
     }

   DBG("connected fd=%d to %s", d->sockfd, buf);
}

static void
_efl_net_connect_async_data_free(Efl_Net_Connect_Async_Data *d)
{
   free(d);
}

static void
_efl_net_connect_async_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Connect_Async_Data *d = data;

#ifdef FD_CLOEXEC
   /* if it wasn't a close on exec, release the socket to be passed to child */
   if ((!d->close_on_exec) && (d->sockfd >= 0))
     {
        int flags = fcntl(d->sockfd, F_GETFD);
        if (flags < 0)
          {
             d->error = errno;
             ERR("fcntl(%d, F_GETFD): %s", d->sockfd, strerror(errno));
             close(d->sockfd);
             d->sockfd = -1;
          }
        else
          {
             flags &= (~FD_CLOEXEC);
             if (fcntl(d->sockfd, F_SETFD, flags) < 0)
               {
                  d->error = errno;
                  ERR("fcntl(%d, F_SETFD, %#x): %s", d->sockfd, flags, strerror(errno));
                  close(d->sockfd);
                  d->sockfd = -1;
               }
          }
     }
#endif
   d->cb((void *)d->data, d->addr, d->addrlen, d->sockfd, d->error);
   _efl_net_connect_async_data_free(d);
}

static void
_efl_net_connect_async_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Connect_Async_Data *d = data;
   if (d->sockfd >= 0) close(d->sockfd);
   _efl_net_connect_async_data_free(d);
}

Ecore_Thread *
efl_net_connect_async_new(const struct sockaddr *addr, socklen_t addrlen, int type, int protocol, Eina_Bool close_on_exec, Efl_Net_Connect_Async_Cb cb, const void *data)
{
   Efl_Net_Connect_Async_Data *d;

   EINA_SAFETY_ON_NULL_RETURN_VAL(addr, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(addrlen < 1, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, NULL);

   d = malloc(sizeof(Efl_Net_Connect_Async_Data) + addrlen);
   EINA_SAFETY_ON_NULL_RETURN_VAL(d, NULL);

   d->cb = cb;
   d->data = data;
   d->addrlen = addrlen;
   d->close_on_exec = close_on_exec;
   d->type = type;
   d->protocol = protocol;
   memcpy(d->addr, addr, addrlen);

   d->sockfd = -1;
   d->error = 0;

   return ecore_thread_run(_efl_net_connect_async_run,
                           _efl_net_connect_async_end,
                           _efl_net_connect_async_cancel,
                           d);
}

static Eina_Bool
_efl_net_ip_no_proxy(const char *host, char * const *no_proxy_strv)
{
   char * const *itr;
   size_t host_len;

   if (!no_proxy_strv)
     return EINA_FALSE;

   host_len = strlen(host);
   for (itr = no_proxy_strv; *itr != NULL; itr++)
     {
        const char *s = *itr;
        size_t slen;

        /* '*' is not a glob/pattern, it matches all */
        if (*s == '*') return EINA_TRUE;

        /* old timers use leading dot to avoid matching partial names
         * due implementation bugs not required anymore
         */
        if (*s == '.') s++;

        slen = strlen(s);
        if (slen == 0) continue;

        if (host_len < slen) continue;
        if (memcmp(host + host_len - slen, s, slen) == 0)
          {
             if (slen == host_len)
               return EINA_TRUE;
             if (host[host_len - slen - 1] == '.')
               return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}

typedef struct _Efl_Net_Ip_Connect_Async_Data
{
   Efl_Net_Connect_Async_Cb cb;
   const void *data;
   char *address;
   char *proxy;
   char *proxy_env;
   char **no_proxy_strv;
   socklen_t addrlen;
   Eina_Bool close_on_exec;
   int type;
   int protocol;
   int sockfd;
   Eina_Error error;
   union {
      struct sockaddr_in addr4;
      struct sockaddr_in6 addr6;
      struct sockaddr addr;
   };
} Efl_Net_Ip_Connect_Async_Data;

static Eina_Error
_efl_net_ip_connect(const struct addrinfo *addr, int *sockfd)
{
   int fd = -1;
   Eina_Error ret = 0;

   /* always close-on-exec since it's not a point to pass an
    * under construction socket to a child process.
    */
   fd = efl_net_socket4(addr->ai_family, addr->ai_socktype, addr->ai_protocol, EINA_TRUE);
   if (fd < 0) ret = errno;
   else
     {
        char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";
        int r;

        EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);
        if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
          {
             if (efl_net_ip_port_fmt(buf, sizeof(buf), addr->ai_addr))
               DBG("connect fd=%d to %s", fd, buf);
          }

        if ((addr->ai_socktype == SOCK_DGRAM) &&
            (addr->ai_family == AF_INET) &&
            (((const struct sockaddr_in *)addr->ai_addr)->sin_addr.s_addr == INADDR_BROADCAST))
          {
             int enable = 1;
             if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) == 0)
               DBG("enabled SO_BROADCAST for socket=%d", fd);
             else
               WRN("could not enable SO_BROADCAST for socket=%d: %s", fd, strerror(errno));
          }

        r = connect(fd, addr->ai_addr, addr->ai_addrlen);
        if (r == 0)
          {
             DBG("connected fd=%d to %s", fd, buf);
             *sockfd = fd;
          }
        else
          {
             ret = errno;
             DBG("couldn't connect fd=%d to %s: %s", fd, buf, strerror(errno));
             close(fd);
          }
        EINA_THREAD_CLEANUP_POP(EINA_FALSE); /* we need sockfd on success */
     }
   return ret;
}

static Eina_Error
_efl_net_ip_resolve_and_connect(const char *host, const char *port, int type, int protocol, int *sockfd, struct sockaddr *addr, socklen_t *p_addrlen)
{
   struct addrinfo *results = NULL;
   struct addrinfo hints = {
     .ai_socktype = type,
     .ai_protocol = protocol,
     .ai_family = AF_UNSPEC,
   };
   Eina_Error ret = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
   int r;

   if (strchr(host, ':')) hints.ai_family = AF_INET6;

   do
     r = getaddrinfo(host, port, &hints, &results);
   while ((r == EAI_AGAIN) || ((r == EAI_SYSTEM) && (errno == EINTR)));

   if (r != 0)
     {
        DBG("couldn't resolve host='%s', port='%s': %s",
            host, port, gai_strerror(r));
        ret = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_HOST;
        *sockfd = -1;
     }
   else
     {
        const struct addrinfo *addrinfo;

        EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)freeaddrinfo, results);
        for (addrinfo = results; addrinfo != NULL; addrinfo = addrinfo->ai_next)
          {
             if (addrinfo->ai_socktype != type) continue;
             if (addrinfo->ai_protocol != protocol) continue;
             ret = _efl_net_ip_connect(addrinfo, sockfd);
             if (ret == 0)
               {
                  memcpy(addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
                  *p_addrlen = addrinfo->ai_addrlen;
                  break;
               }
          }
        if (ret != 0)
          ret = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
        EINA_THREAD_CLEANUP_POP(EINA_TRUE);
     }
   return ret;
}

static void
_efl_net_ip_connect_async_run_direct(Efl_Net_Ip_Connect_Async_Data *d, const char *host, const char *port)
{
   DBG("direct connection to %s:%s", host, port);
   d->error = _efl_net_ip_resolve_and_connect(host, port, d->type, d->protocol, &d->sockfd, &d->addr, &d->addrlen);
}

static Eina_Bool
_efl_net_ip_port_user_pass_split(char *buf, const char **p_host, const char **p_port, const char **p_user, const char **p_pass)
{
   char *p;

   p = strchr(buf, '@');
   if (!p)
     {
        p = buf;
        *p_user = NULL;
        *p_pass = NULL;
     }
   else
     {
        char *s;
        *p_user = buf;
        *p = '\0';
        p++;

        s = strchr(*p_user, ':');
        if (!s)
          *p_pass = NULL;
        else
          {
             *s = '\0';
             s++;
             *p_pass = s;
          }
     }

   return efl_net_ip_port_split(p, p_host, p_port);
}

typedef enum _Efl_Net_Socks4_Request_Command {
  EFL_NET_SOCKS4_REQUEST_COMMAND_CONNECT = 0x01,
  EFL_NET_SOCKS4_REQUEST_COMMAND_BIND = 0x02
} Efl_Net_Socks4_Request_Command;

typedef struct _Efl_Net_Socks4_Request {
   uint8_t version; /* = 0x4 */
   uint8_t command; /* Efl_Net_Socks4_Request_Command */
   uint16_t port;
   uint8_t ipv4[4];
   char indent[];
} Efl_Net_Socks4_Request;

typedef enum _Efl_Net_Socks4_Reply_Status {
  EFL_NET_SOCKS4_REPLY_STATUS_GRANTED = 0x5a,
  EFL_NET_SOCKS4_REPLY_STATUS_REJECTED = 0x5b,
  EFL_NET_SOCKS4_REPLY_STATUS_FAILED_INDENT = 0x5c,
  EFL_NET_SOCKS4_REPLY_STATUS_FAILED_USER = 0x5d
} Efl_Net_Socks4_Reply_Status;

typedef struct _Efl_Net_Socks4_Reply {
   uint8_t null;
   uint8_t status;
   uint16_t port;
   uint8_t ipv4[4];
} Efl_Net_Socks4_Reply;

static Eina_Bool
_efl_net_ip_connect_async_run_socks4_try(Efl_Net_Ip_Connect_Async_Data *d, const char *proxy_host, const char *proxy_port, const struct addrinfo *addrinfo, Efl_Net_Socks4_Request *request, size_t request_len)
{
   char buf[INET_ADDRSTRLEN + sizeof(":65536")];
   struct sockaddr_in *a = (struct sockaddr_in *)addrinfo->ai_addr;
   struct sockaddr_storage proxy_addr;
   socklen_t proxy_addrlen;
   int fd;
   Eina_Error err;
   Eina_Bool ret = EINA_FALSE;
   ssize_t s;

   err = _efl_net_ip_resolve_and_connect(proxy_host, proxy_port, SOCK_STREAM, IPPROTO_TCP, &fd, (struct sockaddr *)&proxy_addr, &proxy_addrlen);
   if (err)
     {
        DBG("couldn't connect to socks4://%s:%s: %s", proxy_host, proxy_port, eina_error_msg_get(err));
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        return EINA_TRUE; /* no point in continuing on this error */
     }

   EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);
   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        if (efl_net_ip_port_fmt(buf, sizeof(buf), addrinfo->ai_addr))
          DBG("resolved address='%s' to %s. Connect using fd=%d socks4://%s:%s", d->address, buf, fd, proxy_host, proxy_port);
     }

   request->port = a->sin_port;
   memcpy(request->ipv4, &a->sin_addr, 4);

   s = send(fd, request, request_len, MSG_NOSIGNAL);
   if (s != (ssize_t)request_len)
     {
        if (s < 0)
          DBG("couldn't request connection to host=%s fd=%d socks4://%s:%s: %s", buf, fd, proxy_host, proxy_port, strerror(errno));
        else
          DBG("couldn't send proxy request: need %zu, did %zd", request_len, s);
     }
   else
     {
        Efl_Net_Socks4_Reply reply;
        s = recv(fd, &reply, sizeof(reply), MSG_NOSIGNAL);
        if (s != sizeof(reply))
          {
             if (s < 0)
               DBG("couldn't recv reply of connection to host=%s fd=%d socks4://%s:%s: %s", buf, fd, proxy_host, proxy_port, strerror(errno));
             else
               DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(reply), s);
          }
        else
          {
             if (reply.status != EFL_NET_SOCKS4_REPLY_STATUS_GRANTED)
               DBG("rejected connection to host=%s fd=%d socks4://%s:%s: reason=%#x", buf, fd, proxy_host, proxy_port, reply.status);
             else
               {
                  memcpy(&d->addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
                  d->addrlen = addrinfo->ai_addrlen;
                  d->sockfd = fd;
                  d->error = 0;
                  ret = EINA_TRUE;
                  DBG("connected to host=%s fd=%d socks4://%s:%s", buf, fd, proxy_host, proxy_port);
               }
          }
     }
   EINA_THREAD_CLEANUP_POP(!ret); /* we need fd on success, on failure just close it */
   return ret;
}

static void
_efl_net_ip_connect_async_run_socks4(Efl_Net_Ip_Connect_Async_Data *d, const char *host, const char *port, const char *proxy)
{
   char *str;
   const char *proxy_user, *proxy_pass, *proxy_host, *proxy_port;
   struct addrinfo *results = NULL;
   struct addrinfo hints = {
     .ai_socktype = d->type,
     .ai_protocol = d->protocol,
     .ai_family = AF_INET,
   };
   int r;

   if (strchr(host, ':'))
     {
        DBG("SOCKSv4 only handles IPv4. Wanted host=%s", host);
        d->error = EAFNOSUPPORT;
        return;
     }

   if ((d->type != SOCK_STREAM) || (d->protocol != IPPROTO_TCP))
     {
        DBG("SOCKSv4 only accepts TCP requests. Wanted type=%#x, protocol=%#x", d->type, d->protocol);
        d->error = EPROTONOSUPPORT;
        return;
     }

   DBG("proxy connection to %s:%s using socks4://%s", host, port, proxy);

   str = strdup(proxy);
   EINA_THREAD_CLEANUP_PUSH(free, str);

   if (!_efl_net_ip_port_user_pass_split(str, &proxy_host, &proxy_port, &proxy_user, &proxy_pass))
     {
        ERR("Invalid proxy string: socks4://%s", proxy);
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }
   if (!proxy_user) proxy_user = "";
   if (!proxy_port) proxy_port = "1080";

   do
     r = getaddrinfo(host, port, &hints, &results);
   while ((r == EAI_AGAIN) || ((r == EAI_SYSTEM) && (errno == EINTR)));
   if (r != 0)
     {
        DBG("couldn't resolve host='%s', port='%s': %s",
            host, port, gai_strerror(r));
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_HOST;
     }
   else
     {
        const struct addrinfo *addrinfo;
        Efl_Net_Socks4_Request *request;
        size_t request_len;

        d->error = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
        EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)freeaddrinfo, results);

        request_len = sizeof(Efl_Net_Socks4_Request) + strlen(proxy_user) + 1;
        request = malloc(request_len);
        if (request)
          {
             request->version = 0x04;
             request->command = EFL_NET_SOCKS4_REQUEST_COMMAND_CONNECT;
             memcpy(request->indent, proxy_user, strlen(proxy_user) + 1);
             EINA_THREAD_CLEANUP_PUSH(free, request);
             for (addrinfo = results; addrinfo != NULL; addrinfo = addrinfo->ai_next)
               {
                  if (addrinfo->ai_socktype != d->type) continue;
                  if (addrinfo->ai_protocol != d->protocol) continue;
                  if (addrinfo->ai_family != AF_INET) continue;
                  if (_efl_net_ip_connect_async_run_socks4_try(d, proxy_host, proxy_port, addrinfo, request, request_len))
                    break;
               }
             EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(request) */
          }
        EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* freeaddrinfo(results) */
     }
 end:
   EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(str) */
}

static void
_efl_net_ip_connect_async_run_socks4a(Efl_Net_Ip_Connect_Async_Data *d, const char *host, const char *port, const char *proxy)
{
   int fd = -1;
   char *str;
   const char *proxy_user, *proxy_pass, *proxy_host, *proxy_port;
   struct sockaddr_storage proxy_addr;
   socklen_t proxy_addrlen;
   Eina_Error err;
   struct addrinfo *results = NULL;
   struct addrinfo hints = {
     .ai_socktype = d->type,
     .ai_protocol = d->protocol,
     .ai_family = AF_INET,
   };
   int r;

   if (strchr(host, ':'))
     {
        DBG("SOCKSv4 only handles IPv4. Wanted host=%s", host);
        d->error = EAFNOSUPPORT;
        return;
     }

   if ((d->type != SOCK_STREAM) || (d->protocol != IPPROTO_TCP))
     {
        DBG("SOCKSv4 only accepts TCP requests. Wanted type=%#x, protocol=%#x", d->type, d->protocol);
        d->error = EPROTONOSUPPORT;
        return;
     }

   DBG("proxy connection to %s:%s using socks4a://%s", host, port, proxy);

   str = strdup(proxy);
   EINA_THREAD_CLEANUP_PUSH(free, str);

   if (!_efl_net_ip_port_user_pass_split(str, &proxy_host, &proxy_port, &proxy_user, &proxy_pass))
     {
        ERR("Invalid proxy string: socks4a://%s", proxy);
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }
   if (!proxy_user) proxy_user = "";
   if (!proxy_port) proxy_port = "1080";

   err = _efl_net_ip_resolve_and_connect(proxy_host, proxy_port, SOCK_STREAM, IPPROTO_TCP, &fd, (struct sockaddr *)&proxy_addr, &proxy_addrlen);
   if (err)
     {
        DBG("couldn't connect to socks4a://%s: %s", proxy, eina_error_msg_get(err));
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }

   DBG("connected fd=%d to socks4a://%s", fd, proxy);
   EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);

   /* we just resolve the port number here */
   do
     r = getaddrinfo(NULL, port, &hints, &results);
   while ((r == EAI_AGAIN) || ((r == EAI_SYSTEM) && (errno == EINTR)));
   if (r != 0)
     {
        DBG("couldn't resolve port='%s': %s", port, gai_strerror(r));
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_HOST;
     }
   else
     {
        const struct addrinfo *addrinfo;
        Efl_Net_Socks4_Request *request;
        size_t request_len;

        d->error = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
        EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)freeaddrinfo, results);

        request_len = sizeof(Efl_Net_Socks4_Request) + strlen(proxy_user) + 1 + strlen(host) + 1;
        request = malloc(request_len);
        if (request)
          {
             request->version = 0x04;
             request->command = EFL_NET_SOCKS4_REQUEST_COMMAND_CONNECT;
             memcpy(request->indent, proxy_user, strlen(proxy_user) + 1);
             memcpy(request->indent + strlen(proxy_user) + 1, host, strlen(host) + 1);
             EINA_THREAD_CLEANUP_PUSH(free, request);
             for (addrinfo = results; addrinfo != NULL; addrinfo = addrinfo->ai_next)
               {
                  struct sockaddr_in *a = (struct sockaddr_in *)addrinfo->ai_addr;
                  ssize_t s;

                  if (addrinfo->ai_socktype != d->type) continue;
                  if (addrinfo->ai_protocol != d->protocol) continue;
                  if (addrinfo->ai_family != AF_INET) continue;

                  request->port = a->sin_port;
                  request->ipv4[0] = 0;
                  request->ipv4[1] = 0;
                  request->ipv4[2] = 0;
                  request->ipv4[3] = 255;

                  s = send(fd, request, request_len, MSG_NOSIGNAL);
                  if (s != (ssize_t)request_len)
                    {
                       if (s < 0)
                         DBG("couldn't send proxy request: %s", strerror(errno));
                       else
                         DBG("couldn't send proxy request: need %zu, did %zd", request_len, s);
                    }
                  else
                    {
                       Efl_Net_Socks4_Reply reply;
                       s = recv(fd, &reply, sizeof(reply), MSG_NOSIGNAL);
                       if (s != sizeof(reply))
                         {
                            if (s < 0)
                              DBG("couldn't recv proxy reply: %s", strerror(errno));
                            else
                              DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(reply), s);
                         }
                       else
                         {
                            if (reply.status != EFL_NET_SOCKS4_REPLY_STATUS_GRANTED)
                              DBG("proxy rejected request status=%#x", reply.status);
                            else
                              {
                                 d->addr4.sin_family = AF_INET;
                                 d->addr4.sin_port = a->sin_port;
                                 memcpy(&d->addr4.sin_addr, reply.ipv4, 4);
                                 d->addrlen = sizeof(struct sockaddr_in);
                                 d->sockfd = fd;
                                 d->error = 0;
                              }
                         }
                    }
                  break;
               }
             EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(request) */
          }
        EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* freeaddrinfo(results) */
     }
   EINA_THREAD_CLEANUP_POP(d->sockfd == -1); /* we need fd only on success */
 end:
   EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(str) */
}

typedef enum _Efl_Net_Socks5_Auth {
  EFL_NET_SOCKS5_AUTH_NONE = 0x00,
  EFL_NET_SOCKS5_AUTH_GSSAPI = 0x01,
  EFL_NET_SOCKS5_AUTH_USER_PASS = 0x02,
  EFL_NET_SOCKS5_AUTH_FAILED = 0xff
} Efl_Net_Socks5_Auth;

typedef struct _Efl_Net_Socks5_Greeting {
   uint8_t version; /* = 0x5 */
   uint8_t auths_count;
   uint8_t auths[1]; /* series of Efl_Net_Socks5_Auth */
} Efl_Net_Socks5_Greeting;

typedef struct _Efl_Net_Socks5_Greeting_Reply {
   uint8_t version; /* = 0x5 */
   uint8_t auth; /* Efl_Net_Socks5_Auth */
} Efl_Net_Socks5_Greeting_Reply;

typedef enum _Efl_Net_Socks5_Request_Command {
  EFL_NET_SOCKS5_REQUEST_COMMAND_TCP_CONNECT = 0x01,
  EFL_NET_SOCKS5_REQUEST_COMMAND_TCP_BIND = 0x02,
  EFL_NET_SOCKS5_REQUEST_COMMAND_UDP_ASSOCIATE = 0x03
} Efl_Net_Socks5_Request_Command;

typedef enum _Efl_Net_Socks5_Address_Type {
  EFL_NET_SOCKS5_ADDRESS_TYPE_IPV4 = 0x01,
  EFL_NET_SOCKS5_ADDRESS_TYPE_NAME = 0x03,
  EFL_NET_SOCKS5_ADDRESS_TYPE_IPV6 = 0x04
} Efl_Net_Socks5_Address_Type;

typedef struct _Efl_Net_Socks5_Request {
   uint8_t version; /* = 0x5 */
   uint8_t command; /* Efl_Net_Socks5_Command */
   uint8_t reserved;
   uint8_t address_type; /* Efl_Net_Socks5_Address_Type */
   /* follows:
    *  - one of:
    *    - 4 bytes for IPv4
    *    - 16 bytes for IPv6
    *    - 1 byte (size) + N bytes of name
    *  - uint16_t port
    */
} Efl_Net_Socks5_Request;

typedef struct _Efl_Net_Socks5_Address_Ipv4 {
   uint8_t address[4];
   uint16_t port;
} Efl_Net_Socks5_Address_Ipv4;

typedef struct _Efl_Net_Socks5_Address_Ipv6 {
   uint8_t address[16];
   uint16_t port;
} Efl_Net_Socks5_Address_Ipv6;

typedef struct _Efl_Net_Socks5_Request_Ipv4 {
   Efl_Net_Socks5_Request base;
   Efl_Net_Socks5_Address_Ipv4 ipv4;
} Efl_Net_Socks5_Request_Ipv4;

 typedef struct _Efl_Net_Socks5_Request_Ipv6 {
   Efl_Net_Socks5_Request base;
   Efl_Net_Socks5_Address_Ipv6 ipv6;
} Efl_Net_Socks5_Request_Ipv6;

static Efl_Net_Socks5_Request *
efl_net_socks5_request_addr_new(Efl_Net_Socks5_Request_Command command, const struct sockaddr *addr, size_t *p_request_len)
{
   if (addr->sa_family == AF_INET)
     {
        const struct sockaddr_in *a = (const struct sockaddr_in *)addr;
        Efl_Net_Socks5_Request_Ipv4 *request;

        *p_request_len = sizeof(Efl_Net_Socks5_Request_Ipv4);
        request = malloc(*p_request_len);
        EINA_SAFETY_ON_NULL_RETURN_VAL(request, NULL);
        request->base.version = 0x05;
        request->base.command = command;
        request->base.reserved = 0;
        request->base.address_type = EFL_NET_SOCKS5_ADDRESS_TYPE_IPV4;
        memcpy(request->ipv4.address, &a->sin_addr, 4);
        request->ipv4.port = a->sin_port;
        return &request->base;
     }
   else
     {
        const struct sockaddr_in6 *a = (const struct sockaddr_in6 *)addr;
        Efl_Net_Socks5_Request_Ipv6 *request;

        *p_request_len = sizeof(Efl_Net_Socks5_Request_Ipv6);
        request = malloc(*p_request_len);
        EINA_SAFETY_ON_NULL_RETURN_VAL(request, NULL);
        request->base.version = 0x05;
        request->base.command = command;
        request->base.reserved = 0;
        request->base.address_type = EFL_NET_SOCKS5_ADDRESS_TYPE_IPV6;
        memcpy(request->ipv6.address, &a->sin6_addr, 16);
        request->ipv6.port = a->sin6_port;
        return &request->base;
     }
}

/* port must be network endianess */
static Efl_Net_Socks5_Request *
efl_net_socks5_request_name_new(Efl_Net_Socks5_Request_Command command, const char *name, uint16_t port, size_t *p_request_len)
{
   Efl_Net_Socks5_Request *request;
   uint8_t namelen = strlen(name);
   uint8_t *p;

   *p_request_len = sizeof(Efl_Net_Socks5_Request) + 1 + namelen + 2;
   request = malloc(*p_request_len);
   EINA_SAFETY_ON_NULL_RETURN_VAL(request, NULL);
   request->version = 0x05;
   request->command = command;
   request->reserved = 0;
   request->address_type = EFL_NET_SOCKS5_ADDRESS_TYPE_NAME;

   p = (uint8_t *)request + sizeof(Efl_Net_Socks5_Request);
   *p = namelen;
   p++;
   memcpy(p, name, namelen);
   p += namelen;

   memcpy(p, &port, sizeof(port));
   return request;
}

typedef enum _Efl_Net_Socks5_Reply_Status {
  EFL_NET_SOCKS5_REPLY_STATUS_GRANTED = 0x00,
  EFL_NET_SOCKS5_REPLY_STATUS_GENERAL_FAILURE = 0x01,
  EFL_NET_SOCKS5_REPLY_STATUS_REJECTED_BY_RULESET = 0x02,
  EFL_NET_SOCKS5_REPLY_STATUS_NETWORK_UNREACHABLE = 0x03,
  EFL_NET_SOCKS5_REPLY_STATUS_HOST_UNREACHABLE = 0x04,
  EFL_NET_SOCKS5_REPLY_STATUS_CONNECTION_REFUSED = 0x05,
  EFL_NET_SOCKS5_REPLY_STATUS_TTL_EXPIRED = 0x06,
  EFL_NET_SOCKS5_REPLY_STATUS_PROTOCOL_ERROR = 0x07,
  EFL_NET_SOCKS5_REPLY_STATUS_ADDRESS_TYPE_UNSUPORTED = 0x08,
} Efl_Net_Socks5_Reply_Status;

typedef struct _Efl_Net_Socks5_Reply {
   uint8_t version; /* = 0x5 */
   uint8_t status;
   uint8_t null; /* = 0 */
   uint8_t address_type; /* Efl_Net_Socks5_Address_Type */
   /* follows:
    *  - one of:
    *    - 4 bytes for IPv4
    *    - 16 bytes for IPv6
    *    - 1 byte (size) + N bytes name
    *  - uint16_t port
    */
} Efl_Net_Socks5_Reply;

typedef struct _Efl_Net_Socks5_Reply_Ipv4 {
   Efl_Net_Socks5_Reply base;
   Efl_Net_Socks5_Address_Ipv4 ipv4;
} Efl_Net_Socks5_Reply_Ipv4;

 typedef struct _Efl_Net_Socks5_Reply_Ipv6 {
   Efl_Net_Socks5_Reply base;
   Efl_Net_Socks5_Address_Ipv6 ipv6;
} Efl_Net_Socks5_Reply_Ipv6;

static Eina_Bool
_efl_net_ip_connect_async_run_socks5_auth_user_pass(int fd, const char *user, const char *pass, const char *proxy_protocol, const char *proxy_host, const char *proxy_port)
{
   uint8_t user_len = user ? strlen(user) : 0;
   uint8_t pass_len = pass ? strlen(pass) : 0;
   size_t len = 1 + 1 + user_len + 1 + pass_len;
   char *msg;
   Eina_Bool ret = EINA_FALSE;
   ssize_t s;

   msg = malloc(len);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, EINA_FALSE);
   EINA_THREAD_CLEANUP_PUSH(free, msg);

   msg[0] = 0x01; /* version */
   msg[1] = user_len;
   if (user) memcpy(msg + 1 + 1, user, user_len);
   msg[1 + 1 + user_len] = pass_len;
   if (pass) memcpy(msg + 1 + 1 + user_len + 1, pass, pass_len);

   DBG("authenticate user='%s' pass=%hhu (bytes) to proxy %s://%s:%s", user, pass_len, proxy_protocol, proxy_host, proxy_port);

   s = send(fd, msg, len, MSG_NOSIGNAL);
   if (s != (ssize_t)len)
     {
        if (s < 0)
          DBG("couldn't send user-password authentication to fd=%d %s://%s:%s: %s", fd, proxy_protocol, proxy_host, proxy_port, strerror(errno));
        else
          DBG("couldn't send user-password authentication: need %zu, did %zd", len, s);
     }
   else
     {
        uint8_t reply[2];

        s = recv(fd, &reply, sizeof(reply), MSG_NOSIGNAL);
        if (s != (ssize_t)sizeof(reply))
          {
             if (s < 0)
               DBG("couldn't recv user-password authentication reply from fd=%d %s://%s:%s: %s", fd, proxy_protocol, proxy_host, proxy_port, strerror(errno));
             else
               DBG("couldn't recv user-password authentication reply: need %zu, did %zd", len, s);
          }
        else
          {
             if (reply[1] != 0)
               DBG("proxy authentication failed user='%s' pass=%hhu (bytes) to proxy %s://%s:%s: reason=%#x", user, pass_len, proxy_protocol, proxy_host, proxy_port, reply[1]);
             else
               {
                DBG("successfully authenticated user=%s with proxy fd=%d %s://%s:%s", user, fd, proxy_protocol, proxy_host, proxy_port);
                ret = EINA_TRUE;
             }
          }
     }

   EINA_THREAD_CLEANUP_POP(EINA_TRUE);

   return ret;
}

static Eina_Bool
_efl_net_ip_connect_async_run_socks5_try(Efl_Net_Ip_Connect_Async_Data *d, const char *proxy_host, const char *proxy_port, const char *proxy_user, const char *proxy_pass, Efl_Net_Socks5_Request_Command cmd, const struct addrinfo *addrinfo)
{
   char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")];
   Efl_Net_Socks5_Greeting greeting = {
     .version = 0x05,
     .auths_count = 1,
     .auths = { proxy_user ? EFL_NET_SOCKS5_AUTH_USER_PASS : EFL_NET_SOCKS5_AUTH_NONE },
   };
   struct sockaddr_storage proxy_addr;
   socklen_t proxy_addrlen;
   int fd;
   Eina_Error err;
   Eina_Bool ret = EINA_FALSE;
   ssize_t s;

   err = _efl_net_ip_resolve_and_connect(proxy_host, proxy_port, SOCK_STREAM, IPPROTO_TCP, &fd, (struct sockaddr *)&proxy_addr, &proxy_addrlen);
   if (err)
     {
        DBG("couldn't connect to socks5://%s:%s: %s", proxy_host, proxy_port, eina_error_msg_get(err));
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        return EINA_TRUE; /* no point in continuing on this error */
     }

   EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);
   if (eina_log_domain_level_check(_ecore_con_log_dom, EINA_LOG_LEVEL_DBG))
     {
        if (efl_net_ip_port_fmt(buf, sizeof(buf), addrinfo->ai_addr))
          DBG("resolved address='%s' to %s. Connect using fd=%d socks5://%s:%s", d->address, buf, fd, proxy_host, proxy_port);
     }

   s = send(fd, &greeting, sizeof(greeting), MSG_NOSIGNAL);
   if (s != (ssize_t)sizeof(greeting))
     {
        if (s < 0)
          DBG("couldn't request connection to host=%s fd=%d socks5://%s:%s: %s", buf, fd, proxy_host, proxy_port, strerror(errno));
        else
          DBG("couldn't send proxy request: need %zu, did %zd", sizeof(greeting), s);
     }
   else
     {
        Efl_Net_Socks5_Greeting_Reply greeting_reply;
        s = recv(fd, &greeting_reply, sizeof(greeting_reply), MSG_NOSIGNAL);
        if (s != sizeof(greeting_reply))
          {
             if (s < 0)
               DBG("couldn't recv greeting reply of connection to host=%s fd=%d socks5://%s:%s: %s", buf, fd, proxy_host, proxy_port, strerror(errno));
             else
               DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(greeting_reply), s);
          }
        else
          {
             if (greeting_reply.auth != greeting.auths[0])
               DBG("proxy server rejected authentication %#x trying connection to host=%s fd=%d socks5://%s:%s", greeting.auths[0], buf, fd, proxy_host, proxy_port);
             else
               {
                  if ((greeting_reply.auth == EFL_NET_SOCKS5_AUTH_USER_PASS) &&
                      (!_efl_net_ip_connect_async_run_socks5_auth_user_pass(fd, proxy_user, proxy_pass, "socks5", proxy_host, proxy_port)))
                    {
                       d->error = EFL_NET_DIALER_ERROR_PROXY_AUTHENTICATION_FAILED;
                    }
                  else
                    {
                       Efl_Net_Socks5_Request *request;
                       size_t request_len;

                       request = efl_net_socks5_request_addr_new(cmd, addrinfo->ai_addr, &request_len);
                       if (request)
                         {
                            EINA_THREAD_CLEANUP_PUSH(free, request);

                            s = send(fd, request, request_len, MSG_NOSIGNAL);
                            if (s != (ssize_t)request_len)
                              {
                                 if (s < 0)
                                   DBG("couldn't request connection to host=%s fd=%d socks5://%s:%s: %s", buf, fd, proxy_host, proxy_port, strerror(errno));
                                 else
                                   DBG("couldn't send proxy request: need %zu, did %zd", request_len, s);
                              }
                            else if (addrinfo->ai_family == AF_INET)
                              {
                                 Efl_Net_Socks5_Reply_Ipv4 reply;
                                 s = recv(fd, &reply, sizeof(reply), MSG_NOSIGNAL);
                                 if (s != sizeof(reply))
                                   {
                                      if (s < 0)
                                        DBG("couldn't recv reply of connection to host=%s fd=%d socks5://%s:%s: %s", buf, fd, proxy_host, proxy_port, strerror(errno));
                                      else
                                        DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(reply), s);
                                   }
                                 else
                                   {
                                      if (reply.base.status != EFL_NET_SOCKS5_REPLY_STATUS_GRANTED)
                                        DBG("rejected IPv4 connection to host=%s fd=%d socks5://%s:%s: reason=%#x", buf, fd, proxy_host, proxy_port, reply.base.status);
                                      else
                                        {
                                           memcpy(&d->addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
                                           d->addrlen = addrinfo->ai_addrlen;
                                           d->sockfd = fd;
                                           d->error = 0;
                                           ret = EINA_TRUE;
                                           DBG("connected IPv4 to host=%s fd=%d socks5://%s:%s", buf, fd, proxy_host, proxy_port);
                                        }
                                   }
                              }
                            else if (addrinfo->ai_family == AF_INET6)
                              {
                                 Efl_Net_Socks5_Reply_Ipv6 reply;
                                 s = recv(fd, &reply, sizeof(reply), MSG_NOSIGNAL);
                                 if (s != sizeof(reply))
                                   {
                                      if (s < 0)
                                        DBG("couldn't recv reply of connection to host=%s fd=%d socks5://%s:%s: %s", buf, fd, proxy_host, proxy_port, strerror(errno));
                                      else
                                        DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(reply), s);
                                   }
                                 else
                                   {
                                      if (reply.base.status != EFL_NET_SOCKS5_REPLY_STATUS_GRANTED)
                                        DBG("rejected IPv6 connection to host=%s fd=%d socks5://%s:%s: reason=%#x", buf, fd, proxy_host, proxy_port, reply.base.status);
                                      else
                                        {
                                           memcpy(&d->addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
                                           d->addrlen = addrinfo->ai_addrlen;
                                           d->sockfd = fd;
                                           d->error = 0;
                                           ret = EINA_TRUE;
                                           DBG("connected IPv6 to host=%s fd=%d socks5://%s:%s", buf, fd, proxy_host, proxy_port);
                                        }
                                   }
                              }
                            EINA_THREAD_CLEANUP_POP(EINA_TRUE);
                         }
                    }
               }
          }
     }
   EINA_THREAD_CLEANUP_POP(!ret); /* we need fd on success, on failure just close it */
   return ret;
}

static void
_efl_net_ip_connect_async_run_socks5(Efl_Net_Ip_Connect_Async_Data *d, const char *host, const char *port, const char *proxy)
{
   char *str;
   const char *proxy_user, *proxy_pass, *proxy_host, *proxy_port;
   struct addrinfo *results = NULL;
   struct addrinfo hints = {
     .ai_socktype = d->type,
     .ai_protocol = d->protocol,
     .ai_family = AF_UNSPEC,
   };
   Efl_Net_Socks5_Request_Command cmd;
   int r;

   if ((d->type == SOCK_STREAM) && (d->protocol == IPPROTO_TCP))
     cmd = EFL_NET_SOCKS5_REQUEST_COMMAND_TCP_CONNECT;
   else
     {
        DBG("EFL SOCKSv5 only accepts TCP requests at this moment. Wanted type=%#x, protocol=%#x", d->type, d->protocol);
        d->error = EPROTONOSUPPORT;
        return;
     }

   if (strchr(host, ':')) hints.ai_family = AF_INET6;

   DBG("proxy connection to %s:%s using socks5://%s", host, port, proxy);

   str = strdup(proxy);
   EINA_THREAD_CLEANUP_PUSH(free, str);

   if (!_efl_net_ip_port_user_pass_split(str, &proxy_host, &proxy_port, &proxy_user, &proxy_pass))
     {
        ERR("Invalid proxy string: socks5://%s", proxy);
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }
   if (!proxy_port) proxy_port = "1080";

   do
     r = getaddrinfo(host, port, &hints, &results);
   while ((r == EAI_AGAIN) || ((r == EAI_SYSTEM) && (errno == EINTR)));
   if (r != 0)
     {
        DBG("couldn't resolve host='%s', port='%s': %s",
            host, port, gai_strerror(r));
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_HOST;
     }
   else
     {
        const struct addrinfo *addrinfo;

        d->error = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
        EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)freeaddrinfo, results);
        for (addrinfo = results; addrinfo != NULL; addrinfo = addrinfo->ai_next)
          {
             if (addrinfo->ai_socktype != d->type) continue;
             if (addrinfo->ai_protocol != d->protocol) continue;
             if (_efl_net_ip_connect_async_run_socks5_try(d, proxy_host, proxy_port, proxy_user, proxy_pass, cmd, addrinfo))
               break;
          }
        EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* freeaddrinfo(results) */
     }
 end:
   EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(str) */
}

static void
_efl_net_ip_connect_async_run_socks5h(Efl_Net_Ip_Connect_Async_Data *d, const char *host, const char *port, const char *proxy)
{
   int fd = -1;
   char *str;
   const char *proxy_user, *proxy_pass, *proxy_host, *proxy_port;
   struct sockaddr_storage proxy_addr;
   socklen_t proxy_addrlen;
   Eina_Error err;
   Efl_Net_Socks5_Greeting greeting = {
     .version = 0x05,
     .auths_count = 1,
   };
   Efl_Net_Socks5_Request_Command cmd;
   ssize_t s;
   int r;

   if ((d->type == SOCK_STREAM) && (d->protocol == IPPROTO_TCP))
     cmd = EFL_NET_SOCKS5_REQUEST_COMMAND_TCP_CONNECT;
   else
     {
        DBG("EFL SOCKSv5 only accepts TCP requests at this moment. Wanted type=%#x, protocol=%#x", d->type, d->protocol);
        d->error = EPROTONOSUPPORT;
        return;
     }

   DBG("proxy connection to %s:%s using socks5h://%s", host, port, proxy);

   str = strdup(proxy);
   EINA_THREAD_CLEANUP_PUSH(free, str);

   if (!_efl_net_ip_port_user_pass_split(str, &proxy_host, &proxy_port, &proxy_user, &proxy_pass))
     {
        ERR("Invalid proxy string: socks5h://%s", proxy);
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }
   if (!proxy_port) proxy_port = "1080";

   greeting.auths[0] = proxy_user ? EFL_NET_SOCKS5_AUTH_USER_PASS : EFL_NET_SOCKS5_AUTH_NONE;

   err = _efl_net_ip_resolve_and_connect(proxy_host, proxy_port, SOCK_STREAM, IPPROTO_TCP, &fd, (struct sockaddr *)&proxy_addr, &proxy_addrlen);
   if (err)
     {
        DBG("couldn't connect to socks5h://%s: %s", proxy, eina_error_msg_get(err));
        d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
        goto end;
     }

   DBG("connected fd=%d to socks5h://%s", fd, proxy);
   EINA_THREAD_CLEANUP_PUSH(_cleanup_close, &fd);

   s = send(fd, &greeting, sizeof(greeting), MSG_NOSIGNAL);
   if (s != (ssize_t)sizeof(greeting))
     {
        if (s < 0)
          DBG("couldn't request connection to host=%s:%s fd=%d socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, strerror(errno));
        else
          DBG("couldn't send proxy request: need %zu, did %zd", sizeof(greeting), s);
     }
   else
     {
        Efl_Net_Socks5_Greeting_Reply greeting_reply;
        s = recv(fd, &greeting_reply, sizeof(greeting_reply), MSG_NOSIGNAL);
        if (s != sizeof(greeting_reply))
          {
             if (s < 0)
               DBG("couldn't recv greeting reply of connection to host=%s:%s fd=%d socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, strerror(errno));
             else
               DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(greeting_reply), s);
          }
        else
          {
             if (greeting_reply.auth != greeting.auths[0])
               DBG("proxy server rejected authentication %#x trying connection to host=%s:%s fd=%d socks5h://%s:%s", greeting.auths[0], host, port, fd, proxy_host, proxy_port);
             else
               {
                  if ((greeting_reply.auth == EFL_NET_SOCKS5_AUTH_USER_PASS) &&
                      (!_efl_net_ip_connect_async_run_socks5_auth_user_pass(fd, proxy_user, proxy_pass, "socks5h", proxy_host, proxy_port)))
                    {
                       d->error = EFL_NET_DIALER_ERROR_PROXY_AUTHENTICATION_FAILED;
                    }
                  else
                    {
                       struct addrinfo *results = NULL;
                       struct addrinfo hints = {
                         .ai_socktype = d->type,
                         .ai_protocol = d->protocol,
                         .ai_family = AF_UNSPEC,
                       };

                       if (strchr(host, ':')) hints.ai_family = AF_INET6;

                       /* we just resolve the port number here */
                       do
                         r = getaddrinfo(NULL, port, &hints, &results);
                       while ((r == EAI_AGAIN) || ((r == EAI_SYSTEM) && (errno == EINTR)));
                       if (r != 0)
                         {
                            DBG("couldn't resolve port='%s': %s", port, gai_strerror(r));
                            d->error = EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_HOST;
                         }
                       else
                         {
                            const struct addrinfo *addrinfo;

                            d->error = EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
                            EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)freeaddrinfo, results);
                            for (addrinfo = results; addrinfo != NULL; addrinfo = addrinfo->ai_next)
                              {
                                 Efl_Net_Socks5_Request *request;
                                 size_t request_len;
                                 uint16_t port_num;

                                 if (addrinfo->ai_socktype != d->type) continue;
                                 if (addrinfo->ai_protocol != d->protocol) continue;

                                 if (addrinfo->ai_family == AF_INET)
                                   port_num = ((const struct sockaddr_in *)addrinfo->ai_addr)->sin_port;
                                 else
                                   port_num = ((const struct sockaddr_in6 *)addrinfo->ai_addr)->sin6_port;

                                 request = efl_net_socks5_request_name_new(cmd, host, port_num, &request_len);
                                 if (request)
                                   {
                                      EINA_THREAD_CLEANUP_PUSH(free, request);

                                      s = send(fd, request, request_len, MSG_NOSIGNAL);
                                      if (s != (ssize_t)request_len)
                                        {
                                           if (s < 0)
                                             DBG("couldn't request connection to host=%s:%s fd=%d socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, strerror(errno));
                                           else
                                             DBG("couldn't send proxy request: need %zu, did %zd", request_len, s);
                                        }
                                      else
                                        {
                                           Efl_Net_Socks5_Reply reply;

                                           s = recv(fd, &reply, sizeof(reply), MSG_NOSIGNAL);
                                           if (s != sizeof(reply))
                                             {
                                                if (s < 0)
                                                  DBG("couldn't recv reply of connection to host=%s:%s fd=%d socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, strerror(errno));
                                                else
                                                  DBG("couldn't recv proxy reply: need %zu, did %zd", sizeof(reply), s);
                                             }
                                           else
                                             {
                                                if (reply.status != EFL_NET_SOCKS5_REPLY_STATUS_GRANTED)
                                                  DBG("rejected connection to host=%s:%s fd=%d socks5h://%s:%s: reason=%#x", host, port, fd, proxy_host, proxy_port, reply.status);
                                                else if (reply.address_type == EFL_NET_SOCKS5_ADDRESS_TYPE_IPV4)
                                                  {
                                                     Efl_Net_Socks5_Address_Ipv4 ipv4;

                                                     s = recv(fd, &ipv4, sizeof(ipv4), MSG_NOSIGNAL);
                                                     if (s != sizeof(ipv4))
                                                       {
                                                          if (s < 0)
                                                            DBG("couldn't recv ipv4 of connection to host=%s:%s fd=%d socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, strerror(errno));
                                                          else
                                                            DBG("couldn't recv proxy ipv4: need %zu, did %zd", sizeof(ipv4), s);
                                                       }
                                                     else
                                                       {
                                                          d->addr4.sin_family = AF_INET;
                                                          d->addr4.sin_port = ipv4.port;
                                                          memcpy(&d->addr4.sin_addr, ipv4.address, 4);
                                                          d->addrlen = sizeof(struct sockaddr_in);
                                                          d->sockfd = fd;
                                                          d->error = 0;
                                                          DBG("connected IPv4 to host=%s:%s fd=%d socks5h://%s:%s", host, port, fd, proxy_host, proxy_port);
                                                       }
                                                  }
                                                else if (reply.address_type == EFL_NET_SOCKS5_ADDRESS_TYPE_IPV6)
                                                  {
                                                     Efl_Net_Socks5_Address_Ipv6 ipv6;

                                                     s = recv(fd, &ipv6, sizeof(ipv6), MSG_NOSIGNAL);
                                                     if (s != sizeof(ipv6))
                                                       {
                                                          if (s < 0)
                                                            DBG("couldn't recv ipv6 of connection to host=%s:%s fd=%d socks5h://%s:%s: %s", host, port, fd, proxy_host, proxy_port, strerror(errno));
                                                          else
                                                            DBG("couldn't recv proxy ipv6: need %zu, did %zd", sizeof(ipv6), s);
                                                       }
                                                     else
                                                       {
                                                          d->addr6.sin6_family = AF_INET;
                                                          d->addr6.sin6_port = ipv6.port;
                                                          memcpy(&d->addr6.sin6_addr, ipv6.address, 16);
                                                          d->addrlen = sizeof(struct sockaddr_in);
                                                          d->sockfd = fd;
                                                          d->error = 0;
                                                          DBG("connected IPv6 to host=%s:%s fd=%d socks5h://%s:%s", host, port, fd, proxy_host, proxy_port);
                                                       }
                                                  }
                                                else
                                                  {
                                                     /* most proxy servers will return a failure instead of this, but let's guard and log */
                                                     DBG("couldn't resolve host %s:%s fd=%d socks5h://%s:%s",  host, port, fd, proxy_host, proxy_port);
                                                  }
                                             }
                                        }

                                      EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(request) */
                                      break;
                                   }
                              }
                            EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* freeaddrinfo(results) */
                         }
                    }
               }
          }
     }
   EINA_THREAD_CLEANUP_POP(d->sockfd == -1); /* we need fd only on success */
 end:
   EINA_THREAD_CLEANUP_POP(EINA_TRUE); /* free(str) */
}

static void
_efl_net_ip_connect_async_run(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Ip_Connect_Async_Data *d = data;
   const char *host, *port, *proxy;
   char *addrcopy;
   char **proxies = NULL;
   int proxies_idx = 0;
   Eina_Bool is_libproxy = EINA_FALSE;

   addrcopy = strdup(d->address);
   if (!addrcopy)
     {
        d->error = errno;
        return;
     }

   if (!efl_net_ip_port_split(addrcopy, &host, &port))
     {
        d->error = EINVAL;
        free(addrcopy);
        return;
     }
   if (!port) port = "0";
   EINA_THREAD_CLEANUP_PUSH(free, addrcopy);

   proxy = d->proxy;

   if ((!proxy) && (_ecore_con_libproxy.factory))
     {
        /* libproxy is thread-safe but not cancellable.  the provided
         * parameter must be a URL with schema, otherwise it won't
         * return anything.
         */
        Eina_Stringshare *url;

        url = eina_stringshare_printf("%s://%s:%s", d->protocol == IPPROTO_UDP ? "udp" : "tcp", host, port);
        if (!url)
          {
             ERR("Could not assemble URL");
          }
        else
          {
             proxies = ecore_con_libproxy_proxies_get(url);
             eina_stringshare_del(url);
          }
     }

   EINA_THREAD_CLEANUP_PUSH((Eina_Free_Cb)ecore_con_libproxy_proxies_free, proxies);
 next_proxy:
   if ((!proxy) && (proxies) && (proxies_idx >= 0))
     {
        proxy = proxies[proxies_idx];
        if (!proxy)
          {
             is_libproxy = EINA_FALSE;
             proxies_idx = -1;
          }
        else
          {
             if (strcmp(proxy, "direct://") == 0)
               {
                  /* give a chance to try envvars */
                  proxy = NULL;
                  is_libproxy = EINA_FALSE;
               }
             else
               {
                  DBG("libproxy said %s for host='%s' port='%s'", proxy, host, port);
                  is_libproxy = EINA_TRUE;
               }
             proxies_idx++;
          }
     }

   if (!proxy)
     {
        proxy = d->proxy_env;
        if (!proxy)
          proxy = "";
        else
          {
             if (_efl_net_ip_no_proxy(host, d->no_proxy_strv))
               proxy = "";
             else
               DBG("using proxy %s from envvar", proxy);
          }
     }

   /* allows ecore_thread_cancel() to cancel at some points, see
    * man:pthreads(7).
    */
   eina_thread_cancellable_set(EINA_TRUE, NULL);

   if (!proxy[0])
     _efl_net_ip_connect_async_run_direct(d, host, port);
   else if (eina_str_has_prefix(proxy, "socks4://"))
     _efl_net_ip_connect_async_run_socks4(d, host, port, proxy + strlen("socks4://"));
   else if (eina_str_has_prefix(proxy, "socks5://"))
     _efl_net_ip_connect_async_run_socks5(d, host, port, proxy + strlen("socks5://"));
   else if (eina_str_has_prefix(proxy, "socks4a://"))
     _efl_net_ip_connect_async_run_socks4a(d, host, port, proxy + strlen("socks4a://"));
   else if (eina_str_has_prefix(proxy, "socks5h://"))
     _efl_net_ip_connect_async_run_socks5h(d, host, port, proxy + strlen("socks5h://"));
   else if (eina_str_has_prefix(proxy, "socks://"))
     {
        _efl_net_ip_connect_async_run_socks5(d, host, port, proxy + strlen("socks://"));
        if (d->error)
          _efl_net_ip_connect_async_run_socks4(d, host, port, proxy + strlen("socks://"));
     }
   else if (!strstr(proxy, "://"))
     {
        _efl_net_ip_connect_async_run_socks5(d, host, port, proxy);
        if (d->error)
          _efl_net_ip_connect_async_run_socks4(d, host, port, proxy);
     }
   else
     {
        if (d->proxy)
          {
             d->error = ENOTSUP;
             ERR("proxy protocol not supported '%s'", proxy);
          }
        else
          {
             if (is_libproxy)
               {
                  DBG("libproxy said %s but it's not supported, try next proxy", proxy);
                  proxy = NULL;
                  goto next_proxy;
               }
             /* maybe bogus envvar, ignore it */
             WRN("proxy protocol not supported '%s', connect directly", proxy);
             _efl_net_ip_connect_async_run_direct(d, host, port);
          }
     }

   if ((d->error) && (!d->proxy) && (proxy[0] != '\0'))
     {
        if (is_libproxy)
          {
             DBG("libproxy said %s but it failed, try next proxy", proxy);
             proxy = NULL;
             goto next_proxy;
          }

        WRN("error using proxy '%s' from environment, try direct connect", proxy);
        _efl_net_ip_connect_async_run_direct(d, host, port);
     }

   eina_thread_cancellable_set(EINA_FALSE, NULL);
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);
}

static void
_efl_net_ip_connect_async_data_free(Efl_Net_Ip_Connect_Async_Data *d)
{
   free(d->address);
   free(d->proxy);
   free(d->proxy_env);
   if (d->no_proxy_strv)
     {
        free(d->no_proxy_strv[0]);
        free(d->no_proxy_strv);
     }
   free(d);
}

static void
_efl_net_ip_connect_async_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Ip_Connect_Async_Data *d = data;

#ifdef FD_CLOEXEC
   /* if it wasn't a close on exec, release the socket to be passed to child */
   if ((!d->close_on_exec) && (d->sockfd >= 0))
     {
        int flags = fcntl(d->sockfd, F_GETFD);
        if (flags < 0)
          {
             d->error = errno;
             ERR("fcntl(%d, F_GETFD): %s", d->sockfd, strerror(errno));
             close(d->sockfd);
             d->sockfd = -1;
          }
        else
          {
             flags &= (~FD_CLOEXEC);
             if (fcntl(d->sockfd, F_SETFD, flags) < 0)
               {
                  d->error = errno;
                  ERR("fcntl(%d, F_SETFD, %#x): %s", d->sockfd, flags, strerror(errno));
                  close(d->sockfd);
                  d->sockfd = -1;
               }
          }
     }
#endif

   d->cb((void *)d->data, &d->addr, d->addrlen, d->sockfd, d->error);
   _efl_net_ip_connect_async_data_free(d);
}

static void
_efl_net_ip_connect_async_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Efl_Net_Ip_Connect_Async_Data *d = data;
   if (d->sockfd >= 0) close(d->sockfd);
   _efl_net_ip_connect_async_data_free(d);
}

Ecore_Thread *
efl_net_ip_connect_async_new(const char *address, const char *proxy, const char *proxy_env, const char *no_proxy_env, int type, int protocol, Eina_Bool close_on_exec, Efl_Net_Connect_Async_Cb cb, const void *data)
{
   Efl_Net_Ip_Connect_Async_Data *d;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, NULL);

   d = calloc(1, sizeof(Efl_Net_Ip_Connect_Async_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(d, NULL);

   d->address = strdup(address);
   EINA_SAFETY_ON_NULL_GOTO(d->address, error_address);

   if (proxy)
     {
        d->proxy = strdup(proxy);
        EINA_SAFETY_ON_NULL_GOTO(d->proxy, error_proxy);
     }
   if (proxy_env)
     {
        d->proxy_env = strdup(proxy_env);
        EINA_SAFETY_ON_NULL_GOTO(d->proxy_env, error_proxy_env);
     }
   if (no_proxy_env)
     {
        d->no_proxy_strv = eina_str_split(no_proxy_env, ",", 0);
        EINA_SAFETY_ON_NULL_GOTO(d->no_proxy_strv, error_no_proxy_strv);
     }

   d->cb = cb;
   d->data = data;
   d->addrlen = 0;
   d->close_on_exec = close_on_exec;
   d->type = type;
   d->protocol = protocol;

   d->sockfd = -1;
   d->error = 0;


   return ecore_thread_run(_efl_net_ip_connect_async_run,
                           _efl_net_ip_connect_async_end,
                           _efl_net_ip_connect_async_cancel,
                           d);

 error_no_proxy_strv:
   free(d->proxy_env);
 error_proxy_env:
   free(d->proxy);
 error_proxy:
   free(d->address);
 error_address:
   free(d);
   return NULL;
}

Eina_Bool
ecore_con_libproxy_init(void)
{
   if (!_ecore_con_libproxy.mod)
     {
#define LOAD(x)                                                         \
          if (!_ecore_con_libproxy.mod) {                               \
             _ecore_con_libproxy.mod = eina_module_new(x);              \
             if (_ecore_con_libproxy.mod) {                             \
                if (!eina_module_load(_ecore_con_libproxy.mod)) {       \
                   eina_module_free(_ecore_con_libproxy.mod);           \
                   _ecore_con_libproxy.mod = NULL;                      \
                }                                                       \
             }                                                          \
          }
#if defined(_WIN32) || defined(__CYGWIN__)
        LOAD("libproxy-1.dll");
        LOAD("libproxy.dll");
#elif defined(__APPLE__) && defined(__MACH__)
        LOAD("libproxy.1.dylib");
        LOAD("libproxy.dylib");
#else
        LOAD("libproxy.so.1");
        LOAD("libproxy.so");
#endif
#undef LOAD
        if (!_ecore_con_libproxy.mod)
          {
             DBG("Couldn't find libproxy in your system. Continue without it");
             return EINA_FALSE;
          }

#define SYM(x)                                                          \
        if ((_ecore_con_libproxy.x = eina_module_symbol_get(_ecore_con_libproxy.mod, #x)) == NULL) { \
           ERR("libproxy (%s) missing symbol %s", eina_module_file_get(_ecore_con_libproxy.mod), #x); \
           eina_module_free(_ecore_con_libproxy.mod);                   \
           _ecore_con_libproxy.mod = NULL;                              \
           return EINA_FALSE;                                           \
        }

        SYM(px_proxy_factory_new);
        SYM(px_proxy_factory_free);
        SYM(px_proxy_factory_get_proxies);
#undef SYM
        DBG("using libproxy=%s", eina_module_file_get(_ecore_con_libproxy.mod));
     }

   if (!_ecore_con_libproxy.factory)
     _ecore_con_libproxy.factory = _ecore_con_libproxy.px_proxy_factory_new();

   return !!_ecore_con_libproxy.factory;
}

char **
ecore_con_libproxy_proxies_get(const char *url)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_ecore_con_libproxy.px_proxy_factory_get_proxies, NULL);
   return _ecore_con_libproxy.px_proxy_factory_get_proxies(_ecore_con_libproxy.factory, url);
}

void
ecore_con_libproxy_proxies_free(char **proxies)
{
   char **itr;

   if (!proxies) return;

   for (itr = proxies; *itr != NULL; itr++)
     free(*itr);
   free(proxies);
}
