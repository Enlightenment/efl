
ELM_API Elm_Widget_Item *
elm_genlist_item_prev_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_prev_get(obj);
}

ELM_API Elm_Widget_Item *
elm_genlist_item_next_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_next_get(obj);
}

ELM_API Elm_Widget_Item *
elm_genlist_item_parent_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_parent_item_get(obj);
}

ELM_API const Eina_List *
elm_genlist_item_subitems_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_subitems_get(obj);
}

ELM_API void
elm_genlist_item_selected_set(Elm_Genlist_Item *obj, Eina_Bool selected)
{
   elm_obj_genlist_item_selected_set(obj, selected);
}

ELM_API Eina_Bool
elm_genlist_item_selected_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_selected_get(obj);
}

ELM_API void
elm_genlist_item_expanded_set(Elm_Genlist_Item *obj, Eina_Bool expanded)
{
   elm_obj_genlist_item_expanded_set(obj, expanded);
}

ELM_API Eina_Bool
elm_genlist_item_expanded_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_expanded_get(obj);
}

ELM_API int
elm_genlist_item_expanded_depth_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_expanded_depth_get(obj);
}

ELM_API const Elm_Genlist_Item_Class *
elm_genlist_item_item_class_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_class_get(obj);
}

ELM_API int
elm_genlist_item_index_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_index_get(obj);
}

ELM_API const char *
elm_genlist_item_decorate_mode_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_decorate_mode_get(obj);
}

ELM_API void
elm_genlist_item_flip_set(Elm_Genlist_Item *obj, Eina_Bool flip)
{
   elm_obj_genlist_item_flip_set(obj, flip);
}

ELM_API Eina_Bool
elm_genlist_item_flip_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_flip_get(obj);
}

ELM_API void
elm_genlist_item_select_mode_set(Elm_Genlist_Item *obj, Elm_Object_Select_Mode mode)
{
   elm_obj_genlist_item_select_mode_set(obj, mode);
}

ELM_API Elm_Object_Select_Mode
elm_genlist_item_select_mode_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_select_mode_get(obj);
}

ELM_API Elm_Genlist_Item_Type
elm_genlist_item_type_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_type_get(obj);
}

ELM_API void
elm_genlist_item_pin_set(Elm_Genlist_Item *obj, Eina_Bool pin)
{
   elm_obj_genlist_item_pin_set(obj, pin);
}

ELM_API Eina_Bool
elm_genlist_item_pin_get(const Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_pin_get(obj);
}

ELM_API unsigned int
elm_genlist_item_subitems_count(Elm_Genlist_Item *obj)
{
   return elm_obj_genlist_item_subitems_count(obj);
}

ELM_API void
elm_genlist_item_subitems_clear(Elm_Genlist_Item *obj)
{
   elm_obj_genlist_item_subitems_clear(obj);
}

ELM_API void
elm_genlist_item_promote(Elm_Genlist_Item *obj)
{
   elm_obj_genlist_item_promote(obj);
}

ELM_API void
elm_genlist_item_demote(Elm_Genlist_Item *obj)
{
   elm_obj_genlist_item_demote(obj);
}

ELM_API void
elm_genlist_item_show(Elm_Genlist_Item *obj, Elm_Genlist_Item_Scrollto_Type type)
{
   elm_obj_genlist_item_show(obj, type);
}

ELM_API void
elm_genlist_item_bring_in(Elm_Genlist_Item *obj, Elm_Genlist_Item_Scrollto_Type type)
{
   elm_obj_genlist_item_bring_in(obj, type);
}

ELM_API void
elm_genlist_item_all_contents_unset(Elm_Genlist_Item *obj, Eina_List **l)
{
   elm_obj_genlist_item_all_contents_unset(obj, l);
}

ELM_API void
elm_genlist_item_update(Elm_Genlist_Item *obj)
{
   elm_obj_genlist_item_update(obj);
}

ELM_API void
elm_genlist_item_fields_update(Elm_Genlist_Item *obj, const char *parts, Elm_Genlist_Item_Field_Type itf)
{
   elm_obj_genlist_item_fields_update(obj, parts, itf);
}

ELM_API void
elm_genlist_item_item_class_update(Elm_Genlist_Item *obj, const Elm_Genlist_Item_Class *itc)
{
   elm_obj_genlist_item_class_update(obj, itc);
}

ELM_API void
elm_genlist_item_decorate_mode_set(Elm_Genlist_Item *obj, const char *decorate_it_type, Eina_Bool decorate_it_set)
{
   elm_obj_genlist_item_decorate_mode_set(obj, decorate_it_type, decorate_it_set);
}
