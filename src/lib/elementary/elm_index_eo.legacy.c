
EAPI void
elm_index_autohide_disabled_set(Elm_Index *obj, Eina_Bool disabled)
{
   elm_obj_index_autohide_disabled_set(obj, disabled);
}

EAPI Eina_Bool
elm_index_autohide_disabled_get(const Elm_Index *obj)
{
   return elm_obj_index_autohide_disabled_get(obj);
}

EAPI void
elm_index_omit_enabled_set(Elm_Index *obj, Eina_Bool enabled)
{
   elm_obj_index_omit_enabled_set(obj, enabled);
}

EAPI Eina_Bool
elm_index_omit_enabled_get(const Elm_Index *obj)
{
   return elm_obj_index_omit_enabled_get(obj);
}

EAPI void
elm_index_standard_priority_set(Elm_Index *obj, int priority)
{
   elm_obj_index_standard_priority_set(obj, priority);
}

EAPI int
elm_index_standard_priority_get(const Elm_Index *obj)
{
   return elm_obj_index_standard_priority_get(obj);
}

EAPI void
elm_index_delay_change_time_set(Elm_Index *obj, double dtime)
{
   elm_obj_index_delay_change_time_set(obj, dtime);
}

EAPI double
elm_index_delay_change_time_get(const Elm_Index *obj)
{
   return elm_obj_index_delay_change_time_get(obj);
}

EAPI void
elm_index_indicator_disabled_set(Elm_Index *obj, Eina_Bool disabled)
{
   elm_obj_index_indicator_disabled_set(obj, disabled);
}

EAPI Eina_Bool
elm_index_indicator_disabled_get(const Elm_Index *obj)
{
   return elm_obj_index_indicator_disabled_get(obj);
}

EAPI void
elm_index_item_level_set(Elm_Index *obj, int level)
{
   elm_obj_index_item_level_set(obj, level);
}

EAPI int
elm_index_item_level_get(const Elm_Index *obj)
{
   return elm_obj_index_item_level_get(obj);
}

EAPI void
elm_index_level_go(Elm_Index *obj, int level)
{
   elm_obj_index_level_go(obj, level);
}

EAPI Elm_Widget_Item *
elm_index_item_prepend(Elm_Index *obj, const char *letter, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_index_item_prepend(obj, letter, func, data);
}

EAPI void
elm_index_item_clear(Elm_Index *obj)
{
   elm_obj_index_item_clear(obj);
}

EAPI Elm_Widget_Item *
elm_index_item_insert_after(Elm_Index *obj, Elm_Widget_Item *after, const char *letter, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_index_item_insert_after(obj, after, letter, func, data);
}

EAPI Elm_Widget_Item *
elm_index_item_find(Elm_Index *obj, const void *data)
{
   return elm_obj_index_item_find(obj, data);
}

EAPI Elm_Widget_Item *
elm_index_item_insert_before(Elm_Index *obj, Elm_Widget_Item *before, const char *letter, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_index_item_insert_before(obj, before, letter, func, data);
}

EAPI Elm_Widget_Item *
elm_index_item_append(Elm_Index *obj, const char *letter, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_index_item_append(obj, letter, func, data);
}

EAPI Elm_Widget_Item *
elm_index_selected_item_get(const Elm_Index *obj, int level)
{
   return elm_obj_index_selected_item_get(obj, level);
}

EAPI Elm_Widget_Item *
elm_index_item_sorted_insert(Elm_Index *obj, const char *letter, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func, Eina_Compare_Cb cmp_data_func)
{
   return elm_obj_index_item_sorted_insert(obj, letter, func, data, cmp_func, cmp_data_func);
}
