#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"
#include "Ecore_Evas.h"
#include "ecore_evas_private.h"
#include <Efl_Core.h>

typedef struct {
   Ecore_Evas_Selection_Callbacks callbacks[ECORE_EVAS_SELECTION_BUFFER_LAST];
   int seat;
} Ecore_Evas_Fallback_Selection_Data;

static Ecore_Evas_Fallback_Selection_Data data[ECORE_EVAS_SELECTION_BUFFER_LAST];

void
fallback_selection_shutdown(Ecore_Evas *ee)
{
   for (int i = 0; i < ECORE_EVAS_SELECTION_BUFFER_LAST; ++i)
     {
        if (data->callbacks[i].cancel)
          data->callbacks[i].cancel(ee, data->seat, i);
     }
}

Eina_Bool
fallback_selection_claim(Ecore_Evas *ee, unsigned int seat, Ecore_Evas_Selection_Buffer selection, Eina_Array *available_types, Ecore_Evas_Selection_Internal_Delivery delivery, Ecore_Evas_Selection_Internal_Cancel cancel)
{
   Ecore_Evas_Selection_Callbacks *callbacks = &data->callbacks[selection];

   if (callbacks->cancel)
     {
        callbacks->cancel(ee, data->seat, selection);
        eina_array_free(callbacks->available_types);
     }

   callbacks->delivery = delivery;
   callbacks->cancel = cancel;
   callbacks->available_types = available_types;
   data->seat = seat;

   if (ee->func.fn_selection_changed)
     ee->func.fn_selection_changed(ee, seat, selection);

   return EINA_TRUE;
}

Eina_Bool
fallback_selection_has_owner(Ecore_Evas *ee EINA_UNUSED, unsigned int seat EINA_UNUSED, Ecore_Evas_Selection_Buffer selection EINA_UNUSED)
{
   return EINA_FALSE; //if the real selection buffer does not contain it, then we dont know it either.
}

Eina_Stringshare*
available_types(Eina_Array *acceptable_types, Eina_Array *available_types)
{
   Eina_Stringshare *found_type = NULL;
   Eina_Stringshare *type;

   for (unsigned int i = 0; i < eina_array_count_get(available_types); ++i)
     {
        unsigned int out = -1;
        type = eina_array_data_get(available_types, i);

        if (!found_type && eina_array_find(acceptable_types, type, &out))
          {
             found_type = eina_stringshare_ref(type);
          }
        eina_stringshare_del(type);
     }
  eina_array_free(acceptable_types);

  return found_type;
}

Eina_Future*
fallback_selection_request(Ecore_Evas *ee EINA_UNUSED, unsigned int seat, Ecore_Evas_Selection_Buffer selection, Eina_Array *acceptable_type)
{
   Ecore_Evas_Selection_Callbacks callbacks = data->callbacks[selection];
   Eina_Content *result;
   Eina_Stringshare *serving_type;
   Eina_Rw_Slice slice_data;
   Eina_Value value;

   if (!callbacks.delivery)
     return eina_future_resolved(efl_loop_future_scheduler_get(efl_main_loop_get()), eina_value_int_init(0));

   serving_type = available_types(acceptable_type, callbacks.available_types);
   if (!serving_type)
     return NULL; //Silent return cause we cannot deliver a good type

   EINA_SAFETY_ON_FALSE_RETURN_VAL(callbacks.delivery(ee, seat, selection, serving_type, &slice_data), NULL);
   result = eina_content_new(eina_rw_slice_slice_get(slice_data), serving_type);
   value = eina_value_content_init(result);
   eina_content_free(result);

   return eina_future_resolved(efl_loop_future_scheduler_get(efl_main_loop_get()), value);
}
Eina_Bool
fallback_dnd_start(Ecore_Evas *ee EINA_UNUSED, unsigned int seat EINA_UNUSED, Eina_Array *available_types EINA_UNUSED, Ecore_Evas *drag_rep EINA_UNUSED, Ecore_Evas_Selection_Internal_Delivery delivery EINA_UNUSED, Ecore_Evas_Selection_Internal_Cancel cancel EINA_UNUSED, const char* action EINA_UNUSED)
{
   return EINA_FALSE;
}

Eina_Bool
fallback_dnd_stop(Ecore_Evas *ee EINA_UNUSED, unsigned int seat EINA_UNUSED)
{
   return EINA_FALSE;
}
