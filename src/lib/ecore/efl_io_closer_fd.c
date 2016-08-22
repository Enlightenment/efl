#define EFL_IO_CLOSER_FD_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"

#define MY_CLASS EFL_IO_CLOSER_FD_CLASS

typedef struct _Efl_Io_Closer_Fd_Data
{
   int fd;
} Efl_Io_Closer_Fd_Data;

EOLIAN static void
_efl_io_closer_fd_closer_fd_set(Eo *o EINA_UNUSED, Efl_Io_Closer_Fd_Data *pd, int fd)
{
   pd->fd = fd;
}

EOLIAN static int
_efl_io_closer_fd_closer_fd_get(Eo *o EINA_UNUSED, Efl_Io_Closer_Fd_Data *pd)
{
   return pd->fd;
}

EOLIAN static Eina_Error
_efl_io_closer_fd_efl_io_closer_close(Eo *o, Efl_Io_Closer_Fd_Data *pd EINA_UNUSED)
{
   int fd = efl_io_closer_fd_closer_fd_get(o);
   Eina_Error err = 0;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(fd < 0, EBADF);

   efl_io_closer_fd_closer_fd_set(o, -1);
   if (close(fd) < 0) err = errno;
   efl_event_callback_legacy_call(o, EFL_IO_CLOSER_EVENT_CLOSED, NULL);
   return err;
}

EOLIAN static Eina_Bool
_efl_io_closer_fd_efl_io_closer_closed_get(Eo *o, Efl_Io_Closer_Fd_Data *pd EINA_UNUSED)
{
   return efl_io_closer_fd_closer_fd_get(o) < 0;
}

#include "efl_io_closer_fd.eo.c"
