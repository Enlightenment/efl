#define EFL_NET_SERVER_TCP_PROTECTED 1
#define EFL_NET_SERVER_FD_PROTECTED 1
#define EFL_NET_SERVER_PROTECTED 1
#define EFL_LOOP_FD_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

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
#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#define MY_CLASS EFL_NET_SERVER_TCP_CLASS

typedef struct _Efl_Net_Server_Tcp_Data
{
   Ecore_Thread *resolver;
   Eina_Bool ipv6_only;
} Efl_Net_Server_Tcp_Data;

EOLIAN Efl_Object *
_efl_net_server_tcp_efl_object_constructor(Eo *o, Efl_Net_Server_Tcp_Data *pd)
{
   pd->ipv6_only = 0xff;
   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN void
_efl_net_server_tcp_efl_object_destructor(Eo *o, Efl_Net_Server_Tcp_Data *pd)
{
   if (pd->resolver)
     {
        ecore_thread_cancel(pd->resolver);
        pd->resolver = NULL;
     }
   efl_destructor(efl_super(o, MY_CLASS));
}

static Eina_Error
_efl_net_server_tcp_resolved_bind(Eo *o, Efl_Net_Server_Tcp_Data *pd, const struct addrinfo *addr)
{
   Eina_Error err = 0;
   char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")];
   socklen_t addrlen = addr->ai_addrlen;
   SOCKET fd;
   int r;

   efl_net_server_fd_family_set(o, addr->ai_family);

   fd = efl_net_socket4(addr->ai_family, addr->ai_socktype, addr->ai_protocol,
                        efl_net_server_fd_close_on_exec_get(o));
   if (fd == INVALID_SOCKET)
     {
        err = efl_net_socket_error_get();
        ERR("socket(%d, %d, %d): %s",
            addr->ai_family, addr->ai_socktype, addr->ai_protocol,
            eina_error_msg_get(err));
        return err;
     }

   efl_loop_fd_set(o, fd);

   /* apply pending value BEFORE bind() */
   if (addr->ai_family == AF_INET6)
     {
        if (pd->ipv6_only == 0xff)
          efl_net_server_tcp_ipv6_only_get(o); /* fetch & sync */
        else
          efl_net_server_tcp_ipv6_only_set(o, pd->ipv6_only);
     }

   r = bind(fd, addr->ai_addr, addrlen);
   if (r != 0)
     {
        err = efl_net_socket_error_get();
        efl_net_ip_port_fmt(buf, sizeof(buf), addr->ai_addr);
        DBG("bind(%d, %s): %s", fd, buf, eina_error_msg_get(err));
        goto error;
     }

   r = listen(fd, 0);
   if (r != 0)
     {
        err = efl_net_socket_error_get();
        DBG("listen(%d): %s", fd, eina_error_msg_get(err));
        goto error;
     }

   if (getsockname(fd, addr->ai_addr, &addrlen) != 0)
     {
        err = efl_net_socket_error_get();
        ERR("getsockname(%d): %s", fd, eina_error_msg_get(err));
        goto error;
     }
   else if (efl_net_ip_port_fmt(buf, sizeof(buf), addr->ai_addr))
     efl_net_server_address_set(o, buf);

   DBG("fd=%d serving at %s", fd, buf);
   efl_net_server_serving_set(o, EINA_TRUE);
   return 0;

 error:
   efl_net_server_fd_family_set(o, AF_UNSPEC);
   efl_loop_fd_set(o, INVALID_SOCKET);
   closesocket(fd);
   return err;
}

static void
_efl_net_server_tcp_resolved(void *data, const char *host EINA_UNUSED, const char *port EINA_UNUSED, const struct addrinfo *hints EINA_UNUSED, struct addrinfo *result, int gai_error)
{
   Eo *o = data;
   Efl_Net_Server_Tcp_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const struct addrinfo *addr;
   Eina_Error err;

   pd->resolver = NULL;

   efl_ref(o); /* we're emitting callbacks then continuing the workflow */

   if (gai_error)
     {
        err = EFL_NET_SERVER_ERROR_COULDNT_RESOLVE_HOST;
        goto end;
     }

   for (addr = result; addr != NULL; addr = addr->ai_next)
     {
        err = _efl_net_server_tcp_resolved_bind(o, pd, addr);
        if (err == 0) break;
     }
   freeaddrinfo(result);

 end:
   if (err) efl_event_callback_call(o, EFL_NET_SERVER_EVENT_ERROR, &err);

   efl_unref(o);
}

EOLIAN static Eina_Error
_efl_net_server_tcp_efl_net_server_serve(Eo *o, Efl_Net_Server_Tcp_Data *pd, const char *address)
{
   char *str;
   const char *host, *port;
   struct addrinfo hints = {
     .ai_socktype = SOCK_STREAM,
     .ai_protocol = IPPROTO_TCP,
     .ai_family = AF_UNSPEC,
     .ai_flags = AI_ADDRCONFIG | AI_V4MAPPED,
   };

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);

   str = strdup(address);
   if (!efl_net_ip_port_split(str, &host, &port))
     {
        free(str);
        return EINVAL;
     }
   if (!port) port = "0";
   if (strchr(host, ':')) hints.ai_family = AF_INET6;

   pd->resolver = efl_net_ip_resolve_async_new(host, port, &hints,
                                               _efl_net_server_tcp_resolved, o);
   free(str);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->resolver, EINVAL);
   return 0;
}

static Efl_Callback_Array_Item *_efl_net_server_tcp_client_cbs(void);

static void
_efl_net_server_tcp_client_event_closed(void *data, const Efl_Event *event)
{
   Eo *server = data;
   Eo *client = event->object;

   efl_event_callback_array_del(client, _efl_net_server_tcp_client_cbs(), server);
   if (efl_parent_get(client) == server)
     efl_parent_set(client, NULL);

   efl_net_server_clients_count_set(server, efl_net_server_clients_count_get(server) - 1);
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_net_server_tcp_client_cbs,
                           { EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_tcp_client_event_closed });

static void
_efl_net_server_tcp_efl_net_server_fd_client_add(Eo *o, Efl_Net_Server_Tcp_Data *pd EINA_UNUSED, int client_fd)
{
   Eo *client = efl_add(EFL_NET_SOCKET_TCP_CLASS, o,
                        efl_event_callback_array_add(efl_added, _efl_net_server_tcp_client_cbs(), o),
                        efl_io_closer_close_on_exec_set(efl_added, efl_net_server_fd_close_on_exec_get(o)),
                        efl_io_closer_close_on_destructor_set(efl_added, EINA_TRUE),
                        efl_loop_fd_set(efl_added, client_fd));
   if (!client)
     {
        ERR("could not create client object fd=%d", client_fd);
        closesocket(client_fd);
        return;
     }

   efl_net_server_clients_count_set(o, efl_net_server_clients_count_get(o) + 1);
   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_ADD, client);

   if (efl_ref_get(client) == 1) /* users must take a reference themselves */
     {
        DBG("client %s was not handled, closing it...",
            efl_net_socket_address_remote_get(client));
        efl_del(client);
     }
}

static void
_efl_net_server_tcp_efl_net_server_fd_client_reject(Eo *o, Efl_Net_Server_Tcp_Data *pd EINA_UNUSED, int client_fd)
{
   struct sockaddr_storage addr;
   socklen_t addrlen;
   char str[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";

   addrlen = sizeof(addr);
   if (getpeername(client_fd, (struct sockaddr *)&addr, &addrlen) != 0)
     ERR("getpeername(%d): %s", client_fd, eina_error_msg_get(efl_net_socket_error_get()));
   else
     efl_net_ip_port_fmt(str, sizeof(str), (struct sockaddr *)&addr);

   closesocket(client_fd);
   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_REJECTED, str);
}

EOLIAN void
_efl_net_server_tcp_ipv6_only_set(Eo *o, Efl_Net_Server_Tcp_Data *pd, Eina_Bool ipv6_only)
{
#ifdef IPV6_V6ONLY
   Eina_Bool old = pd->ipv6_only;
   SOCKET fd = efl_loop_fd_get(o);
#ifdef _WIN32
   DWORD value = ipv6_only;
#else
   int value = ipv6_only;
#endif
#endif

   pd->ipv6_only = ipv6_only;

#ifdef IPV6_V6ONLY
   if (fd == INVALID_SOCKET) return;
   if (efl_net_server_fd_family_get(o) != AF_INET6) return;

   if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &value, sizeof(value)) != 0)
     {
        ERR("could not set socket=%d IPV6_V6ONLY=%d: %s", fd, value, eina_error_msg_get(efl_net_socket_error_get()));
        pd->ipv6_only = old;
     }
#endif
}

EOLIAN Eina_Bool
_efl_net_server_tcp_ipv6_only_get(Eo *o EINA_UNUSED, Efl_Net_Server_Tcp_Data *pd)
{
#ifdef IPV6_V6ONLY
   SOCKET fd = efl_loop_fd_get(o);
#ifdef _WIN32
   DWORD value = 0;
   int valuelen;
#else
   int value = 0;
   socklen_t valuelen;
#endif

   if (fd == INVALID_SOCKET) goto end;
   if (efl_net_server_fd_family_get(o) != AF_INET6) goto end;

   valuelen = sizeof(value);
   if (getsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &value, &valuelen) != 0)
     {
        WRN("getsockopt(%d, IPPROTO_IPV6, IPV6_V6ONLY): %s", fd, eina_error_msg_get(efl_net_socket_error_get()));
        goto end;
     }
   pd->ipv6_only = !!value;

 end:
#endif
   return pd->ipv6_only;
}


#include "efl_net_server_tcp.eo.c"
