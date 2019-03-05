
EAPI void
elm_flip_interaction_set(Efl_Ui_Flip *obj, Efl_Ui_Flip_Interaction mode)
{
   efl_ui_flip_interaction_set(obj, mode);
}

EAPI Efl_Ui_Flip_Interaction
elm_flip_interaction_get(const Efl_Ui_Flip *obj)
{
   return efl_ui_flip_interaction_get(obj);
}

EAPI Eina_Bool
elm_flip_front_visible_get(const Efl_Ui_Flip *obj)
{
   return efl_ui_flip_front_visible_get(obj);
}

EAPI void
elm_flip_go(Efl_Ui_Flip *obj, Efl_Ui_Flip_Mode mode)
{
   efl_ui_flip_go(obj, mode);
}

EAPI void
elm_flip_go_to(Efl_Ui_Flip *obj, Eina_Bool front, Efl_Ui_Flip_Mode mode)
{
   efl_ui_flip_go_to(obj, front, mode);
}
