#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1

#include <Ecore.h>

#include "ecore_private.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MY_CLASS EFL_APPTHREAD_CLASS

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

EOLIAN static Efl_Object *
_efl_appthread_efl_object_constructor(Eo *obj, Efl_Appthread_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->fd.in = -1;
   pd->fd.out = -1;
   pd->fd.can_write = EINA_TRUE;
   pd->ctrl.in = -1;
   pd->ctrl.out = -1;
   return obj;
}

EOLIAN static void
_efl_appthread_efl_object_destructor(Eo *obj, Efl_Appthread_Data *pd)
{
   if (pd->fd.in >= 0)
     {
//        efl_del(pd->fd.in_handler);
//        efl_del(pd->fd.out_handler);
//        efl_del(pd->ctrl.in_handler);
//        efl_del(pd->ctrl.out_handler);
        close(pd->fd.in);
        close(pd->fd.out);
        close(pd->ctrl.in);
        close(pd->ctrl.out);
        pd->fd.in_handler = NULL;
        pd->fd.out_handler = NULL;
        pd->ctrl.in_handler = NULL;
        pd->ctrl.out_handler = NULL;
        pd->fd.in = -1;
        pd->fd.out = -1;
        pd->ctrl.in = -1;
        pd->ctrl.out = -1;
     }
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Error
_efl_appthread_efl_io_closer_close(Eo *obj, Efl_Appthread_Data *pd)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(obj), EBADF);
   efl_io_writer_can_write_set(obj, EINA_FALSE);
   efl_io_reader_can_read_set(obj, EINA_FALSE);
   efl_io_reader_eos_set(obj, EINA_TRUE);
   if (pd->fd.in >= 0) close(pd->fd.in);
   if (pd->fd.out >= 0) close(pd->fd.out);
   if (pd->fd.in_handler) efl_del(pd->fd.in_handler);
   if (pd->fd.out_handler) efl_del(pd->fd.out_handler);
   pd->fd.in = -1;
   pd->fd.out = -1;
   pd->fd.in_handler = NULL;
   pd->fd.out_handler = NULL;
   return 0;
}

EOLIAN static Eina_Bool
_efl_appthread_efl_io_closer_closed_get(const Eo *obj EINA_UNUSED, Efl_Appthread_Data *pd)
{
   if ((pd->fd.in == -1) && (pd->fd.out == -1)) return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static Eina_Error
_efl_appthread_efl_io_reader_read(Eo *obj, Efl_Appthread_Data *pd, Eina_Rw_Slice *rw_slice)
{
   ssize_t r;

   errno = 0;
   if (pd->fd.out == -1) goto err;

   do
     {
        errno = 0;
        r = read(pd->fd.out, rw_slice->mem, rw_slice->len);
        if (r == -1)
          {
             if (errno == EINTR) continue;
             goto err;
          }
     }
   while (r == -1);

   rw_slice->len = r;
   if (r == 0)
     {
        efl_io_reader_can_read_set(obj, EINA_FALSE);
        efl_io_reader_eos_set(obj, EINA_TRUE);
        close(pd->fd.out);
        pd->fd.out = -1;
        efl_del(pd->fd.out_handler);
        pd->fd.out_handler = NULL;
        return EPIPE;
     }
   return 0;
err:
   if ((pd->fd.out != -1) && (errno != EAGAIN))
     {
        close(pd->fd.out);
        pd->fd.out = -1;
        efl_del(pd->fd.out_handler);
        pd->fd.out_handler = NULL;
     }
   rw_slice->len = 0;
   rw_slice->mem = NULL;
   efl_io_reader_can_read_set(obj, EINA_FALSE);
   return EINVAL;
}

EOLIAN static void
_efl_appthread_efl_io_reader_can_read_set(Eo *obj, Efl_Appthread_Data *pd, Eina_Bool can_read)
{
   Eina_Bool old = efl_io_reader_can_read_get(obj);
   if (old == can_read) return;
   pd->fd.can_read = can_read;
   if (can_read)
     efl_loop_handler_active_set(pd->fd.in_handler, 0);
   else
     efl_loop_handler_active_set(pd->fd.in_handler,
                                 EFL_LOOP_HANDLER_FLAGS_READ);
   efl_event_callback_call(obj, EFL_IO_READER_EVENT_CAN_READ_CHANGED, &can_read);
}

EOLIAN static Eina_Bool
_efl_appthread_efl_io_reader_can_read_get(const Eo *obj EINA_UNUSED, Efl_Appthread_Data *pd)
{
   return pd->fd.can_read;
}

EOLIAN static void
_efl_appthread_efl_io_reader_eos_set(Eo *obj, Efl_Appthread_Data *pd, Eina_Bool is_eos)
{
   Eina_Bool old = efl_io_reader_eos_get(obj);
   if (old == is_eos) return;

   pd->fd.eos_read = is_eos;
   if (!is_eos) return;
   if (pd->fd.out_handler)
     efl_loop_handler_active_set(pd->fd.out_handler, 0);
   efl_event_callback_call(obj, EFL_IO_READER_EVENT_EOS, NULL);
}

EOLIAN static Eina_Bool
_efl_appthread_efl_io_reader_eos_get(const Eo *obj EINA_UNUSED, Efl_Appthread_Data *pd)
{
   return pd->fd.eos_read;
}

EOLIAN static Eina_Error
_efl_appthread_efl_io_writer_write(Eo *obj, Efl_Appthread_Data *pd, Eina_Slice *slice, Eina_Slice *remaining)
{
   ssize_t r;

   errno = 0;
   if (pd->fd.in == -1) goto err;

   do
     {
        errno = 0;
        r = write(pd->fd.in, slice->mem, slice->len);
        if (r == -1)
          {
             if (errno == EINTR) continue;
             goto err;
          }
     }
   while (r == -1);

   if (remaining)
     {
        remaining->len = slice->len - r;
        remaining->bytes = slice->bytes + r;
     }
   slice->len = r;

   if ((slice) && (slice->len > 0))
     efl_io_writer_can_write_set(obj, EINA_FALSE);
   if (r == 0)
     {
        close(pd->fd.in);
        pd->fd.in = -1;
        efl_del(pd->fd.in_handler);
        pd->fd.in_handler = NULL;
        return EPIPE;
     }
   return 0;
err:
   if ((pd->fd.in != -1) && (errno != EAGAIN))
     {
        close(pd->fd.in);
        pd->fd.in = -1;
        efl_del(pd->fd.in_handler);
        pd->fd.in_handler = NULL;
     }
   if (remaining) *remaining = *slice;
   slice->len = 0;
   slice->mem = NULL;
   efl_io_writer_can_write_set(obj, EINA_FALSE);
   return EINVAL;
}

EOLIAN static void
_efl_appthread_efl_io_writer_can_write_set(Eo *obj, Efl_Appthread_Data *pd, Eina_Bool can_write)
{
   Eina_Bool old = efl_io_writer_can_write_get(obj);
   if (old == can_write) return;
   pd->fd.can_write = can_write;
   if (can_write)
     efl_loop_handler_active_set(pd->fd.in_handler, 0);
   else
     efl_loop_handler_active_set(pd->fd.in_handler,
                                 EFL_LOOP_HANDLER_FLAGS_WRITE);
   efl_event_callback_call(obj, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, &can_write);
}

EOLIAN static Eina_Bool
_efl_appthread_efl_io_writer_can_write_get(const Eo *obj EINA_UNUSED, Efl_Appthread_Data *pd)
{
   return pd->fd.can_write;
}

void _appthread_threadio_call(Eo *obj, Efl_Appthread_Data *pd, void *func_data, EFlThreadIOCall func, Eina_Free_Cb func_free_cb);

EOLIAN static void
_efl_appthread_efl_threadio_call(Eo *obj, Efl_Appthread_Data *pd, void *func_data, EFlThreadIOCall func, Eina_Free_Cb func_free_cb)
{
   _appthread_threadio_call(obj, pd, func_data, func, func_free_cb);
}

void *_appthread_threadio_call_sync(Eo *obj, Efl_Appthread_Data *pd, void *func_data, EFlThreadIOCallSync func, Eina_Free_Cb func_free_cb);

EOLIAN static void *
_efl_appthread_efl_threadio_call_sync(Eo *obj, Efl_Appthread_Data *pd, void *func_data, EFlThreadIOCallSync func, Eina_Free_Cb func_free_cb)
{
   return _appthread_threadio_call_sync(obj, pd, func_data, func, func_free_cb);
}

//////////////////////////////////////////////////////////////////////////

#include "efl_appthread.eo.c"
