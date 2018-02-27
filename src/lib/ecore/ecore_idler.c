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

   const Efl_Callback_Array_Item *desc;

   short         references;
   Eina_Bool     delete_me : 1;
};

void
_ecore_factorized_idle_event_del(void *data, const Efl_Event *event EINA_UNUSED)
{
   _ecore_factorized_idle_del(data);
}

void
_ecore_factorized_idle_process(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Factorized_Idle *idler = data;

   idler->references++;
   if (!_ecore_call_task_cb(idler->func, idler->data))
     idler->delete_me = EINA_TRUE;
   idler->references--;

   if (idler->delete_me &&
       idler->references == 0)
     _ecore_factorized_idle_del(idler);
}

static Eina_Mempool *idler_mp = NULL;

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

   efl_event_callback_array_del(_mainloop_singleton, idler->desc, idler);

   data = idler->data;
   eina_mempool_free(idler_mp, idler);
   return data;
}

Ecore_Factorized_Idle *
_ecore_factorized_idle_add(const Efl_Callback_Array_Item *desc,
                           Ecore_Task_Cb func,
                           const void   *data)
{
   Ecore_Factorized_Idle *ret;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   if (!func)
     {
        ERR("callback function must be set up for an object of Ecore_Idler.");
        return NULL;
     }

   if (!idler_mp)
     {
        idler_mp = eina_mempool_add("chained_mempool", "Ecore_Idle*", NULL, sizeof (Ecore_Factorized_Idle), 23);
        if (!idler_mp) return NULL;
     }

   ret = eina_mempool_malloc(idler_mp, sizeof (Ecore_Factorized_Idle));
   if (!ret) return NULL;

   ret->func = func;
   ret->data = (void*) data;
   ret->desc = desc;
   ret->references = 0;
   ret->delete_me = EINA_FALSE;

   efl_event_callback_array_add(_mainloop_singleton, desc, ret);

   return ret;
}

/* Specific to Ecore_Idler implementation */

EFL_CALLBACKS_ARRAY_DEFINE(ecore_idler_callbacks,
                          { EFL_LOOP_EVENT_IDLE, _ecore_factorized_idle_process },
                          { EFL_EVENT_DEL, _ecore_factorized_idle_event_del });

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
