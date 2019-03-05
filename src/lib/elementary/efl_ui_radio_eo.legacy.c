
EAPI void
elm_radio_state_value_set(Efl_Ui_Radio *obj, int value)
{
   efl_ui_radio_state_value_set(obj, value);
}

EAPI int
elm_radio_state_value_get(const Efl_Ui_Radio *obj)
{
   return efl_ui_radio_state_value_get(obj);
}

EAPI void
elm_radio_value_pointer_set(Efl_Ui_Radio *obj, int *valuep)
{
   efl_ui_radio_value_pointer_set(obj, valuep);
}

EAPI Efl_Canvas_Object *
elm_radio_selected_object_get(const Efl_Ui_Radio *obj)
{
   return efl_ui_radio_selected_object_get(obj);
}

EAPI void
elm_radio_group_add(Efl_Ui_Radio *obj, Efl_Ui_Radio *group)
{
   efl_ui_radio_group_add(obj, group);
}
