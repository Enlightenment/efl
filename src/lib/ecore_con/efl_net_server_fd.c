#define EFL_NET_SERVER_FD_PROTECTED 1
#define EFL_NET_SERVER_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

#include <fcntl.h>
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef _WIN32
# include <Evil.h>
#endif

#ifdef HAVE_SYSTEMD
# include <systemd/sd-daemon.h>
#endif

#define MY_CLASS EFL_NET_SERVER_FD_CLASS

typedef struct _Efl_Net_Server_Fd_Data
{
   Eina_Stringshare *address;
   int family;
   unsigned int clients_count;
   unsigned int clients_limit;
   Eina_Bool clients_reject_excess;
   Eina_Bool waiting_accept;
   Eina_Bool serving;
   Eina_Bool close_on_exec;
   Eina_Bool reuse_address;
   Eina_Bool reuse_port;
} Efl_Net_Server_Fd_Data;

static SOCKET
efl_net_accept4(SOCKET fd, struct sockaddr *addr, socklen_t *addrlen, Eina_Bool close_on_exec)
{
#ifdef HAVE_ACCEPT4
   int flags = 0;
   if (close_on_exec) flags |= SOCK_CLOEXEC;
   return accept4(fd, addr, addrlen, flags);
#else
   SOCKET client = accept(fd, addr, addrlen);
   if (client == INVALID_SOCKET) return client;

#ifdef FD_CLOEXEC
   if (close_on_exec)
     {
        if (!eina_file_close_on_exec(client, EINA_TRUE))
          {
             int errno_bkp = errno;
             ERR("fcntl(" SOCKET_FMT ", F_SETFD, FD_CLOEXEC): %s", client, strerror(errno));
             closesocket(client);
             errno = errno_bkp;
             return INVALID_SOCKET;
          }
     }
#else
   (void)close_on_exec;
#endif

   return client;
#endif
}

static void
_efl_net_server_fd_event_read(void *data EINA_UNUSED, const Efl_Event *event)
{
   efl_net_server_fd_process_incoming_data(event->object);
}

static void
_efl_net_server_fd_event_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *o = event->object;
   Eina_Error err = EBADF;

   efl_net_server_serving_set(o, EINA_FALSE);
   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_ERROR, &err);
}

EOLIAN static Efl_Object *
_efl_net_server_fd_efl_object_finalize(Eo *o, Efl_Net_Server_Fd_Data *pd)
{
   o = efl_finalize(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   pd->waiting_accept = EINA_TRUE;
   efl_event_callback_add(o, EFL_LOOP_FD_EVENT_READ, _efl_net_server_fd_event_read, NULL);
   efl_event_callback_add(o, EFL_LOOP_FD_EVENT_ERROR, _efl_net_server_fd_event_error, NULL);
   return o;
}

EOLIAN static Efl_Object *
_efl_net_server_fd_efl_object_constructor(Eo *o, Efl_Net_Server_Fd_Data *pd)
{
   pd->family = AF_UNSPEC;
   pd->close_on_exec = EINA_TRUE;
   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_server_fd_efl_object_destructor(Eo *o, Efl_Net_Server_Fd_Data *pd)
{
   SOCKET fd = efl_loop_fd_get(o);

   if (fd != INVALID_SOCKET)
     {
        efl_loop_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
        closesocket(fd);
     }

   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address, NULL);
}

EOLIAN static void
_efl_net_server_fd_efl_loop_fd_fd_set(Eo *o, Efl_Net_Server_Fd_Data *pd, int pfd)
{
   SOCKET fd = (SOCKET)pfd;

   efl_loop_fd_set(efl_super(o, MY_CLASS), pfd);

   if (fd != INVALID_SOCKET)
     {
        /* apply postponed values */
        efl_net_server_fd_close_on_exec_set(o, pd->close_on_exec);
        efl_net_server_fd_reuse_address_set(o, pd->reuse_address);
        efl_net_server_fd_reuse_port_set(o, pd->reuse_port);

        if (pd->family == AF_UNSPEC)
          {
             ERR("efl_loop_fd_set() must be called after efl_net_server_fd_family_set()");
             return;
          }
     }
   else
     {
        efl_net_server_address_set(o, NULL);
     }
}

EOLIAN static void
_efl_net_server_fd_efl_net_server_address_set(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address, address);
}

EOLIAN static const char *
_efl_net_server_fd_efl_net_server_address_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd)
{
   return pd->address;
}

EOLIAN static unsigned int
_efl_net_server_fd_efl_net_server_clients_count_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd)
{
   return pd->clients_count;
}

EOLIAN static void
_efl_net_server_fd_efl_net_server_clients_count_set(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd, unsigned int count)
{
   pd->clients_count = count;
   if ((pd->clients_limit == 0) || (pd->clients_limit > count))
     {
        if (!pd->waiting_accept)
          {
             pd->waiting_accept = EINA_TRUE;
             efl_event_callback_add(o, EFL_LOOP_FD_EVENT_READ, _efl_net_server_fd_event_read, NULL);
          }
     }
}

EOLIAN static void
_efl_net_server_fd_efl_net_server_clients_limit_set(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd, unsigned int limit, Eina_Bool reject_excess)
{
   pd->clients_limit = limit;
   pd->clients_reject_excess = reject_excess;
}

EOLIAN static void
_efl_net_server_fd_efl_net_server_clients_limit_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd, unsigned int *limit, Eina_Bool *reject_excess)
{
   if (limit) *limit = pd->clients_limit;
   if (reject_excess) *reject_excess = pd->clients_reject_excess;
}

EOLIAN static void
_efl_net_server_fd_efl_net_server_serving_set(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd, Eina_Bool serving)
{
   if (pd->serving == serving) return;
   pd->serving = serving;
   if (serving)
     efl_event_callback_call(o, EFL_NET_SERVER_EVENT_SERVING, NULL);
}

EOLIAN static Eina_Error
_efl_net_server_fd_efl_net_server_serve(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd EINA_UNUSED, const char *address)
{
   DBG("address=%s", address);
   return 0;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_efl_net_server_serving_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd)
{
   return pd->serving;
}

EOLIAN static Eina_Error
_efl_net_server_fd_socket_activate(Eo *o, Efl_Net_Server_Fd_Data *pd EINA_UNUSED, const char *address)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL((SOCKET)efl_loop_fd_get(o) != INVALID_SOCKET, EALREADY);
   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);

#ifndef HAVE_SYSTEMD
   DBG("systemd support is disabled");
   return ENOENT;
#else
   if (!sd_fd_max)
     {
        DBG("This service was not socket-activated, no $LISTEN_FDS");
        return ENOENT;
     }
   else if (sd_fd_index >= sd_fd_max)
     {
        WRN("No more systemd sockets available. Configuration mismatch?");
        return ENOENT;
     }
   else
     {
        SOCKET fd = SD_LISTEN_FDS_START + sd_fd_index;
        int family;
        socklen_t len = sizeof(family);

        if (getsockopt(fd, SOL_SOCKET, SO_DOMAIN, (char *)&family, &len) != 0)
          {
             WRN("socket " SOCKET_FMT " failed to return family: %s", fd, eina_error_msg_get(efl_net_socket_error_get()));
             return EINVAL;
          }

        sd_fd_index++;
        efl_net_server_fd_family_set(o, family);
        efl_loop_fd_set(o, fd);
        if ((SOCKET)efl_loop_fd_get(o) == INVALID_SOCKET)
          {
             sd_fd_index--;
             WRN("socket " SOCKET_FMT " could not be used by %p (%s)",
                 fd, o, efl_class_name_get(efl_class_get(o)));
             return EINVAL;
          }

        /* by default they all come with close_on_exec set
         * and we must apply our local conf.
         */
        efl_net_server_fd_close_on_exec_set(o, pd->close_on_exec);
        return 0;
     }
#endif
}

EOLIAN static Eina_Bool
_efl_net_server_fd_close_on_exec_set(Eo *o, Efl_Net_Server_Fd_Data *pd, Eina_Bool close_on_exec)
{
#ifdef FD_CLOEXEC
   SOCKET fd;
   Eina_Bool old = pd->close_on_exec;
#endif

   pd->close_on_exec = close_on_exec;

#ifdef FD_CLOEXEC
   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return EINA_TRUE; /* postpone until fd_set() */

   if (!eina_file_close_on_exec(fd, close_on_exec))
     {
        ERR("fcntl(" SOCKET_FMT ", F_SETFD,): %s", fd, strerror(errno));
        pd->close_on_exec = old;
        return EINA_FALSE;
     }
#else
   DBG("close on exec is not supported on your platform");
   (void)close_on_exec;
   (void)o;
#endif

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_close_on_exec_get(const Eo *o, Efl_Net_Server_Fd_Data *pd)
{
#ifdef FD_CLOEXEC
   SOCKET fd;
   int flags;

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return pd->close_on_exec;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   flags = fcntl(fd, F_GETFD);
   if (flags < 0)
     {
        ERR("fcntl(" SOCKET_FMT ", F_GETFD): %s", fd, strerror(errno));
        return EINA_FALSE;
     }

   pd->close_on_exec = !!(flags & FD_CLOEXEC); /* sync */
#else
   (void)o;
#endif
   return pd->close_on_exec;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_reuse_address_set(Eo *o, Efl_Net_Server_Fd_Data *pd, Eina_Bool reuse_address)
{
   SOCKET fd;
   int value;
   Eina_Bool old = pd->reuse_address;

   pd->reuse_address = reuse_address;

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return EINA_TRUE; /* postpone until fd_set() */

   value = reuse_address;
   if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value)) != 0)
     {
        ERR("setsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_REUSEADDR, %d): %s",
            fd, value, eina_error_msg_get(efl_net_socket_error_get()));
        pd->reuse_address = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_reuse_address_get(const Eo *o, Efl_Net_Server_Fd_Data *pd)
{
   SOCKET fd;
   int value = 0;
   socklen_t valuelen;

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return pd->reuse_address;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&value, &valuelen) != 0)
     {
        ERR("getsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_REUSEADDR): %s",
            fd, eina_error_msg_get(efl_net_socket_error_get()));
        return EINA_FALSE;
     }

   pd->reuse_address = !!value; /* sync */
   return pd->reuse_address;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_reuse_port_set(Eo *o, Efl_Net_Server_Fd_Data *pd, Eina_Bool reuse_port)
{
#ifdef SO_REUSEPORT
   SOCKET fd;
   int value;
   Eina_Bool old = pd->reuse_port;
#endif

   pd->reuse_port = reuse_port;

#ifdef SO_REUSEPORT
   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return EINA_TRUE; /* postpone until fd_set() */

   value = reuse_port;
   if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (const char *)&value, sizeof(value)) != 0)
     {
        ERR("setsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_REUSEPORT, %d): %s",
            fd, value, eina_error_msg_get(efl_net_socket_error_get()));
        pd->reuse_port = old;
        return EINA_FALSE;
     }
#else
   (void)o;
#endif

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_reuse_port_get(const Eo *o, Efl_Net_Server_Fd_Data *pd)
{
#ifdef SO_REUSEPORT
   SOCKET fd;
   int value = 0;
   socklen_t valuelen;

   fd = efl_loop_fd_get(o);
   if (fd == INVALID_SOCKET) return pd->reuse_port;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char *)&value, &valuelen) != 0)
     {
        ERR("getsockopt(" SOCKET_FMT ", SOL_SOCKET, SO_REUSEPORT): %s",
            fd, eina_error_msg_get(efl_net_socket_error_get()));
        return EINA_FALSE;
     }

   pd->reuse_port = !!value; /* sync */
#else
   (void)o;
#endif

   return pd->reuse_port;
}

EOLIAN static void
_efl_net_server_fd_family_set(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd, int family)
{
   pd->family = family;
}

EOLIAN static int
_efl_net_server_fd_family_get(const Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd)
{
   return pd->family;
}

EOLIAN static void
_efl_net_server_fd_process_incoming_data(Eo *o, Efl_Net_Server_Fd_Data *pd)
{
   Eina_Bool do_reject = EINA_FALSE;
   struct sockaddr_storage addr;
   SOCKET client, fd;
   socklen_t addrlen;

   if ((pd->clients_limit > 0) && (pd->clients_count >= pd->clients_limit))
     {
        if (!pd->clients_reject_excess)
          {
             if (pd->waiting_accept)
               {
                  pd->waiting_accept = EINA_FALSE;
                  efl_event_callback_del(o, EFL_LOOP_FD_EVENT_READ, _efl_net_server_fd_event_read, NULL);
               }
             return;
          }
        do_reject = EINA_TRUE;
     }

   fd = efl_loop_fd_get(o);

   addrlen = sizeof(addr);
   client = efl_net_accept4(fd, (struct sockaddr *)&addr, &addrlen,
                            efl_net_server_fd_close_on_exec_get(o));
   if (client == INVALID_SOCKET)
     {
        Eina_Error err = efl_net_socket_error_get();
        ERR("accept(" SOCKET_FMT "): %s", fd, eina_error_msg_get(err));
        efl_event_callback_call(o, EFL_NET_SERVER_EVENT_ERROR, &err);
        return;
     }

   if (do_reject)
     efl_net_server_fd_client_reject(o, client);
   else
     efl_net_server_fd_client_add(o, client);
}

static void
_efl_net_server_fd_client_event_closed(void *data, const Efl_Event *event)
{
   Eo *server = data;
   Eo *client = event->object;

   efl_event_callback_del(client, EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_fd_client_event_closed, server);
   if (efl_parent_get(client) == server)
     efl_parent_set(client, NULL);

   efl_net_server_clients_count_set(server, efl_net_server_clients_count_get(server) - 1);
}

static Eina_Bool
_efl_net_server_fd_efl_net_server_client_announce(Eo *o, Efl_Net_Server_Fd_Data *pd EINA_UNUSED, Eo *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_GOTO(efl_isa(client, EFL_NET_SOCKET_INTERFACE), wrong_type);
   EINA_SAFETY_ON_FALSE_GOTO(efl_parent_get(client) == o, wrong_parent);

   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_CLIENT_ADD, client);

   if (efl_parent_get(client) != o)
     {
        DBG("client %s was reparented! Ignoring it...",
            efl_net_socket_address_remote_get(client));
        return EINA_TRUE;
     }

   if (efl_ref_count(client) == 1) /* users must take a reference themselves */
     {
        DBG("client %s was not handled, closing it...",
            efl_net_socket_address_remote_get(client));
        efl_del(client);
        return EINA_FALSE;
     }
   else if (efl_io_closer_closed_get(client))
     {
        DBG("client %s was closed from 'client,add', delete it...",
            efl_net_socket_address_remote_get(client));
        efl_del(client);
        return EINA_FALSE;
     }

   efl_net_server_clients_count_set(o, efl_net_server_clients_count_get(o) + 1);
   efl_event_callback_add(client, EFL_IO_CLOSER_EVENT_CLOSED, _efl_net_server_fd_client_event_closed, o);
   return EINA_TRUE;

 wrong_type:
   ERR("%p client %p (%s) doesn't implement Efl.Net.Socket interface, deleting it.", o, client, efl_class_name_get(efl_class_get(client)));
   efl_io_closer_close(client);
   efl_del(client);
   return EINA_FALSE;

 wrong_parent:
   ERR("%p client %p (%s) parent=%p is not our child, deleting it.", o, client, efl_class_name_get(efl_class_get(client)), efl_parent_get(client));
   efl_io_closer_close(client);
   efl_del(client);
   return EINA_FALSE;
}

#include "efl_net_server_fd.eo.c"
