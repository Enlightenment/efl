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

#define MY_CLASS EFL_NET_SERVER_UDP_CLASS

typedef struct _Efl_Net_Server_Udp_Data
{
   Ecore_Thread *resolver;
   Eina_Hash *clients; /* addr (string) -> client (Efl.Net.Server.Udp.Client) */
   struct {
      Eina_List *groups; /* list of newly allocated strings */
      Eina_List *pending; /* list of nodes of groups pending join */
      uint8_t ttl;
      Eina_Bool loopback;
      Eina_Bool ttl_set;
   } multicast;
   Eina_Bool dont_route;
} Efl_Net_Server_Udp_Data;

EOLIAN Efl_Object *
_efl_net_server_udp_efl_object_constructor(Eo *o, Efl_Net_Server_Udp_Data *pd)
{
   pd->clients = eina_hash_string_superfast_new(NULL);
   pd->multicast.ttl = 1;
   pd->multicast.ttl_set = EINA_FALSE;
   pd->multicast.loopback = 0xff;
   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN void
_efl_net_server_udp_efl_object_destructor(Eo *o, Efl_Net_Server_Udp_Data *pd)
{
   if (pd->multicast.pending)
     {
        eina_list_free(pd->multicast.pending);
        pd->multicast.pending = NULL;
     }

   while (pd->multicast.groups)
     efl_net_server_udp_multicast_leave(o, pd->multicast.groups->data);

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
   Eina_List *node;
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
        efl_net_server_ip_ipv6_only_set(o, efl_net_server_ip_ipv6_only_get(o));
     }

   efl_net_server_udp_dont_route_set(o, pd->dont_route);

   r = bind(fd, addr->ai_addr, addrlen);
   if (r != 0)
     {
        err = efl_net_socket_error_get();
        efl_net_ip_port_fmt(buf, sizeof(buf), addr->ai_addr);
        DBG("bind(" SOCKET_FMT ", %s): %s", fd, buf, eina_error_msg_get(err));
        goto error;
     }

   if (getsockname(fd, addr->ai_addr, &addrlen) != 0)
     {
        err = efl_net_socket_error_get();
        ERR("getsockname(" SOCKET_FMT "): %s", fd, eina_error_msg_get(err));
        goto error;
     }
   else if (efl_net_ip_port_fmt(buf, sizeof(buf), addr->ai_addr))
     efl_net_server_address_set(o, buf);

   DBG("fd=" SOCKET_FMT " serving at %s", fd, buf);
   efl_net_server_serving_set(o, EINA_TRUE);

   EINA_LIST_FREE(pd->multicast.pending, node)
     {
        const char *mcast_addr = node->data;
        Eina_Error mr = efl_net_multicast_join(fd, addr->ai_family, mcast_addr);
        if (mr)
          {
             ERR("could not join pending multicast group '%s': %s", mcast_addr, eina_error_msg_get(mr));
             efl_event_callback_call(o, EFL_NET_SERVER_EVENT_SERVER_ERROR, &mr);
          }
     }

   if (!pd->multicast.ttl_set)
     efl_net_server_udp_multicast_time_to_live_get(o); /* fetch & sync */
   else
     efl_net_server_udp_multicast_time_to_live_set(o, pd->multicast.ttl);

   if (pd->multicast.loopback == 0xff)
     efl_net_server_udp_multicast_loopback_get(o); /* fetch & sync */
   else
     efl_net_server_udp_multicast_loopback_set(o, pd->multicast.loopback);

   return 0;

 error:
   efl_net_server_fd_family_set(o, AF_UNSPEC);
   efl_loop_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
   closesocket(fd);
   return err;
}

static void
_efl_net_server_udp_resolved(void *data, const char *host EINA_UNUSED, const char *port EINA_UNUSED, const struct addrinfo *hints EINA_UNUSED, struct addrinfo *result, int gai_error)
{
   Eo *o = data;
   Efl_Net_Server_Udp_Data *pd = efl_data_scope_get(o, MY_CLASS);
   const struct addrinfo *addr;
   Eina_Error err = EINA_ERROR_NO_ERROR;

   pd->resolver = NULL;

   efl_ref(o); /* we're emitting callbacks then continuing the workflow */

   if (gai_error)
     {
        err = EFL_NET_ERROR_COULDNT_RESOLVE_HOST;
        goto end;
     }

   for (addr = result; addr != NULL; addr = addr->ai_next)
     {
        err = _efl_net_server_udp_resolved_bind(o, pd, addr);
        if (err == 0) break;
     }
   freeaddrinfo(result);

 end:
   if (err) efl_event_callback_call(o, EFL_NET_SERVER_EVENT_SERVER_ERROR, &err);

   efl_unref(o);
}

EOLIAN static Eina_Error
_efl_net_server_udp_efl_net_server_fd_socket_activate(Eo *o, Efl_Net_Server_Udp_Data *pd EINA_UNUSED, const char *address)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((SOCKET)efl_loop_fd_get(o) != INVALID_SOCKET, EALREADY);
   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);

#ifndef HAVE_SYSTEMD
   return efl_net_server_fd_socket_activate(efl_super(o, MY_CLASS), address);
#else
   {
      char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")];
      Eina_Error err;
      struct sockaddr_storage addr;
      socklen_t addrlen;
      SOCKET fd;

      err = efl_net_ip_socket_activate_check(address, AF_UNSPEC, SOCK_DGRAM, NULL);
      if (err) return err;

      err = efl_net_server_fd_socket_activate(efl_super(o, MY_CLASS), address);
      if (err) return err;

      fd = efl_loop_fd_get(o);

      addrlen = sizeof(addr);
      if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) != 0)
        {
           err = efl_net_socket_error_get();
           ERR("getsockname(" SOCKET_FMT "): %s", fd, eina_error_msg_get(err));
           goto error;
        }
      else if (efl_net_ip_port_fmt(buf, sizeof(buf), (struct sockaddr *)&addr))
        efl_net_server_address_set(o, buf);

      DBG("fd=" SOCKET_FMT " serving at %s", fd, address);
      efl_net_server_serving_set(o, EINA_TRUE);
      return 0;

   error:
      efl_net_server_fd_family_set(o, AF_UNSPEC);
      efl_loop_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
      closesocket(fd);
      return err;
   }
#endif
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
   struct sockaddr_storage ss;
   Eina_Error err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);

   str = strdup(address);
   if (!efl_net_ip_port_split(str, &host, &port))
     {
        free(str);
        return EINVAL;
     }
   if (!port) port = "0";
   if (strchr(host, ':')) hints.ai_family = AF_INET6;

   if (efl_net_ip_port_parse_split(host, port, &ss))
     {
        struct addrinfo ai = hints;

        ai.ai_family = ss.ss_family;
        ai.ai_addr = (struct sockaddr *)&ss;
        ai.ai_addrlen = ss.ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

        err = _efl_net_server_udp_resolved_bind(o, pd, &ai);
        free(str);
     }
   else
     {
        pd->resolver = efl_net_ip_resolve_async_new(host, port, &hints,
                                                    _efl_net_server_udp_resolved, o);
        free(str);
        EINA_SAFETY_ON_NULL_RETURN_VAL(pd->resolver, EINVAL);
        err = 0;
     }

   return err;
}

static void
_efl_net_server_udp_client_event_closed(void *data, const Efl_Event *event)
{
   Eo *server = data;
   Eo *client = event->object;
   Efl_Net_Server_Udp_Data *pd = efl_data_scope_get(server, MY_CLASS);

   efl_event_callback_del(client, EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_udp_client_event_closed, server);
   eina_hash_del(pd->clients, efl_net_socket_address_remote_get(client), client);
}

EOLIAN static void
_efl_net_server_udp_efl_net_server_fd_process_incoming_data(Eo *o, Efl_Net_Server_Udp_Data *pd)
{
   unsigned int count, limit;
   Eina_Bool reject_excess;
   struct sockaddr_storage addr;
   Eo *client;
   SOCKET fd;
   socklen_t addrlen = sizeof(addr);
   char str[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";
   char *buf;
   size_t buflen;
   ssize_t r;
   Eina_Rw_Slice slice;

   fd = efl_loop_fd_get(o);
   buflen = efl_net_udp_datagram_size_query(fd);
   buf = malloc(buflen);
   EINA_SAFETY_ON_NULL_RETURN(buf);

   r = recvfrom(fd, buf, buflen, 0, (struct sockaddr *)&addr, &addrlen);
   if (r < 0)
     {
        Eina_Error err = efl_net_socket_error_get();
        ERR("recvfrom(" SOCKET_FMT ", %p, %zu, 0, %p, %d): %s", fd, buf, buflen, &addr, addrlen, eina_error_msg_get(err));
        free(buf);
        efl_event_callback_call(o, EFL_NET_SERVER_EVENT_SERVER_ERROR, &err);
        return;
     }
   if ((size_t)r < buflen)
     {
        void *tmp = realloc(buf, r);
        if (tmp) buf = tmp;
        else
          {
             Eina_Error err = efl_net_socket_error_get();

             free(buf);
             ERR("Out of memory on efl net udp data incoming");
             efl_event_callback_call(o, EFL_NET_SERVER_EVENT_SERVER_ERROR, &err);
             return;
          }
     }
   slice = (Eina_Rw_Slice){.mem = buf, .len = r };

   efl_net_ip_port_fmt(str, sizeof(str), (struct sockaddr *)&addr);
   client = eina_hash_find(pd->clients, str);
   if (client)
     {
        free(buf);
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
                    efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE),

                    efl_net_socket_address_local_set(efl_added, efl_net_server_address_get(o)),
                    _efl_net_server_udp_client_init(efl_added, fd, (const struct sockaddr *)&addr, addrlen, str),
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
   efl_event_callback_add(client, EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_udp_client_event_closed, o);

   if (!efl_net_server_client_announce(o, client))
     {
        free(buf);
        return;
     }

   free(buf);
   _efl_net_server_udp_client_feed(client, slice);
}

EOLIAN static Eina_Bool
_efl_net_server_udp_dont_route_set(Eo *o, Efl_Net_Server_Udp_Data *pd, Eina_Bool dont_route)
{
   Eina_Bool old = pd->dont_route;
   SOCKET fd = efl_loop_fd_get(o);
#ifdef _WIN32
   DWORD value = dont_route;
#else
   int value = dont_route;
#endif

   pd->dont_route = dont_route;

   if (fd == INVALID_SOCKET) return EINA_TRUE;

   if (setsockopt(fd, SOL_SOCKET, SO_DONTROUTE, (const char *)&value, sizeof(value)) != 0)
     {
        Eina_Error err = efl_net_socket_error_get();
        ERR("setsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_DONTROUTE, %u): %s", fd, dont_route, eina_error_msg_get(err));
        pd->dont_route = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_server_udp_dont_route_get(const Eo *o, Efl_Net_Server_Udp_Data *pd)
{
   SOCKET fd = efl_loop_fd_get(o);
#ifdef _WIN32
   DWORD value;
#else
   int value;
#endif
   socklen_t valuelen;

   if (fd == INVALID_SOCKET) return pd->dont_route;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, SOL_SOCKET, SO_DONTROUTE, (char *)&value, &valuelen) != 0)
     {
        Eina_Error err = efl_net_socket_error_get();
        ERR("getsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_DONTROUTE): %s", fd, eina_error_msg_get(err));
        return EINA_FALSE;
     }

   pd->dont_route = !!value; /* sync */
   return pd->dont_route;
}

static Eina_List *
_efl_net_server_udp_multicast_find(const Eina_List *lst, const char *address)
{
   const char *str;
   const Eina_List *node;

   EINA_LIST_FOREACH(lst, node, str)
     {
        if (strcmp(str, address) == 0)
          return (Eina_List *)node;
     }

   return NULL;
}

EOLIAN static Eina_Error
_efl_net_server_udp_multicast_join(Eo *o, Efl_Net_Server_Udp_Data *pd, const char *address)
{
   const Eina_List *found;
   SOCKET fd = efl_loop_fd_get(o);

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);

   found = _efl_net_server_udp_multicast_find(pd->multicast.groups, address);
   if (found) return EEXIST;

   pd->multicast.groups = eina_list_append(pd->multicast.groups, strdup(address));

   if (fd == INVALID_SOCKET)
     {
        pd->multicast.pending = eina_list_append(pd->multicast.pending, eina_list_last(pd->multicast.groups));
        return 0;
     }

   return efl_net_multicast_join(fd, efl_net_server_fd_family_get(o), address);
}

EOLIAN static Eina_Error
_efl_net_server_udp_multicast_leave(Eo *o, Efl_Net_Server_Udp_Data *pd, const char *address)
{
   Eina_List *found;
   SOCKET fd = efl_loop_fd_get(o);
   Eina_Error err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);

   found = _efl_net_server_udp_multicast_find(pd->multicast.groups, address);
   if (!found) return ENOENT;

   if (fd == INVALID_SOCKET)
     {
        free(found->data);
        pd->multicast.pending = eina_list_remove(pd->multicast.pending, found);
        pd->multicast.groups = eina_list_remove_list(pd->multicast.groups, found);
        return 0;
     }

   err = efl_net_multicast_leave(fd, efl_net_server_fd_family_get(o), address);

   free(found->data);
   pd->multicast.groups = eina_list_remove_list(pd->multicast.groups, found);
   return err;
}

EOLIAN static Eina_Iterator *
_efl_net_server_udp_multicast_groups_get(Eo *o EINA_UNUSED, Efl_Net_Server_Udp_Data *pd)
{
   return eina_list_iterator_new(pd->multicast.groups);
}

EOLIAN static Eina_Error
_efl_net_server_udp_multicast_time_to_live_set(Eo *o, Efl_Net_Server_Udp_Data *pd, uint8_t ttl)
{
   SOCKET fd = efl_loop_fd_get(o);
   Eina_Error err;
   uint8_t old = pd->multicast.ttl;

   pd->multicast.ttl_set = EINA_TRUE;
   pd->multicast.ttl = ttl;

   if (fd == INVALID_SOCKET) return 0;

   err = efl_net_multicast_ttl_set(fd, efl_net_server_fd_family_get(o), ttl);
   if (err)
     {
        ERR("could not set multicast time to live=%hhu: %s", ttl, eina_error_msg_get(err));
        pd->multicast.ttl = old;
     }

   return err;
}

EOLIAN static uint8_t
_efl_net_server_udp_multicast_time_to_live_get(const Eo *o, Efl_Net_Server_Udp_Data *pd)
{
   SOCKET fd = efl_loop_fd_get(o);
   Eina_Error err;
   uint8_t ttl = pd->multicast.ttl;

   if (fd == INVALID_SOCKET) return pd->multicast.ttl;

   err = efl_net_multicast_ttl_get(fd, efl_net_server_fd_family_get(o), &ttl);
   if (err)
     ERR("could not get multicast time to live: %s", eina_error_msg_get(err));
   else
     pd->multicast.ttl = ttl;

   return pd->multicast.ttl;
}

EOLIAN static Eina_Error
_efl_net_server_udp_multicast_loopback_set(Eo *o, Efl_Net_Server_Udp_Data *pd, Eina_Bool loopback)
{
   SOCKET fd = efl_loop_fd_get(o);
   Eina_Error err;
   Eina_Bool old = pd->multicast.loopback;

   pd->multicast.loopback = loopback;

   if (fd == INVALID_SOCKET) return 0;

   err = efl_net_multicast_loopback_set(fd, efl_net_server_fd_family_get(o), loopback);
   if (err)
     {
        ERR("could not set multicast loopback=%hhu: %s", loopback, eina_error_msg_get(err));
        pd->multicast.loopback = old;
     }

   return err;
}

EOLIAN static Eina_Bool
_efl_net_server_udp_multicast_loopback_get(const Eo *o, Efl_Net_Server_Udp_Data *pd)
{
   SOCKET fd = efl_loop_fd_get(o);
   Eina_Error err;
   Eina_Bool loopback = pd->multicast.loopback;

   if (fd == INVALID_SOCKET) return pd->multicast.loopback;

   err = efl_net_multicast_loopback_get(fd, efl_net_server_fd_family_get(o), &loopback);
   if (err)
     ERR("could not get multicast loopback: %s", eina_error_msg_get(err));
   else
     pd->multicast.loopback = loopback;

   return pd->multicast.loopback;
}

#include "efl_net_server_udp.eo.c"
