
ELM_API Eina_Bool
elm_prefs_data_set(Elm_Prefs *obj, Elm_Prefs_Data *data)
{
   return elm_obj_prefs_data_set(obj, data);
}

ELM_API Elm_Prefs_Data *
elm_prefs_data_get(const Elm_Prefs *obj)
{
   return elm_obj_prefs_data_get(obj);
}

ELM_API void
elm_prefs_autosave_set(Elm_Prefs *obj, Eina_Bool autosave)
{
   elm_obj_prefs_autosave_set(obj, autosave);
}

ELM_API Eina_Bool
elm_prefs_autosave_get(const Elm_Prefs *obj)
{
   return elm_obj_prefs_autosave_get(obj);
}

ELM_API void
elm_prefs_reset(Elm_Prefs *obj, Elm_Prefs_Reset_Mode mode)
{
   elm_obj_prefs_reset(obj, mode);
}

ELM_API Eina_Bool
elm_prefs_item_value_set(Elm_Prefs *obj, const char *name, const Eina_Value *value)
{
   return elm_obj_prefs_item_value_set(obj, name, value);
}

ELM_API Eina_Bool
elm_prefs_item_value_get(const Elm_Prefs *obj, const char *name, Eina_Value *value)
{
   return elm_obj_prefs_item_value_get(obj, name, value);
}

ELM_API const Efl_Canvas_Object *
elm_prefs_item_object_get(Elm_Prefs *obj, const char *name)
{
   return elm_obj_prefs_item_object_get(obj, name);
}

ELM_API void
elm_prefs_item_disabled_set(Elm_Prefs *obj, const char *name, Eina_Bool disabled)
{
   elm_obj_prefs_item_disabled_set(obj, name, disabled);
}

ELM_API Eina_Bool
elm_prefs_item_disabled_get(const Elm_Prefs *obj, const char *name)
{
   return elm_obj_prefs_item_disabled_get(obj, name);
}

ELM_API Eina_Bool
elm_prefs_item_swallow(Elm_Prefs *obj, const char *name, Efl_Canvas_Object *child)
{
   return elm_obj_prefs_item_swallow(obj, name, child);
}

ELM_API void
elm_prefs_item_editable_set(Elm_Prefs *obj, const char *name, Eina_Bool editable)
{
   elm_obj_prefs_item_editable_set(obj, name, editable);
}

ELM_API Eina_Bool
elm_prefs_item_editable_get(const Elm_Prefs *obj, const char *name)
{
   return elm_obj_prefs_item_editable_get(obj, name);
}

ELM_API Efl_Canvas_Object *
elm_prefs_item_unswallow(Elm_Prefs *obj, const char *name)
{
   return elm_obj_prefs_item_unswallow(obj, name);
}

ELM_API void
elm_prefs_item_visible_set(Elm_Prefs *obj, const char *name, Eina_Bool visible)
{
   elm_obj_prefs_item_visible_set(obj, name, visible);
}

ELM_API Eina_Bool
elm_prefs_item_visible_get(const Elm_Prefs *obj, const char *name)
{
   return elm_obj_prefs_item_visible_get(obj, name);
}
