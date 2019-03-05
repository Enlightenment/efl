
EAPI void
elm_hoversel_item_icon_set(Elm_Hoversel_Item *obj, const char *icon_file, const char *icon_group, Elm_Icon_Type icon_type)
{
   elm_obj_hoversel_item_icon_set(obj, icon_file, icon_group, icon_type);
}

EAPI void
elm_hoversel_item_icon_get(const Elm_Hoversel_Item *obj, const char **icon_file, const char **icon_group, Elm_Icon_Type *icon_type)
{
   elm_obj_hoversel_item_icon_get(obj, icon_file, icon_group, icon_type);
}
