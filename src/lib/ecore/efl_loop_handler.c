#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

#define MY_CLASS EFL_LOOP_HANDLER_CLASS

//////////////////////////////////////////////////////////////////////////

typedef struct _Efl_Loop_Handler_Data Efl_Loop_Handler_Data;

struct _Efl_Loop_Handler_Data
{
   Eo *loop;
   Efl_Loop_Data *loop_data;
   Ecore_Fd_Handler *handler_fd;
   Ecore_Win32_Handler *handler_win32;

   void *win32;
   int fd;

   struct {
      unsigned short read;
      unsigned short write;
      unsigned short error;
      unsigned short buffer;
      unsigned short prepare;
   } references;

   Efl_Loop_Handler_Flags flags : 8;
   Eina_Bool file : 1;

   Eina_Bool constructed : 1;
   Eina_Bool finalized : 1;
};

//////////////////////////////////////////////////////////////////////////

static Eina_Bool _cb_handler_fd(void *data, Ecore_Fd_Handler *fd_handler);
static Eina_Bool _cb_handler_buffer(void *data, Ecore_Fd_Handler *fd_handler);
static Eina_Bool _cb_handler_win32(void *data, Ecore_Win32_Handler *win32_handler);
static void      _cb_handler_prepare(void *data, Ecore_Fd_Handler *fd_handler);

//////////////////////////////////////////////////////////////////////////

static void
_handler_clear(Efl_Loop_Handler_Data *pd)
{
   Eo *obj = pd->loop;
   Efl_Loop_Data *loop = pd->loop_data;

   if (pd->handler_fd)
     {
        _ecore_main_fd_handler_del(obj, loop, pd->handler_fd);
        pd->handler_fd = NULL;
     }
   else if (pd->handler_win32)
     {
        _ecore_main_win32_handler_del(obj, loop, pd->handler_win32);
        pd->handler_win32 = NULL;
     }
}

static Ecore_Fd_Handler_Flags
_handler_flags_get(Efl_Loop_Handler_Data *pd)
{
   return (((pd->flags & EFL_LOOP_HANDLER_FLAGS_READ) &&
            (pd->references.read > 0)) ? ECORE_FD_READ  : 0) |
          (((pd->flags & EFL_LOOP_HANDLER_FLAGS_WRITE) &&
            (pd->references.write > 0)) ? ECORE_FD_WRITE  : 0) |
          (((pd->flags & EFL_LOOP_HANDLER_FLAGS_ERROR) &&
            (pd->references.error > 0)) ? ECORE_FD_ERROR  : 0);
}

static void
_handler_active_update(Eo *obj, Efl_Loop_Handler_Data *pd)
{
   Ecore_Fd_Handler_Flags flags = _handler_flags_get(pd);

   ecore_main_fd_handler_active_set(pd->handler_fd, flags);
   if (pd->references.prepare)
     ecore_main_fd_handler_prepare_callback_set
       (pd->handler_fd, _cb_handler_prepare, obj);
   else
     ecore_main_fd_handler_prepare_callback_set
       (pd->handler_fd, NULL, NULL);
}

static void
_handler_reset(Eo *obj, Efl_Loop_Handler_Data *pd)
{
   if ((pd->fd < 0) && (!pd->win32))
     {
        _handler_clear(pd);
        return;
     }

   if ((!pd->constructed) || (!pd->finalized)) return;

   if (pd->fd >= 0)
     {
        Ecore_Fd_Cb buffer_func = NULL;
        void *buffer_data = NULL;

        if (pd->references.buffer > 0)
          {
             buffer_func = _cb_handler_buffer;
             buffer_data = obj;
          }

        if (pd->handler_fd)
          _handler_active_update(obj, pd);
        else
          {
             pd->handler_fd = _ecore_main_fd_handler_add
               (pd->loop, pd->loop_data, obj, pd->fd, _handler_flags_get(pd),
                _cb_handler_fd, obj, buffer_func, buffer_data,
                pd->file ? EINA_TRUE : EINA_FALSE);
             if (pd->handler_fd) _handler_active_update(obj, pd);
          }
     }
   else if (pd->win32)
     {
        pd->handler_win32 = _ecore_main_win32_handler_add
          (pd->loop, pd->loop_data, obj, pd->win32, _cb_handler_win32, obj);
     }
}

static Eina_Bool
_event_references_update(Efl_Loop_Handler_Data *pd, const Efl_Event *event, int increment)
{
   const Efl_Callback_Array_Item_Full *array = event->info;
   int i;
   Eina_Bool need_reset = EINA_FALSE;

   for (i = 0; array[i].desc != NULL; i++)
     {
#define REFERENCES_MAP(_desc, _refs) \
   if (array[i].desc == _desc) { \
      pd->references._refs += increment; \
      need_reset = EINA_TRUE; \
      continue; \
   }
        REFERENCES_MAP(EFL_LOOP_HANDLER_EVENT_READ,    read);
        REFERENCES_MAP(EFL_LOOP_HANDLER_EVENT_WRITE,   write);
        REFERENCES_MAP(EFL_LOOP_HANDLER_EVENT_ERROR,   error);
        REFERENCES_MAP(EFL_LOOP_HANDLER_EVENT_BUFFER,  buffer);
        REFERENCES_MAP(EFL_LOOP_HANDLER_EVENT_PREPARE, prepare);
     }
   return need_reset;
}

//////////////////////////////////////////////////////////////////////////

static Eina_Bool
_cb_handler_fd(void *data, Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   Eo *obj = data;

   efl_ref(obj);
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     efl_event_callback_call(obj, EFL_LOOP_HANDLER_EVENT_READ, NULL);
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     efl_event_callback_call(obj, EFL_LOOP_HANDLER_EVENT_WRITE, NULL);
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_ERROR))
     efl_event_callback_call(obj, EFL_LOOP_HANDLER_EVENT_ERROR, NULL);
   efl_unref(obj);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_cb_handler_buffer(void *data, Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   Eo *obj = data;

   efl_event_callback_call(obj, EFL_LOOP_HANDLER_EVENT_BUFFER, NULL);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_cb_handler_win32(void *data, Ecore_Win32_Handler *win32_handler EINA_UNUSED)
{
   Eo *obj = data;

   efl_event_callback_call(obj, EFL_LOOP_HANDLER_EVENT_READ, NULL);
   return ECORE_CALLBACK_RENEW;
}

static void
_cb_handler_prepare(void *data, Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   Eo *obj = data;

   efl_event_callback_call(obj, EFL_LOOP_HANDLER_EVENT_PREPARE, NULL);
}

static void
_cb_event_callback_add(void *data, const Efl_Event *event)
{
   Efl_Loop_Handler_Data *pd = data;
   if (_event_references_update(pd, event, 1))
     _handler_reset(event->object, pd);
}

static void
_cb_event_callback_del(void *data, const Efl_Event *event)
{
   Efl_Loop_Handler_Data *pd = data;
   if (_event_references_update(pd, event, -1))
     _handler_reset(event->object, pd);
}

//////////////////////////////////////////////////////////////////////////

EFL_CALLBACKS_ARRAY_DEFINE(_event_callback_watch,
                          { EFL_EVENT_CALLBACK_ADD, _cb_event_callback_add },
                          { EFL_EVENT_CALLBACK_DEL, _cb_event_callback_del });

static void
_efl_loop_handler_active_set(Eo *obj, Efl_Loop_Handler_Data *pd, Efl_Loop_Handler_Flags flags)
{
   pd->flags = flags;
   _handler_reset(obj, pd);
}

static Efl_Loop_Handler_Flags
_efl_loop_handler_active_get(const Eo *obj EINA_UNUSED, Efl_Loop_Handler_Data *pd)
{
   return pd->flags;
}

static void
_efl_loop_handler_fd_set(Eo *obj, Efl_Loop_Handler_Data *pd, int fd)
{
   pd->fd = fd;
   pd->file = EINA_FALSE;
   pd->win32 = NULL;
   _handler_reset(obj, pd);
}

static int
_efl_loop_handler_fd_get(const Eo *obj EINA_UNUSED, Efl_Loop_Handler_Data *pd)
{
   if (pd->win32) return -1;
   return pd->file ? -1 : pd->fd;
}

static void
_efl_loop_handler_fd_file_set(Eo *obj, Efl_Loop_Handler_Data *pd, int fd)
{
   pd->fd = fd;
   pd->file = EINA_TRUE;
   pd->win32 = NULL;
   _handler_reset(obj, pd);
}

static int
_efl_loop_handler_fd_file_get(const Eo *obj EINA_UNUSED, Efl_Loop_Handler_Data *pd)
{
   if (pd->win32) return -1;
   return pd->file ? pd->fd : -1;
}

static void
_efl_loop_handler_win32_set(Eo *obj, Efl_Loop_Handler_Data *pd, void *handle)
{
   pd->fd = -1;
   pd->file = EINA_FALSE;
   pd->win32 = handle;
   _handler_reset(obj, pd);
}

static void *
_efl_loop_handler_win32_get(const Eo *obj EINA_UNUSED, Efl_Loop_Handler_Data *pd)
{
   return pd->win32;
}

static void
_efl_loop_handler_efl_object_parent_set(Eo *obj, Efl_Loop_Handler_Data *pd, Efl_Object *parent)
{
   efl_parent_set(efl_super(obj, MY_CLASS), parent);

   if ((!pd->constructed) || (!pd->finalized)) return;

   _handler_clear(pd);

   if (pd->loop)
     {
        pd->loop_data->fd_handlers_obj = eina_list_remove
          (pd->loop_data->fd_handlers_obj, obj);
        pd->loop = NULL;
        pd->loop_data = NULL;
     }

   if (parent == NULL) return;

   pd->loop = efl_provider_find(obj, EFL_LOOP_CLASS);
   pd->loop_data = efl_data_scope_get(pd->loop, EFL_LOOP_CLASS);
   if (pd->loop_data)
     pd->loop_data->fd_handlers_obj =
       eina_list_append(pd->loop_data->fd_handlers_obj, obj);
   _handler_reset(obj, pd);
}

static Efl_Object *
_efl_loop_handler_efl_object_constructor(Eo *obj, Efl_Loop_Handler_Data *pd)
{
   efl_constructor(efl_super(obj, MY_CLASS));
   efl_event_callback_array_add(obj, _event_callback_watch(), pd);
   pd->constructed = EINA_TRUE;
   return obj;
}

static Efl_Object *
_efl_loop_handler_efl_object_finalize(Eo *obj, Efl_Loop_Handler_Data *pd)
{
   pd->loop = efl_provider_find(obj, EFL_LOOP_CLASS);
   pd->loop_data = efl_data_scope_get(pd->loop, EFL_LOOP_CLASS);
   if (pd->loop_data)
     pd->loop_data->fd_handlers_obj =
       eina_list_append(pd->loop_data->fd_handlers_obj, obj);
   pd->finalized = EINA_TRUE;
   _handler_reset(obj, pd);
   return efl_finalize(efl_super(obj, MY_CLASS));
}

static void
_efl_loop_handler_efl_object_destructor(Eo *obj, Efl_Loop_Handler_Data *pd)
{
   if (pd->loop_data)
     pd->loop_data->fd_handlers_obj =
       eina_list_remove(pd->loop_data->fd_handlers_obj, obj);
   _handler_clear(pd);
   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "efl_loop_handler.eo.c"
