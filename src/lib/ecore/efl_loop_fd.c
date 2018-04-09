#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

#define MY_CLASS EFL_LOOP_FD_CLASS

typedef struct _Efl_Loop_Fd_Data Efl_Loop_Fd_Data;
struct _Efl_Loop_Fd_Data
{
   Ecore_Fd_Handler *handler;

   struct {
      unsigned int read;
      unsigned int write;
      unsigned int error;
   } references;

   int fd;

   Eina_Bool file : 1;
};

static Eina_Bool
_efl_loop_fd_read_cb(void *data, Ecore_Fd_Handler *fd_handler)
{
   Eo *obj = data;

   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     {
        efl_event_callback_call(obj, EFL_LOOP_FD_EVENT_READ, NULL);
     }
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     {
        efl_event_callback_call(obj, EFL_LOOP_FD_EVENT_WRITE, NULL);
     }
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_ERROR))
     {
        efl_event_callback_call(obj, EFL_LOOP_FD_EVENT_ERROR, NULL);
     }

   return ECORE_CALLBACK_RENEW;
}

static void
_efl_loop_fd_reset(Eo *obj, Efl_Loop_Fd_Data *pd)
{
   int flags = 0;

   if (pd->fd < 0)
     {
        if (pd->handler)
          {
             ecore_main_fd_handler_del(pd->handler);
             pd->handler = NULL;
          }
        return;
     }
   flags |= pd->references.read > 0 ? ECORE_FD_READ : 0;
   flags |= pd->references.write > 0 ? ECORE_FD_WRITE : 0;
   flags |= pd->references.error > 0 ? ECORE_FD_ERROR : 0;
   if (flags == 0)
     {
        if (pd->handler)
          {
             ecore_main_fd_handler_del(pd->handler);
             pd->handler = NULL;
          }
        return;
     }

   if (pd->handler)
     ecore_main_fd_handler_active_set(pd->handler, flags);
   else if (pd->file)
     pd->handler = ecore_main_fd_handler_file_add(pd->fd, flags, _efl_loop_fd_read_cb, obj, NULL, NULL);
   else
     pd->handler = ecore_main_fd_handler_add(pd->fd, flags, _efl_loop_fd_read_cb, obj, NULL, NULL);
}

static void
_efl_loop_fd_fd_set(Eo *obj, Efl_Loop_Fd_Data *pd, int fd)
{
   pd->fd = fd;
   pd->file = EINA_FALSE;
   _efl_loop_fd_reset(obj, pd);
}

static int
_efl_loop_fd_fd_get(const Eo *obj EINA_UNUSED, Efl_Loop_Fd_Data *pd)
{
   return pd->file ? -1 : pd->fd;
}

static void
_efl_loop_fd_fd_file_set(Eo *obj, Efl_Loop_Fd_Data *pd, int fd)
{
   pd->fd = fd;
   pd->file = EINA_TRUE;
   _efl_loop_fd_reset(obj, pd);
}

static int
_efl_loop_fd_fd_file_get(const Eo *obj EINA_UNUSED, Efl_Loop_Fd_Data *pd)
{
   return pd->file ? pd->fd : -1;
}

static void
_check_fd_event_catcher_add(void *data, const Efl_Event *event)
{
   const Efl_Callback_Array_Item_Full *array = event->info;
   Efl_Loop_Fd_Data *fd = data;
   Eina_Bool need_reset = EINA_FALSE;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_FD_EVENT_READ)
          {
             if (fd->references.read++ > 0) continue;
             need_reset = EINA_TRUE;
          }
        else if (array[i].desc == EFL_LOOP_FD_EVENT_WRITE)
          {
             if (fd->references.write++ > 0) continue;
             need_reset = EINA_TRUE;
          }
        if (array[i].desc == EFL_LOOP_FD_EVENT_ERROR)
          {
             if (fd->references.error++ > 0) continue;
             need_reset = EINA_TRUE;
          }
     }

   if (need_reset)
     _efl_loop_fd_reset(event->object, fd);
}

static void
_check_fd_event_catcher_del(void *data, const Efl_Event *event)
{
   const Efl_Callback_Array_Item_Full *array = event->info;
   Efl_Loop_Fd_Data *fd = data;
   Eina_Bool need_reset = EINA_FALSE;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_FD_EVENT_READ)
          {
             if (fd->references.read-- > 1) continue;
             need_reset = EINA_TRUE;
          }
        else if (array[i].desc == EFL_LOOP_FD_EVENT_WRITE)
          {
             if (fd->references.write-- > 1) continue;
             need_reset = EINA_TRUE;
          }
        if (array[i].desc == EFL_LOOP_FD_EVENT_ERROR)
          {
             if (fd->references.error-- > 1) continue;
             need_reset = EINA_TRUE;
          }
     }

   if (need_reset)
     _efl_loop_fd_reset(event->object, fd);
}

EFL_CALLBACKS_ARRAY_DEFINE(fd_watch,
                          { EFL_EVENT_CALLBACK_ADD, _check_fd_event_catcher_add },
                          { EFL_EVENT_CALLBACK_DEL, _check_fd_event_catcher_del });

static Efl_Object *
_efl_loop_fd_efl_object_constructor(Eo *obj, Efl_Loop_Fd_Data *pd)
{
   efl_constructor(efl_super(obj, MY_CLASS));

   efl_event_callback_array_add(obj, fd_watch(), pd);

   pd->fd = -1;

   return obj;
}

static void
_efl_loop_fd_efl_object_parent_set(Eo *obj, Efl_Loop_Fd_Data *pd, Efl_Object *parent)
{
   if (pd->handler) ecore_main_fd_handler_del(pd->handler);
   pd->handler = NULL;

   efl_parent_set(efl_super(obj, MY_CLASS), parent);

   if (parent == NULL) return ;

   _efl_loop_fd_reset(obj, pd);
}

static void
_efl_loop_fd_efl_object_invalidate(Eo *obj, Efl_Loop_Fd_Data *pd)
{
   ecore_main_fd_handler_del(pd->handler);

   efl_invalidate(efl_super(obj, MY_CLASS));
}

#include "efl_loop_fd.eo.c"
