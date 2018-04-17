#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_WRITER_FD_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"

#define MY_CLASS EFL_IO_WRITER_FD_CLASS

typedef struct _Efl_Io_Writer_Fd_Data
{
   int fd;
   Eina_Bool can_write;
} Efl_Io_Writer_Fd_Data;

EOLIAN static void
_efl_io_writer_fd_writer_fd_set(Eo *o EINA_UNUSED, Efl_Io_Writer_Fd_Data *pd, int fd)
{
   pd->fd = fd;
}

EOLIAN static int
_efl_io_writer_fd_writer_fd_get(const Eo *o EINA_UNUSED, Efl_Io_Writer_Fd_Data *pd)
{
   return pd->fd;
}

EOLIAN static Eina_Error
_efl_io_writer_fd_efl_io_writer_write(Eo *o, Efl_Io_Writer_Fd_Data *pd EINA_UNUSED, Eina_Slice *ro_slice, Eina_Slice *remaining)
{
   int fd = efl_io_writer_fd_get(o);
   ssize_t r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ro_slice, EINVAL);
   if (fd < 0) goto error;

   do
     {
        r = write(fd, ro_slice->mem, ro_slice->len);
        if (r < 0)
          {
             if (errno == EINTR) continue;

             if (remaining) *remaining = *ro_slice;
             ro_slice->len = 0;
             ro_slice->mem = NULL;
             efl_io_writer_can_write_set(o, EINA_FALSE);
             return errno;
          }
     }
   while (r < 0);

   if (remaining)
     {
        remaining->len = ro_slice->len - r;
        remaining->bytes = ro_slice->bytes + r;
     }
   ro_slice->len = r;
   if (r == 0) efl_io_writer_can_write_set(o, EINA_FALSE);
   return 0;

 error:
   if (remaining) *remaining = *ro_slice;
   ro_slice->len = 0;
   ro_slice->mem = NULL;
   efl_io_writer_can_write_set(o, EINA_FALSE);
   return EINVAL;

}

EOLIAN static Eina_Bool
_efl_io_writer_fd_efl_io_writer_can_write_get(const Eo *o EINA_UNUSED, Efl_Io_Writer_Fd_Data *pd)
{
   return pd->can_write;
}

EOLIAN static void
_efl_io_writer_fd_efl_io_writer_can_write_set(Eo *o, Efl_Io_Writer_Fd_Data *pd, Eina_Bool can_write)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_writer_fd_get(o) < 0 && can_write);
   if (pd->can_write == can_write) return;
   pd->can_write = can_write;
   efl_event_callback_call(o, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, NULL);
}

#include "efl_io_writer_fd.eo.c"
