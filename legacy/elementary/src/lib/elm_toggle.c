#include <Elementary.h>
#include "elm_priv.h"

EAPI Evas_Object *
elm_toggle_add(Evas_Object *parent)
{
   Evas_Object *obj;
   
   obj = elm_check_add(parent);
   elm_object_style_set(obj, "toggle");
   elm_check_states_labels_set(obj, E_("ON"), E_("OFF"));
   return obj;
}

EAPI void
elm_toggle_label_set(Evas_Object *obj, const char *label)
{
   elm_object_text_set(obj, label);
}

EAPI const char *
elm_toggle_label_get(const Evas_Object *obj)
{
   return elm_object_text_get(obj);
}

EAPI void
elm_toggle_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   elm_check_icon_set(obj, icon);
}

EAPI Evas_Object *
elm_toggle_icon_get(const Evas_Object *obj)
{
   return elm_check_icon_get(obj);
}

EAPI Evas_Object *
elm_toggle_icon_unset(Evas_Object *obj)
{
   return elm_check_icon_unset(obj);
}

EAPI void
elm_toggle_states_labels_set(Evas_Object *obj, const char *onlabel, const char *offlabel)
{
   elm_check_states_labels_set(obj, onlabel, offlabel);
}

EAPI void
elm_toggle_states_labels_get(const Evas_Object *obj, const char **onlabel, const char **offlabel)
{
   elm_check_states_labels_get(obj, onlabel, offlabel);
}

EAPI void
elm_toggle_state_set(Evas_Object *obj, Eina_Bool state)
{
   elm_check_state_set(obj, state);
}

EAPI Eina_Bool
elm_toggle_state_get(const Evas_Object *obj)
{
   return elm_check_state_get(obj);
}

EAPI void
elm_toggle_state_pointer_set(Evas_Object *obj, Eina_Bool *statep)
{
   elm_check_state_pointer_set(obj, statep);
}
