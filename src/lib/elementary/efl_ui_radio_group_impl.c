#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_RADIO_GROUP_IMPL_CLASS

static Eina_Hash *radio_group_map;

typedef struct {
   Efl_Ui_Radio *selected;
   Efl_Ui_Radio *fallback_object;
   Eina_List *registered_set;
   Eina_Bool in_value_change;
} Efl_Ui_Radio_Group_Impl_Data;

EOLIAN static void
_efl_ui_radio_group_impl_efl_ui_single_selectable_allow_manual_deselection_set(Eo *obj EINA_UNUSED, Efl_Ui_Radio_Group_Impl_Data *pd EINA_UNUSED, Eina_Bool allow_manual_deselection EINA_UNUSED)
{
   if (allow_manual_deselection == EINA_FALSE)
     ERR("This is right now not supported.");
}

EOLIAN static Eina_Bool
_efl_ui_radio_group_impl_efl_ui_single_selectable_allow_manual_deselection_get(const Eo *obj EINA_UNUSED, Efl_Ui_Radio_Group_Impl_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_radio_group_impl_efl_ui_single_selectable_fallback_selection_set(Eo *obj EINA_UNUSED, Efl_Ui_Radio_Group_Impl_Data *pd, Efl_Ui_Selectable *fallback)
{
   pd->fallback_object = fallback;

   if (!pd->selected)
     efl_ui_selectable_selected_set(pd->fallback_object, EINA_TRUE);
}

EOLIAN static Efl_Ui_Selectable*
_efl_ui_radio_group_impl_efl_ui_single_selectable_fallback_selection_get(const Eo *obj EINA_UNUSED, Efl_Ui_Radio_Group_Impl_Data *pd)
{
   return pd->fallback_object;
}

EOLIAN static Efl_Ui_Radio*
_efl_ui_radio_group_impl_efl_ui_single_selectable_last_selected_get(const Eo *obj EINA_UNUSED, Efl_Ui_Radio_Group_Impl_Data *pd)
{
   return pd->selected;
}

EOLIAN static void
_efl_ui_radio_group_impl_efl_ui_radio_group_selected_value_set(Eo *obj EINA_UNUSED, Efl_Ui_Radio_Group_Impl_Data *pd, int selected_value)
{
   Efl_Ui_Radio *reged;
   Eina_List *n;

   EINA_LIST_FOREACH(pd->registered_set, n, reged)
     {
        if (efl_ui_radio_state_value_get(reged) == selected_value)
          {
             efl_ui_selectable_selected_set(reged, EINA_TRUE);
             return;
          }
     }
   ERR("Value %d not assosiated with any radio button", selected_value);
}

EOLIAN static int
_efl_ui_radio_group_impl_efl_ui_radio_group_selected_value_get(const Eo *obj EINA_UNUSED, Efl_Ui_Radio_Group_Impl_Data *pd)
{
   return pd->selected ? efl_ui_radio_state_value_get(pd->selected) : -1;
}

static void
_selected_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Radio_Group_Impl_Data *pd = efl_data_scope_safe_get(data, EFL_UI_RADIO_GROUP_IMPL_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(pd);

   if (efl_ui_selectable_selected_get(ev->object))
     {
        if (pd->selected)
          {
             pd->in_value_change = EINA_TRUE;
             efl_ui_selectable_selected_set(pd->selected, EINA_FALSE);
          }
        pd->in_value_change = EINA_FALSE;
        EINA_SAFETY_ON_FALSE_RETURN(!pd->selected);
        pd->selected = ev->object;
     }
   else
     {
        //if something was unselected, we need to make sure that we are unsetting the internal pointer to NULL
        if (pd->selected == ev->object)
          {
             pd->selected = NULL;
          }
        //checkout if we want to do fallback handling
        if (!pd->in_value_change)
          {
             if (!pd->selected && pd->fallback_object)
               efl_ui_selectable_selected_set(pd->fallback_object, EINA_TRUE);
          }
     }

   if (!pd->in_value_change)
     {
        int value;
        if (pd->selected)
          value = efl_ui_radio_state_value_get(pd->selected);
        else
          value = -1;
        efl_event_callback_call(data, EFL_UI_RADIO_GROUP_EVENT_VALUE_CHANGED, &value);
        efl_event_callback_call(data, EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED, NULL);
     }
}

static void
_invalidate_cb(void *data, const Efl_Event *ev)
{
   efl_ui_radio_group_unregister(data, ev->object);
}

EFL_CALLBACKS_ARRAY_DEFINE(radio_btn_cb,
  {EFL_UI_EVENT_SELECTED_CHANGED, _selected_cb},
  {EFL_EVENT_INVALIDATE, _invalidate_cb},
)

EOLIAN static void
_efl_ui_radio_group_impl_efl_ui_radio_group_register(Eo *obj, Efl_Ui_Radio_Group_Impl_Data *pd, Efl_Ui_Radio *radio)
{
   Efl_Ui_Radio *reged;
   Eina_List *n;

   if (eina_hash_find(radio_group_map, &radio))
     {
        ERR("Radio button %p is already part of another group", radio);
        return;
     }

   EINA_LIST_FOREACH(pd->registered_set, n, reged)
     {
        EINA_SAFETY_ON_TRUE_RETURN(radio == reged);
        EINA_SAFETY_ON_TRUE_RETURN(efl_ui_radio_state_value_get(radio) == efl_ui_radio_state_value_get(reged));
     }
   EINA_SAFETY_ON_TRUE_RETURN(efl_ui_radio_state_value_get(radio) == -1);

   pd->registered_set = eina_list_append(pd->registered_set, radio);
   eina_hash_add(radio_group_map, &radio, obj);
   efl_event_callback_array_add(radio, radio_btn_cb(), obj);
}

EOLIAN static void
_efl_ui_radio_group_impl_efl_ui_radio_group_unregister(Eo *obj, Efl_Ui_Radio_Group_Impl_Data *pd, Efl_Ui_Radio *radio)
{
   if (pd->selected == radio)
     efl_ui_selectable_selected_set(pd->selected, EINA_FALSE);

   efl_event_callback_array_del(radio, radio_btn_cb(), obj);
   pd->registered_set = eina_list_remove(pd->registered_set, radio);
   eina_hash_del(radio_group_map, &radio, obj);
}

EOLIAN static void
_efl_ui_radio_group_impl_efl_object_destructor(Eo *obj, Efl_Ui_Radio_Group_Impl_Data *pd)
{
   Eo *radio;

   EINA_LIST_FREE(pd->registered_set, radio)
     {
        efl_event_callback_array_del(radio, radio_btn_cb(), obj);
        eina_hash_del(radio_group_map, &radio, obj);
     }
   efl_destructor(efl_super(obj, MY_CLASS));
}

void
_efl_ui_radio_group_impl_class_constructor(Efl_Class *klass EINA_UNUSED)
{
   radio_group_map = eina_hash_pointer_new(NULL);
}

#include "efl_ui_radio_group_impl.eo.c"
