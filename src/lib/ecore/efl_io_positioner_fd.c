#define EFL_IO_POSITIONER_FD_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"

#define MY_CLASS EFL_IO_POSITIONER_FD_CLASS

typedef struct _Efl_Io_Positioner_Fd_Data
{
   int fd;
} Efl_Io_Positioner_Fd_Data;

EOLIAN static void
_efl_io_positioner_fd_positioner_fd_set(Eo *o EINA_UNUSED, Efl_Io_Positioner_Fd_Data *pd, int fd)
{
   pd->fd = fd;
}

EOLIAN static int
_efl_io_positioner_fd_positioner_fd_get(const Eo *o EINA_UNUSED, Efl_Io_Positioner_Fd_Data *pd)
{
   return pd->fd;
}

static inline int
_efl_io_positioner_whence_convert(Efl_Io_Positioner_Whence whence)
{
   switch (whence)
     {
      case EFL_IO_POSITIONER_WHENCE_START: return SEEK_SET;
      case EFL_IO_POSITIONER_WHENCE_CURRENT: return SEEK_CUR;
      case EFL_IO_POSITIONER_WHENCE_END: return SEEK_END;
     }
   return SEEK_SET;
}

EOLIAN static Eina_Error
_efl_io_positioner_fd_efl_io_positioner_seek(Eo *o, Efl_Io_Positioner_Fd_Data *pd EINA_UNUSED, int64_t offset, Efl_Io_Positioner_Whence whence)
{
   int fd = efl_io_positioner_fd_get(o);
   if (lseek(fd, (off_t)offset, _efl_io_positioner_whence_convert(whence)) < 0)
     return errno;
   efl_event_callback_call(o, EFL_IO_POSITIONER_EVENT_POSITION_CHANGED, NULL);
   return 0;
}

EOLIAN static uint64_t
_efl_io_positioner_fd_efl_io_positioner_position_get(const Eo *o, Efl_Io_Positioner_Fd_Data *pd EINA_UNUSED)
{
   int fd = efl_io_positioner_fd_get(o);
   off_t offset;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(fd < 0, 0);

   offset = lseek(fd, 0, SEEK_CUR);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(offset < 0, 0);

   return offset;
}

#include "efl_io_positioner_fd.eo.c"
