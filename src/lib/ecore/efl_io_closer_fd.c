#define EFL_IO_CLOSER_FD_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <fcntl.h>
#include "ecore_private.h"

#define MY_CLASS EFL_IO_CLOSER_FD_CLASS

typedef struct _Efl_Io_Closer_Fd_Data
{
   int fd;

   Eina_Bool close_on_exec;
   Eina_Bool close_on_destructor;
   Eina_Bool initialized;
} Efl_Io_Closer_Fd_Data;

static void
_efl_io_closer_initialize(Efl_Io_Closer_Fd_Data *pd)
{
   if (pd->initialized) return ;
   pd->fd = -1;
   pd->initialized = EINA_TRUE;
}

EOLIAN static void
_efl_io_closer_fd_closer_fd_set(Eo *o EINA_UNUSED, Efl_Io_Closer_Fd_Data *pd, int fd)
{
   _efl_io_closer_initialize(pd);
   pd->fd = fd;
}

EOLIAN static int
_efl_io_closer_fd_closer_fd_get(Eo *o EINA_UNUSED, Efl_Io_Closer_Fd_Data *pd)
{
   _efl_io_closer_initialize(pd);
   return pd->fd;
}

EOLIAN static Eina_Error
_efl_io_closer_fd_efl_io_closer_close(Eo *o, Efl_Io_Closer_Fd_Data *pd EINA_UNUSED)
{
   int fd;
   Eina_Error err = 0;

   _efl_io_closer_initialize(pd);
   fd = efl_io_closer_fd_get(o);

   EINA_SAFETY_ON_TRUE_RETURN_VAL(fd < 0, EBADF);

   efl_io_closer_fd_set(o, -1);
   if (close(fd) < 0) err = errno;
   efl_event_callback_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);
   return err;
}

EOLIAN static Eina_Bool
_efl_io_closer_fd_efl_io_closer_closed_get(Eo *o, Efl_Io_Closer_Fd_Data *pd EINA_UNUSED)
{
   _efl_io_closer_initialize(pd);
   return efl_io_closer_fd_get(o) < 0;
}

EOLIAN static Eina_Bool
_efl_io_closer_fd_efl_io_closer_close_on_exec_set(Eo *o, Efl_Io_Closer_Fd_Data *pd, Eina_Bool close_on_exec)
{
   _efl_io_closer_initialize(pd);

#ifdef _WIN32
   DBG("close on exec is not supported on windows");
   pd->close_on_exec = close_on_exec;
   return EINA_FALSE;
   (void)o;
#else
   int fd;
   Eina_Bool old = pd->close_on_exec;

   pd->close_on_exec = close_on_exec;

   fd = efl_io_closer_fd_get(o);
   if (fd < 0) return EINA_TRUE; /* postpone until fd_set(), users
                                  * must apply MANUALLY if it's not
                                  * already set!
                                  */

   if (!eina_file_close_on_exec(fd, close_on_exec))
     {
        ERR("eina_file_close_on_exec(%d) failed", fd);
        pd->close_on_exec = old;
        return EINA_FALSE;
     }

   return EINA_TRUE;
#endif
}

EOLIAN static Eina_Bool
_efl_io_closer_fd_efl_io_closer_close_on_exec_get(Eo *o, Efl_Io_Closer_Fd_Data *pd)
{
   _efl_io_closer_initialize(pd);

#ifdef _WIN32
   return pd->close_on_exec;
   (void)o;
#else
   int flags, fd;

   fd = efl_io_closer_fd_get(o);
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
#endif
}

EOLIAN static void
_efl_io_closer_fd_efl_io_closer_close_on_destructor_set(Eo *o EINA_UNUSED, Efl_Io_Closer_Fd_Data *pd, Eina_Bool close_on_destructor)
{
   _efl_io_closer_initialize(pd);
   pd->close_on_destructor = close_on_destructor;
}

EOLIAN static Eina_Bool
_efl_io_closer_fd_efl_io_closer_close_on_destructor_get(Eo *o EINA_UNUSED, Efl_Io_Closer_Fd_Data *pd)
{
   _efl_io_closer_initialize(pd);
   return pd->close_on_destructor;
}

#include "efl_io_closer_fd.eo.c"
