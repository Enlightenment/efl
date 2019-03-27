#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_READER_FD_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"

#define MY_CLASS EFL_IO_READER_FD_CLASS

typedef struct _Efl_Io_Reader_Fd_Data
{
   int fd;
   Eina_Bool can_read;
   Eina_Bool eos;
} Efl_Io_Reader_Fd_Data;

EOLIAN static void
_efl_io_reader_fd_reader_fd_set(Eo *o EINA_UNUSED, Efl_Io_Reader_Fd_Data *pd, int fd)
{
   pd->fd = fd;
}

EOLIAN static int
_efl_io_reader_fd_reader_fd_get(const Eo *o EINA_UNUSED, Efl_Io_Reader_Fd_Data *pd)
{
   return pd->fd;
}

EOLIAN static Eina_Error
_efl_io_reader_fd_efl_io_reader_read(Eo *o, Efl_Io_Reader_Fd_Data *pd EINA_UNUSED, Eina_Rw_Slice *rw_slice)
{
   int fd = efl_io_reader_fd_get(o);
   ssize_t r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(rw_slice, EINVAL);
   if (fd < 0) goto error;
   do
     {
        r = read(fd, rw_slice->mem, rw_slice->len);
        if (r < 0)
          {
             if (errno == EINTR) continue;

             rw_slice->len = 0;
             rw_slice->mem = NULL;
             efl_io_reader_can_read_set(o, EINA_FALSE);
             return errno;
          }
     }
   while (r < 0);

   rw_slice->len = r;
   if (r == 0)
     {
        efl_io_reader_can_read_set(o, EINA_FALSE);
        efl_io_reader_eos_set(o, EINA_TRUE);
     }
   return 0;

 error:
   rw_slice->len = 0;
   rw_slice->mem = NULL;
   efl_io_reader_can_read_set(o, EINA_FALSE);
   return EINVAL;
}

EOLIAN static Eina_Bool
_efl_io_reader_fd_efl_io_reader_can_read_get(const Eo *o EINA_UNUSED, Efl_Io_Reader_Fd_Data *pd)
{
   return pd->can_read;
}

EOLIAN static void
_efl_io_reader_fd_efl_io_reader_can_read_set(Eo *o, Efl_Io_Reader_Fd_Data *pd, Eina_Bool can_read)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_reader_fd_get(o) < 0 && can_read);
   if (pd->can_read == can_read) return;
   pd->can_read = can_read;
   efl_event_callback_call(o, EFL_IO_READER_EVENT_CAN_READ_CHANGED, &can_read);
}

EOLIAN static Eina_Bool
_efl_io_reader_fd_efl_io_reader_eos_get(const Eo *o EINA_UNUSED, Efl_Io_Reader_Fd_Data *pd)
{
   return pd->eos;
}

EOLIAN static void
_efl_io_reader_fd_efl_io_reader_eos_set(Eo *o, Efl_Io_Reader_Fd_Data *pd, Eina_Bool is_eos)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_io_reader_fd_get(o) < 0 && !is_eos);
   if (pd->eos == is_eos) return;
   pd->eos = is_eos;
   if (is_eos)
     efl_event_callback_call(o, EFL_IO_READER_EVENT_EOS, NULL);
}

#include "efl_io_reader_fd.eo.c"
