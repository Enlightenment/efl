#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <Eo.h>

#include "Ecore.h"
#include "ecore_private.h"

struct _Ecore_Idler
{
   Ecore_Task_Cb func;
   void         *data;
};

static void *_ecore_idler_del(Ecore_Idler *idler);

static Eina_Bool
_ecore_idler_event_del(void *data, const Eo_Event *event EINA_UNUSED)
{
   _ecore_idler_del(data);

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_ecore_idler_process(void *data, const Eo_Event *event EINA_UNUSED)
{
   Ecore_Idler *idler = data;

   if (!_ecore_call_task_cb(idler->func, idler->data))
     _ecore_idler_del(idler);

   return EO_CALLBACK_CONTINUE;
}

EO_CALLBACKS_ARRAY_DEFINE(ecore_idler_callbacks,
                          { ECORE_MAINLOOP_EVENT_IDLE, _ecore_idler_process },
                          { EO_BASE_EVENT_DEL, _ecore_idler_event_del });

static void *
_ecore_idler_del(Ecore_Idler *idler)
{
   void *data;

   if (!idler) return NULL;

   eo_event_callback_array_del(_mainloop_singleton, ecore_idler_callbacks(), idler);

   data = idler->data;
   free(idler);
   return data;
}

EAPI Ecore_Idler *
ecore_idler_add(Ecore_Task_Cb func,
                const void   *data)
{
   Ecore_Idler *ret;

   if (EINA_UNLIKELY(!eina_main_loop_is()))
     {
        EINA_MAIN_LOOP_CHECK_RETURN;
     }

   if (!func)
     {
        ERR("callback function must be set up for an object of Ecore_Idler.");
        return NULL;
     }

   ret = malloc(sizeof (Ecore_Idler));
   if (!ret) return NULL;

   ret->func = func;
   ret->data = (void*) data;

   eo_event_callback_array_add(_mainloop_singleton, ecore_idler_callbacks(), ret);

   return ret;
}

EAPI void *
ecore_idler_del(Ecore_Idler *idler)
{
   if (!idler) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   return _ecore_idler_del(idler);
}

void
_ecore_idler_all_call(Eo *loop)
{
   eo_event_callback_call(loop, ECORE_MAINLOOP_EVENT_IDLE, NULL);
}

int
_ecore_idler_exist(Eo *loop)
{
   Ecore_Mainloop_Data *dt = eo_data_scope_get(loop, ECORE_MAINLOOP_CLASS);

   return dt->idlers;
}
