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
#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#define MY_CLASS EFL_NET_SERVER_FD_CLASS

typedef struct _Efl_Net_Server_Fd_Data
{
   Eina_Stringshare *address;
   int family;
   unsigned int clients_count;
   unsigned int clients_limit;
   Eina_Bool clients_reject_excess;
   Eina_Bool serving;
   Eina_Bool close_on_exec;
   Eina_Bool reuse_address;
   Eina_Bool reuse_port;
} Efl_Net_Server_Fd_Data;

static int
efl_net_accept4(int fd, struct sockaddr *addr, socklen_t *addrlen, Eina_Bool close_on_exec)
{
#ifdef HAVE_ACCEPT4
   int flags = 0;
   if (close_on_exec) flags |= SOCK_CLOEXEC;
   return accept4(fd, addr, addrlen, flags);
#else
   int client = accept(fd, addr, addrlen);
   if (client < 0) return client;

   if (close_on_exec)
     {
        if (fcntl(client, F_SETFD, FD_CLOEXEC) < 0)
          {
             int errno_bkp = errno;
             ERR("fcntl(%d, F_SETFD, FD_CLOEXEC): %s", client, strerror(errno));
             close(client);
             errno = errno_bkp;
             return -1;
          }
     }
   return client;
#endif
}

static void
_efl_net_server_fd_event_read(void *data EINA_UNUSED, const Eo_Event *event)
{
   Eo *o = event->object;
   unsigned int count, limit;
   Eina_Bool reject_excess, do_reject = EINA_FALSE;
   struct sockaddr_storage addr;
   int client, fd;
   socklen_t addrlen;

   count = efl_net_server_clients_count_get(o);
   efl_net_server_clients_limit_get(o, &limit, &reject_excess);

   if ((limit > 0) && (count >= limit))
     {
        if (!reject_excess)
          {
             // TODO: disconnect 'read' so stops calling?
             return;
          }
        do_reject = EINA_TRUE;
     }

   fd = efl_loop_fd_get(o);

   addrlen = sizeof(addr);
   client = efl_net_accept4(fd, (struct sockaddr *)&addr, &addrlen,
                            efl_net_server_fd_close_on_exec_get(o));
   if (client < 0)
     {
        Eina_Error err = errno;
        ERR("accept(%d): %s", fd, strerror(errno));
        efl_event_callback_call(o, EFL_NET_SERVER_EVENT_ERROR, &err);
        return;
     }

   if (do_reject)
     efl_net_server_fd_client_reject(o, client);
   else
     efl_net_server_fd_client_add(o, client);
}

static void
_efl_net_server_fd_event_error(void *data EINA_UNUSED, const Eo_Event *event)
{
   Eo *o = event->object;
   Eina_Error err = EBADF;

   efl_net_server_serving_set(o, EINA_FALSE);
   efl_event_callback_call(o, EFL_NET_SERVER_EVENT_ERROR, &err);
}

EOLIAN static Efl_Object *
_efl_net_server_fd_efl_object_finalize(Eo *o, Efl_Net_Server_Fd_Data *pd EINA_UNUSED)
{
   o = efl_finalize(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   // TODO: only register "read" if "can_read" is being monitored?
   efl_event_callback_add(o, EFL_LOOP_FD_EVENT_READ, _efl_net_server_fd_event_read, NULL);
   efl_event_callback_add(o, EFL_LOOP_FD_EVENT_ERROR, _efl_net_server_fd_event_error, NULL);
   return o;
}

EOLIAN static Efl_Object *
_efl_net_server_fd_efl_object_constructor(Eo *o, Efl_Net_Server_Fd_Data *pd EINA_UNUSED)
{
   pd->family = AF_UNSPEC;
   return efl_constructor(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_server_fd_efl_object_destructor(Eo *o, Efl_Net_Server_Fd_Data *pd)
{
   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address, NULL);
}

EOLIAN static void
_efl_net_server_fd_efl_loop_fd_fd_set(Eo *o, Efl_Net_Server_Fd_Data *pd, int fd)
{
   efl_loop_fd_set(efl_super(o, MY_CLASS), fd);

   if (fd >= 0)
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
_efl_net_server_fd_efl_net_server_address_get(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd)
{
   return pd->address;
}

EOLIAN static unsigned int
_efl_net_server_fd_efl_net_server_clients_count_get(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd)
{
   return pd->clients_count;
}

EOLIAN static void
_efl_net_server_fd_efl_net_server_clients_count_set(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd, unsigned int count)
{
   pd->clients_count = count;
}

EOLIAN static void
_efl_net_server_fd_efl_net_server_clients_limit_set(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd, unsigned int limit, Eina_Bool reject_excess)
{
   pd->clients_limit = limit;
   pd->clients_reject_excess = reject_excess;
}

EOLIAN static void
_efl_net_server_fd_efl_net_server_clients_limit_get(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd, unsigned int *limit, Eina_Bool *reject_excess)
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
_efl_net_server_fd_efl_net_server_serving_get(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd)
{
   return pd->serving;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_close_on_exec_set(Eo *o, Efl_Net_Server_Fd_Data *pd, Eina_Bool close_on_exec)
{
   int flags, fd;
   Eina_Bool old = pd->close_on_exec;

   pd->close_on_exec = close_on_exec;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return EINA_TRUE; /* postpone until fd_set() */

   flags = fcntl(fd, F_GETFD);
   if (flags < 0)
     {
        ERR("fcntl(%d, F_GETFD): %s", fd, strerror(errno));
        return EINA_FALSE;
     }
   if (close_on_exec)
     flags |= FD_CLOEXEC;
   else
     flags &= (~FD_CLOEXEC);
   if (fcntl(fd, F_SETFD, flags) < 0)
     {
        ERR("fcntl(%d, F_SETFD, %#x): %s", fd, flags, strerror(errno));
        pd->close_on_exec = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_close_on_exec_get(Eo *o, Efl_Net_Server_Fd_Data *pd)
{
   int flags, fd;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return pd->close_on_exec;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   flags = fcntl(fd, F_GETFD);
   if (flags < 0)
     {
        ERR("fcntl(%d, F_GETFD): %s", fd, strerror(errno));
        return EINA_FALSE;
     }

   pd->close_on_exec = !!(flags & FD_CLOEXEC); /* sync */
   return pd->close_on_exec;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_reuse_address_set(Eo *o, Efl_Net_Server_Fd_Data *pd, Eina_Bool reuse_address)
{
   int value, fd;
   Eina_Bool old = pd->reuse_address;

   pd->reuse_address = reuse_address;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return EINA_TRUE; /* postpone until fd_set() */

   value = reuse_address;
   if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) < 0)
     {
        ERR("setsockopt(%d, SOL_SOCKET, SO_REUSEADDR, %d): %s",
            fd, value, strerror(errno));
        pd->reuse_address = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_reuse_address_get(Eo *o, Efl_Net_Server_Fd_Data *pd)
{
   int value = 0, fd;
   socklen_t valuelen;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return pd->reuse_address;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value, &valuelen) < 0)
     {
        ERR("getsockopt(%d, SOL_SOCKET, SO_REUSEADDR): %s",
            fd, strerror(errno));
        return EINA_FALSE;
     }

   pd->reuse_address = !!value; /* sync */
   return pd->reuse_address;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_reuse_port_set(Eo *o, Efl_Net_Server_Fd_Data *pd, Eina_Bool reuse_port)
{
   int value, fd;
   Eina_Bool old = pd->reuse_port;

   pd->reuse_port = reuse_port;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return EINA_TRUE; /* postpone until fd_set() */

   value = reuse_port;
   if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &value, sizeof(value)) < 0)
     {
        ERR("setsockopt(%d, SOL_SOCKET, SO_REUSEPORT, %d): %s",
            fd, value, strerror(errno));
        pd->reuse_port = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_net_server_fd_reuse_port_get(Eo *o, Efl_Net_Server_Fd_Data *pd)
{
   int value = 0, fd;
   socklen_t valuelen;

   fd = efl_loop_fd_get(o);
   if (fd < 0) return pd->reuse_port;

   /* if there is a fd, always query it directly as it may be modified
    * elsewhere by nasty users.
    */
   valuelen = sizeof(value);
   if (getsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &value, &valuelen) < 0)
     {
        ERR("getsockopt(%d, SOL_SOCKET, SO_REUSEPORT): %s",
            fd, strerror(errno));
        return EINA_FALSE;
     }

   pd->reuse_port = !!value; /* sync */
   return pd->reuse_port;
}

EOLIAN static void
_efl_net_server_fd_family_set(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd, int family)
{
   pd->family = family;
}

EOLIAN static int
_efl_net_server_fd_family_get(Eo *o EINA_UNUSED, Efl_Net_Server_Fd_Data *pd)
{
   return pd->family;
}

#include "efl_net_server_fd.eo.c"
