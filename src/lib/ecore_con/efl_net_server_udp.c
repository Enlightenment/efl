#define EFL_NET_SERVER_UDP_PROTECTED 1
#define EFL_NET_SERVER_FD_PROTECTED 1
#define EFL_NET_SERVER_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
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
#ifdef HAVE_NETINET_UDP_H
# include <netinet/udp.h>
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

#include <sys/ioctl.h>

#define MY_CLASS EFL_NET_SERVER_UDP_CLASS

typedef struct _Efl_Net_Server_Udp_Data
{
   Ecore_Thread *resolver;
   Eina_Hash *clients; /* addr (string) -> client (Efl.Net.Server.Udp.Client) */
   Eina_Bool ipv6_only;
} Efl_Net_Server_Udp_Data;

EOLIAN Efl_Object *
_efl_net_server_udp_efl_object_constructor(Eo *o, Efl_Net_Server_Udp_Data *pd)
{
   pd->ipv6_only = 0xff;
   pd->clients = eina_hash_string_superfast_new(NULL);
   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN void
_efl_net_server_udp_efl_object_destructor(Eo *o, Efl_Net_Server_Udp_Data *pd)
{
   if (pd->resolver)
     {
        ecore_thread_cancel(pd->resolver);
        pd->resolver = NULL;
     }
   efl_destructor(efl_super(o, MY_CLASS));

   if (pd->clients)
     {
        eina_hash_free(pd->clients);
        pd->clients = NULL;
     }
}

static Eina_Error
_efl_net_server_udp_resolved_bind(Eo *o, Efl_Net_Server_Udp_Data *pd, const struct addrinfo *addr)
{
   Eina_Error err = 0;
   char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")];
   socklen_t addrlen = addr->ai_addrlen;
   int fd, r;

   efl_net_server_fd_family_set(o, addr->ai_family);

   fd = efl_net_socket4(addr->ai_family, addr->ai_socktype, addr->ai_protocol,
                        efl_net_server_fd_close_on_exec_get(o));
   if (fd < 0)
     {
        ERR("socket(%d, %d, %d): %s",
            addr->ai_family, addr->ai_socktype, addr->ai_protocol,
            strerror(errno));
        return errno;
     }

   efl_loop_fd_set(o, fd);

   /* apply pending value BEFORE bind() */
   if (addr->ai_family == AF_INET6)
     {
        if (pd->ipv6_only == 0xff)
          efl_net_server_udp_ipv6_only_get(o); /* fetch & sync */
        else
          efl_net_server_udp_ipv6_only_set(o, pd->ipv6_only);
     }

   r = bind(fd, addr->ai_addr, addrlen);
   if (r < 0)
     {
        err = errno;
        efl_net_ip_port_fmt(buf, sizeof(buf), addr->ai_addr);
        DBG("bind(%d, %s): %s", fd, buf, strerror(errno));
        goto error;
     }

   if (getsockname(fd, addr->ai_addr, &addrlen) != 0)
     {
        ERR("getsockname(%d): %s", fd, strerror(errno));
        goto error;
     }
   else if (efl_net_ip_port_fmt(buf, sizeof(buf), addr->ai_addr))
     efl_net_server_address_set(o, buf);

   DBG("fd=%d serving at %s", fd, buf);
   efl_net_server_serving_set(o, EINA_TRUE);
   return 0;

 error:
   efl_net_server_fd_family_set(o, AF_UNSPEC);
   efl_loop_fd_set(o, -1);
   close(fd);
   return err;
}

static void
_efl_net_server_udp_resolved(void *data, const char *host EINA_UNUSED, const char *port EINA_UNUSED, const struct addrinfo *hints EINA_UNUSED, struct addrinfo *result, int gai_error)
{
   Eo *o = data;
   Efl_Net_Server_Udp_Data *pd = efl_data_scope_get(o, MY_CLASS);
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
        err = _efl_net_server_udp_resolved_bind(o, pd, addr);
        if (err == 0) break;
     }
   freeaddrinfo(result);

 end:
   if (err) efl_event_callback_call(o, EFL_NET_SERVER_EVENT_ERROR, &err);

   efl_unref(o);
}

EOLIAN static Eina_Error
_efl_net_server_udp_efl_net_server_serve(Eo *o, Efl_Net_Server_Udp_Data *pd, const char *address)
{
   char *str;
   const char *host, *port;
   struct addrinfo hints = {
     .ai_socktype = SOCK_DGRAM,
     .ai_protocol = IPPROTO_UDP,
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
                                               _efl_net_server_udp_resolved, o);
   free(str);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->resolver, EINVAL);
   return 0;
}

static Efl_Callback_Array_Item *_efl_net_server_udp_client_cbs(void);

static void
_efl_net_server_udp_client_event_closed(void *data, const Efl_Event *event)
{
   Eo *server = data;
   Eo *client = event->object;
   Efl_Net_Server_Udp_Data *pd = efl_data_scope_get(server, MY_CLASS);

   efl_event_callback_array_del(client, _efl_net_server_udp_client_cbs(), server);
   efl_net_server_clients_count_set(server, efl_net_server_clients_count_get(server) - 1);

   eina_hash_del(pd->clients, efl_net_socket_address_remote_get(client), client);
   if (efl_parent_get(client) == server)
     efl_parent_set(client, NULL);
}

EFL_CALLBACKS_ARRAY_DEFINE(_efl_net_server_udp_client_cbs,
                           { EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_udp_client_event_closed });

static size_t
_udp_datagram_size_query(int fd)
{
#ifdef _WIN32
   unsigned long size;
   if (ioctlsocket(fd, FIONREAD, &size) == 0)
     return size;
#else
   int size;
   if (ioctl(fd, FIONREAD, &size) == 0)
     return size;
#endif
   return 8 * 1024;
}

EOLIAN static void
_efl_net_server_udp_efl_net_server_fd_process_incoming_data(Eo *o, Efl_Net_Server_Udp_Data *pd)
{
   unsigned int count, limit;
   Eina_Bool reject_excess;
   struct sockaddr_storage addr;
   Eo *client;
   int fd;
   socklen_t addrlen = sizeof(addr);
   char str[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";
   char *buf;
   size_t buflen;
   ssize_t r;
   Eina_Rw_Slice slice;

   fd = efl_loop_fd_get(o);
   buflen = _udp_datagram_size_query(fd);
   buf = malloc(buflen);
   EINA_SAFETY_ON_NULL_RETURN(buf);

   r = recvfrom(fd, buf, buflen, 0, (struct sockaddr *)&addr, &addrlen);
   if (r < 0)
     {
        Eina_Error err = efl_net_socket_error_get();
        ERR("recvfrom(%d, %p, %zu, 0, %p, %d): %s", fd, buf, buflen, &addr, addrlen, eina_error_msg_get(err));
        free(buf);
        efl_event_callback_call(o, EFL_NET_SERVER_EVENT_ERROR, &err);
        return;
     }
   slice = (Eina_Rw_Slice){.mem = buf, .len = r };

   efl_net_ip_port_fmt(str, sizeof(str), (struct sockaddr *)&addr);
   client = eina_hash_find(pd->clients, str);
   if (client)
     {
        _efl_net_server_udp_client_feed(client, slice);
        return;
     }

   count = efl_net_server_clients_count_get(o);
   efl_net_server_clients_limit_get(o, &limit, &reject_excess);

   if ((limit > 0) && (count >= limit))
     {
        if (reject_excess)
          {
             free(buf);
             efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_REJECTED, str);
             return;
          }
     }

   client = efl_add(EFL_NET_SERVER_UDP_CLIENT_CLASS, o,
                    efl_event_callback_array_add(efl_added, _efl_net_server_udp_client_cbs(), o),
                    efl_io_closer_close_on_destructor_set(efl_added, EINA_TRUE),

                    efl_net_socket_address_local_set(efl_added, efl_net_server_address_get(o)),
                    _efl_net_server_udp_init(efl_added, fd, (const struct sockaddr *)&addr, addrlen, str),
                    efl_io_writer_can_write_set(efl_added, EINA_TRUE));
   if (!client)
     {
        ERR("could not create client object for %s", str);
        free(buf);
        efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_REJECTED, str);
        return;
     }

   if (!eina_hash_direct_add(pd->clients, efl_net_socket_address_remote_get(client), client))
     {
        ERR("could not create client object for %s", str);
        free(buf);
        efl_del(client);
        efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_REJECTED, str);
        return;
     }

   efl_net_server_clients_count_set(o, efl_net_server_clients_count_get(o) + 1);
   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_ADD, client);

   if (efl_ref_get(client) == 1) /* users must take a reference themselves */
     {
        DBG("client %s was not handled, closing it...",
            efl_net_socket_address_remote_get(client));
        free(buf);
        efl_del(client);
        return;
     }

   _efl_net_server_udp_client_feed(client, slice);
}

EOLIAN void
_efl_net_server_udp_ipv6_only_set(Eo *o, Efl_Net_Server_Udp_Data *pd, Eina_Bool ipv6_only)
{
   Eina_Bool old = pd->ipv6_only;
   int fd = efl_loop_fd_get(o);
   int value = ipv6_only;

   pd->ipv6_only = ipv6_only;

   if (fd < 0) return;
   if (efl_net_server_fd_family_get(o) != AF_INET6) return;

#ifdef IPV6_V6ONLY
   if (setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &value, sizeof(value)) < 0)
     {
        ERR("could not set socket=%d IPV6_V6ONLY=%d: %s", fd, value, strerror(errno));
        pd->ipv6_only = old;
     }
#endif
}

EOLIAN Eina_Bool
_efl_net_server_udp_ipv6_only_get(Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Data *pd)
{
#ifdef IPV6_V6ONLY
   int fd = efl_loop_fd_get(o);
   int value = 0;
   socklen_t size = sizeof(value);

   if (fd < 0) goto end;
   if (efl_net_server_fd_family_get(o) != AF_INET6) goto end;

   if (getsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &value, &size) < 0)
     {
        WRN("getsockopt(%d, IPPROTO_IPV6, IPV6_V6ONLY): %s", fd, strerror(errno));
        goto end;
     }
   pd->ipv6_only = !!value;

 end:
#endif
   return pd->ipv6_only;
}

#include "efl_net_server_udp.eo.c"
