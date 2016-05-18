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
        eo_event_callback_call(obj, EFL_LOOP_FD_EVENT_READ, NULL);
     }
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     {
        eo_event_callback_call(obj, EFL_LOOP_FD_EVENT_WRITE, NULL);
     }
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_ERROR))
     {
        eo_event_callback_call(obj, EFL_LOOP_FD_EVENT_ERROR, NULL);
     }

   return ECORE_CALLBACK_RENEW;
}

static void
_efl_loop_fd_reset(Eo *obj, Efl_Loop_Fd_Data *pd)
{
   int flags = 0;

   if (pd->handler) ecore_main_fd_handler_del(pd->handler);
   pd->handler = NULL;
   if (pd->fd < 0) return ;
   flags |= pd->references.read > 0 ? ECORE_FD_READ : 0;
   flags |= pd->references.write > 0 ? ECORE_FD_WRITE : 0;
   flags |= pd->references.error > 0 ? ECORE_FD_ERROR : 0;
   if (flags == 0) return ;

   if (pd->file)
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
_efl_loop_fd_fd_get(Eo *obj EINA_UNUSED, Efl_Loop_Fd_Data *pd)
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
_efl_loop_fd_fd_file_get(Eo *obj EINA_UNUSED, Efl_Loop_Fd_Data *pd)
{
   return pd->file ? pd->fd : -1;
}

static Eina_Bool
_check_fd_event_catcher_add(void *data, const Eo_Event *event)
{
   const Eo_Callback_Array_Item *array = event->info;
   Efl_Loop_Fd_Data *fd = data;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_FD_EVENT_READ)
          {
             if (fd->references.read++ > 0) continue;
             _efl_loop_fd_reset(event->object, fd);
          }
        else if (array[i].desc == EFL_LOOP_FD_EVENT_WRITE)
          {
             if (fd->references.write++ > 0) continue;
             _efl_loop_fd_reset(event->object, fd);
          }
        if (array[i].desc == EFL_LOOP_FD_EVENT_ERROR)
          {
             if (fd->references.error++ > 0) continue;
             _efl_loop_fd_reset(event->object, fd);
          }
     }

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_check_fd_event_catcher_del(void *data, const Eo_Event *event)
{
   const Eo_Callback_Array_Item *array = event->info;
   Efl_Loop_Fd_Data *fd = data;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_FD_EVENT_READ)
          {
             if (fd->references.read++ > 0) continue;
             _efl_loop_fd_reset(event->object, fd);
          }
        else if (array[i].desc == EFL_LOOP_FD_EVENT_WRITE)
          {
             if (fd->references.write++ > 0) continue;
             _efl_loop_fd_reset(event->object, fd);
          }
        if (array[i].desc == EFL_LOOP_FD_EVENT_ERROR)
          {
             if (fd->references.error++ > 0) continue;
             _efl_loop_fd_reset(event->object, fd);
          }
     }

   return EO_CALLBACK_CONTINUE;
}

EO_CALLBACKS_ARRAY_DEFINE(fd_watch,
                          { EO_EVENT_CALLBACK_ADD, _check_fd_event_catcher_add },
                          { EO_EVENT_CALLBACK_DEL, _check_fd_event_catcher_del });

static Eo_Base *
_efl_loop_fd_eo_base_constructor(Eo *obj, Efl_Loop_Fd_Data *pd)
{
   eo_constructor(eo_super(obj, MY_CLASS));

   eo_event_callback_array_add(obj, fd_watch(), pd);

   pd->fd = -1;

   return obj;
}

static void
_efl_loop_fd_eo_base_destructor(Eo *obj, Efl_Loop_Fd_Data *pd)
{
   eo_destructor(eo_super(obj, MY_CLASS));

   ecore_main_fd_handler_del(pd->handler);
}

#include "efl_loop_fd.eo.c"
