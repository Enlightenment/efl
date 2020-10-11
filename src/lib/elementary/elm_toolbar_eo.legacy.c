
ELM_API Elm_Widget_Item *
elm_toolbar_selected_item_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_selected_item_get(obj);
}

ELM_API Elm_Widget_Item *
elm_toolbar_first_item_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_first_item_get(obj);
}

ELM_API Elm_Widget_Item *
elm_toolbar_last_item_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_last_item_get(obj);
}

ELM_API Eina_Iterator *
elm_toolbar_items_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_items_get(obj);
}

ELM_API void
elm_toolbar_homogeneous_set(Elm_Toolbar *obj, Eina_Bool homogeneous)
{
   elm_obj_toolbar_homogeneous_set(obj, homogeneous);
}

ELM_API Eina_Bool
elm_toolbar_homogeneous_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_homogeneous_get(obj);
}

ELM_API void
elm_toolbar_align_set(Elm_Toolbar *obj, double align)
{
   elm_obj_toolbar_align_set(obj, align);
}

ELM_API double
elm_toolbar_align_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_align_get(obj);
}

ELM_API void
elm_toolbar_select_mode_set(Elm_Toolbar *obj, Elm_Object_Select_Mode mode)
{
   elm_obj_toolbar_select_mode_set(obj, mode);
}

ELM_API Elm_Object_Select_Mode
elm_toolbar_select_mode_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_select_mode_get(obj);
}

ELM_API void
elm_toolbar_icon_size_set(Elm_Toolbar *obj, int icon_size)
{
   elm_obj_toolbar_icon_size_set(obj, icon_size);
}

ELM_API int
elm_toolbar_icon_size_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_icon_size_get(obj);
}

ELM_API void
elm_toolbar_shrink_mode_set(Elm_Toolbar *obj, Elm_Toolbar_Shrink_Mode shrink_mode)
{
   elm_obj_toolbar_shrink_mode_set(obj, shrink_mode);
}

ELM_API Elm_Toolbar_Shrink_Mode
elm_toolbar_shrink_mode_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_shrink_mode_get(obj);
}

ELM_API void
elm_toolbar_menu_parent_set(Elm_Toolbar *obj, Efl_Canvas_Object *parent)
{
   elm_obj_toolbar_menu_parent_set(obj, parent);
}

ELM_API Efl_Canvas_Object *
elm_toolbar_menu_parent_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_menu_parent_get(obj);
}

ELM_API void
elm_toolbar_standard_priority_set(Elm_Toolbar *obj, int priority)
{
   elm_obj_toolbar_standard_priority_set(obj, priority);
}

ELM_API int
elm_toolbar_standard_priority_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_standard_priority_get(obj);
}

ELM_API Elm_Widget_Item *
elm_toolbar_more_item_get(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_more_item_get(obj);
}

ELM_API Elm_Widget_Item *
elm_toolbar_item_insert_before(Elm_Toolbar *obj, Elm_Widget_Item *before, const char *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_toolbar_item_insert_before(obj, before, icon, label, func, data);
}

ELM_API Elm_Widget_Item *
elm_toolbar_item_insert_after(Elm_Toolbar *obj, Elm_Widget_Item *after, const char *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_toolbar_item_insert_after(obj, after, icon, label, func, data);
}

ELM_API Elm_Widget_Item *
elm_toolbar_item_append(Elm_Toolbar *obj, const char *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_toolbar_item_append(obj, icon, label, func, data);
}

ELM_API unsigned int
elm_toolbar_items_count(const Elm_Toolbar *obj)
{
   return elm_obj_toolbar_items_count(obj);
}

ELM_API Elm_Widget_Item *
elm_toolbar_item_prepend(Elm_Toolbar *obj, const char *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_toolbar_item_prepend(obj, icon, label, func, data);
}

ELM_API Elm_Widget_Item *
elm_toolbar_item_find_by_label(const Elm_Toolbar *obj, const char *label)
{
   return elm_obj_toolbar_item_find_by_label(obj, label);
}
