#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_READER_FD_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"

#define MY_CLASS EFL_IO_STDIN_CLASS

static void
_efl_io_stdin_event_read(void *data EINA_UNUSED, const Efl_Event *event)
{
   efl_io_reader_can_read_set(event->object, EINA_TRUE);
   efl_io_reader_eos_set(event->object, EINA_FALSE);
}

static void
_efl_io_stdin_event_error(void *data EINA_UNUSED, const Efl_Event *event)
{
   efl_io_reader_can_read_set(event->object, EINA_FALSE);
   efl_io_reader_eos_set(event->object, EINA_TRUE);
}

EOLIAN static void
_efl_io_stdin_efl_loop_fd_fd_set(Eo *o, void *pd EINA_UNUSED, int fd)
{
   efl_loop_fd_file_set(efl_super(o, MY_CLASS), fd);
   efl_io_reader_fd_set(o, fd);
}

EOLIAN static Efl_Object *
_efl_io_stdin_efl_object_finalize(Eo *o, void *pd EINA_UNUSED)
{
   int fd = efl_loop_fd_get(o);
   if (fd < 0) efl_loop_fd_set(o, STDIN_FILENO);

   o = efl_finalize(efl_super(o, MY_CLASS));
   if (!o) return NULL;

   efl_event_callback_add(o, EFL_LOOP_FD_EVENT_READ, _efl_io_stdin_event_read, NULL);
   efl_event_callback_add(o, EFL_LOOP_FD_EVENT_ERROR, _efl_io_stdin_event_error, NULL);
   return o;
}

EOLIAN static Eina_Error
_efl_io_stdin_efl_io_reader_read(Eo *o, void *pd EINA_UNUSED, Eina_Rw_Slice *rw_slice)
{
   Eina_Error ret;

   ret = efl_io_reader_read(efl_super(o, MY_CLASS), rw_slice);
   if (rw_slice && rw_slice->len > 0)
     efl_io_reader_can_read_set(o, EINA_FALSE); /* wait Efl.Loop.Fd "read" */

   return ret;
}

EOLIAN static void
_efl_io_stdin_efl_io_reader_can_read_set(Eo *o, void *pd EINA_UNUSED, Eina_Bool value)
{
   Eina_Bool old = efl_io_reader_can_read_get(o);
   if (old == value) return;

   efl_io_reader_can_read_set(efl_super(o, MY_CLASS), value);

   if (value)
     {
        /* stop monitoring the FD, we need to wait the user to read and clear the kernel flag */
        efl_event_callback_del(o, EFL_LOOP_FD_EVENT_READ, _efl_io_stdin_event_read, NULL);
     }
   else
     {
        /* kernel flag is clear, resume monitoring the FD */
        efl_event_callback_add(o, EFL_LOOP_FD_EVENT_READ, _efl_io_stdin_event_read, NULL);
     }
}

#include "efl_io_stdin.eo.c"
