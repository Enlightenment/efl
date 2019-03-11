
EAPI void
elm_naviframe_item_pop_to(Elm_Naviframe_Item *obj)
{
   elm_obj_naviframe_item_pop_to(obj);
}

EAPI Eina_Bool
elm_naviframe_item_title_enabled_get(const Elm_Naviframe_Item *obj)
{
   return elm_obj_naviframe_item_title_enabled_get(obj);
}

EAPI void
elm_naviframe_item_title_enabled_set(Elm_Naviframe_Item *obj, Eina_Bool enable, Eina_Bool transition)
{
   elm_obj_naviframe_item_title_enabled_set(obj, enable, transition);
}

EAPI void
elm_naviframe_item_promote(Elm_Naviframe_Item *obj)
{
   elm_obj_naviframe_item_promote(obj);
}

EAPI void
elm_naviframe_item_pop_cb_set(Elm_Naviframe_Item *obj, Elm_Naviframe_Item_Pop_Cb func, void *data)
{
   elm_obj_naviframe_item_pop_cb_set(obj, func, data);
}
