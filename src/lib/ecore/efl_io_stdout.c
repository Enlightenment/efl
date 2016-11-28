#define EFL_IO_WRITER_FD_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"

#define MY_CLASS EFL_IO_STDOUT_CLASS

static void
_efl_io_stdout_event_write(void *data EINA_UNUSED, const Efl_Event *event)
{
   efl_io_writer_can_write_set(event->object, EINA_TRUE);
}

static void
_efl_io_stdout_event_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   efl_io_writer_can_write_set(event->object, EINA_FALSE);
}

EOLIAN static void
_efl_io_stdout_efl_loop_fd_fd_set(Eo *o, void *pd EINA_UNUSED, int fd)
{
   efl_loop_fd_file_set(efl_super(o, MY_CLASS), fd);
   efl_io_writer_fd_set(o, fd);
}

EOLIAN static Efl_Object *
_efl_io_stdout_efl_object_finalize(Eo *o, void *pd EINA_UNUSED)
{
   int fd = efl_loop_fd_get(o);
   if (fd < 0) efl_loop_fd_set(o, STDOUT_FILENO);

   o = efl_finalize(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   efl_event_callback_add(o, EFL_LOOP_FD_EVENT_WRITE, _efl_io_stdout_event_write, NULL);
   efl_event_callback_add(o, EFL_LOOP_FD_EVENT_ERROR, _efl_io_stdout_event_error, NULL);
   return o;
}

EOLIAN static Eina_Error
_efl_io_stdout_efl_io_writer_write(Eo *o, void *pd EINA_UNUSED, Eina_Slice *ro_slice, Eina_Slice *remaining)
{
   Eina_Error ret;

   ret = efl_io_writer_write(efl_super(o, MY_CLASS), ro_slice, remaining);
   if (ro_slice && ro_slice->len > 0)
     efl_io_writer_can_write_set(o, EINA_FALSE); /* wait Efl.Loop.Fd "write" */

   return ret;
}

EOLIAN static void
_efl_io_stdout_efl_io_writer_can_write_set(Eo *o, void *pd EINA_UNUSED, Eina_Bool value)
{
   Eina_Bool old = efl_io_writer_can_write_get(o);
   if (old == value) return;

   efl_io_writer_can_write_set(efl_super(o, MY_CLASS), value);

   if (value)
     {
        /* stop monitoring the FD, we need to wait the user to write and clear the kernel flag */
        efl_event_callback_del(o, EFL_LOOP_FD_EVENT_WRITE, _efl_io_stdout_event_write, NULL);
     }
   else
     {
        /* kernel flag is clear, resume monitoring the FD */
        efl_event_callback_add(o, EFL_LOOP_FD_EVENT_WRITE, _efl_io_stdout_event_write, NULL);
     }
}

#include "efl_io_stdout.eo.c"
