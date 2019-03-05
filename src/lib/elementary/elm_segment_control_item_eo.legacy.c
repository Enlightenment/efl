
EAPI int
elm_segment_control_item_index_get(const Elm_Segment_Control_Item *obj)
{
   return elm_obj_segment_control_item_index_get(obj);
}

EAPI Efl_Canvas_Object *
elm_segment_control_item_object_get(const Elm_Segment_Control_Item *obj)
{
   return elm_obj_segment_control_item_object_get(obj);
}

EAPI void
elm_segment_control_item_selected_set(Elm_Segment_Control_Item *obj, Eina_Bool selected)
{
   elm_obj_segment_control_item_selected_set(obj, selected);
}
