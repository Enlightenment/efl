#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_READER_FD_PROTECTED 1
#define EFL_IO_WRITER_FD_PROTECTED 1
#define EFL_IO_CLOSER_FD_PROTECTED 1
#define EFL_IO_SIZER_FD_PROTECTED 1
#define EFL_IO_POSITIONER_FD_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef O_CLOEXEC
/* If a platform doesn't define O_CLOEXEC, then use 0 as we'll "| 0"
 * and "& ~0", which have no effect.
 *
 * This should be the case on _WIN32.
 */
#define O_CLOEXEC (0)
#endif

#define MY_CLASS EFL_IO_FILE_CLASS

typedef struct _Efl_Io_File_Data
{
   uint32_t flags;
   uint32_t mode;
   uint64_t last_position;
   // TODO: monitor reader.can_read,changed/writer.can_write,changed events in order to dynamically connect to Loop_Fd events.
} Efl_Io_File_Data;

static void
_efl_io_file_state_update(Eo *o, Efl_Io_File_Data *pd)
{
   uint64_t pos = efl_io_positioner_position_get(o);
   uint64_t size = efl_io_sizer_size_get(o);
   uint32_t flags = pd->flags & O_ACCMODE;

   if ((flags == O_RDWR) || (flags == O_RDONLY))
     {
        efl_io_reader_can_read_set(o, pos < size);
        efl_io_reader_eos_set(o, pos >= size);
     }

   if ((flags == O_RDWR) || (flags == O_WRONLY))
     efl_io_writer_can_write_set(o, EINA_TRUE);

   if (pd->last_position != pos)
     {
        pd->last_position = pos;
        efl_event_callback_call(o, EFL_IO_POSITIONER_EVENT_POSITION_CHANGED, NULL);
     }
}

EOLIAN static void
_efl_io_file_efl_loop_fd_fd_file_set(Eo *o, Efl_Io_File_Data *pd, int fd)
{
   efl_loop_fd_file_set(efl_super(o, MY_CLASS), fd);
   efl_io_positioner_fd_set(o, fd);
   efl_io_sizer_fd_set(o, fd);
   efl_io_reader_fd_set(o, fd);
   efl_io_writer_fd_set(o, fd);
   efl_io_closer_fd_set(o, fd);
   if (fd >= 0) _efl_io_file_state_update(o, pd);
}

EOLIAN static void
_efl_io_file_flags_set(Eo *o, Efl_Io_File_Data *pd, uint32_t flags)
{
   Eina_Bool close_on_exec;

   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));

   pd->flags = flags;

   close_on_exec = !!(flags & O_CLOEXEC);
   if (close_on_exec != efl_io_closer_close_on_exec_get(o))
     efl_io_closer_close_on_exec_set(o, close_on_exec);
}

EOLIAN static uint32_t
_efl_io_file_flags_get(const Eo *o EINA_UNUSED, Efl_Io_File_Data *pd)
{
   return pd->flags; // TODO: query from fd?
}

EOLIAN static void
_efl_io_file_mode_set(Eo *o, Efl_Io_File_Data *pd, uint32_t mode)
{
   EINA_SAFETY_ON_TRUE_RETURN(efl_finalized_get(o));
   pd->mode = mode;
}

EOLIAN static uint32_t
_efl_io_file_mode_get(const Eo *o EINA_UNUSED, Efl_Io_File_Data *pd)
{
   return pd->mode;
}

EOLIAN static Eo *
_efl_io_file_efl_object_constructor(Eo *o, Efl_Io_File_Data *pd)
{
   pd->flags = O_RDONLY | O_CLOEXEC;

   o = efl_constructor(efl_super(o, MY_CLASS));

   efl_io_closer_close_on_exec_set(o, EINA_TRUE);
   efl_io_closer_close_on_invalidate_set(o, EINA_TRUE);
   efl_io_positioner_fd_set(o, -1);
   efl_io_sizer_fd_set(o, -1);
   efl_io_reader_fd_set(o, -1);
   efl_io_writer_fd_set(o, -1);
   efl_io_closer_fd_set(o, -1);

   return o;
}

EOLIAN static void
_efl_io_file_efl_object_destructor(Eo *o, Efl_Io_File_Data *pd EINA_UNUSED)
{
   if (efl_io_closer_close_on_invalidate_get(o) &&
       (!efl_io_closer_closed_get(o)))
     {
        efl_event_freeze(o);
        efl_io_closer_close(o);
        efl_event_thaw(o);
     }

   efl_destructor(efl_super(o, MY_CLASS));
}

EOLIAN static Efl_Object *
_efl_io_file_efl_object_finalize(Eo *o, Efl_Io_File_Data *pd)
{
   int fd = efl_loop_fd_file_get(o);
   if (fd < 0)
     {
        const char *path = efl_file_get(o);
        EINA_SAFETY_ON_NULL_RETURN_VAL(path, NULL);

        if (pd->mode)
          fd = open(path, pd->flags, pd->mode);
        else
          fd = open(path, pd->flags);

        if (fd < 0)
          {
             eina_error_set(errno);
             ERR("Could not open file '%s': %s", path, strerror(errno));
             return NULL;
          }

        efl_loop_fd_file_set(o, fd);
     }

   return efl_finalize(efl_super(o, MY_CLASS));
}

EOLIAN static Eina_Error
_efl_io_file_efl_io_reader_read(Eo *o, Efl_Io_File_Data *pd, Eina_Rw_Slice *rw_slice)
{
   Eina_Error err = efl_io_reader_read(efl_super(o, MY_CLASS), rw_slice);
   if (err) return err;
   _efl_io_file_state_update(o, pd);
   return 0;
}

EOLIAN static Eina_Error
_efl_io_file_efl_io_writer_write(Eo *o, Efl_Io_File_Data *pd, Eina_Slice *slice, Eina_Slice *remaining)
{
   Eina_Error err = efl_io_writer_write(efl_super(o, MY_CLASS), slice, remaining);
   if (err) return err;
   _efl_io_file_state_update(o, pd);
   return 0;
}

EOLIAN static Eina_Error
_efl_io_file_efl_io_closer_close(Eo *o, Efl_Io_File_Data *pd EINA_UNUSED)
{
   Eina_Error ret;
   efl_io_reader_can_read_set(o, EINA_FALSE);
   efl_io_reader_eos_set(o, EINA_TRUE);
   efl_io_writer_can_write_set(o, EINA_FALSE);

   ret = efl_io_closer_close(efl_super(o, MY_CLASS));

   efl_loop_fd_file_set(o, -1);

   return ret;
}

EOLIAN static Eina_Error
_efl_io_file_efl_io_sizer_resize(Eo *o, Efl_Io_File_Data *pd, uint64_t size)
{
   Eina_Error err = efl_io_sizer_resize(efl_super(o, MY_CLASS), size);
   if (err) return err;
   _efl_io_file_state_update(o, pd);
   return 0;
}

EOLIAN static Eina_Error
_efl_io_file_efl_io_positioner_seek(Eo *o, Efl_Io_File_Data *pd, int64_t offset, Efl_Io_Positioner_Whence whence)
{
   Eina_Error err = efl_io_positioner_seek(efl_super(o, MY_CLASS), offset, whence);
   if (err) return err;
   _efl_io_file_state_update(o, pd);
   return 0;
}

EOLIAN static Eina_Bool
_efl_io_file_efl_io_closer_close_on_exec_set(Eo *o, Efl_Io_File_Data *pd, Eina_Bool close_on_exec)
{
   if (close_on_exec)
     pd->flags |= O_CLOEXEC;
   else
     pd->flags &= (~O_CLOEXEC);

   return efl_io_closer_close_on_exec_set(efl_super(o, MY_CLASS), close_on_exec);
}

#include "efl_io_file.eo.c"
