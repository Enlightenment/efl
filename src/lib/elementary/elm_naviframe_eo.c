EWAPI const Efl_Event_Description _ELM_NAVIFRAME_EVENT_TRANSITION_FINISHED =
   EFL_EVENT_DESCRIPTION("transition,finished");
EWAPI const Efl_Event_Description _ELM_NAVIFRAME_EVENT_TITLE_TRANSITION_FINISHED =
   EFL_EVENT_DESCRIPTION("title,transition,finished");
EWAPI const Efl_Event_Description _ELM_NAVIFRAME_EVENT_TITLE_CLICKED =
   EFL_EVENT_DESCRIPTION("title,clicked");
EWAPI const Efl_Event_Description _ELM_NAVIFRAME_EVENT_ITEM_ACTIVATED =
   EFL_EVENT_DESCRIPTION("item,activated");

void _elm_naviframe_event_enabled_set(Eo *obj, Elm_Naviframe_Data *pd, Eina_Bool enabled);


static Eina_Error
__eolian_elm_naviframe_event_enabled_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_naviframe_event_enabled_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_naviframe_event_enabled_set, EFL_FUNC_CALL(enabled), Eina_Bool enabled);

Eina_Bool _elm_naviframe_event_enabled_get(const Eo *obj, Elm_Naviframe_Data *pd);


static Eina_Value
__eolian_elm_naviframe_event_enabled_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_naviframe_event_enabled_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_naviframe_event_enabled_get, Eina_Bool, 0);

void _elm_naviframe_content_preserve_on_pop_set(Eo *obj, Elm_Naviframe_Data *pd, Eina_Bool preserve);


static Eina_Error
__eolian_elm_naviframe_content_preserve_on_pop_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_naviframe_content_preserve_on_pop_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_naviframe_content_preserve_on_pop_set, EFL_FUNC_CALL(preserve), Eina_Bool preserve);

Eina_Bool _elm_naviframe_content_preserve_on_pop_get(const Eo *obj, Elm_Naviframe_Data *pd);


static Eina_Value
__eolian_elm_naviframe_content_preserve_on_pop_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_naviframe_content_preserve_on_pop_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_naviframe_content_preserve_on_pop_get, Eina_Bool, 0);

void _elm_naviframe_prev_btn_auto_pushed_set(Eo *obj, Elm_Naviframe_Data *pd, Eina_Bool auto_pushed);


static Eina_Error
__eolian_elm_naviframe_prev_btn_auto_pushed_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_naviframe_prev_btn_auto_pushed_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_naviframe_prev_btn_auto_pushed_set, EFL_FUNC_CALL(auto_pushed), Eina_Bool auto_pushed);

Eina_Bool _elm_naviframe_prev_btn_auto_pushed_get(const Eo *obj, Elm_Naviframe_Data *pd);


static Eina_Value
__eolian_elm_naviframe_prev_btn_auto_pushed_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_naviframe_prev_btn_auto_pushed_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_naviframe_prev_btn_auto_pushed_get, Eina_Bool, 0);

Eina_List *_elm_naviframe_items_get(const Eo *obj, Elm_Naviframe_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_naviframe_items_get, Eina_List *, NULL);

Elm_Widget_Item *_elm_naviframe_top_item_get(const Eo *obj, Elm_Naviframe_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_naviframe_top_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_naviframe_bottom_item_get(const Eo *obj, Elm_Naviframe_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_naviframe_bottom_item_get, Elm_Widget_Item *, NULL);

Efl_Canvas_Object *_elm_naviframe_item_pop(Eo *obj, Elm_Naviframe_Data *pd);

EOAPI EFL_FUNC_BODY(elm_obj_naviframe_item_pop, Efl_Canvas_Object *, NULL);

Elm_Widget_Item *_elm_naviframe_item_insert_before(Eo *obj, Elm_Naviframe_Data *pd, Elm_Widget_Item *before, const char *title_label, Efl_Canvas_Object *prev_btn, Efl_Canvas_Object *next_btn, Efl_Canvas_Object *content, const char *item_style);

EOAPI EFL_FUNC_BODYV(elm_obj_naviframe_item_insert_before, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(before, title_label, prev_btn, next_btn, content, item_style), Elm_Widget_Item *before, const char *title_label, Efl_Canvas_Object *prev_btn, Efl_Canvas_Object *next_btn, Efl_Canvas_Object *content, const char *item_style);

Elm_Widget_Item *_elm_naviframe_item_push(Eo *obj, Elm_Naviframe_Data *pd, const char *title_label, Efl_Canvas_Object *prev_btn, Efl_Canvas_Object *next_btn, Efl_Canvas_Object *content, const char *item_style);

EOAPI EFL_FUNC_BODYV(elm_obj_naviframe_item_push, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(title_label, prev_btn, next_btn, content, item_style), const char *title_label, Efl_Canvas_Object *prev_btn, Efl_Canvas_Object *next_btn, Efl_Canvas_Object *content, const char *item_style);

void _elm_naviframe_item_simple_promote(Eo *obj, Elm_Naviframe_Data *pd, Efl_Canvas_Object *content);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_naviframe_item_simple_promote, EFL_FUNC_CALL(content), Efl_Canvas_Object *content);

Elm_Widget_Item *_elm_naviframe_item_insert_after(Eo *obj, Elm_Naviframe_Data *pd, Elm_Widget_Item *after, const char *title_label, Efl_Canvas_Object *prev_btn, Efl_Canvas_Object *next_btn, Efl_Canvas_Object *content, const char *item_style);

EOAPI EFL_FUNC_BODYV(elm_obj_naviframe_item_insert_after, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(after, title_label, prev_btn, next_btn, content, item_style), Elm_Widget_Item *after, const char *title_label, Efl_Canvas_Object *prev_btn, Efl_Canvas_Object *next_btn, Efl_Canvas_Object *content, const char *item_style);

Efl_Object *_elm_naviframe_efl_object_constructor(Eo *obj, Elm_Naviframe_Data *pd);


void _elm_naviframe_efl_gfx_entity_visible_set(Eo *obj, Elm_Naviframe_Data *pd, Eina_Bool v);


void _elm_naviframe_efl_ui_widget_on_access_update(Eo *obj, Elm_Naviframe_Data *pd, Eina_Bool enable);


void _elm_naviframe_efl_ui_l10n_translation_update(Eo *obj, Elm_Naviframe_Data *pd);


Eina_Error _elm_naviframe_efl_ui_widget_theme_apply(Eo *obj, Elm_Naviframe_Data *pd);


Eina_Bool _elm_naviframe_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Naviframe_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


void _elm_naviframe_efl_layout_signal_signal_emit(Eo *obj, Elm_Naviframe_Data *pd, const char *emission, const char *source);


const Efl_Access_Action_Data *_elm_naviframe_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Naviframe_Data *pd);


void _elm_naviframe_efl_ui_widget_resize_object_set(Eo *obj, Elm_Naviframe_Data *pd, Efl_Canvas_Object *sobj);


Efl_Object *_elm_naviframe_efl_part_part_get(const Eo *obj, Elm_Naviframe_Data *pd, const char *name);


static Eina_Bool
_elm_naviframe_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_NAVIFRAME_EXTRA_OPS
#define ELM_NAVIFRAME_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_event_enabled_set, _elm_naviframe_event_enabled_set),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_event_enabled_get, _elm_naviframe_event_enabled_get),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_content_preserve_on_pop_set, _elm_naviframe_content_preserve_on_pop_set),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_content_preserve_on_pop_get, _elm_naviframe_content_preserve_on_pop_get),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_prev_btn_auto_pushed_set, _elm_naviframe_prev_btn_auto_pushed_set),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_prev_btn_auto_pushed_get, _elm_naviframe_prev_btn_auto_pushed_get),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_items_get, _elm_naviframe_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_top_item_get, _elm_naviframe_top_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_bottom_item_get, _elm_naviframe_bottom_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_item_pop, _elm_naviframe_item_pop),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_item_insert_before, _elm_naviframe_item_insert_before),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_item_push, _elm_naviframe_item_push),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_item_simple_promote, _elm_naviframe_item_simple_promote),
      EFL_OBJECT_OP_FUNC(elm_obj_naviframe_item_insert_after, _elm_naviframe_item_insert_after),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_naviframe_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_visible_set, _elm_naviframe_efl_gfx_entity_visible_set),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_naviframe_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_translation_update, _elm_naviframe_efl_ui_l10n_translation_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_naviframe_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_naviframe_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_layout_signal_emit, _elm_naviframe_efl_layout_signal_signal_emit),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_naviframe_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_resize_object_set, _elm_naviframe_efl_ui_widget_resize_object_set),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_naviframe_efl_part_part_get),
      ELM_NAVIFRAME_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"event_enabled", __eolian_elm_naviframe_event_enabled_set_reflect, __eolian_elm_naviframe_event_enabled_get_reflect},
      {"content_preserve_on_pop", __eolian_elm_naviframe_content_preserve_on_pop_set_reflect, __eolian_elm_naviframe_content_preserve_on_pop_get_reflect},
      {"prev_btn_auto_pushed", __eolian_elm_naviframe_prev_btn_auto_pushed_set_reflect, __eolian_elm_naviframe_prev_btn_auto_pushed_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_naviframe_class_desc = {
   EO_VERSION,
   "Elm.Naviframe",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Naviframe_Data),
   _elm_naviframe_class_initializer,
   _elm_naviframe_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_naviframe_class_get, &_elm_naviframe_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_naviframe_eo.legacy.c"
