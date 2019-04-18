
EAPI const Eina_List *
elm_flipselector_items_get(const Elm_Flipselector *obj)
{
   return elm_obj_flipselector_items_get(obj);
}

EAPI Elm_Widget_Item *
elm_flipselector_first_item_get(const Elm_Flipselector *obj)
{
   return elm_obj_flipselector_first_item_get(obj);
}

EAPI Elm_Widget_Item *
elm_flipselector_last_item_get(const Elm_Flipselector *obj)
{
   return elm_obj_flipselector_last_item_get(obj);
}

EAPI Elm_Widget_Item *
elm_flipselector_selected_item_get(const Elm_Flipselector *obj)
{
   return elm_obj_flipselector_selected_item_get(obj);
}

EAPI void
elm_flipselector_first_interval_set(Elm_Flipselector *obj, double interval)
{
   elm_obj_flipselector_first_interval_set(obj, interval);
}

EAPI double
elm_flipselector_first_interval_get(const Elm_Flipselector *obj)
{
   return elm_obj_flipselector_first_interval_get(obj);
}

EAPI Elm_Widget_Item *
elm_flipselector_item_prepend(Elm_Flipselector *obj, const char *label, Evas_Smart_Cb func, void *data)
{
   return elm_obj_flipselector_item_prepend(obj, label, func, data);
}

EAPI void
elm_flipselector_flip_next(Elm_Flipselector *obj)
{
   elm_obj_flipselector_flip_next(obj);
}

EAPI Elm_Widget_Item *
elm_flipselector_item_append(Elm_Flipselector *obj, const char *label, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_flipselector_item_append(obj, label, func, data);
}

EAPI void
elm_flipselector_flip_prev(Elm_Flipselector *obj)
{
   elm_obj_flipselector_flip_prev(obj);
}
