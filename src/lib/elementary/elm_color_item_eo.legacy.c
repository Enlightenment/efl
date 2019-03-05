
EAPI void
elm_color_item_color_set(Elm_Color_Item *obj, int r, int g, int b, int a)
{
   elm_obj_color_item_color_set(obj, r, g, b, a);
}

EAPI void
elm_color_item_color_get(const Elm_Color_Item *obj, int *r, int *g, int *b, int *a)
{
   elm_obj_color_item_color_get(obj, r, g, b, a);
}

EAPI void
elm_color_item_selected_set(Elm_Color_Item *obj, Eina_Bool selected)
{
   elm_obj_color_item_selected_set(obj, selected);
}

EAPI Eina_Bool
elm_color_item_selected_get(const Elm_Color_Item *obj)
{
   return elm_obj_color_item_selected_get(obj);
}
