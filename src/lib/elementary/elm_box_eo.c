EWAPI const Efl_Event_Description _ELM_BOX_EVENT_CHILD_ADDED =
   EFL_EVENT_DESCRIPTION("child,added");
EWAPI const Efl_Event_Description _ELM_BOX_EVENT_CHILD_REMOVED =
   EFL_EVENT_DESCRIPTION("child,removed");

void _elm_box_homogeneous_set(Eo *obj, Elm_Box_Data *pd, Eina_Bool homogeneous);


static Eina_Error
__eolian_elm_box_homogeneous_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_box_homogeneous_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_box_homogeneous_set, EFL_FUNC_CALL(homogeneous), Eina_Bool homogeneous);

Eina_Bool _elm_box_homogeneous_get(const Eo *obj, Elm_Box_Data *pd);


static Eina_Value
__eolian_elm_box_homogeneous_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_box_homogeneous_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_box_homogeneous_get, Eina_Bool, 0);

void _elm_box_align_set(Eo *obj, Elm_Box_Data *pd, double horizontal, double vertical);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_box_align_set, EFL_FUNC_CALL(horizontal, vertical), double horizontal, double vertical);

void _elm_box_align_get(const Eo *obj, Elm_Box_Data *pd, double *horizontal, double *vertical);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_box_align_get, EFL_FUNC_CALL(horizontal, vertical), double *horizontal, double *vertical);

void _elm_box_horizontal_set(Eo *obj, Elm_Box_Data *pd, Eina_Bool horizontal);


static Eina_Error
__eolian_elm_box_horizontal_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_box_horizontal_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_box_horizontal_set, EFL_FUNC_CALL(horizontal), Eina_Bool horizontal);

Eina_Bool _elm_box_horizontal_get(const Eo *obj, Elm_Box_Data *pd);


static Eina_Value
__eolian_elm_box_horizontal_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_box_horizontal_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_box_horizontal_get, Eina_Bool, 0);

void _elm_box_padding_set(Eo *obj, Elm_Box_Data *pd, int horizontal, int vertical);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_box_padding_set, EFL_FUNC_CALL(horizontal, vertical), int horizontal, int vertical);

void _elm_box_padding_get(const Eo *obj, Elm_Box_Data *pd, int *horizontal, int *vertical);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_box_padding_get, EFL_FUNC_CALL(horizontal, vertical), int *horizontal, int *vertical);

Eina_List *_elm_box_children_get(const Eo *obj, Elm_Box_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_box_children_get, Eina_List *, NULL);

void _elm_box_pack_end(Eo *obj, Elm_Box_Data *pd, Efl_Canvas_Object *subobj);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_box_pack_end, EFL_FUNC_CALL(subobj), Efl_Canvas_Object *subobj);

void _elm_box_unpack_all(Eo *obj, Elm_Box_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_box_unpack_all);

void _elm_box_unpack(Eo *obj, Elm_Box_Data *pd, Efl_Canvas_Object *subobj);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_box_unpack, EFL_FUNC_CALL(subobj), Efl_Canvas_Object *subobj);

void _elm_box_pack_after(Eo *obj, Elm_Box_Data *pd, Efl_Canvas_Object *subobj, Efl_Canvas_Object *after);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_box_pack_after, EFL_FUNC_CALL(subobj, after), Efl_Canvas_Object *subobj, Efl_Canvas_Object *after);

void _elm_box_pack_start(Eo *obj, Elm_Box_Data *pd, Efl_Canvas_Object *subobj);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_box_pack_start, EFL_FUNC_CALL(subobj), Efl_Canvas_Object *subobj);

void _elm_box_recalculate(Eo *obj, Elm_Box_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_box_recalculate);

void _elm_box_pack_before(Eo *obj, Elm_Box_Data *pd, Efl_Canvas_Object *subobj, Efl_Canvas_Object *before);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_box_pack_before, EFL_FUNC_CALL(subobj, before), Efl_Canvas_Object *subobj, Efl_Canvas_Object *before);

void _elm_box_clear(Eo *obj, Elm_Box_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_box_clear);

Efl_Object *_elm_box_efl_object_constructor(Eo *obj, Elm_Box_Data *pd);


void _elm_box_efl_canvas_group_group_calculate(Eo *obj, Elm_Box_Data *pd);


Eina_Error _elm_box_efl_ui_widget_theme_apply(Eo *obj, Elm_Box_Data *pd);


Eina_Bool _elm_box_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Box_Data *pd, Efl_Canvas_Object *sub_obj);


void _elm_box_efl_ui_focus_composition_prepare(Eo *obj, Elm_Box_Data *pd);


static Eina_Bool
_elm_box_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_BOX_EXTRA_OPS
#define ELM_BOX_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_box_homogeneous_set, _elm_box_homogeneous_set),
      EFL_OBJECT_OP_FUNC(elm_obj_box_homogeneous_get, _elm_box_homogeneous_get),
      EFL_OBJECT_OP_FUNC(elm_obj_box_align_set, _elm_box_align_set),
      EFL_OBJECT_OP_FUNC(elm_obj_box_align_get, _elm_box_align_get),
      EFL_OBJECT_OP_FUNC(elm_obj_box_horizontal_set, _elm_box_horizontal_set),
      EFL_OBJECT_OP_FUNC(elm_obj_box_horizontal_get, _elm_box_horizontal_get),
      EFL_OBJECT_OP_FUNC(elm_obj_box_padding_set, _elm_box_padding_set),
      EFL_OBJECT_OP_FUNC(elm_obj_box_padding_get, _elm_box_padding_get),
      EFL_OBJECT_OP_FUNC(elm_obj_box_children_get, _elm_box_children_get),
      EFL_OBJECT_OP_FUNC(elm_obj_box_pack_end, _elm_box_pack_end),
      EFL_OBJECT_OP_FUNC(elm_obj_box_unpack_all, _elm_box_unpack_all),
      EFL_OBJECT_OP_FUNC(elm_obj_box_unpack, _elm_box_unpack),
      EFL_OBJECT_OP_FUNC(elm_obj_box_pack_after, _elm_box_pack_after),
      EFL_OBJECT_OP_FUNC(elm_obj_box_pack_start, _elm_box_pack_start),
      EFL_OBJECT_OP_FUNC(elm_obj_box_recalculate, _elm_box_recalculate),
      EFL_OBJECT_OP_FUNC(elm_obj_box_pack_before, _elm_box_pack_before),
      EFL_OBJECT_OP_FUNC(elm_obj_box_clear, _elm_box_clear),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_box_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_calculate, _elm_box_efl_canvas_group_group_calculate),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_box_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_del, _elm_box_efl_ui_widget_widget_sub_object_del),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_composition_prepare, _elm_box_efl_ui_focus_composition_prepare),
      ELM_BOX_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"homogeneous", __eolian_elm_box_homogeneous_set_reflect, __eolian_elm_box_homogeneous_get_reflect},
      {"horizontal", __eolian_elm_box_horizontal_set_reflect, __eolian_elm_box_horizontal_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_box_class_desc = {
   EO_VERSION,
   "Elm.Box",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Box_Data),
   _elm_box_class_initializer,
   _elm_box_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_box_class_get, &_elm_box_class_desc, EFL_UI_WIDGET_CLASS, EFL_UI_FOCUS_COMPOSITION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_box_eo.legacy.c"
