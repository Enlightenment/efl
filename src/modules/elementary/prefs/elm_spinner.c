#include "private.h"
#include "elm_spinner_eo.h"

static Elm_Prefs_Item_Type supported_types[] =
{
   ELM_PREFS_TYPE_INT,
   ELM_PREFS_TYPE_FLOAT,
   ELM_PREFS_TYPE_UNKNOWN
};

static void
_item_changed_cb(void *data, const Efl_Event *event)
{
    Elm_Prefs_Item_Changed_Cb prefs_it_changed_cb = data;

    prefs_it_changed_cb(event->object);
}

static Evas_Object *
elm_prefs_spinner_add(const Elm_Prefs_Item_Iface *iface EINA_UNUSED,
                      Evas_Object *prefs,
                      const Elm_Prefs_Item_Type type,
                      const Elm_Prefs_Item_Spec spec,
                      Elm_Prefs_Item_Changed_Cb cb)
{
   Evas_Object *obj = elm_spinner_add(prefs);

   evas_object_data_set(obj, "prefs_type", (void *)type);

   efl_event_callback_add
     (obj, ELM_SPINNER_EVENT_CHANGED, _item_changed_cb, cb);

   if (type == ELM_PREFS_TYPE_INT)
     {
        elm_spinner_step_set(obj, 1.0);
        elm_spinner_min_max_set(obj, spec.i.min, spec.i.max);
        elm_spinner_value_set(obj, spec.i.def);
     }
   else if (type == ELM_PREFS_TYPE_FLOAT)
     {
        elm_spinner_label_format_set(obj, "%1.2f");
        elm_spinner_step_set(obj, 0.1);
        elm_spinner_min_max_set(obj, spec.f.min, spec.f.max);
        elm_spinner_value_set(obj, spec.f.def);
     }

   return obj;
}

static Eina_Bool
elm_prefs_spinner_value_set(Evas_Object *obj,
                            Eina_Value *value)
{
   union
   {
      int   i;
      float f;
   } val;

   Elm_Prefs_Item_Type pt =
     (Elm_Prefs_Item_Type) evas_object_data_get(obj, "prefs_type");

   const Eina_Value_Type *vt = eina_value_type_get(value);
   if (!vt) return EINA_FALSE;

   if ((pt == ELM_PREFS_TYPE_INT) && (vt == EINA_VALUE_TYPE_INT))
     {
        eina_value_get(value, &(val.i));
        elm_spinner_value_set(obj, val.i);
     }
   else if ((pt == ELM_PREFS_TYPE_FLOAT) && (vt == EINA_VALUE_TYPE_FLOAT))
     {
        eina_value_get(value, &(val.f));
        elm_spinner_value_set(obj, val.f);
     }
   else
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_spinner_value_get(Evas_Object *obj,
                            Eina_Value *value)
{
   union
   {
      int   i;
      float f;
   } val;

   Elm_Prefs_Item_Type pt =
     (Elm_Prefs_Item_Type) evas_object_data_get(obj, "prefs_type");

   if (pt == ELM_PREFS_TYPE_INT)
     {
        val.i = elm_spinner_value_get(obj);

        if (!eina_value_setup(value, EINA_VALUE_TYPE_INT)) return EINA_FALSE;
        if (!eina_value_set(value, val.i)) return EINA_FALSE;
     }
   else if (pt == ELM_PREFS_TYPE_FLOAT)
      {
        val.f = elm_spinner_value_get(obj);

        if (!eina_value_setup(value, EINA_VALUE_TYPE_FLOAT)) return EINA_FALSE;
        if (!eina_value_set(value, val.f)) return EINA_FALSE;
      }
   else
     return EINA_FALSE;

   return EINA_TRUE;
}

PREFS_ITEM_WIDGET_ADD(spinner,
                      supported_types,
                      elm_prefs_spinner_value_set,
                      elm_prefs_spinner_value_get,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL);
