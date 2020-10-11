
ELM_API void
elm_flip_interaction_set(Elm_Flip *obj, Elm_Flip_Interaction mode)
{
   efl_ui_flip_interaction_set(obj, (Efl_Ui_Flip_Interaction)mode);
}

ELM_API Elm_Flip_Interaction
elm_flip_interaction_get(const Elm_Flip *obj)
{
   return (Elm_Flip_Interaction)efl_ui_flip_interaction_get(obj);
}

ELM_API Eina_Bool
elm_flip_front_visible_get(const Elm_Flip *obj)
{
   return efl_ui_flip_front_visible_get(obj);
}

ELM_API void
elm_flip_go(Elm_Flip *obj, Elm_Flip_Mode mode)
{
   efl_ui_flip_go(obj, (Efl_Ui_Flip_Mode)mode);
}

ELM_API void
elm_flip_go_to(Elm_Flip *obj, Eina_Bool front, Elm_Flip_Mode mode)
{
   efl_ui_flip_go_to(obj, front, (Efl_Ui_Flip_Mode)mode);
}
