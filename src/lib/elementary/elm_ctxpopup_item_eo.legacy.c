
EAPI Elm_Widget_Item *
elm_ctxpopup_item_prev_get(const Elm_Ctxpopup_Item *obj)
{
   return elm_obj_ctxpopup_item_prev_get(obj);
}

EAPI Elm_Widget_Item *
elm_ctxpopup_item_next_get(const Elm_Ctxpopup_Item *obj)
{
   return elm_obj_ctxpopup_item_next_get(obj);
}

EAPI void
elm_ctxpopup_item_selected_set(Elm_Ctxpopup_Item *obj, Eina_Bool selected)
{
   elm_obj_ctxpopup_item_selected_set(obj, selected);
}

EAPI Eina_Bool
elm_ctxpopup_item_selected_get(const Elm_Ctxpopup_Item *obj)
{
   return elm_obj_ctxpopup_item_selected_get(obj);
}

EAPI void
elm_ctxpopup_item_init(Elm_Ctxpopup_Item *obj, Evas_Smart_Cb func, const void *data)
{
   elm_obj_ctxpopup_item_init(obj, func, data);
}
