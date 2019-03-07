
Eina_Bool _elm_widget_item_tooltip_window_mode_set(Eo *obj, Elm_Widget_Item_Data *pd, Eina_Bool disable);


static Eina_Error
__eolian_elm_widget_item_tooltip_window_mode_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_wdg_item_tooltip_window_mode_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_FUNC_BODYV(elm_wdg_item_tooltip_window_mode_set, Eina_Bool, 0, EFL_FUNC_CALL(disable), Eina_Bool disable);

Eina_Bool _elm_widget_item_tooltip_window_mode_get(const Eo *obj, Elm_Widget_Item_Data *pd);


static Eina_Value
__eolian_elm_widget_item_tooltip_window_mode_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_wdg_item_tooltip_window_mode_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_wdg_item_tooltip_window_mode_get, Eina_Bool, 0);

void _elm_widget_item_tooltip_style_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *style);


static Eina_Error
__eolian_elm_widget_item_tooltip_style_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_wdg_item_tooltip_style_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_tooltip_style_set, EFL_FUNC_CALL(style), const char *style);

const char *_elm_widget_item_tooltip_style_get(const Eo *obj, Elm_Widget_Item_Data *pd);


static Eina_Value
__eolian_elm_widget_item_tooltip_style_get_reflect(Eo *obj)
{
   const char *val = elm_wdg_item_tooltip_style_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_wdg_item_tooltip_style_get, const char *, NULL);

void _elm_widget_item_cursor_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *cursor);


static Eina_Error
__eolian_elm_widget_item_cursor_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_wdg_item_cursor_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_cursor_set, EFL_FUNC_CALL(cursor), const char *cursor);

const char *_elm_widget_item_cursor_get(const Eo *obj, Elm_Widget_Item_Data *pd);


static Eina_Value
__eolian_elm_widget_item_cursor_get_reflect(Eo *obj)
{
   const char *val = elm_wdg_item_cursor_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_wdg_item_cursor_get, const char *, NULL);

void _elm_widget_item_cursor_style_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *style);


static Eina_Error
__eolian_elm_widget_item_cursor_style_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_wdg_item_cursor_style_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_cursor_style_set, EFL_FUNC_CALL(style), const char *style);

const char *_elm_widget_item_cursor_style_get(const Eo *obj, Elm_Widget_Item_Data *pd);


static Eina_Value
__eolian_elm_widget_item_cursor_style_get_reflect(Eo *obj)
{
   const char *val = elm_wdg_item_cursor_style_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_wdg_item_cursor_style_get, const char *, NULL);

void _elm_widget_item_cursor_engine_only_set(Eo *obj, Elm_Widget_Item_Data *pd, Eina_Bool engine_only);


static Eina_Error
__eolian_elm_widget_item_cursor_engine_only_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_wdg_item_cursor_engine_only_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_cursor_engine_only_set, EFL_FUNC_CALL(engine_only), Eina_Bool engine_only);

Eina_Bool _elm_widget_item_cursor_engine_only_get(const Eo *obj, Elm_Widget_Item_Data *pd);


static Eina_Value
__eolian_elm_widget_item_cursor_engine_only_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_wdg_item_cursor_engine_only_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_wdg_item_cursor_engine_only_get, Eina_Bool, 0);

void _elm_widget_item_part_content_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *part, Efl_Canvas_Object *content);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_part_content_set, EFL_FUNC_CALL(part, content), const char *part, Efl_Canvas_Object *content);

Efl_Canvas_Object *_elm_widget_item_part_content_get(const Eo *obj, Elm_Widget_Item_Data *pd, const char *part);

EOAPI EFL_FUNC_BODYV_CONST(elm_wdg_item_part_content_get, Efl_Canvas_Object *, NULL, EFL_FUNC_CALL(part), const char *part);

void _elm_widget_item_part_text_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *part, const char *label);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_part_text_set, EFL_FUNC_CALL(part, label), const char *part, const char *label);

const char *_elm_widget_item_part_text_get(const Eo *obj, Elm_Widget_Item_Data *pd, const char *part);

EOAPI EFL_FUNC_BODYV_CONST(elm_wdg_item_part_text_get, const char *, NULL, EFL_FUNC_CALL(part), const char *part);

void _elm_widget_item_part_text_custom_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *part, const char *label);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_part_text_custom_set, EFL_FUNC_CALL(part, label), const char *part, const char *label);

const char *_elm_widget_item_part_text_custom_get(const Eo *obj, Elm_Widget_Item_Data *pd, const char *part);

EOAPI EFL_FUNC_BODYV_CONST(elm_wdg_item_part_text_custom_get, const char *, NULL, EFL_FUNC_CALL(part), const char *part);

void _elm_widget_item_item_focus_set(Eo *obj, Elm_Widget_Item_Data *pd, Eina_Bool focused);


static Eina_Error
__eolian_elm_widget_item_item_focus_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_wdg_item_focus_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_focus_set, EFL_FUNC_CALL(focused), Eina_Bool focused);

Eina_Bool _elm_widget_item_item_focus_get(const Eo *obj, Elm_Widget_Item_Data *pd);


static Eina_Value
__eolian_elm_widget_item_item_focus_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_wdg_item_focus_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_wdg_item_focus_get, Eina_Bool, 0);

void _elm_widget_item_style_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *style);


static Eina_Error
__eolian_elm_widget_item_style_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_wdg_item_style_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_style_set, EFL_FUNC_CALL(style), const char *style);

const char *_elm_widget_item_style_get(const Eo *obj, Elm_Widget_Item_Data *pd);


static Eina_Value
__eolian_elm_widget_item_style_get_reflect(Eo *obj)
{
   const char *val = elm_wdg_item_style_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_wdg_item_style_get, const char *, NULL);

void _elm_widget_item_disabled_set(Eo *obj, Elm_Widget_Item_Data *pd, Eina_Bool disable);


static Eina_Error
__eolian_elm_widget_item_disabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_wdg_item_disabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_disabled_set, EFL_FUNC_CALL(disable), Eina_Bool disable);

Eina_Bool _elm_widget_item_disabled_get(const Eo *obj, Elm_Widget_Item_Data *pd);


static Eina_Value
__eolian_elm_widget_item_disabled_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_wdg_item_disabled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_wdg_item_disabled_get, Eina_Bool, 0);

const Eina_List *_elm_widget_item_access_order_get(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_FUNC_BODY(elm_wdg_item_access_order_get, const Eina_List *, NULL);

void _elm_widget_item_access_order_set(Eo *obj, Elm_Widget_Item_Data *pd, Eina_List *objs);

static void
_elm_wdg_item_access_order_set_ownership_fallback(Eina_List *objs)
{
   eina_list_free(objs);
}

EOAPI EFL_VOID_FUNC_BODYV_FALLBACK(elm_wdg_item_access_order_set, _elm_wdg_item_access_order_set_ownership_fallback(objs);, EFL_FUNC_CALL(objs), Eina_List *objs);

Efl_Canvas_Object *_elm_widget_item_widget_get(const Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_wdg_item_widget_get, Efl_Canvas_Object *, NULL);

void _elm_widget_item_pre_notify_del(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_wdg_item_pre_notify_del);

void _elm_widget_item_tooltip_text_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *text);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_tooltip_text_set, EFL_FUNC_CALL(text), const char *text);

void _elm_widget_item_tooltip_translatable_text_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *text);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_tooltip_translatable_text_set, EFL_FUNC_CALL(text), const char *text);

void _elm_widget_item_tooltip_unset(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_wdg_item_tooltip_unset);

void _elm_widget_item_cursor_unset(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_wdg_item_cursor_unset);

Efl_Canvas_Object *_elm_widget_item_part_content_unset(Eo *obj, Elm_Widget_Item_Data *pd, const char *part);

EOAPI EFL_FUNC_BODYV(elm_wdg_item_part_content_unset, Efl_Canvas_Object *, NULL, EFL_FUNC_CALL(part), const char *part);

void _elm_widget_item_part_text_custom_update(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_wdg_item_part_text_custom_update);

void _elm_widget_item_signal_callback_add(Eo *obj, Elm_Widget_Item_Data *pd, const char *emission, const char *source, Elm_Object_Item_Signal_Cb func, void *data);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_signal_callback_add, EFL_FUNC_CALL(emission, source, func, data), const char *emission, const char *source, Elm_Object_Item_Signal_Cb func, void *data);

void *_elm_widget_item_signal_callback_del(Eo *obj, Elm_Widget_Item_Data *pd, const char *emission, const char *source, Elm_Object_Item_Signal_Cb func);

EOAPI EFL_FUNC_BODYV(elm_wdg_item_signal_callback_del, void *, NULL, EFL_FUNC_CALL(emission, source, func), const char *emission, const char *source, Elm_Object_Item_Signal_Cb func);

void _elm_widget_item_signal_emit(Eo *obj, Elm_Widget_Item_Data *pd, const char *emission, const char *source);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_signal_emit, EFL_FUNC_CALL(emission, source), const char *emission, const char *source);

void _elm_widget_item_access_info_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *txt);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_access_info_set, EFL_FUNC_CALL(txt), const char *txt);

Efl_Canvas_Object *_elm_widget_item_access_object_get(const Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_wdg_item_access_object_get, Efl_Canvas_Object *, NULL);

void _elm_widget_item_domain_translatable_part_text_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *part, const char *domain, const char *label);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_domain_translatable_part_text_set, EFL_FUNC_CALL(part, domain, label), const char *part, const char *domain, const char *label);

const char *_elm_widget_item_translatable_part_text_get(const Eo *obj, Elm_Widget_Item_Data *pd, const char *part);

EOAPI EFL_FUNC_BODYV_CONST(elm_wdg_item_translatable_part_text_get, const char *, NULL, EFL_FUNC_CALL(part), const char *part);

void _elm_widget_item_translate(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_wdg_item_translate);

void _elm_widget_item_domain_part_text_translatable_set(Eo *obj, Elm_Widget_Item_Data *pd, const char *part, const char *domain, Eina_Bool translatable);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_domain_part_text_translatable_set, EFL_FUNC_CALL(part, domain, translatable), const char *part, const char *domain, Eina_Bool translatable);

Efl_Canvas_Object *_elm_widget_item_track(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_FUNC_BODY(elm_wdg_item_track, Efl_Canvas_Object *, NULL);

void _elm_widget_item_untrack(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_wdg_item_untrack);

int _elm_widget_item_track_get(const Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_wdg_item_track_get, int, 0);

void _elm_widget_item_track_cancel(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_wdg_item_track_cancel);

void _elm_widget_item_del_cb_set(Eo *obj, Elm_Widget_Item_Data *pd, Evas_Smart_Cb del_cb);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_del_cb_set, EFL_FUNC_CALL(del_cb), Evas_Smart_Cb del_cb);

void _elm_widget_item_tooltip_content_cb_set(Eo *obj, Elm_Widget_Item_Data *pd, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_tooltip_content_cb_set, EFL_FUNC_CALL(func, data, del_cb), Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

Efl_Canvas_Object *_elm_widget_item_access_register(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_FUNC_BODY(elm_wdg_item_access_register, Efl_Canvas_Object *, NULL);

void _elm_widget_item_access_unregister(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_wdg_item_access_unregister);

void _elm_widget_item_access_order_unset(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_wdg_item_access_order_unset);

void _elm_widget_item_disable(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_wdg_item_disable);

void _elm_widget_item_del_pre(Eo *obj, Elm_Widget_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_wdg_item_del_pre);

Efl_Canvas_Object *_elm_widget_item_focus_next_object_get(const Eo *obj, Elm_Widget_Item_Data *pd, Elm_Focus_Direction dir);

EOAPI EFL_FUNC_BODYV_CONST(elm_wdg_item_focus_next_object_get, Efl_Canvas_Object *, NULL, EFL_FUNC_CALL(dir), Elm_Focus_Direction dir);

void _elm_widget_item_focus_next_object_set(Eo *obj, Elm_Widget_Item_Data *pd, Efl_Canvas_Object *next, Elm_Focus_Direction dir);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_focus_next_object_set, EFL_FUNC_CALL(next, dir), Efl_Canvas_Object *next, Elm_Focus_Direction dir);

Elm_Widget_Item *_elm_widget_item_focus_next_item_get(const Eo *obj, Elm_Widget_Item_Data *pd, Elm_Focus_Direction dir);

EOAPI EFL_FUNC_BODYV_CONST(elm_wdg_item_focus_next_item_get, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(dir), Elm_Focus_Direction dir);

void _elm_widget_item_focus_next_item_set(Eo *obj, Elm_Widget_Item_Data *pd, Elm_Widget_Item *next_item, Elm_Focus_Direction dir);

EOAPI EFL_VOID_FUNC_BODYV(elm_wdg_item_focus_next_item_set, EFL_FUNC_CALL(next_item, dir), Elm_Widget_Item *next_item, Elm_Focus_Direction dir);

Efl_Object *_elm_widget_item_efl_object_constructor(Eo *obj, Elm_Widget_Item_Data *pd);


void _elm_widget_item_efl_object_destructor(Eo *obj, Elm_Widget_Item_Data *pd);


void _elm_widget_item_efl_object_invalidate(Eo *obj, Elm_Widget_Item_Data *pd);


Efl_Access_State_Set _elm_widget_item_efl_access_object_state_set_get(const Eo *obj, Elm_Widget_Item_Data *pd);


Eina_List *_elm_widget_item_efl_access_object_attributes_get(const Eo *obj, Elm_Widget_Item_Data *pd);


Eina_Bool _elm_widget_item_efl_access_component_extents_set(Eo *obj, Elm_Widget_Item_Data *pd, Eina_Bool screen_coords, Eina_Rect rect);


Eina_Rect _elm_widget_item_efl_access_component_extents_get(const Eo *obj, Elm_Widget_Item_Data *pd, Eina_Bool screen_coords);


Eina_Bool _elm_widget_item_efl_access_component_focus_grab(Eo *obj, Elm_Widget_Item_Data *pd);


static Eina_Bool
_elm_widget_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_WIDGET_ITEM_EXTRA_OPS
#define ELM_WIDGET_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_window_mode_set, _elm_widget_item_tooltip_window_mode_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_window_mode_get, _elm_widget_item_tooltip_window_mode_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_style_set, _elm_widget_item_tooltip_style_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_style_get, _elm_widget_item_tooltip_style_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_cursor_set, _elm_widget_item_cursor_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_cursor_get, _elm_widget_item_cursor_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_cursor_style_set, _elm_widget_item_cursor_style_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_cursor_style_get, _elm_widget_item_cursor_style_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_cursor_engine_only_set, _elm_widget_item_cursor_engine_only_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_cursor_engine_only_get, _elm_widget_item_cursor_engine_only_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_set, _elm_widget_item_part_content_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_get, _elm_widget_item_part_content_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_set, _elm_widget_item_part_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_get, _elm_widget_item_part_text_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_custom_set, _elm_widget_item_part_text_custom_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_custom_get, _elm_widget_item_part_text_custom_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_set, _elm_widget_item_item_focus_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_get, _elm_widget_item_item_focus_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_style_set, _elm_widget_item_style_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_style_get, _elm_widget_item_style_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_disabled_set, _elm_widget_item_disabled_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_disabled_get, _elm_widget_item_disabled_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_access_order_get, _elm_widget_item_access_order_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_access_order_set, _elm_widget_item_access_order_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_widget_get, _elm_widget_item_widget_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_pre_notify_del, _elm_widget_item_pre_notify_del),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_text_set, _elm_widget_item_tooltip_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_translatable_text_set, _elm_widget_item_tooltip_translatable_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_unset, _elm_widget_item_tooltip_unset),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_cursor_unset, _elm_widget_item_cursor_unset),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_unset, _elm_widget_item_part_content_unset),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_custom_update, _elm_widget_item_part_text_custom_update),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_callback_add, _elm_widget_item_signal_callback_add),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_callback_del, _elm_widget_item_signal_callback_del),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_emit, _elm_widget_item_signal_emit),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_access_info_set, _elm_widget_item_access_info_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_access_object_get, _elm_widget_item_access_object_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_domain_translatable_part_text_set, _elm_widget_item_domain_translatable_part_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_translatable_part_text_get, _elm_widget_item_translatable_part_text_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_translate, _elm_widget_item_translate),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_domain_part_text_translatable_set, _elm_widget_item_domain_part_text_translatable_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_track, _elm_widget_item_track),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_untrack, _elm_widget_item_untrack),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_track_get, _elm_widget_item_track_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_track_cancel, _elm_widget_item_track_cancel),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_del_cb_set, _elm_widget_item_del_cb_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_content_cb_set, _elm_widget_item_tooltip_content_cb_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_access_register, _elm_widget_item_access_register),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_access_unregister, _elm_widget_item_access_unregister),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_access_order_unset, _elm_widget_item_access_order_unset),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_disable, _elm_widget_item_disable),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_del_pre, _elm_widget_item_del_pre),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_next_object_get, _elm_widget_item_focus_next_object_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_next_object_set, _elm_widget_item_focus_next_object_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_next_item_get, _elm_widget_item_focus_next_item_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_next_item_set, _elm_widget_item_focus_next_item_set),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_widget_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_widget_item_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_invalidate, _elm_widget_item_efl_object_invalidate),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_widget_item_efl_access_object_state_set_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_attributes_get, _elm_widget_item_efl_access_object_attributes_get),
      EFL_OBJECT_OP_FUNC(efl_access_component_extents_set, _elm_widget_item_efl_access_component_extents_set),
      EFL_OBJECT_OP_FUNC(efl_access_component_extents_get, _elm_widget_item_efl_access_component_extents_get),
      EFL_OBJECT_OP_FUNC(efl_access_component_focus_grab, _elm_widget_item_efl_access_component_focus_grab),
      ELM_WIDGET_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"tooltip_window_mode", __eolian_elm_widget_item_tooltip_window_mode_set_reflect, __eolian_elm_widget_item_tooltip_window_mode_get_reflect},
      {"tooltip_style", __eolian_elm_widget_item_tooltip_style_set_reflect, __eolian_elm_widget_item_tooltip_style_get_reflect},
      {"cursor", __eolian_elm_widget_item_cursor_set_reflect, __eolian_elm_widget_item_cursor_get_reflect},
      {"cursor_style", __eolian_elm_widget_item_cursor_style_set_reflect, __eolian_elm_widget_item_cursor_style_get_reflect},
      {"cursor_engine_only", __eolian_elm_widget_item_cursor_engine_only_set_reflect, __eolian_elm_widget_item_cursor_engine_only_get_reflect},
      {"item_focus", __eolian_elm_widget_item_item_focus_set_reflect, __eolian_elm_widget_item_item_focus_get_reflect},
      {"style", __eolian_elm_widget_item_style_set_reflect, __eolian_elm_widget_item_style_get_reflect},
      {"disabled", __eolian_elm_widget_item_disabled_set_reflect, __eolian_elm_widget_item_disabled_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_widget_item_class_desc = {
   EO_VERSION,
   "Elm.Widget.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Widget_Item_Data),
   _elm_widget_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_widget_item_class_get, &_elm_widget_item_class_desc, EFL_OBJECT_CLASS, EFL_ACCESS_OBJECT_MIXIN, EFL_ACCESS_COMPONENT_MIXIN, NULL);

#include "elm_widget_item_eo.legacy.c"
