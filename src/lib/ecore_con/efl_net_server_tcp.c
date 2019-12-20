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

#define MY_CLASS EFL_NET_SERVER_TCP_CLASS

typedef struct _Efl_Net_Server_Tcp_Data
{
   Ecore_Thread *resolver;
} Efl_Net_Server_Tcp_Data;

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
_efl_net_server_tcp_resolved_bind(Eo *o, Efl_Net_Server_Tcp_Data *pd EINA_UNUSED, const struct addrinfo *addr)
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
        efl_net_server_ip_ipv6_only_set(o, efl_net_server_ip_ipv6_only_get(o));
     }

   r = bind(fd, addr->ai_addr, addrlen);
   if (r != 0)
     {
        err = efl_net_socket_error_get();
        efl_net_ip_port_fmt(buf, sizeof(buf), addr->ai_addr);
        DBG("bind(" SOCKET_FMT ", %s): %s", fd, buf, eina_error_msg_get(err));
        goto error;
     }

   r = listen(fd, 0);
   if (r != 0)
     {
        err = efl_net_socket_error_get();
        DBG("listen(" SOCKET_FMT "): %s", fd, eina_error_msg_get(err));
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
   return 0;

 error:
   efl_net_server_fd_family_set(o, AF_UNSPEC);
   efl_loop_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
   closesocket(fd);
   return err;
}

static void
_efl_net_server_tcp_resolved(void *data, const char *host EINA_UNUSED, const char *port EINA_UNUSED, const struct addrinfo *hints EINA_UNUSED, struct addrinfo *result, int gai_error)
{
   Eo *o = data;
   Efl_Net_Server_Tcp_Data *pd = efl_data_scope_get(o, MY_CLASS);
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
        err = _efl_net_server_tcp_resolved_bind(o, pd, addr);
        if (err == 0) break;
     }
   freeaddrinfo(result);

 end:
   if (err) efl_event_callback_call(o, EFL_NET_SERVER_EVENT_SERVER_ERROR, &err);

   efl_unref(o);
}

EOLIAN static Eina_Error
_efl_net_server_tcp_efl_net_server_fd_socket_activate(Eo *o, Efl_Net_Server_Tcp_Data *pd EINA_UNUSED, const char *address)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((SOCKET)efl_loop_fd_get(o) != INVALID_SOCKET, EALREADY);
   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);

#ifndef HAVE_SYSTEMD
   return efl_net_server_fd_socket_activate(efl_super(o, MY_CLASS), address);
#else
   {
      char buf[INET6_ADDRSTRLEN + sizeof("[]:65536")];
      Eina_Bool listening;
      Eina_Error err;
      struct sockaddr_storage addr;
      socklen_t addrlen;
      SOCKET fd;

      err = efl_net_ip_socket_activate_check(address, AF_UNSPEC, SOCK_STREAM, &listening);
      if (err) return err;

      err = efl_net_server_fd_socket_activate(efl_super(o, MY_CLASS), address);
      if (err) return err;

      fd = efl_loop_fd_get(o);

      if (!listening)
        {
           if (listen(fd, 0) != 0)
             {
                err = efl_net_socket_error_get();
                DBG("listen(" SOCKET_FMT "): %s", fd, eina_error_msg_get(err));
                goto error;
             }
        }

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

   if (efl_net_ip_port_parse_split(host, port, &ss))
     {
        struct addrinfo ai = hints;

        ai.ai_family = ss.ss_family;
        ai.ai_addr = (struct sockaddr *)&ss;
        ai.ai_addrlen = ss.ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

        err = _efl_net_server_tcp_resolved_bind(o, pd, &ai);
        free(str);
     }
   else
     {
        pd->resolver = efl_net_ip_resolve_async_new(host, port, &hints,
                                                    _efl_net_server_tcp_resolved, o);
        free(str);
        EINA_SAFETY_ON_NULL_RETURN_VAL(pd->resolver, EINVAL);
        err = 0;
     }

   return err;
}

static void
_efl_net_server_tcp_efl_net_server_fd_client_add(Eo *o, Efl_Net_Server_Tcp_Data *pd EINA_UNUSED, int client_fd)
{
   Eo *client = efl_add(EFL_NET_SOCKET_TCP_CLASS, o,
                        efl_io_closer_close_on_exec_set(efl_added, efl_net_server_fd_close_on_exec_get(o)),
                        efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE),
                        efl_loop_fd_set(efl_added, client_fd));
   if (!client)
     {
        ERR("could not create client object fd=" SOCKET_FMT, (SOCKET)client_fd);
        closesocket(client_fd);
        return;
     }

   efl_net_server_client_announce(o, client);
}

static void
_efl_net_server_tcp_efl_net_server_fd_client_reject(Eo *o, Efl_Net_Server_Tcp_Data *pd EINA_UNUSED, int client_fd)
{
   struct sockaddr_storage addr;
   socklen_t addrlen;
   char str[INET6_ADDRSTRLEN + sizeof("[]:65536")] = "";

   addrlen = sizeof(addr);
   if (getpeername(client_fd, (struct sockaddr *)&addr, &addrlen) != 0)
     ERR("getpeername(" SOCKET_FMT "): %s", (SOCKET)client_fd, eina_error_msg_get(efl_net_socket_error_get()));
   else
     efl_net_ip_port_fmt(str, sizeof(str), (struct sockaddr *)&addr);

   closesocket(client_fd);
   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_REJECTED, str);
}

#include "efl_net_server_tcp.eo.c"
