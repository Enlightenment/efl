EWAPI const Efl_Event_Description _ELM_LABEL_EVENT_SLIDE_END =
   EFL_EVENT_DESCRIPTION("slide,end");

void _elm_label_wrap_width_set(Eo *obj, Elm_Label_Data *pd, int w);


static Eina_Error
__eolian_elm_label_wrap_width_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_label_wrap_width_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_label_wrap_width_set, EFL_FUNC_CALL(w), int w);

int _elm_label_wrap_width_get(const Eo *obj, Elm_Label_Data *pd);


static Eina_Value
__eolian_elm_label_wrap_width_get_reflect(Eo *obj)
{
   int val = elm_obj_label_wrap_width_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_label_wrap_width_get, int, 0);

void _elm_label_slide_speed_set(Eo *obj, Elm_Label_Data *pd, double speed);


static Eina_Error
__eolian_elm_label_slide_speed_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_label_slide_speed_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_label_slide_speed_set, EFL_FUNC_CALL(speed), double speed);

double _elm_label_slide_speed_get(const Eo *obj, Elm_Label_Data *pd);


static Eina_Value
__eolian_elm_label_slide_speed_get_reflect(Eo *obj)
{
   double val = elm_obj_label_slide_speed_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_label_slide_speed_get, double, 0);

void _elm_label_slide_mode_set(Eo *obj, Elm_Label_Data *pd, Elm_Label_Slide_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_label_slide_mode_set, EFL_FUNC_CALL(mode), Elm_Label_Slide_Mode mode);

Elm_Label_Slide_Mode _elm_label_slide_mode_get(const Eo *obj, Elm_Label_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_label_slide_mode_get, Elm_Label_Slide_Mode, 0);

void _elm_label_slide_duration_set(Eo *obj, Elm_Label_Data *pd, double duration);


static Eina_Error
__eolian_elm_label_slide_duration_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   double cval;
   if (!eina_value_double_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_label_slide_duration_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_label_slide_duration_set, EFL_FUNC_CALL(duration), double duration);

double _elm_label_slide_duration_get(const Eo *obj, Elm_Label_Data *pd);


static Eina_Value
__eolian_elm_label_slide_duration_get_reflect(Eo *obj)
{
   double val = elm_obj_label_slide_duration_get(obj);
   return eina_value_double_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_label_slide_duration_get, double, 0);

void _elm_label_line_wrap_set(Eo *obj, Elm_Label_Data *pd, Elm_Wrap_Type wrap);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_label_line_wrap_set, EFL_FUNC_CALL(wrap), Elm_Wrap_Type wrap);

Elm_Wrap_Type _elm_label_line_wrap_get(const Eo *obj, Elm_Label_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_label_line_wrap_get, Elm_Wrap_Type, 0);

void _elm_label_ellipsis_set(Eo *obj, Elm_Label_Data *pd, Eina_Bool ellipsis);


static Eina_Error
__eolian_elm_label_ellipsis_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_label_ellipsis_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_label_ellipsis_set, EFL_FUNC_CALL(ellipsis), Eina_Bool ellipsis);

Eina_Bool _elm_label_ellipsis_get(const Eo *obj, Elm_Label_Data *pd);


static Eina_Value
__eolian_elm_label_ellipsis_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_label_ellipsis_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_label_ellipsis_get, Eina_Bool, 0);

void _elm_label_slide_go(Eo *obj, Elm_Label_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_label_slide_go);

Efl_Object *_elm_label_efl_object_constructor(Eo *obj, Elm_Label_Data *pd);


Eina_Error _elm_label_efl_ui_widget_theme_apply(Eo *obj, Elm_Label_Data *pd);


Efl_Object *_elm_label_efl_part_part_get(const Eo *obj, Elm_Label_Data *pd, const char *name);


static Eina_Bool
_elm_label_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_LABEL_EXTRA_OPS
#define ELM_LABEL_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_label_wrap_width_set, _elm_label_wrap_width_set),
      EFL_OBJECT_OP_FUNC(elm_obj_label_wrap_width_get, _elm_label_wrap_width_get),
      EFL_OBJECT_OP_FUNC(elm_obj_label_slide_speed_set, _elm_label_slide_speed_set),
      EFL_OBJECT_OP_FUNC(elm_obj_label_slide_speed_get, _elm_label_slide_speed_get),
      EFL_OBJECT_OP_FUNC(elm_obj_label_slide_mode_set, _elm_label_slide_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_label_slide_mode_get, _elm_label_slide_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_label_slide_duration_set, _elm_label_slide_duration_set),
      EFL_OBJECT_OP_FUNC(elm_obj_label_slide_duration_get, _elm_label_slide_duration_get),
      EFL_OBJECT_OP_FUNC(elm_obj_label_line_wrap_set, _elm_label_line_wrap_set),
      EFL_OBJECT_OP_FUNC(elm_obj_label_line_wrap_get, _elm_label_line_wrap_get),
      EFL_OBJECT_OP_FUNC(elm_obj_label_ellipsis_set, _elm_label_ellipsis_set),
      EFL_OBJECT_OP_FUNC(elm_obj_label_ellipsis_get, _elm_label_ellipsis_get),
      EFL_OBJECT_OP_FUNC(elm_obj_label_slide_go, _elm_label_slide_go),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_label_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_label_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_label_efl_part_part_get),
      ELM_LABEL_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"wrap_width", __eolian_elm_label_wrap_width_set_reflect, __eolian_elm_label_wrap_width_get_reflect},
      {"slide_speed", __eolian_elm_label_slide_speed_set_reflect, __eolian_elm_label_slide_speed_get_reflect},
      {"slide_duration", __eolian_elm_label_slide_duration_set_reflect, __eolian_elm_label_slide_duration_get_reflect},
      {"ellipsis", __eolian_elm_label_ellipsis_set_reflect, __eolian_elm_label_ellipsis_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_label_class_desc = {
   EO_VERSION,
   "Elm.Label",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Label_Data),
   _elm_label_class_initializer,
   _elm_label_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_label_class_get, &_elm_label_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_label_eo.legacy.c"
