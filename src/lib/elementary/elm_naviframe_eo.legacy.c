
EAPI void
elm_naviframe_event_enabled_set(Elm_Naviframe *obj, Eina_Bool enabled)
{
   elm_obj_naviframe_event_enabled_set(obj, enabled);
}

EAPI Eina_Bool
elm_naviframe_event_enabled_get(const Elm_Naviframe *obj)
{
   return elm_obj_naviframe_event_enabled_get(obj);
}

EAPI void
elm_naviframe_content_preserve_on_pop_set(Elm_Naviframe *obj, Eina_Bool preserve)
{
   elm_obj_naviframe_content_preserve_on_pop_set(obj, preserve);
}

EAPI Eina_Bool
elm_naviframe_content_preserve_on_pop_get(const Elm_Naviframe *obj)
{
   return elm_obj_naviframe_content_preserve_on_pop_get(obj);
}

EAPI void
elm_naviframe_prev_btn_auto_pushed_set(Elm_Naviframe *obj, Eina_Bool auto_pushed)
{
   elm_obj_naviframe_prev_btn_auto_pushed_set(obj, auto_pushed);
}

EAPI Eina_Bool
elm_naviframe_prev_btn_auto_pushed_get(const Elm_Naviframe *obj)
{
   return elm_obj_naviframe_prev_btn_auto_pushed_get(obj);
}

EAPI Eina_List *
elm_naviframe_items_get(const Elm_Naviframe *obj)
{
   return elm_obj_naviframe_items_get(obj);
}

EAPI Elm_Widget_Item *
elm_naviframe_top_item_get(const Elm_Naviframe *obj)
{
   return elm_obj_naviframe_top_item_get(obj);
}

EAPI Elm_Widget_Item *
elm_naviframe_bottom_item_get(const Elm_Naviframe *obj)
{
   return elm_obj_naviframe_bottom_item_get(obj);
}

EAPI Efl_Canvas_Object *
elm_naviframe_item_pop(Elm_Naviframe *obj)
{
   return elm_obj_naviframe_item_pop(obj);
}

EAPI Elm_Widget_Item *
elm_naviframe_item_insert_before(Elm_Naviframe *obj, Elm_Widget_Item *before, const char *title_label, Efl_Canvas_Object *prev_btn, Efl_Canvas_Object *next_btn, Efl_Canvas_Object *content, const char *item_style)
{
   return elm_obj_naviframe_item_insert_before(obj, before, title_label, prev_btn, next_btn, content, item_style);
}

EAPI Elm_Widget_Item *
elm_naviframe_item_push(Elm_Naviframe *obj, const char *title_label, Efl_Canvas_Object *prev_btn, Efl_Canvas_Object *next_btn, Efl_Canvas_Object *content, const char *item_style)
{
   return elm_obj_naviframe_item_push(obj, title_label, prev_btn, next_btn, content, item_style);
}

EAPI void
elm_naviframe_item_simple_promote(Elm_Naviframe *obj, Efl_Canvas_Object *content)
{
   elm_obj_naviframe_item_simple_promote(obj, content);
}

EAPI Elm_Widget_Item *
elm_naviframe_item_insert_after(Elm_Naviframe *obj, Elm_Widget_Item *after, const char *title_label, Efl_Canvas_Object *prev_btn, Efl_Canvas_Object *next_btn, Efl_Canvas_Object *content, const char *item_style)
{
   return elm_obj_naviframe_item_insert_after(obj, after, title_label, prev_btn, next_btn, content, item_style);
}
