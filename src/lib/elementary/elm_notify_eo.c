EWAPI const Efl_Event_Description _ELM_NOTIFY_EVENT_BLOCK_CLICKED =
   EFL_EVENT_DESCRIPTION("block,clicked");
EWAPI const Efl_Event_Description _ELM_NOTIFY_EVENT_TIMEOUT =
   EFL_EVENT_DESCRIPTION("timeout");
EWAPI const Efl_Event_Description _ELM_NOTIFY_EVENT_DISMISSED =
   EFL_EVENT_DESCRIPTION("dismissed");

void _elm_notify_align_set(Eo *obj, Elm_Notify_Data *pd, double horizontal, double vertical);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_notify_align_set, EFL_FUNC_CALL(horizontal, vertical), double horizontal, double vertical);

void _elm_notify_align_get(const Eo *obj, Elm_Notify_Data *pd, double *horizontal, double *vertical);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_notify_align_get, EFL_FUNC_CALL(horizontal, vertical), double *horizontal, double *vertical);

void _elm_notify_allow_events_set(Eo *obj, Elm_Notify_Data *pd, Eina_Bool allow);


static Eina_Error
__eolian_elm_notify_allow_events_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_notify_allow_events_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_notify_allow_events_set, EFL_FUNC_CALL(allow), Eina_Bool allow);

Eina_Bool _elm_notify_allow_events_get(const Eo *obj, Elm_Notify_Data *pd);


static Eina_Value
__eolian_elm_notify_allow_events_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_notify_allow_events_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_notify_allow_events_get, Eina_Bool, 0);

void _elm_notify_timeout_set(Eo *obj, Elm_Notify_Data *pd, double timeout);


static Eina_Error
__eolian_elm_notify_timeout_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_notify_timeout_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_notify_timeout_set, EFL_FUNC_CALL(timeout), double timeout);

double _elm_notify_timeout_get(const Eo *obj, Elm_Notify_Data *pd);


static Eina_Value
__eolian_elm_notify_timeout_get_reflect(const Eo *obj)
{
   double val = elm_obj_notify_timeout_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_notify_timeout_get, double, 0);

void _elm_notify_dismiss(Eo *obj, Elm_Notify_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_notify_dismiss);

Efl_Object *_elm_notify_efl_object_constructor(Eo *obj, Elm_Notify_Data *pd);


void _elm_notify_efl_gfx_entity_visible_set(Eo *obj, Elm_Notify_Data *pd, Eina_Bool v);


void _elm_notify_efl_gfx_entity_position_set(Eo *obj, Elm_Notify_Data *pd, Eina_Position2D pos);


void _elm_notify_efl_gfx_entity_size_set(Eo *obj, Elm_Notify_Data *pd, Eina_Size2D size);


void _elm_notify_efl_ui_widget_widget_parent_set(Eo *obj, Elm_Notify_Data *pd, Efl_Ui_Widget *parent);


Efl_Ui_Widget *_elm_notify_efl_ui_widget_widget_parent_get(const Eo *obj, Elm_Notify_Data *pd);


Eina_Error _elm_notify_efl_ui_widget_theme_apply(Eo *obj, Elm_Notify_Data *pd);


Eina_Bool _elm_notify_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Notify_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Bool _elm_notify_efl_content_content_set(Eo *obj, Elm_Notify_Data *pd, Efl_Gfx_Entity *content);


Efl_Gfx_Entity *_elm_notify_efl_content_content_get(const Eo *obj, Elm_Notify_Data *pd);


Efl_Gfx_Entity *_elm_notify_efl_content_content_unset(Eo *obj, Elm_Notify_Data *pd);


Efl_Object *_elm_notify_efl_part_part_get(const Eo *obj, Elm_Notify_Data *pd, const char *name);


static Eina_Bool
_elm_notify_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_NOTIFY_EXTRA_OPS
#define ELM_NOTIFY_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_notify_align_set, _elm_notify_align_set),
      EFL_OBJECT_OP_FUNC(elm_obj_notify_align_get, _elm_notify_align_get),
      EFL_OBJECT_OP_FUNC(elm_obj_notify_allow_events_set, _elm_notify_allow_events_set),
      EFL_OBJECT_OP_FUNC(elm_obj_notify_allow_events_get, _elm_notify_allow_events_get),
      EFL_OBJECT_OP_FUNC(elm_obj_notify_timeout_set, _elm_notify_timeout_set),
      EFL_OBJECT_OP_FUNC(elm_obj_notify_timeout_get, _elm_notify_timeout_get),
      EFL_OBJECT_OP_FUNC(elm_obj_notify_dismiss, _elm_notify_dismiss),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_notify_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_visible_set, _elm_notify_efl_gfx_entity_visible_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _elm_notify_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_notify_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_parent_set, _elm_notify_efl_ui_widget_widget_parent_set),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_parent_get, _elm_notify_efl_ui_widget_widget_parent_get),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_notify_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_del, _elm_notify_efl_ui_widget_widget_sub_object_del),
      EFL_OBJECT_OP_FUNC(efl_content_set, _elm_notify_efl_content_content_set),
      EFL_OBJECT_OP_FUNC(efl_content_get, _elm_notify_efl_content_content_get),
      EFL_OBJECT_OP_FUNC(efl_content_unset, _elm_notify_efl_content_content_unset),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_notify_efl_part_part_get),
      ELM_NOTIFY_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"allow_events", __eolian_elm_notify_allow_events_set_reflect, __eolian_elm_notify_allow_events_get_reflect},
      {"timeout", __eolian_elm_notify_timeout_set_reflect, __eolian_elm_notify_timeout_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_notify_class_desc = {
   EO_VERSION,
   "Elm.Notify",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Notify_Data),
   _elm_notify_class_initializer,
   _elm_notify_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_notify_class_get, &_elm_notify_class_desc, EFL_UI_WIDGET_CLASS, EFL_UI_FOCUS_LAYER_MIXIN, EFL_CONTENT_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_notify_eo.legacy.c"
