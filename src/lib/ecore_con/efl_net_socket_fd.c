#define EFL_NET_SOCKET_FD_PROTECTED 1
#define EFL_LOOP_FD_PROTECTED 1
#define EFL_IO_READER_FD_PROTECTED 1
#define EFL_IO_WRITER_FD_PROTECTED 1
#define EFL_IO_CLOSER_FD_PROTECTED 1
#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1

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

#define MY_CLASS EFL_NET_SOCKET_FD_CLASS

typedef struct _Efl_Net_Socket_Fd_Data
{
   Eina_Stringshare *address_local;
   Eina_Stringshare *address_remote;
   int family;
} Efl_Net_Socket_Fd_Data;

static void
_efl_net_socket_fd_event_read(void *data EINA_UNUSED, const Efl_Event *event)
{
   if (efl_io_closer_closed_get(event->object))
     return;
   efl_io_reader_can_read_set(event->object, EINA_TRUE);
}

static void
_efl_net_socket_fd_event_write(void *data EINA_UNUSED, const Efl_Event *event)
{
   if (efl_io_closer_closed_get(event->object))
     return;
   efl_io_writer_can_write_set(event->object, EINA_TRUE);
}

static void
_efl_net_socket_fd_event_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   if (efl_io_closer_closed_get(event->object))
     return;
   efl_io_writer_can_write_set(event->object, EINA_FALSE);
   efl_io_reader_can_read_set(event->object, EINA_FALSE);
   efl_io_reader_eos_set(event->object, EINA_TRUE);
}

EOLIAN static Efl_Object *
_efl_net_socket_fd_efl_object_finalize(Eo *o, Efl_Net_Socket_Fd_Data *pd EINA_UNUSED)
{
   o = efl_finalize(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   efl_event_callback_add(o, EFL_LOOP_FD_EVENT_WRITE, _efl_net_socket_fd_event_write, NULL);
   efl_event_callback_add(o, EFL_LOOP_FD_EVENT_READ, _efl_net_socket_fd_event_read, NULL);
   efl_event_callback_add(o, EFL_LOOP_FD_EVENT_ERROR, _efl_net_socket_fd_event_error, NULL);
   return o;
}

EOLIAN static Efl_Object *
_efl_net_socket_fd_efl_object_constructor(Eo *o, Efl_Net_Socket_Fd_Data *pd)
{
   pd->family = AF_UNSPEC;
   o = efl_constructor(efl_super(o, MY_CLASS));

   efl_io_closer_close_on_exec_set(o, EINA_TRUE);
   efl_io_closer_close_on_invalidate_set(o, EINA_TRUE);
   efl_io_reader_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
   efl_io_writer_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
   efl_io_closer_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));

   return o;
}

EOLIAN static void
_efl_net_socket_fd_efl_object_invalidate(Eo *o, Efl_Net_Socket_Fd_Data *pd EINA_UNUSED)
{
   if (efl_io_closer_close_on_invalidate_get(o) &&
       (!efl_io_closer_closed_get(o)))
     {
        efl_event_freeze(o);
        efl_io_closer_close(o);
        efl_event_thaw(o);
     }

   efl_invalidate(efl_super(o, MY_CLASS));
}

EOLIAN static void
_efl_net_socket_fd_efl_object_destructor(Eo *o, Efl_Net_Socket_Fd_Data *pd)
{
   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address_local, NULL);
   eina_stringshare_replace(&pd->address_remote, NULL);
}

static void
_efl_net_socket_fd_set(Eo *o, Efl_Net_Socket_Fd_Data *pd, SOCKET fd)
{
   Eina_Bool close_on_exec = efl_io_closer_close_on_exec_get(o); /* get cached value, otherwise will query from set fd */
   efl_io_reader_fd_set(o, fd);
   efl_io_writer_fd_set(o, fd);
   efl_io_closer_fd_set(o, fd);

   /* apply postponed values */
   efl_io_closer_close_on_exec_set(o, close_on_exec);
   if (pd->family == AF_UNSPEC)
     {
        ERR("efl_loop_fd_set() must be called after efl_net_server_fd_family_set()");
        return;
     }
}

static void
_efl_net_socket_fd_unset(Eo *o)
{
   efl_io_reader_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
   efl_io_writer_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
   efl_io_closer_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));

   efl_net_socket_address_local_set(o, NULL);
   efl_net_socket_address_remote_set(o, NULL);
}

EOLIAN static void
_efl_net_socket_fd_efl_loop_fd_fd_set(Eo *o, Efl_Net_Socket_Fd_Data *pd, int pfd)
{
   SOCKET fd = (SOCKET)pfd;

   if ((pd->family == AF_UNSPEC) && (fd != INVALID_SOCKET))
     {
        struct sockaddr_storage addr;
        socklen_t addrlen = sizeof(addr);
        if (getsockname(fd, (struct sockaddr *)&addr, &addrlen) != 0)
          ERR("getsockname(" SOCKET_FMT "): %s", fd, eina_error_msg_get(efl_net_socket_error_get()));
        else
          efl_net_socket_fd_family_set(o, addr.ss_family);
     }

   efl_loop_fd_set(efl_super(o, MY_CLASS), fd);

   if (fd != INVALID_SOCKET) _efl_net_socket_fd_set(o, pd, fd);
   else _efl_net_socket_fd_unset(o);
}

EOLIAN static Eina_Error
_efl_net_socket_fd_efl_io_closer_close(Eo *o, Efl_Net_Socket_Fd_Data *pd EINA_UNUSED)
{
   SOCKET fd = efl_io_closer_fd_get(o);
   Eina_Error ret = 0;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(o), EBADF);

   efl_io_writer_can_write_set(o, EINA_FALSE);
   efl_io_reader_can_read_set(o, EINA_FALSE);
   efl_io_reader_eos_set(o, EINA_TRUE);

   /* skip _efl_net_socket_fd_efl_loop_fd_fd_set() since we want to
    * retain efl_io_closer_fd_get() so close(super()) works
    * and we emit the events with proper addresses.
    */
   efl_loop_fd_set(efl_super(o, MY_CLASS), SOCKET_TO_LOOP_FD(INVALID_SOCKET));

   efl_io_closer_fd_set(o, SOCKET_TO_LOOP_FD(INVALID_SOCKET));
   if (!((pd->family == AF_UNSPEC) && (fd == 0))) /* if nothing is set, fds are all zero, avoid closing STDOUT */
     if (closesocket(fd) != 0) ret = efl_net_socket_error_get();
   efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);

   /* do the cleanup our _efl_net_socket_fd_efl_loop_fd_fd_set() would do */
   _efl_net_socket_fd_unset(o);

   return ret;
}

EOLIAN static Eina_Bool
_efl_net_socket_fd_efl_io_closer_closed_get(const Eo *o, Efl_Net_Socket_Fd_Data *pd)
{
   if (pd->family == AF_UNSPEC) return EINA_FALSE;
   return (SOCKET)efl_io_closer_fd_get(o) == INVALID_SOCKET;
}

EOLIAN static Eina_Error
_efl_net_socket_fd_efl_io_reader_read(Eo *o, Efl_Net_Socket_Fd_Data *pd EINA_UNUSED, Eina_Rw_Slice *rw_slice)
{
   SOCKET fd = efl_io_reader_fd_get(o);
   ssize_t r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rw_slice, EINVAL);
   if (fd == INVALID_SOCKET) goto error;
   do
     {
        r = recv(fd, rw_slice->mem, rw_slice->len, 0);
        if (r == SOCKET_ERROR)
          {
             Eina_Error err = efl_net_socket_error_get();

             if (err == EINTR) continue;

             rw_slice->len = 0;
             rw_slice->mem = NULL;

             efl_io_reader_can_read_set(o, EINA_FALSE);
             return err;
          }
     }
   while (r == SOCKET_ERROR);

   rw_slice->len = r;
   efl_io_reader_can_read_set(o, EINA_FALSE); /* wait Efl.Loop.Fd "read" */
   if (r == 0)
     efl_io_reader_eos_set(o, EINA_TRUE);

   return 0;

 error:
   rw_slice->len = 0;
   rw_slice->mem = NULL;
   efl_io_reader_can_read_set(o, EINA_FALSE);
   return EINVAL;
}

EOLIAN static void
_efl_net_socket_fd_efl_io_reader_can_read_set(Eo *o, Efl_Net_Socket_Fd_Data *pd EINA_UNUSED, Eina_Bool value)
{
   Eina_Bool old = efl_io_reader_can_read_get(o);
   if (old == value) return;

   efl_io_reader_can_read_set(efl_super(o, MY_CLASS), value);

   if (value)
     {
        /* stop monitoring the FD, we need to wait the user to read and clear the kernel flag */
        efl_event_callback_del(o, EFL_LOOP_FD_EVENT_READ, _efl_net_socket_fd_event_read, NULL);
     }
   else
     {
        /* kernel flag is clear, resume monitoring the FD */
        efl_event_callback_add(o, EFL_LOOP_FD_EVENT_READ, _efl_net_socket_fd_event_read, NULL);
     }
}

EOLIAN static void
_efl_net_socket_fd_efl_io_reader_eos_set(Eo *o, Efl_Net_Socket_Fd_Data *pd EINA_UNUSED, Eina_Bool value)
{
   Eina_Bool old = efl_io_reader_eos_get(o);
   if (old == value) return;

   efl_io_reader_eos_set(efl_super(o, MY_CLASS), value);

   if (!value) return;

   /* stop monitoring the FD, it's closed */
   efl_event_callback_del(o, EFL_LOOP_FD_EVENT_READ, _efl_net_socket_fd_event_read, NULL);
   efl_event_callback_del(o, EFL_LOOP_FD_EVENT_WRITE, _efl_net_socket_fd_event_write, NULL);
}

EOLIAN static Eina_Error
_efl_net_socket_fd_efl_io_writer_write(Eo *o, Efl_Net_Socket_Fd_Data *pd EINA_UNUSED, Eina_Slice *ro_slice, Eina_Slice *remaining)
{
   SOCKET fd = efl_io_writer_fd_get(o);
   ssize_t r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ro_slice, EINVAL);
   if (fd == INVALID_SOCKET) goto error;

   do
     {
        r = send(fd, ro_slice->mem, ro_slice->len, 0);
        if (r == SOCKET_ERROR)
          {
             Eina_Error err = efl_net_socket_error_get();

             if (err == EINTR) continue;

             if (remaining) *remaining = *ro_slice;
             ro_slice->len = 0;
             ro_slice->mem = NULL;
             efl_io_writer_can_write_set(o, EINA_FALSE);
             return err;
          }
     }
   while (r == SOCKET_ERROR);

   if (remaining)
     {
        remaining->len = ro_slice->len - r;
        remaining->bytes = ro_slice->bytes + r;
     }
   ro_slice->len = r;
   efl_io_writer_can_write_set(o, EINA_FALSE); /* wait Efl.Loop.Fd "write" */

   return 0;

 error:
   if (remaining) *remaining = *ro_slice;
   ro_slice->len = 0;
   ro_slice->mem = NULL;
   efl_io_writer_can_write_set(o, EINA_FALSE);
   return EINVAL;
}

EOLIAN static void
_efl_net_socket_fd_efl_io_writer_can_write_set(Eo *o, Efl_Net_Socket_Fd_Data *pd EINA_UNUSED, Eina_Bool value)
{
   Eina_Bool old = efl_io_writer_can_write_get(o);
   if (old == value) return;

   efl_io_writer_can_write_set(efl_super(o, MY_CLASS), value);

   if (value)
     {
        /* stop monitoring the FD, we need to wait the user to write and clear the kernel flag */
        efl_event_callback_del(o, EFL_LOOP_FD_EVENT_WRITE, _efl_net_socket_fd_event_write, NULL);
     }
   else
     {
        /* kernel flag is clear, resume monitoring the FD */
        efl_event_callback_add(o, EFL_LOOP_FD_EVENT_WRITE, _efl_net_socket_fd_event_write, NULL);
     }
}

EOLIAN static void
_efl_net_socket_fd_efl_net_socket_address_local_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Fd_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_local, address);
}

EOLIAN static const char *
_efl_net_socket_fd_efl_net_socket_address_local_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Fd_Data *pd)
{
   return pd->address_local;
}

EOLIAN static void
_efl_net_socket_fd_efl_net_socket_address_remote_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Fd_Data *pd, const char *address)
{
   eina_stringshare_replace(&pd->address_remote, address);
}

EOLIAN static const char *
_efl_net_socket_fd_efl_net_socket_address_remote_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Fd_Data *pd)
{
   return pd->address_remote;
}

EOLIAN static void
_efl_net_socket_fd_family_set(Eo *o EINA_UNUSED, Efl_Net_Socket_Fd_Data *pd, int family)
{
   pd->family = family;
}

EOLIAN static int
_efl_net_socket_fd_family_get(const Eo *o EINA_UNUSED, Efl_Net_Socket_Fd_Data *pd)
{
   return pd->family;
}

#include "efl_net_socket_fd.eo.c"
