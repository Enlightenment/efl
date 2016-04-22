#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <Eo.h>

#include "Ecore.h"
#include "ecore_private.h"

struct _Ecore_Factorized_Idle
{
   Ecore_Task_Cb func;
   void         *data;

   const Eo_Callback_Array_Item *desc;

   short         references;
   Eina_Bool     delete_me : 1;
};

Eina_Bool
_ecore_factorized_idle_event_del(void *data, const Eo_Event *event EINA_UNUSED)
{
   _ecore_factorized_idle_del(data);

   return EO_CALLBACK_CONTINUE;
}

Eina_Bool
_ecore_factorized_idle_process(void *data, const Eo_Event *event EINA_UNUSED)
{
   Ecore_Factorized_Idle *idler = data;

   idler->references++;
   if (!_ecore_call_task_cb(idler->func, idler->data))
     idler->delete_me = EINA_TRUE;
   idler->references--;

   if (idler->delete_me &&
       idler->references == 0)
     _ecore_factorized_idle_del(idler);

   return EO_CALLBACK_CONTINUE;
}

void *
_ecore_factorized_idle_del(Ecore_Idler *idler)
{
   void *data;

   if (!idler) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   if (idler->references > 0)
     {
        idler->delete_me = EINA_TRUE;
        return idler->data;
     }

   eo_event_callback_array_del(_mainloop_singleton, idler->desc, idler);

   data = idler->data;
   free(idler);
   return data;
}

Ecore_Factorized_Idle *
_ecore_factorized_idle_add(const Eo_Callback_Array_Item *desc,
                           Ecore_Task_Cb func,
                           const void   *data)
{
   Ecore_Factorized_Idle *ret;

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
   ret->desc = desc;
   ret->references = 0;
   ret->delete_me = EINA_FALSE;

   eo_event_callback_array_add(_mainloop_singleton, desc, ret);

   return ret;
}

/* Specific to Ecore_Idler implementation */

EO_CALLBACKS_ARRAY_DEFINE(ecore_idler_callbacks,
                          { EFL_LOOP_EVENT_IDLE, _ecore_factorized_idle_process },
                          { EO_BASE_EVENT_DEL, _ecore_factorized_idle_event_del });

EAPI Ecore_Idler *
ecore_idler_add(Ecore_Task_Cb func,
                const void   *data)
{
   return _ecore_factorized_idle_add(ecore_idler_callbacks(), func, data);
}

EAPI void *
ecore_idler_del(Ecore_Idler *idler)
{
   return _ecore_factorized_idle_del(idler);
}

void
_ecore_idler_all_call(Eo *loop)
{
   eo_event_callback_call(loop, EFL_LOOP_EVENT_IDLE, NULL);
}

int
_ecore_idler_exist(Eo *loop)
{
   Efl_Loop_Data *dt = eo_data_scope_get(loop, EFL_LOOP_CLASS);

   return dt->idlers;
}
