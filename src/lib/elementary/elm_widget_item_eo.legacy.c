
EAPI Eina_Bool
elm_object_item_tooltip_window_mode_set(Elm_Widget_Item *obj, Eina_Bool disable)
{
   return elm_wdg_item_tooltip_window_mode_set(obj, disable);
}

EAPI Eina_Bool
elm_object_item_tooltip_window_mode_get(const Elm_Widget_Item *obj)
{
   return elm_wdg_item_tooltip_window_mode_get(obj);
}

EAPI void
elm_object_item_tooltip_style_set(Elm_Widget_Item *obj, const char *style)
{
   elm_wdg_item_tooltip_style_set(obj, style);
}

EAPI const char *
elm_object_item_tooltip_style_get(const Elm_Widget_Item *obj)
{
   return elm_wdg_item_tooltip_style_get(obj);
}

EAPI void
elm_object_item_cursor_set(Elm_Widget_Item *obj, const char *cursor)
{
   elm_wdg_item_cursor_set(obj, cursor);
}

EAPI const char *
elm_object_item_cursor_get(const Elm_Widget_Item *obj)
{
   return elm_wdg_item_cursor_get(obj);
}

EAPI void
elm_object_item_cursor_style_set(Elm_Widget_Item *obj, const char *style)
{
   elm_wdg_item_cursor_style_set(obj, style);
}

EAPI const char *
elm_object_item_cursor_style_get(const Elm_Widget_Item *obj)
{
   return elm_wdg_item_cursor_style_get(obj);
}

EAPI void
elm_object_item_cursor_engine_only_set(Elm_Widget_Item *obj, Eina_Bool engine_only)
{
   elm_wdg_item_cursor_engine_only_set(obj, engine_only);
}

EAPI Eina_Bool
elm_object_item_cursor_engine_only_get(const Elm_Widget_Item *obj)
{
   return elm_wdg_item_cursor_engine_only_get(obj);
}

EAPI void
elm_object_item_part_content_set(Elm_Widget_Item *obj, const char *part, Efl_Canvas_Object *content)
{
   elm_wdg_item_part_content_set(obj, part, content);
}

EAPI Efl_Canvas_Object *
elm_object_item_part_content_get(const Elm_Widget_Item *obj, const char *part)
{
   return elm_wdg_item_part_content_get(obj, part);
}

EAPI void
elm_object_item_part_text_set(Elm_Widget_Item *obj, const char *part, const char *label)
{
   elm_wdg_item_part_text_set(obj, part, label);
}

EAPI const char *
elm_object_item_part_text_get(const Elm_Widget_Item *obj, const char *part)
{
   return elm_wdg_item_part_text_get(obj, part);
}

EAPI void
elm_object_item_focus_set(Elm_Widget_Item *obj, Eina_Bool focused)
{
   elm_wdg_item_focus_set(obj, focused);
}

EAPI Eina_Bool
elm_object_item_focus_get(const Elm_Widget_Item *obj)
{
   return elm_wdg_item_focus_get(obj);
}

EAPI void
elm_object_item_style_set(Elm_Widget_Item *obj, const char *style)
{
   elm_wdg_item_style_set(obj, style);
}

EAPI const char *
elm_object_item_style_get(const Elm_Widget_Item *obj)
{
   return elm_wdg_item_style_get(obj);
}

EAPI void
elm_object_item_disabled_set(Elm_Widget_Item *obj, Eina_Bool disable)
{
   elm_wdg_item_disabled_set(obj, disable);
}

EAPI Eina_Bool
elm_object_item_disabled_get(const Elm_Widget_Item *obj)
{
   return elm_wdg_item_disabled_get(obj);
}

EAPI const Eina_List *
elm_object_item_access_order_get(Elm_Widget_Item *obj)
{
   return elm_wdg_item_access_order_get(obj);
}

EAPI void
elm_object_item_access_order_set(Elm_Widget_Item *obj, Eina_List *objs)
{
   elm_wdg_item_access_order_set(obj, objs);
}

EAPI Efl_Canvas_Object *
elm_object_item_widget_get(const Elm_Widget_Item *obj)
{
   return elm_wdg_item_widget_get(obj);
}

EAPI void
elm_object_item_tooltip_text_set(Elm_Widget_Item *obj, const char *text)
{
   elm_wdg_item_tooltip_text_set(obj, text);
}

EAPI void
elm_object_item_tooltip_unset(Elm_Widget_Item *obj)
{
   elm_wdg_item_tooltip_unset(obj);
}

EAPI void
elm_object_item_cursor_unset(Elm_Widget_Item *obj)
{
   elm_wdg_item_cursor_unset(obj);
}

EAPI Efl_Canvas_Object *
elm_object_item_part_content_unset(Elm_Widget_Item *obj, const char *part)
{
   return elm_wdg_item_part_content_unset(obj, part);
}

EAPI void
elm_object_item_signal_callback_add(Elm_Widget_Item *obj, const char *emission, const char *source, Elm_Object_Item_Signal_Cb func, void *data)
{
   elm_wdg_item_signal_callback_add(obj, emission, source, func, data);
}

EAPI void *
elm_object_item_signal_callback_del(Elm_Widget_Item *obj, const char *emission, const char *source, Elm_Object_Item_Signal_Cb func)
{
   return elm_wdg_item_signal_callback_del(obj, emission, source, func);
}

EAPI void
elm_object_item_signal_emit(Elm_Widget_Item *obj, const char *emission, const char *source)
{
   elm_wdg_item_signal_emit(obj, emission, source);
}

EAPI void
elm_object_item_access_info_set(Elm_Widget_Item *obj, const char *txt)
{
   elm_wdg_item_access_info_set(obj, txt);
}

EAPI Efl_Canvas_Object *
elm_object_item_access_object_get(const Elm_Widget_Item *obj)
{
   return elm_wdg_item_access_object_get(obj);
}

EAPI void
elm_object_item_domain_translatable_part_text_set(Elm_Widget_Item *obj, const char *part, const char *domain, const char *label)
{
   elm_wdg_item_domain_translatable_part_text_set(obj, part, domain, label);
}

EAPI const char *
elm_object_item_translatable_part_text_get(const Elm_Widget_Item *obj, const char *part)
{
   return elm_wdg_item_translatable_part_text_get(obj, part);
}

EAPI void
elm_object_item_domain_part_text_translatable_set(Elm_Widget_Item *obj, const char *part, const char *domain, Eina_Bool translatable)
{
   elm_wdg_item_domain_part_text_translatable_set(obj, part, domain, translatable);
}

EAPI Efl_Canvas_Object *
elm_object_item_track(Elm_Widget_Item *obj)
{
   return elm_wdg_item_track(obj);
}

EAPI void
elm_object_item_untrack(Elm_Widget_Item *obj)
{
   elm_wdg_item_untrack(obj);
}

EAPI int
elm_object_item_track_get(const Elm_Widget_Item *obj)
{
   return elm_wdg_item_track_get(obj);
}

EAPI void
elm_object_item_del_cb_set(Elm_Widget_Item *obj, Evas_Smart_Cb del_cb)
{
   elm_wdg_item_del_cb_set(obj, del_cb);
}

EAPI void
elm_object_item_tooltip_content_cb_set(Elm_Widget_Item *obj, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb)
{
   elm_wdg_item_tooltip_content_cb_set(obj, func, data, del_cb);
}

EAPI Efl_Canvas_Object *
elm_object_item_access_register(Elm_Widget_Item *obj)
{
   return elm_wdg_item_access_register(obj);
}

EAPI void
elm_object_item_access_unregister(Elm_Widget_Item *obj)
{
   elm_wdg_item_access_unregister(obj);
}

EAPI void
elm_object_item_access_order_unset(Elm_Widget_Item *obj)
{
   elm_wdg_item_access_order_unset(obj);
}

EAPI Efl_Canvas_Object *
elm_object_item_focus_next_object_get(const Elm_Widget_Item *obj, Elm_Focus_Direction dir)
{
   return elm_wdg_item_focus_next_object_get(obj, dir);
}

EAPI void
elm_object_item_focus_next_object_set(Elm_Widget_Item *obj, Efl_Canvas_Object *next, Elm_Focus_Direction dir)
{
   elm_wdg_item_focus_next_object_set(obj, next, dir);
}

EAPI Elm_Widget_Item *
elm_object_item_focus_next_item_get(const Elm_Widget_Item *obj, Elm_Focus_Direction dir)
{
   return elm_wdg_item_focus_next_item_get(obj, dir);
}

EAPI void
elm_object_item_focus_next_item_set(Elm_Widget_Item *obj, Elm_Widget_Item *next_item, Elm_Focus_Direction dir)
{
   elm_wdg_item_focus_next_item_set(obj, next_item, dir);
}
