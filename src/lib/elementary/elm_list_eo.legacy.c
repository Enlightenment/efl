
ELM_API void
elm_list_horizontal_set(Elm_List *obj, Eina_Bool horizontal)
{
   elm_obj_list_horizontal_set(obj, horizontal);
}

ELM_API Eina_Bool
elm_list_horizontal_get(const Elm_List *obj)
{
   return elm_obj_list_horizontal_get(obj);
}

ELM_API void
elm_list_select_mode_set(Elm_List *obj, Elm_Object_Select_Mode mode)
{
   elm_obj_list_select_mode_set(obj, mode);
}

ELM_API Elm_Object_Select_Mode
elm_list_select_mode_get(const Elm_List *obj)
{
   return elm_obj_list_select_mode_get(obj);
}

ELM_API void
elm_list_focus_on_selection_set(Elm_List *obj, Eina_Bool enabled)
{
   elm_obj_list_focus_on_selection_set(obj, enabled);
}

ELM_API Eina_Bool
elm_list_focus_on_selection_get(const Elm_List *obj)
{
   return elm_obj_list_focus_on_selection_get(obj);
}

ELM_API void
elm_list_multi_select_set(Elm_List *obj, Eina_Bool multi)
{
   elm_obj_list_multi_select_set(obj, multi);
}

ELM_API Eina_Bool
elm_list_multi_select_get(const Elm_List *obj)
{
   return elm_obj_list_multi_select_get(obj);
}

ELM_API void
elm_list_multi_select_mode_set(Elm_List *obj, Elm_Object_Multi_Select_Mode mode)
{
   elm_obj_list_multi_select_mode_set(obj, mode);
}

ELM_API Elm_Object_Multi_Select_Mode
elm_list_multi_select_mode_get(const Elm_List *obj)
{
   return elm_obj_list_multi_select_mode_get(obj);
}

ELM_API void
elm_list_mode_set(Elm_List *obj, Elm_List_Mode mode)
{
   elm_obj_list_mode_set(obj, mode);
}

ELM_API Elm_List_Mode
elm_list_mode_get(const Elm_List *obj)
{
   return elm_obj_list_mode_get(obj);
}

ELM_API Elm_Widget_Item *
elm_list_selected_item_get(const Elm_List *obj)
{
   return elm_obj_list_selected_item_get(obj);
}

ELM_API const Eina_List *
elm_list_items_get(const Elm_List *obj)
{
   return elm_obj_list_items_get(obj);
}

ELM_API Elm_Widget_Item *
elm_list_first_item_get(const Elm_List *obj)
{
   return elm_obj_list_first_item_get(obj);
}

ELM_API const Eina_List *
elm_list_selected_items_get(const Elm_List *obj)
{
   return elm_obj_list_selected_items_get(obj);
}

ELM_API Elm_Widget_Item *
elm_list_last_item_get(const Elm_List *obj)
{
   return elm_obj_list_last_item_get(obj);
}

ELM_API Elm_Widget_Item *
elm_list_item_insert_before(Elm_List *obj, Elm_Widget_Item *before, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_list_item_insert_before(obj, before, label, icon, end, func, data);
}

ELM_API void
elm_list_go(Elm_List *obj)
{
   elm_obj_list_go(obj);
}

ELM_API Elm_Widget_Item *
elm_list_item_insert_after(Elm_List *obj, Elm_Widget_Item *after, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_list_item_insert_after(obj, after, label, icon, end, func, data);
}

ELM_API Elm_Widget_Item *
elm_list_at_xy_item_get(const Elm_List *obj, int x, int y, int *posret)
{
   return elm_obj_list_at_xy_item_get(obj, x, y, posret);
}

ELM_API Elm_Widget_Item *
elm_list_item_append(Elm_List *obj, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_list_item_append(obj, label, icon, end, func, data);
}

ELM_API Elm_Widget_Item *
elm_list_item_prepend(Elm_List *obj, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_list_item_prepend(obj, label, icon, end, func, data);
}

ELM_API void
elm_list_clear(Elm_List *obj)
{
   elm_obj_list_clear(obj);
}

ELM_API Elm_Widget_Item *
elm_list_item_sorted_insert(Elm_List *obj, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func)
{
   return elm_obj_list_item_sorted_insert(obj, label, icon, end, func, data, cmp_func);
}
