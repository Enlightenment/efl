
ELM_API Elm_Widget_Item *
elm_gengrid_item_prev_get(const Elm_Gengrid_Item *obj)
{
   return elm_obj_gengrid_item_prev_get(obj);
}

ELM_API Elm_Widget_Item *
elm_gengrid_item_next_get(const Elm_Gengrid_Item *obj)
{
   return elm_obj_gengrid_item_next_get(obj);
}

ELM_API void
elm_gengrid_item_selected_set(Elm_Gengrid_Item *obj, Eina_Bool selected)
{
   elm_obj_gengrid_item_selected_set(obj, selected);
}

ELM_API Eina_Bool
elm_gengrid_item_selected_get(const Elm_Gengrid_Item *obj)
{
   return elm_obj_gengrid_item_selected_get(obj);
}

ELM_API const Elm_Gengrid_Item_Class *
elm_gengrid_item_item_class_get(const Elm_Gengrid_Item *obj)
{
   return elm_obj_gengrid_item_class_get(obj);
}

ELM_API int
elm_gengrid_item_index_get(const Elm_Gengrid_Item *obj)
{
   return elm_obj_gengrid_item_index_get(obj);
}

ELM_API void
elm_gengrid_item_pos_get(const Elm_Gengrid_Item *obj, unsigned int *x, unsigned int *y)
{
   elm_obj_gengrid_item_pos_get(obj, x, y);
}

ELM_API void
elm_gengrid_item_select_mode_set(Elm_Gengrid_Item *obj, Elm_Object_Select_Mode mode)
{
   elm_obj_gengrid_item_select_mode_set(obj, mode);
}

ELM_API Elm_Object_Select_Mode
elm_gengrid_item_select_mode_get(const Elm_Gengrid_Item *obj)
{
   return elm_obj_gengrid_item_select_mode_get(obj);
}

ELM_API void
elm_gengrid_item_custom_size_set(Elm_Gengrid_Item *obj, int w, int h)
{
   elm_obj_gengrid_item_custom_size_set(obj, w, h);
}

ELM_API void
elm_gengrid_item_custom_size_get(const Elm_Gengrid_Item *obj, int *w, int *h)
{
   elm_obj_gengrid_item_custom_size_get(obj, w, h);
}

ELM_API void
elm_gengrid_item_show(Elm_Gengrid_Item *obj, Elm_Gengrid_Item_Scrollto_Type type)
{
   elm_obj_gengrid_item_show(obj, type);
}

ELM_API void
elm_gengrid_item_bring_in(Elm_Gengrid_Item *obj, Elm_Gengrid_Item_Scrollto_Type type)
{
   elm_obj_gengrid_item_bring_in(obj, type);
}

ELM_API void
elm_gengrid_item_update(Elm_Gengrid_Item *obj)
{
   elm_obj_gengrid_item_update(obj);
}

ELM_API void
elm_gengrid_item_fields_update(Elm_Gengrid_Item *obj, const char *parts, Elm_Gengrid_Item_Field_Type itf)
{
   elm_obj_gengrid_item_fields_update(obj, parts, itf);
}

ELM_API void
elm_gengrid_item_item_class_update(Elm_Gengrid_Item *obj, const Elm_Gengrid_Item_Class *itc)
{
   elm_obj_gengrid_item_class_update(obj, itc);
}

ELM_API void
elm_gengrid_item_all_contents_unset(Elm_Gengrid_Item *obj, Eina_List **l)
{
   elm_obj_gengrid_item_all_contents_unset(obj, l);
}
