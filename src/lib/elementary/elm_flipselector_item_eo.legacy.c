
ELM_API void
elm_flipselector_item_selected_set(Elm_Flipselector_Item *obj, Eina_Bool selected)
{
   elm_obj_flipselector_item_selected_set(obj, selected);
}

ELM_API Eina_Bool
elm_flipselector_item_selected_get(const Elm_Flipselector_Item *obj)
{
   return elm_obj_flipselector_item_selected_get(obj);
}

ELM_API Elm_Widget_Item *
elm_flipselector_item_prev_get(const Elm_Flipselector_Item *obj)
{
   return elm_obj_flipselector_item_prev_get(obj);
}

ELM_API Elm_Widget_Item *
elm_flipselector_item_next_get(const Elm_Flipselector_Item *obj)
{
   return elm_obj_flipselector_item_next_get(obj);
}
