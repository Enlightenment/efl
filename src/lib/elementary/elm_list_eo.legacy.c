
EAPI void
elm_list_horizontal_set(Elm_List *obj, Eina_Bool horizontal)
{
   elm_obj_list_horizontal_set(obj, horizontal);
}

EAPI Eina_Bool
elm_list_horizontal_get(const Elm_List *obj)
{
   return elm_obj_list_horizontal_get(obj);
}

EAPI void
elm_list_select_mode_set(Elm_List *obj, Elm_Object_Select_Mode mode)
{
   elm_obj_list_select_mode_set(obj, mode);
}

EAPI Elm_Object_Select_Mode
elm_list_select_mode_get(const Elm_List *obj)
{
   return elm_obj_list_select_mode_get(obj);
}

EAPI void
elm_list_focus_on_selection_set(Elm_List *obj, Eina_Bool enabled)
{
   elm_obj_list_focus_on_selection_set(obj, enabled);
}

EAPI Eina_Bool
elm_list_focus_on_selection_get(const Elm_List *obj)
{
   return elm_obj_list_focus_on_selection_get(obj);
}

EAPI void
elm_list_multi_select_set(Elm_List *obj, Eina_Bool multi)
{
   elm_obj_list_multi_select_set(obj, multi);
}

EAPI Eina_Bool
elm_list_multi_select_get(const Elm_List *obj)
{
   return elm_obj_list_multi_select_get(obj);
}

EAPI void
elm_list_multi_select_mode_set(Elm_List *obj, Elm_Object_Multi_Select_Mode mode)
{
   elm_obj_list_multi_select_mode_set(obj, mode);
}

EAPI Elm_Object_Multi_Select_Mode
elm_list_multi_select_mode_get(const Elm_List *obj)
{
   return elm_obj_list_multi_select_mode_get(obj);
}

EAPI void
elm_list_mode_set(Elm_List *obj, Elm_List_Mode mode)
{
   elm_obj_list_mode_set(obj, mode);
}

EAPI Elm_List_Mode
elm_list_mode_get(const Elm_List *obj)
{
   return elm_obj_list_mode_get(obj);
}

EAPI Elm_Widget_Item *
elm_list_selected_item_get(const Elm_List *obj)
{
   return elm_obj_list_selected_item_get(obj);
}

EAPI const Eina_List *
elm_list_items_get(const Elm_List *obj)
{
   return elm_obj_list_items_get(obj);
}

EAPI Elm_Widget_Item *
elm_list_first_item_get(const Elm_List *obj)
{
   return elm_obj_list_first_item_get(obj);
}

EAPI const Eina_List *
elm_list_selected_items_get(const Elm_List *obj)
{
   return elm_obj_list_selected_items_get(obj);
}

EAPI Elm_Widget_Item *
elm_list_last_item_get(const Elm_List *obj)
{
   return elm_obj_list_last_item_get(obj);
}

EAPI Elm_Widget_Item *
elm_list_item_insert_before(Elm_List *obj, Elm_Widget_Item *before, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_list_item_insert_before(obj, before, label, icon, end, func, data);
}

EAPI void
elm_list_go(Elm_List *obj)
{
   elm_obj_list_go(obj);
}

EAPI Elm_Widget_Item *
elm_list_item_insert_after(Elm_List *obj, Elm_Widget_Item *after, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_list_item_insert_after(obj, after, label, icon, end, func, data);
}

EAPI Elm_Widget_Item *
elm_list_at_xy_item_get(const Elm_List *obj, int x, int y, int *posret)
{
   return elm_obj_list_at_xy_item_get(obj, x, y, posret);
}

EAPI Elm_Widget_Item *
elm_list_item_append(Elm_List *obj, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_list_item_append(obj, label, icon, end, func, data);
}

EAPI Elm_Widget_Item *
elm_list_item_prepend(Elm_List *obj, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_list_item_prepend(obj, label, icon, end, func, data);
}

EAPI void
elm_list_clear(Elm_List *obj)
{
   elm_obj_list_clear(obj);
}

EAPI Elm_Widget_Item *
elm_list_item_sorted_insert(Elm_List *obj, const char *label, Efl_Canvas_Object *icon, Efl_Canvas_Object *end, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func)
{
   return elm_obj_list_item_sorted_insert(obj, label, icon, end, func, data, cmp_func);
}
