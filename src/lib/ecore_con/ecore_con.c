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
static void        _ecore_con_client_flush(Ecore_Con_Client *cl);

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

#ifdef HAVE_SYSTEMD
int sd_fd_index = 0;
int sd_fd_max = 0;
#endif

void
_ecore_con_client_kill(Ecore_Con_Client *cl)
{
   if (cl->delete_me)
     DBG("Multi kill request for client %p", cl);
   else
     {
        ecore_con_event_client_del(cl);
        if (cl->buf) return;
     }
   INF("Lost client %s", (cl->ip) ? cl->ip : "");
   if (cl->fd_handler)
     ecore_main_fd_handler_del(cl->fd_handler);

   cl->fd_handler = NULL;
}

void
_ecore_con_server_kill(Ecore_Con_Server *svr)
{
   if (svr->delete_me)
     DBG("Multi kill request for svr %p", svr);
   else
     ecore_con_event_server_del(svr);

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
   Ecore_Con_Server *svr;

   if (--_ecore_con_init_count != 0)
     return _ecore_con_init_count;

   eina_log_timing(_ecore_con_log_dom,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   EINA_LIST_FOREACH_SAFE(servers, l, l2, svr)
     {
        Ecore_Con_Event_Server_Add *ev;

        svr->delete_me = EINA_TRUE;
        INF("svr %p is dead", svr);
        /* some pointer hacks here to prevent double frees if people are being stupid */
        EINA_LIST_FREE(svr->event_count, ev)
          ev->server = NULL;
        _ecore_con_server_free(svr);
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
_ecore_con_lookup(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const char *name, Ecore_Con_Dns_Cb done_cb, const void *data)
{
   Ecore_Con_Server *svr;
   Ecore_Con_Lookup *lk;
   struct addrinfo hints;

   if (!name || !done_cb)
     return EINA_FALSE;

   svr = calloc(1, sizeof(Ecore_Con_Server));
   if (!svr)
     return EINA_FALSE;

   lk = malloc(sizeof (Ecore_Con_Lookup));
   if (!lk)
     {
        free(svr);
        return EINA_FALSE;
     }

   lk->done_cb = done_cb;
   lk->data = data;

   svr->name = strdup(name);
   if (!svr->name)
     goto on_error;

   svr->type = ECORE_CON_REMOTE_TCP;
   svr->port = 1025;
   svr->data = lk;
   svr->created = EINA_TRUE;
   svr->reject_excess_clients = EINA_FALSE;
   svr->client_limit = -1;
   svr->clients = NULL;
   svr->ppid = getpid();

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_CANONNAME;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   if (ecore_con_info_get(svr, _ecore_con_lookup_done, svr,
                          &hints))
     return EINA_TRUE;

   free(svr->name);
on_error:
   free(lk);
   free(svr);
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
   Ecore_Con_Server *svr;
   Ecore_Con_Type type;

   if (port < 0 || !name)
     return NULL;  /* local  user   socket: FILE:   ~/.ecore/[name]/[port] */

   /* local  system socket: FILE:   /tmp/.ecore_service|[name]|[port] */
   /* remote system socket: TCP/IP: [name]:[port] */
   svr = calloc(1, sizeof(Ecore_Con_Server));
   if (!svr)
     return NULL;

   ECORE_MAGIC_SET(svr, ECORE_MAGIC_CON_SERVER);
   svr->fd = -1;
   svr->start_time = ecore_time_get();
   svr->type = compl_type;
   svr->port = port;
   svr->data = (void *)data;
   svr->created = EINA_TRUE;
   svr->use_cert = (compl_type & ECORE_CON_SSL & ECORE_CON_LOAD_CERT) == ECORE_CON_LOAD_CERT;
   svr->reject_excess_clients = EINA_FALSE;
   svr->client_limit = -1;
   svr->clients = NULL;
   svr->ppid = getpid();

   servers = eina_list_append(servers, svr);

   svr->name = strdup(name);
   if (!svr->name)
     goto error;

   if (ecore_con_ssl_server_prepare(svr, compl_type & ECORE_CON_SSL))
     goto error;

   type = compl_type & ECORE_CON_TYPE;

   if ((type == ECORE_CON_LOCAL_USER) ||
       (type == ECORE_CON_LOCAL_SYSTEM) ||
       (type == ECORE_CON_LOCAL_ABSTRACT))
   /* Local */
#ifdef _WIN32
     if (!ecore_con_local_listen(svr))
       goto error;
#else
     if (!ecore_con_local_listen(svr, _ecore_con_svr_tcp_handler, svr))
       goto error;
#endif

   if ((type == ECORE_CON_REMOTE_TCP) ||
       (type == ECORE_CON_REMOTE_NODELAY) ||
       (type == ECORE_CON_REMOTE_CORK))
     {
        /* TCP */
        if (!ecore_con_info_tcp_listen(svr, _ecore_con_cb_tcp_listen,
                                       svr))
          goto error;
     }
   else if ((type == ECORE_CON_REMOTE_MCAST) ||
            (type == ECORE_CON_REMOTE_UDP))
     /* UDP and MCAST */
     if (!ecore_con_info_udp_listen(svr, _ecore_con_cb_udp_listen,
                                    svr))
       goto error;

   return svr;

error:
   if (svr->delete_me) return NULL;
   _ecore_con_server_kill(svr);
   return NULL;
}

EAPI Ecore_Con_Server *
ecore_con_server_connect(Ecore_Con_Type compl_type,
                         const char *name,
                         int port,
                         const void *data)
{
   Ecore_Con_Server *svr;
   Ecore_Con_Type type;

   if ((!name) || (!name[0]))
     return NULL;
   /* local  user   socket: FILE:   ~/.ecore/[name]/[port] */
   /* local  system socket: FILE:   /tmp/.ecore_service|[name]|[port] */
   /* remote system socket: TCP/IP: [name]:[port] */
   svr = calloc(1, sizeof(Ecore_Con_Server));
   if (!svr)
     return NULL;

   ECORE_MAGIC_SET(svr, ECORE_MAGIC_CON_SERVER);
   svr->fd = -1;
   svr->type = compl_type;
   svr->port = port;
   svr->data = (void *)data;
   svr->created = EINA_FALSE;
   svr->use_cert = (compl_type & ECORE_CON_SSL & ECORE_CON_LOAD_CERT) == ECORE_CON_LOAD_CERT;
   svr->disable_proxy = (compl_type & ECORE_CON_SUPER_SSL & ECORE_CON_NO_PROXY) == ECORE_CON_NO_PROXY;
   svr->reject_excess_clients = EINA_FALSE;
   svr->clients = NULL;
   svr->client_limit = -1;

   servers = eina_list_append(servers, svr);

   svr->name = strdup(name);
   if (!svr->name)
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
   EINA_SAFETY_ON_TRUE_GOTO(ecore_con_ssl_server_prepare(svr, compl_type & ECORE_CON_SSL), error);

   EINA_SAFETY_ON_TRUE_GOTO(((type == ECORE_CON_REMOTE_TCP) ||
                             (type == ECORE_CON_REMOTE_NODELAY) ||
                             (type == ECORE_CON_REMOTE_CORK) ||
                             (type == ECORE_CON_REMOTE_UDP) ||
                             (type == ECORE_CON_REMOTE_BROADCAST)) &&
                            (port < 0), error);

   if ((type == ECORE_CON_LOCAL_USER) ||
       (type == ECORE_CON_LOCAL_SYSTEM) ||
       (type == ECORE_CON_LOCAL_ABSTRACT))
   /* Local */
#ifdef _WIN32
     if (!ecore_con_local_connect(svr, _ecore_con_cl_handler)) goto error;
#else
     if (!ecore_con_local_connect(svr, _ecore_con_cl_handler, svr)) goto error;
#endif

   if ((type == ECORE_CON_REMOTE_TCP) ||
       (type == ECORE_CON_REMOTE_NODELAY) ||
       (type == ECORE_CON_REMOTE_CORK))
     {
        /* TCP */
        EINA_SAFETY_ON_FALSE_GOTO(ecore_con_info_tcp_connect(svr, _ecore_con_cb_tcp_connect, svr), error);
     }
   else if ((type == ECORE_CON_REMOTE_UDP) || (type == ECORE_CON_REMOTE_BROADCAST))
     /* UDP and MCAST */
     EINA_SAFETY_ON_FALSE_GOTO(ecore_con_info_udp_connect(svr, _ecore_con_cb_udp_connect, svr), error);

   return svr;

error:
   if (svr->delete_me) return NULL;
   _ecore_con_server_kill(svr);
   return NULL;
}

EAPI void
ecore_con_server_timeout_set(Ecore_Con_Server *svr,
                             double timeout)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_timeout_set");
        return;
     }

   if (svr->created)
     svr->client_disconnect_time = timeout;
   else
     svr->disconnect_time = timeout;

   _ecore_con_server_timer_update(svr);
}

EAPI double
ecore_con_server_timeout_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_timeout_get");
        return 0;
     }

   return svr->created ? svr->client_disconnect_time : svr->disconnect_time;
}

EAPI void *
ecore_con_server_del(Ecore_Con_Server *svr)
{
   if (!svr) return NULL;
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_del");
        return NULL;
     }

   if (svr->delete_me)
     return NULL;

   _ecore_con_server_kill(svr);
   return svr->data;
}

EAPI void *
ecore_con_server_data_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_data_get");
        return NULL;
     }

   return svr->data;
}

EAPI void *
ecore_con_server_data_set(Ecore_Con_Server *svr,
                          void *data)
{
   void *ret = NULL;

   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_data_get");
        return NULL;
     }

   ret = svr->data;
   svr->data = data;
   return ret;
}

EAPI Eina_Bool
ecore_con_server_connected_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_connected_get");
        return EINA_FALSE;
     }

   if (svr->connecting)
     return EINA_FALSE;

   return EINA_TRUE;
}

EAPI const Eina_List *
ecore_con_server_clients_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
                         "ecore_con_server_clients_get");
        return NULL;
     }

   return svr->clients;
}

EAPI const char *
ecore_con_server_name_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
                         "ecore_con_server_name_get");
        return NULL;
     }

   return svr->name;
}

EAPI int
ecore_con_server_port_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
                         "ecore_con_server_port_get");
        return -1;
     }
   return svr->port;
}

EAPI int
ecore_con_server_send(Ecore_Con_Server *svr,
                      const void *data,
                      int size)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_send");
        return 0;
     }

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

EAPI void
ecore_con_server_client_limit_set(Ecore_Con_Server *svr,
                                  int client_limit,
                                  char reject_excess_clients)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
                         "ecore_con_server_client_limit_set");
        return;
     }

   svr->client_limit = client_limit;
   svr->reject_excess_clients = reject_excess_clients;
}

EAPI const char *
ecore_con_server_ip_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_ip_get");
        return NULL;
     }

   return svr->ip;
}

EAPI double
ecore_con_server_uptime_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_uptime_get");
        return -1;
     }

   return ecore_time_get() - svr->start_time;
}

EAPI void
ecore_con_server_flush(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, "ecore_con_server_flush");
        return;
     }

   _ecore_con_server_flush(svr);
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
ecore_con_client_send(Ecore_Con_Client *cl,
                      const void *data,
                      int size)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_send");
        return 0;
     }

   EINA_SAFETY_ON_TRUE_RETURN_VAL(cl->delete_me, 0);

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, 0);

   EINA_SAFETY_ON_TRUE_RETURN_VAL(size < 1, 0);

   if (cl->fd_handler)
     ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);

   if (cl->host_server && ((cl->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_UDP))
     sendto(cl->host_server->fd, data, size, 0, (struct sockaddr *)cl->client_addr,
            cl->client_addr_len);
   else 
     {
        if (!cl->buf)
          {
             cl->buf = eina_binbuf_new();
             EINA_SAFETY_ON_NULL_RETURN_VAL(cl->buf, 0);
#ifdef TCP_CORK
             if ((cl->fd >= 0) && ((cl->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_CORK))
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

EAPI Ecore_Con_Server *
ecore_con_client_server_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
                         "ecore_con_client_server_get");
        return NULL;
     }

   return cl->host_server;
}

EAPI Eina_Bool
ecore_con_client_connected_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
                         "ecore_con_client_connected_get");
        return EINA_FALSE;
     }

   return !cl->delete_me;
}

EAPI void
ecore_con_client_timeout_set(Ecore_Con_Client *cl,
                             double timeout)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
                         "ecore_con_client_timeout_set");
        return;
     }

   cl->disconnect_time = timeout;

   _ecore_con_cl_timer_update(cl);
}

EAPI double
ecore_con_client_timeout_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_timeout_get");
        return 0;
     }

   return cl->disconnect_time;
}

EAPI void *
ecore_con_client_del(Ecore_Con_Client *cl)
{
   if (!cl) return NULL;
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_del");
        return NULL;
     }

   _ecore_con_client_kill(cl);
   return cl->data;
}

EAPI void
ecore_con_client_data_set(Ecore_Con_Client *cl,
                          const void *data)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_data_set");
        return;
     }

   cl->data = (void *)data;
}

EAPI void *
ecore_con_client_data_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_data_get");
        return NULL;
     }

   return cl->data;
}

EAPI const char *
ecore_con_client_ip_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_ip_get");
        return NULL;
     }
   if (!cl->ip)
     cl->ip = _ecore_con_pretty_ip(cl->client_addr);

   return cl->ip;
}

EAPI int
ecore_con_client_port_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_port_get");
        return -1;
     }
   if (cl->client_addr->sa_family == AF_INET)
     return ((struct sockaddr_in *)cl->client_addr)->sin_port;
#ifdef HAVE_IPV6
   return ((struct sockaddr_in6 *)cl->client_addr)->sin6_port;
#else
   return -1;
#endif
}

EAPI double
ecore_con_client_uptime_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_uptime_get");
        return -1;
     }

   return ecore_time_get() - cl->start_time;
}

EAPI void
ecore_con_client_flush(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, "ecore_con_client_flush");
        return;
     }

   _ecore_con_client_flush(cl);
}

EAPI int
ecore_con_server_fd_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
        ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER, __func__);
        return -1;
     }
   if (svr->created) return -1;
   if (svr->delete_me) return -1;
   return ecore_main_fd_handler_fd_get(svr->fd_handler);
}

EAPI int
ecore_con_client_fd_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
        ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT, __func__);
        return -1;
     }
   return ecore_main_fd_handler_fd_get(cl->fd_handler);
}

/**
 * @}
 */

void
ecore_con_event_proxy_bind(Ecore_Con_Server *svr)
{
   Ecore_Con_Event_Proxy_Bind *e;
   int ev = ECORE_CON_EVENT_PROXY_BIND;

   e = ecore_con_event_proxy_bind_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   svr->event_count = eina_list_append(svr->event_count, e);
   _ecore_con_server_timer_update(svr);
   e->server = svr;
   e->ip = svr->proxyip;
   e->port = svr->proxyport;
   ecore_event_add(ev, e,
                   _ecore_con_event_server_add_free, NULL);
   _ecore_con_event_count++;
}

void
ecore_con_event_server_add(Ecore_Con_Server *svr)
{
   /* we got our server! */
   Ecore_Con_Event_Server_Add *e;
   int ev = ECORE_CON_EVENT_SERVER_ADD;

   e = ecore_con_event_server_add_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   svr->connecting = EINA_FALSE;
   svr->start_time = ecore_time_get();
   svr->event_count = eina_list_append(svr->event_count, e);
   _ecore_con_server_timer_update(svr);
   e->server = svr;
   if (svr->upgrade) ev = ECORE_CON_EVENT_SERVER_UPGRADE;
   ecore_event_add(ev, e,
                   _ecore_con_event_server_add_free, NULL);
   _ecore_con_event_count++;
}

void
ecore_con_event_server_del(Ecore_Con_Server *svr)
{
   Ecore_Con_Event_Server_Del *e;

   svr->delete_me = EINA_TRUE;
   INF("svr %p is dead", svr);
   e = ecore_con_event_server_del_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   svr->event_count = eina_list_append(svr->event_count, e);
   _ecore_con_server_timer_update(svr);
   e->server = svr;
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
ecore_con_event_server_write(Ecore_Con_Server *svr, int num)
{
   Ecore_Con_Event_Server_Write *e;

   e = ecore_con_event_server_write_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   INF("Wrote %d bytes", num);
   svr->event_count = eina_list_append(svr->event_count, e);
   e->server = svr;
   e->size = num;
   ecore_event_add(ECORE_CON_EVENT_SERVER_WRITE, e,
                   (Ecore_End_Cb)_ecore_con_event_server_write_free, NULL);
   _ecore_con_event_count++;
}

void
ecore_con_event_server_data(Ecore_Con_Server *svr, unsigned char *buf, int num, Eina_Bool duplicate)
{
   Ecore_Con_Event_Server_Data *e;

   e = ecore_con_event_server_data_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   svr->event_count = eina_list_append(svr->event_count, e);
   _ecore_con_server_timer_update(svr);
   e->server = svr;
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
   _ecore_con_event_count++;
}

void
ecore_con_event_client_add(Ecore_Con_Client *cl)
{
   Ecore_Con_Event_Client_Add *e;
   int ev = ECORE_CON_EVENT_CLIENT_ADD;

   e = ecore_con_event_client_add_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   cl->event_count = eina_list_append(cl->event_count, e);
   cl->host_server->event_count = eina_list_append(cl->host_server->event_count, e);
   _ecore_con_cl_timer_update(cl);
   cl->start_time = ecore_time_get();
   e->client = cl;
   if (cl->upgrade) ev = ECORE_CON_EVENT_CLIENT_UPGRADE;
   ecore_event_add(ev, e,
                   (Ecore_End_Cb)_ecore_con_event_client_add_free, cl->host_server);
   _ecore_con_event_count++;
}

void
ecore_con_event_client_del(Ecore_Con_Client *cl)
{
   Ecore_Con_Event_Client_Del *e;

   if (!cl) return;
   cl->delete_me = EINA_TRUE;
   INF("cl %p is dead", cl);
   e = ecore_con_event_client_del_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);
   cl->event_count = eina_list_append(cl->event_count, e);

   cl->host_server->event_count = eina_list_append(cl->host_server->event_count, e);
   _ecore_con_cl_timer_update(cl);
   e->client = cl;
   ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, e,
                   (Ecore_End_Cb)_ecore_con_event_client_del_free, cl->host_server);
   _ecore_con_event_count++;
}

void
ecore_con_event_client_write(Ecore_Con_Client *cl, int num)
{
   Ecore_Con_Event_Client_Write *e;

   e = ecore_con_event_client_write_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   cl->event_count = eina_list_append(cl->event_count, e);
   cl->host_server->event_count = eina_list_append(cl->host_server->event_count, e);
   e->client = cl;
   e->size = num;
   ecore_event_add(ECORE_CON_EVENT_CLIENT_WRITE, e,
                   (Ecore_End_Cb)_ecore_con_event_client_write_free, cl->host_server);
   _ecore_con_event_count++;
}

void
ecore_con_event_client_data(Ecore_Con_Client *cl, unsigned char *buf, int num, Eina_Bool duplicate)
{
   Ecore_Con_Event_Client_Data *e;

   e = ecore_con_event_client_data_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   cl->event_count = eina_list_append(cl->event_count, e);
   cl->host_server->event_count = eina_list_append(cl->host_server->event_count, e);
   _ecore_con_cl_timer_update(cl);
   e->client = cl;
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
   _ecore_con_event_count++;
}

void
ecore_con_server_infos_del(Ecore_Con_Server *svr, void *info)
{
   svr->infos = eina_list_remove(svr->infos, info);
}

void
_ecore_con_event_server_error(Ecore_Con_Server *svr, char *error, Eina_Bool duplicate)
{
   Ecore_Con_Event_Server_Error *e;

   e = ecore_con_event_server_error_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->server = svr;
   e->error = duplicate ? strdup(error) : error;
   DBG("%s", error);
   svr->event_count = eina_list_append(svr->event_count, e);
   ecore_event_add(ECORE_CON_EVENT_SERVER_ERROR, e, (Ecore_End_Cb)_ecore_con_event_server_error_free, NULL);
   _ecore_con_event_count++;
}

void
ecore_con_event_client_error(Ecore_Con_Client *cl, const char *error)
{
   Ecore_Con_Event_Client_Error *e;

   e = ecore_con_event_client_error_alloc();
   EINA_SAFETY_ON_NULL_RETURN(e);

   e->client = cl;
   e->error = strdup(error);
   DBG("%s", error);
   cl->event_count = eina_list_append(cl->event_count, e);
   cl->host_server->event_count = eina_list_append(cl->host_server->event_count, e);
   ecore_event_add(ECORE_CON_EVENT_CLIENT_ERROR, e, (Ecore_End_Cb)_ecore_con_event_client_error_free, cl->host_server);
   _ecore_con_event_count++;
}

static void
_ecore_con_server_free(Ecore_Con_Server *svr)
{
   Ecore_Con_Client *cl;
   double t_start, t;

   if (svr->event_count) return;

   while (svr->infos)
     {
        ecore_con_info_data_clear(svr->infos->data);
        svr->infos = eina_list_remove_list(svr->infos, svr->infos);
     }

   t_start = ecore_time_get();
   while (svr->buf && (!svr->delete_me))
     {
        _ecore_con_server_flush(svr);
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
   ecore_con_local_win32_server_del(svr);
#endif
   if (svr->event_count) return;
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_NONE);

   if (svr->buf)
     eina_binbuf_free(svr->buf);

   EINA_LIST_FREE(svr->clients, cl)
     {
        Ecore_Con_Event_Server_Add *ev;

        /* some pointer hacks here to prevent double frees if people are being stupid */
        EINA_LIST_FREE(cl->event_count, ev)
          ev->server = NULL;
        cl->delete_me = EINA_TRUE;
        INF("cl %p is dead", cl);
        _ecore_con_client_free(cl);
     }
   if ((svr->created) && (svr->path) && (svr->ppid == getpid()))
     unlink(svr->path);

   ecore_con_ssl_server_shutdown(svr);
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

   servers = eina_list_remove(servers, svr);
   svr->data = NULL;
   free(svr);
}

static void
_ecore_con_client_free(Ecore_Con_Client *cl)
{
   double t_start, t;

   if (cl->event_count) return;

   t_start = ecore_time_get();
   while ((cl->buf) && (!cl->delete_me))
     {
        _ecore_con_client_flush(cl);
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
   cl->host_server->clients = eina_list_remove(cl->host_server->clients, cl);
   --cl->host_server->client_count;

#ifdef _WIN32
   ecore_con_local_win32_client_del(cl);
#endif

   if (cl->event_count) return;
   ECORE_MAGIC_SET(cl, ECORE_MAGIC_NONE);

   if (cl->buf) eina_binbuf_free(cl->buf);

   if (cl->host_server->type & ECORE_CON_SSL)
     ecore_con_ssl_client_shutdown(cl);

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
   free(cl);
   return;
}

static Eina_Bool
_ecore_con_server_timer(Ecore_Con_Server *svr)
{
   ecore_con_server_del(svr);

   svr->until_deletion = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_con_server_timer_update(Ecore_Con_Server *svr)
{
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
               svr->until_deletion = ecore_timer_add(svr->disconnect_time, (Ecore_Task_Cb)_ecore_con_server_timer, svr);
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
_ecore_con_client_timer(Ecore_Con_Client *cl)
{
   ecore_con_client_del(cl);

   cl->until_deletion = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_con_cl_timer_update(Ecore_Con_Client *cl)
{
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
        if (cl->host_server->client_disconnect_time > 0)
          {
             if (cl->until_deletion)
               {
                  ecore_timer_interval_set(cl->until_deletion, cl->host_server->client_disconnect_time);
                  ecore_timer_reset(cl->until_deletion);
               }
             else
               cl->until_deletion = ecore_timer_add(cl->host_server->client_disconnect_time, (Ecore_Task_Cb)_ecore_con_client_timer, cl);
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
   Ecore_Con_Server *svr;
   struct linger lin;
   const char *memerr = NULL;

   svr = data;

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
                                               _ecore_con_svr_tcp_handler, svr, NULL, NULL);
   if (!svr->fd_handler)
     {
        memerr = "Memory allocation failure";
        goto error;
     }

   return;

error:
   if (errno || memerr) ecore_con_event_server_error(svr, memerr ? : strerror(errno));
   ecore_con_ssl_server_shutdown(svr);
   _ecore_con_server_kill(svr);
}

static void
_ecore_con_cb_udp_listen(void *data,
                         Ecore_Con_Info *net_info)
{
   Ecore_Con_Server *svr;
   Ecore_Con_Type type;
   struct ip_mreq mreq;
#ifdef HAVE_IPV6
   struct ipv6_mreq mreq6;
#endif
   const int on = 1;
   const char *memerr = NULL;

   svr = data;
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
                               _ecore_con_svr_udp_handler, svr, NULL, NULL);
   if (!svr->fd_handler)
     {
        memerr = "Memory allocation failure";
        goto error;
     }

   svr->ip = eina_stringshare_add(net_info->ip);

   return;

error:
   if (errno || memerr) ecore_con_event_server_error(svr, memerr ? : strerror(errno));
   ecore_con_ssl_server_shutdown(svr);
   _ecore_con_server_kill(svr);
}

static void
_ecore_con_cb_tcp_connect(void *data,
                          Ecore_Con_Info *net_info)
{
   Ecore_Con_Server *svr;
   int res;
   int curstate = 0;
   const char *memerr = NULL;

   svr = data;

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
             _ecore_con_event_server_error(svr, err, EINA_FALSE);
             ecore_con_ssl_server_shutdown(svr);
             _ecore_con_server_kill(svr);
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
                                    _ecore_con_cl_handler, svr, NULL, NULL);
     }
   else
     svr->fd_handler = ecore_main_fd_handler_add(svr->fd, ECORE_FD_READ,
                                                 _ecore_con_cl_handler, svr, NULL, NULL);

   if (svr->type & ECORE_CON_SSL)
     {
        svr->handshaking = EINA_TRUE;
        svr->ssl_state = ECORE_CON_SSL_STATE_INIT;
        DBG("%s ssl handshake", svr->ecs_state ? "Queuing" : "Beginning");
        if ((!svr->ecs_state) && ecore_con_ssl_server_init(svr))
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
   ecore_con_event_server_error(svr,
                                memerr ? : errno ? strerror(errno) : "DNS error");
   ecore_con_ssl_server_shutdown(svr);
   _ecore_con_server_kill(svr);
}

static void
_ecore_con_cb_udp_connect(void *data,
                          Ecore_Con_Info *net_info)
{
   Ecore_Con_Server *svr;
   int curstate = 0;
   int broadcast = 1;
   const char *memerr = NULL;
   svr = data;

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
                                               _ecore_con_cl_udp_handler, svr, NULL, NULL);

   if (!svr->fd_handler)
     {
        memerr = "Memory allocation failure";
        goto error;
     }

   if ((!svr->ecs) || (svr->ecs->lookup))
     svr->ip = eina_stringshare_add(net_info->ip);

   return;

error:
   if (errno || memerr) ecore_con_event_server_error(svr, memerr ? : strerror(errno));
   ecore_con_ssl_server_shutdown(svr);
   _ecore_con_server_kill(svr);
}

static Ecore_Con_State
svr_try_connect_plain(Ecore_Con_Server *svr)
{
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
        ecore_con_event_server_error(svr, strerror(so_err));
        ERR("Connection lost: %s", strerror(so_err));
        _ecore_con_server_kill(svr);
        return ECORE_CON_DISCONNECTED;
     }

   if ((!svr->delete_me) && (!svr->handshaking) && svr->connecting)
     {
        if (svr->ecs)
          {
             if (ecore_con_socks_svr_init(svr))
               return ECORE_CON_INPROGRESS;
          }
        else
          ecore_con_event_server_add(svr);
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
   Ecore_Con_Server *svr;
   Ecore_Con_Client *cl = NULL;
   unsigned char client_addr[256];
   unsigned int client_addr_len;
   const char *clerr = NULL;

   svr = data;
   if (svr->delete_me)
     return ECORE_CALLBACK_RENEW;

   if ((svr->client_limit >= 0) && (!svr->reject_excess_clients) &&
       (svr->client_count >= (unsigned int)svr->client_limit))
     return ECORE_CALLBACK_RENEW;

   /* a new client */

   cl = calloc(1, sizeof(Ecore_Con_Client));
   if (!cl)
     {
        ecore_con_event_server_error(svr, "Memory allocation failure when attempting to add a new client");
        return ECORE_CALLBACK_RENEW;
     }
   cl->host_server = svr;

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
                                              _ecore_con_svr_cl_handler, cl, NULL, NULL);
   if (!cl->fd_handler) goto error;
   ECORE_MAGIC_SET(cl, ECORE_MAGIC_CON_CLIENT);

   if ((!svr->upgrade) && (svr->type & ECORE_CON_SSL))
     {
        cl->handshaking = EINA_TRUE;
        cl->ssl_state = ECORE_CON_SSL_STATE_INIT;
        if (ecore_con_ssl_client_init(cl))
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

   svr->clients = eina_list_append(svr->clients, cl);
   svr->client_count++;

   if ((!cl->delete_me) && (!cl->handshaking))
     ecore_con_event_client_add(cl);

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
   free(cl);
   if (clerr || errno) ecore_con_event_server_error(svr, clerr ? : strerror(errno));
   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_con_cl_read(Ecore_Con_Server *svr)
{
   int num = 0;
   Eina_Bool lost_server = EINA_TRUE;
   unsigned char buf[READBUFSIZ];

   DBG("svr=%p", svr);

   /* only possible with non-ssl connections */
   if (svr->connecting && (svr_try_connect_plain(svr) != ECORE_CON_CONNECTED))
     return;

   if (svr->handshaking && (!svr->ecs_state))
     {
        DBG("Continuing ssl handshake");
        if (!ecore_con_ssl_server_init(svr))
          lost_server = EINA_FALSE;
        _ecore_con_server_timer_update(svr);
     }

   if (svr->ecs_state || !(svr->type & ECORE_CON_SSL))
     {
        errno = 0;
        num = read(svr->fd, buf, sizeof(buf));
        /* 0 is not a valid return value for a tcp socket */
        if ((num > 0) || ((num < 0) && (errno == EAGAIN)))
          lost_server = EINA_FALSE;
        else if (num < 0)
          ecore_con_event_server_error(svr, strerror(errno));
     }
   else
     {
        num = ecore_con_ssl_server_read(svr, buf, sizeof(buf));
        /* this is not an actual 0 return, 0 here just means non-fatal error such as EAGAIN */
        if (num >= 0)
          lost_server = EINA_FALSE;
     }

   if ((!svr->delete_me) && (num > 0))
     {
        if (svr->ecs_state)
          ecore_con_socks_read(svr, buf, num);
        else
          ecore_con_event_server_data(svr, buf, num, EINA_TRUE);
     }

   if (lost_server)
     _ecore_con_server_kill(svr);
}

static Eina_Bool
_ecore_con_cl_handler(void *data,
                      Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Server *svr;
   Eina_Bool want_read, want_write;

   svr = data;
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
        if (ecore_con_ssl_server_init(svr))
          {
             ERR("ssl handshaking failed!");
             svr->handshaking = EINA_FALSE;
          }
        else if (!svr->ssl_state)
          ecore_con_event_server_add(svr);
        return ECORE_CALLBACK_RENEW;
     }
   if (svr->ecs && svr->ecs_state && (svr->ecs_state < ECORE_CON_PROXY_STATE_READ) && (!svr->ecs_buf))
     {
        if (svr->ecs_state < ECORE_CON_PROXY_STATE_INIT)
          {
             INF("PROXY STATE++");
             svr->ecs_state++;
          }
        if (ecore_con_socks_svr_init(svr)) return ECORE_CALLBACK_RENEW;
     }
   if (want_read)
     _ecore_con_cl_read(svr);
   else if (want_write) /* only possible with non-ssl connections */
     {
        if (svr->connecting && (!svr_try_connect_plain(svr)) && (!svr->ecs_state))
          return ECORE_CALLBACK_RENEW;

        _ecore_con_server_flush(svr);
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_con_cl_udp_handler(void *data,
                          Ecore_Fd_Handler *fd_handler)
{
   unsigned char buf[READBUFSIZ];
   int num;
   Ecore_Con_Server *svr;
   Eina_Bool want_read, want_write;

   want_read = ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ);
   want_write = ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE);

   svr = data;
   if (svr->delete_me || ((!want_read) && (!want_write)))
     return ECORE_CALLBACK_RENEW;

   if (want_write)
     {
        _ecore_con_server_flush(svr);
        return ECORE_CALLBACK_RENEW;
     }

   num = read(svr->fd, buf, READBUFSIZ);

   if ((!svr->delete_me) && (num > 0))
     ecore_con_event_server_data(svr, buf, num, EINA_TRUE);

   if (num < 0 && (errno != EAGAIN) && (errno != EINTR))
     {
        ecore_con_event_server_error(svr, strerror(errno));
        _ecore_con_server_kill(svr);
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
   Ecore_Con_Server *svr;
   Ecore_Con_Client *cl = NULL;

   svr = data;

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
        ecore_con_event_server_error(svr, strerror(errno));
        if (!svr->delete_me)
          ecore_con_event_client_del(NULL);
        _ecore_con_server_kill(svr);
        return ECORE_CALLBACK_CANCEL;
     }

/* Create a new client for use in the client data event */
   cl = calloc(1, sizeof(Ecore_Con_Client));
   EINA_SAFETY_ON_NULL_RETURN_VAL(cl, ECORE_CALLBACK_RENEW);

   cl->host_server = svr;
   cl->client_addr = malloc(client_addr_len);
   if (!cl->client_addr)
     {
        free(cl);
        return ECORE_CALLBACK_RENEW;
     }
   cl->client_addr_len = client_addr_len;

   memcpy(cl->client_addr, &client_addr, client_addr_len);
   ECORE_MAGIC_SET(cl, ECORE_MAGIC_CON_CLIENT);
   svr->clients = eina_list_append(svr->clients, cl);
   svr->client_count++;

   ecore_con_event_client_add(cl);
   ecore_con_event_client_data(cl, buf, num, EINA_TRUE);

   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_con_svr_cl_read(Ecore_Con_Client *cl)
{
   int num = 0;
   Eina_Bool lost_client = EINA_TRUE;
   unsigned char buf[READBUFSIZ];

   DBG("cl=%p", cl);

   if (cl->handshaking)
     {
        /* add an extra handshake attempt just before read, even though
         * read also attempts to handshake, to try to finish sooner
         */
        if (ecore_con_ssl_client_init(cl))
          lost_client = EINA_FALSE;

        _ecore_con_cl_timer_update(cl);
     }

   if (!(cl->host_server->type & ECORE_CON_SSL) && (!cl->upgrade))
     {
        num = read(cl->fd, buf, sizeof(buf));
        /* 0 is not a valid return value for a tcp socket */
        if ((num > 0) || ((num < 0) && ((errno == EAGAIN) || (errno == EINTR))))
          lost_client = EINA_FALSE;
        else if (num < 0)
          ecore_con_event_client_error(cl, strerror(errno));
     }
   else
     {
        num = ecore_con_ssl_client_read(cl, buf, sizeof(buf));
        /* this is not an actual 0 return, 0 here just means non-fatal error such as EAGAIN */
        if (num >= 0)
          lost_client = EINA_FALSE;
     }

   if ((!cl->delete_me) && (num > 0))
     ecore_con_event_client_data(cl, buf, num, EINA_TRUE);

   if (lost_client) _ecore_con_client_kill(cl);
}

static Eina_Bool
_ecore_con_svr_cl_handler(void *data,
                          Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Client *cl;

   cl = data;
   if (cl->delete_me)
     return ECORE_CALLBACK_RENEW;

   if (cl->handshaking && ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ | ECORE_FD_WRITE))
     {
        if (ecore_con_ssl_client_init(cl))
          {
             ERR("ssl handshaking failed!");
             _ecore_con_client_kill(cl);
             return ECORE_CALLBACK_RENEW;
          }
        else if (!cl->ssl_state)
          ecore_con_event_client_add(cl);
     }
   else if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     _ecore_con_svr_cl_read(cl);

   else if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     _ecore_con_client_flush(cl);

   return ECORE_CALLBACK_RENEW;
}

static void
_ecore_con_server_flush(Ecore_Con_Server *svr)
{
   int count;
   size_t num;
   size_t buf_len;
   size_t *buf_offset;
   const unsigned char *buf;
   Eina_Binbuf *buf_p;

   DBG("(svr=%p,buf=%p)", svr, svr->buf);
   if (!svr->fd_handler) return;
#ifdef _WIN32
   if (ecore_con_local_win32_server_flush(svr))
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
        if (ecore_con_ssl_server_init(svr))
          _ecore_con_server_kill(svr);
        _ecore_con_server_timer_update(svr);
        return;
     }

   if (svr->ecs_state || (!(svr->type & ECORE_CON_SSL)))
     count = write(svr->fd, buf + *buf_offset, num);
   else
     count = ecore_con_ssl_server_write(svr, buf + *buf_offset, num);

   if (count < 0)
     {
        if ((errno != EAGAIN) && (errno != EINTR))
          {
             ecore_con_event_server_error(svr, strerror(errno));
             _ecore_con_server_kill(svr);
          }
        return;
     }

   if (count && (!svr->ecs_state)) ecore_con_event_server_write(svr, count);

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
_ecore_con_client_flush(Ecore_Con_Client *cl)
{
   int count = 0;
   size_t num = 0;

   if (!cl->fd_handler) return;
#ifdef _WIN32
   if (ecore_con_local_win32_client_flush(cl))
     return;
#endif

   if (!cl->buf)
     {
        ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ);
        return;
     }

   if (cl->handshaking)
     {
        if (ecore_con_ssl_client_init(cl))
          count = -1;

        _ecore_con_cl_timer_update(cl);
     }

   if (!count)
     {
        if (!cl->buf) return;
        num = eina_binbuf_length_get(cl->buf) - cl->buf_offset;
        if (num <= 0) return;
        if (!(cl->host_server->type & ECORE_CON_SSL) && (!cl->upgrade))
          count = write(cl->fd, eina_binbuf_string_get(cl->buf) + cl->buf_offset, num);
        else
          count = ecore_con_ssl_client_write(cl, eina_binbuf_string_get(cl->buf) + cl->buf_offset, num);
     }

   if (count < 0)
     {
        if ((errno != EAGAIN) && (errno != EINTR) && (!cl->delete_me))
          {
             ecore_con_event_client_error(cl, strerror(errno));
             _ecore_con_client_kill(cl);
          }

        return;
     }

   if (count) ecore_con_event_client_write(cl, count);
   cl->buf_offset += count, num -= count;
   if (cl->buf_offset >= eina_binbuf_length_get(cl->buf))
     {
        cl->buf_offset = 0;
        eina_binbuf_free(cl->buf);
        cl->buf = NULL;
#ifdef TCP_CORK
        if ((cl->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_CORK)
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
_ecore_con_event_client_add_free(Ecore_Con_Server *svr,
                                 void *ev)
{
   Ecore_Con_Event_Client_Add *e;

   e = ev;
   if (e->client)
     {
        Eina_Bool svrfreed = EINA_FALSE;

        e->client->event_count = eina_list_remove(e->client->event_count, e);
        if (e->client->host_server)
          {
             e->client->host_server->event_count = eina_list_remove(e->client->host_server->event_count, ev);
             if ((!svr->event_count) && (svr->delete_me))
               {
                  _ecore_con_server_free(svr);
                  svrfreed = EINA_TRUE;
               }
          }
        if (!svrfreed)
          {
             if ((!e->client->event_count) && (e->client->delete_me))
               ecore_con_client_del(e->client);
          }
     }

   ecore_con_event_client_add_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_client_del_free(Ecore_Con_Server *svr,
                                 void *ev)
{
   Ecore_Con_Event_Client_Del *e;

   e = ev;
   if (e->client)
     {
        Eina_Bool svrfreed = EINA_FALSE;

        e->client->event_count = eina_list_remove(e->client->event_count, e);
        if (e->client->host_server)
          {
             e->client->host_server->event_count = eina_list_remove(e->client->host_server->event_count, ev);
             if ((!svr->event_count) && (svr->delete_me))
               {
                  _ecore_con_server_free(svr);
                  svrfreed = EINA_TRUE;
               }
          }
        if (!svrfreed)
          {
             if (!e->client->event_count)
               _ecore_con_client_free(e->client);
          }
     }
   ecore_con_event_client_del_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_client_write_free(Ecore_Con_Server *svr,
                                   Ecore_Con_Event_Client_Write *e)
{
   if (e->client)
     {
        Eina_Bool svrfreed = EINA_FALSE;

        e->client->event_count = eina_list_remove(e->client->event_count, e);
        if (e->client->host_server)
          {
             e->client->host_server->event_count = eina_list_remove(e->client->host_server->event_count, e);
             if ((!svr->event_count) && (svr->delete_me))
               {
                  _ecore_con_server_free(svr);
                  svrfreed = EINA_TRUE;
               }
          }
        if (!svrfreed)
          {
             if (((!e->client->event_count) && (e->client->delete_me)) ||
                 ((e->client->host_server &&
                   ((e->client->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_UDP ||
                    (e->client->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_MCAST))))
               ecore_con_client_del(e->client);
          }
     }
   ecore_con_event_client_write_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_client_data_free(Ecore_Con_Server *svr,
                                  void *ev)
{
   Ecore_Con_Event_Client_Data *e;

   e = ev;
   if (e->client)
     {
        Eina_Bool svrfreed = EINA_FALSE;

        e->client->event_count = eina_list_remove(e->client->event_count, e);
        if (e->client->host_server)
          {
             e->client->host_server->event_count = eina_list_remove(e->client->host_server->event_count, ev);
          }
        if ((!svr->event_count) && (svr->delete_me))
          {
             _ecore_con_server_free(svr);
             svrfreed = EINA_TRUE;
          }
        if (!svrfreed)
          {
             if (((!e->client->event_count) && (e->client->delete_me)) ||
                 ((e->client->host_server &&
                   ((e->client->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_UDP ||
                    (e->client->host_server->type & ECORE_CON_TYPE) == ECORE_CON_REMOTE_MCAST))))
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
        e->server->event_count = eina_list_remove(e->server->event_count, ev);
        if ((!e->server->event_count) && (e->server->delete_me))
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
        e->server->event_count = eina_list_remove(e->server->event_count, ev);
        if (!e->server->event_count)
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
        e->server->event_count = eina_list_remove(e->server->event_count, e);
        if ((!e->server->event_count) && (e->server->delete_me))
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
        e->server->event_count = eina_list_remove(e->server->event_count, ev);
        if ((!e->server->event_count) && (e->server->delete_me))
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
        e->server->event_count = eina_list_remove(e->server->event_count, e);
        if ((!e->server->event_count) && (e->server->delete_me))
          _ecore_con_server_free(e->server);
     }
   free(e->error);
   ecore_con_event_server_error_free(e);
   _ecore_con_event_count--;
   if ((!_ecore_con_event_count) && (!_ecore_con_init_count))
     ecore_con_mempool_shutdown();
}

static void
_ecore_con_event_client_error_free(Ecore_Con_Server *svr, Ecore_Con_Event_Client_Error *e)
{
   if (e->client)
     {
        Eina_Bool svrfreed = EINA_FALSE;

        if (eina_list_data_find(svr->clients, e->client))
          {
             e->client->event_count = eina_list_remove(e->client->event_count, e);
             if ((!e->client->event_count) && (e->client->delete_me))
               {
                  _ecore_con_client_free(e->client);
                  svrfreed = EINA_TRUE;
               }
          }
        svr->event_count = eina_list_remove(svr->event_count, e);
        if (!svrfreed)
          {
             if ((!svr->event_count) && (svr->delete_me))
               _ecore_con_server_free(svr);
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
   Ecore_Con_Server *svr;
   Ecore_Con_Lookup *lk;

   svr = data;
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

#include "ecore_con.eo.c"
