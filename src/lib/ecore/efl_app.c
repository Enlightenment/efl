#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1

#include <Ecore.h>

#include "ecore_private.h"

#ifndef _WIN32
# include <sys/resource.h>
#endif

#define MY_CLASS EFL_APP_CLASS

typedef struct _Efl_App_Data Efl_App_Data;

struct _Efl_App_Data
{
   struct {
      int in, out;
      Eo *in_handler, *out_handler;
      Eina_Bool can_read : 1;
      Eina_Bool eos_read : 1;
      Eina_Bool can_write : 1;
   } fd;
   int read_listeners;
};

Efl_Version _app_efl_version = { 0, 0, 0, 0, NULL, NULL };

//////////////////////////////////////////////////////////////////////////

EAPI Eo *
efl_app_get(void)
{
   return efl_main_loop_get();
}

//////////////////////////////////////////////////////////////////////////

static void
_parent_read_listeners_modify(Efl_App_Data *pd, int mod)
{
   pd->read_listeners += mod;

   if (pd->fd.out_handler)
     {
        if ((pd->read_listeners == 0) && (mod < 0))
          efl_loop_handler_active_set
            (pd->fd.out_handler, EFL_LOOP_HANDLER_FLAGS_NONE);
        else if ((pd->read_listeners == 1) && (mod > 0))
          efl_loop_handler_active_set
            (pd->fd.out_handler, EFL_LOOP_HANDLER_FLAGS_READ);
     }
}

static void
_cb_event_callback_add(void *data, const Efl_Event *event)
{
   Efl_App_Data *pd = data;
   const Efl_Callback_Array_Item_Full *array = event->info;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_IO_READER_EVENT_CAN_READ_CHANGED)
          _parent_read_listeners_modify(pd, 1);
     }
}

static void
_cb_event_callback_del(void *data, const Efl_Event *event)
{
   Efl_App_Data *pd = data;
   const Efl_Callback_Array_Item_Full *array = event->info;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_IO_READER_EVENT_CAN_READ_CHANGED)
          _parent_read_listeners_modify(pd, -1);
     }
}

static void
_cb_out(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   efl_io_reader_can_read_set(obj, EINA_TRUE);
}

static void
_cb_in(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   efl_io_writer_can_write_set(obj, EINA_TRUE);
}


EFL_CALLBACKS_ARRAY_DEFINE(_event_callback_watch,
                           { EFL_EVENT_CALLBACK_ADD, _cb_event_callback_add },
                           { EFL_EVENT_CALLBACK_DEL, _cb_event_callback_del });

//////////////////////////////////////////////////////////////////////////

EOLIAN static Efl_Loop *
_efl_app_loop_main_get(const Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   if (_mainloop_singleton) return _mainloop_singleton;
   _mainloop_singleton = efl_add_ref(EFL_APP_CLASS, NULL);
   _mainloop_singleton_data = efl_data_scope_get(_mainloop_singleton, EFL_LOOP_CLASS);
   return _mainloop_singleton;
}

EOLIAN static const Efl_Version *
_efl_app_build_efl_version_get(const Eo *obj EINA_UNUSED, Efl_App_Data *pd EINA_UNUSED)
{
   return &_app_efl_version;
}

EOLIAN static const Efl_Version *
_efl_app_efl_version_get(const Eo *obj EINA_UNUSED, Efl_App_Data *pd EINA_UNUSED)
{
   /* vanilla EFL: flavor = NULL */
   static const Efl_Version version = {
      .major = VMAJ,
      .minor = VMIN,
      .micro = VMIC,
      .revision = VREV,
      .build_id = EFL_BUILD_ID,
      .flavor = NULL
   };
   return &version;
}

EOLIAN static Efl_Object *
_efl_app_efl_object_constructor(Eo *obj, Efl_App_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_event_callback_array_add(obj, _event_callback_watch(), pd);
   pd->fd.in = 1;
   pd->fd.out = 0;
   pd->fd.can_write = EINA_TRUE;
   pd->fd.in_handler =
     efl_add(EFL_LOOP_HANDLER_CLASS, obj,
             efl_loop_handler_fd_set(efl_added, pd->fd.in),
             efl_event_callback_add(efl_added, EFL_LOOP_HANDLER_EVENT_WRITE, _cb_in, obj));
   pd->fd.out_handler =
     efl_add(EFL_LOOP_HANDLER_CLASS, obj,
             efl_loop_handler_fd_set(efl_added, pd->fd.out),
             efl_event_callback_add(efl_added, EFL_LOOP_HANDLER_EVENT_READ, _cb_out, obj));
   return obj;
}

EOLIAN static void
_efl_app_efl_object_invalidate(Eo *obj, Efl_App_Data *pd)
{
   pd->fd.in_handler = NULL;
   pd->fd.out_handler = NULL;
   pd->fd.in = -1;
   pd->fd.out = -1;

   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Error
_efl_app_efl_io_closer_close(Eo *obj, Efl_App_Data *pd)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(obj), EBADF);
   efl_io_writer_can_write_set(obj, EINA_FALSE);
   efl_io_reader_can_read_set(obj, EINA_FALSE);
   efl_io_reader_eos_set(obj, EINA_TRUE);
   if (pd->fd.in_handler) efl_del(pd->fd.in_handler);
   if (pd->fd.out_handler) efl_del(pd->fd.out_handler);
   pd->fd.in = -1;
   pd->fd.out = -1;
   pd->fd.in_handler = NULL;
   pd->fd.out_handler = NULL;
   return 0;
}

EOLIAN static Eina_Bool
_efl_app_efl_io_closer_closed_get(const Eo *obj EINA_UNUSED, Efl_App_Data *pd)
{
   if ((pd->fd.in == -1) && (pd->fd.out == -1)) return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static Eina_Error
_efl_app_efl_io_reader_read(Eo *obj, Efl_App_Data *pd, Eina_Rw_Slice *rw_slice)
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
_efl_app_efl_io_reader_can_read_set(Eo *obj, Efl_App_Data *pd, Eina_Bool can_read)
{
   Eina_Bool old = efl_io_reader_can_read_get(obj);
   if (old == can_read) return;
   pd->fd.can_read = can_read;
   if (can_read)
     efl_loop_handler_active_set(pd->fd.in_handler, 0);
   else
     efl_loop_handler_active_set(pd->fd.in_handler,
                                 EFL_LOOP_HANDLER_FLAGS_READ);
   efl_event_callback_call(obj, EFL_IO_READER_EVENT_CAN_READ_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_app_efl_io_reader_can_read_get(const Eo *obj EINA_UNUSED, Efl_App_Data *pd)
{
   return pd->fd.can_read;
}

EOLIAN static void
_efl_app_efl_io_reader_eos_set(Eo *obj, Efl_App_Data *pd, Eina_Bool is_eos)
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
_efl_app_efl_io_reader_eos_get(const Eo *obj EINA_UNUSED, Efl_App_Data *pd)
{
   return pd->fd.eos_read;
}

EOLIAN static Eina_Error
_efl_app_efl_io_writer_write(Eo *obj, Efl_App_Data *pd, Eina_Slice *slice, Eina_Slice *remaining)
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
_efl_app_efl_io_writer_can_write_set(Eo *obj, Efl_App_Data *pd, Eina_Bool can_write)
{
   Eina_Bool old = efl_io_writer_can_write_get(obj);
   if (old == can_write) return;
   pd->fd.can_write = can_write;
   if (can_write)
     efl_loop_handler_active_set(pd->fd.in_handler, 0);
   else
     efl_loop_handler_active_set(pd->fd.in_handler,
                                 EFL_LOOP_HANDLER_FLAGS_WRITE);
   efl_event_callback_call(obj, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_app_efl_io_writer_can_write_get(const Eo *obj EINA_UNUSED, Efl_App_Data *pd)
{
   return pd->fd.can_write;
}

#ifdef _WIN32
#else
static const signed char primap[EFL_TASK_PRIORITY_ULTRA + 1] =
{
      10, // EFL_TASK_PRIORITY_NORMAL
      19, // EFL_TASK_PRIORITY_BACKGROUND
      15, // EFL_TASK_PRIORITY_LOW
      5, // EFL_TASK_PRIORITY_HIGH
      0  // EFL_TASK_PRIORITY_ULTRA
};
#endif

EOLIAN static void
_efl_app_efl_task_priority_set(Eo *obj, Efl_App_Data *pd EINA_UNUSED, Efl_Task_Priority priority)
{
   efl_task_priority_set(efl_super(obj, MY_CLASS), priority);
#ifdef _WIN32
#else
   // -20 (high) -> 19 (low)
   int p = 0;

   if ((priority >= EFL_TASK_PRIORITY_NORMAL) &&
       (priority <= EFL_TASK_PRIORITY_ULTRA))
     p = primap[priority];
   setpriority(PRIO_PROCESS, 0, p);
#endif
}

EOLIAN static Efl_Task_Priority
_efl_app_efl_task_priority_get(const Eo *obj, Efl_App_Data *pd EINA_UNUSED)
{
   Efl_Task_Priority pri = EFL_TASK_PRIORITY_NORMAL;
#ifdef _WIN32
#else
   int p, i, dist = 0x7fffffff, d;

   errno = 0;
   p = getpriority(PRIO_PROCESS, 0);
   if (errno != 0)
     return efl_task_priority_get(efl_super(obj, MY_CLASS));

   // find the closest matching priority in primap
   for (i = EFL_TASK_PRIORITY_NORMAL; i <= EFL_TASK_PRIORITY_ULTRA; i++)
     {
        d = primap[i] - p;
        if (d < 0) d = -d;
        if (d < dist)
          {
             pri = i;
             dist = d;
          }
     }

   Efl_Task_Data *td = efl_data_scope_get(obj, EFL_TASK_CLASS);
   if (td) td->priority = pri;
#endif
   return pri;
}

//////////////////////////////////////////////////////////////////////////

#include "efl_app.eo.c"
