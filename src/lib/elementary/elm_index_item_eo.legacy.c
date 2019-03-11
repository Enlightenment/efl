
EAPI void
elm_index_item_selected_set(Elm_Index_Item *obj, Eina_Bool selected)
{
   elm_obj_index_item_selected_set(obj, selected);
}

EAPI void
elm_index_item_priority_set(Elm_Index_Item *obj, int priority)
{
   elm_obj_index_item_priority_set(obj, priority);
}

EAPI const char *
elm_index_item_letter_get(const Elm_Index_Item *obj)
{
   return elm_obj_index_item_letter_get(obj);
}
