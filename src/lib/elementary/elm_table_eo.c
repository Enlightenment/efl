
void _elm_table_homogeneous_set(Eo *obj, void *pd, Eina_Bool homogeneous);


static Eina_Error
__eolian_elm_table_homogeneous_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_table_homogeneous_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_table_homogeneous_set, EFL_FUNC_CALL(homogeneous), Eina_Bool homogeneous);

Eina_Bool _elm_table_homogeneous_get(const Eo *obj, void *pd);


static Eina_Value
__eolian_elm_table_homogeneous_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_table_homogeneous_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_table_homogeneous_get, Eina_Bool, 0);

void _elm_table_padding_set(Eo *obj, void *pd, int horizontal, int vertical);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_table_padding_set, EFL_FUNC_CALL(horizontal, vertical), int horizontal, int vertical);

void _elm_table_padding_get(const Eo *obj, void *pd, int *horizontal, int *vertical);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_table_padding_get, EFL_FUNC_CALL(horizontal, vertical), int *horizontal, int *vertical);

void _elm_table_align_set(Eo *obj, void *pd, double horizontal, double vertical);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_table_align_set, EFL_FUNC_CALL(horizontal, vertical), double horizontal, double vertical);

void _elm_table_align_get(const Eo *obj, void *pd, double *horizontal, double *vertical);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_table_align_get, EFL_FUNC_CALL(horizontal, vertical), double *horizontal, double *vertical);

void _elm_table_clear(Eo *obj, void *pd, Eina_Bool clear);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_table_clear, EFL_FUNC_CALL(clear), Eina_Bool clear);

Efl_Canvas_Object *_elm_table_child_get(const Eo *obj, void *pd, int col, int row);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_table_child_get, Efl_Canvas_Object *, NULL, EFL_FUNC_CALL(col, row), int col, int row);

void _elm_table_pack_set(Eo *obj, void *pd, Efl_Canvas_Object *subobj, int column, int row, int colspan, int rowspan);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_table_pack_set, EFL_FUNC_CALL(subobj, column, row, colspan, rowspan), Efl_Canvas_Object *subobj, int column, int row, int colspan, int rowspan);

void _elm_table_pack_get(Eo *obj, void *pd, Efl_Canvas_Object *subobj, int *column, int *row, int *colspan, int *rowspan);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_table_pack_get, EFL_FUNC_CALL(subobj, column, row, colspan, rowspan), Efl_Canvas_Object *subobj, int *column, int *row, int *colspan, int *rowspan);

void _elm_table_unpack(Eo *obj, void *pd, Efl_Canvas_Object *subobj);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_table_unpack, EFL_FUNC_CALL(subobj), Efl_Canvas_Object *subobj);

void _elm_table_pack(Eo *obj, void *pd, Efl_Canvas_Object *subobj, int column, int row, int colspan, int rowspan);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_table_pack, EFL_FUNC_CALL(subobj, column, row, colspan, rowspan), Efl_Canvas_Object *subobj, int column, int row, int colspan, int rowspan);

Efl_Object *_elm_table_efl_object_constructor(Eo *obj, void *pd);


void _elm_table_efl_canvas_group_group_calculate(Eo *obj, void *pd);


Eina_Error _elm_table_efl_ui_widget_theme_apply(Eo *obj, void *pd);


Eina_Bool _elm_table_efl_ui_widget_widget_sub_object_del(Eo *obj, void *pd, Efl_Canvas_Object *sub_obj);


void _elm_table_efl_ui_focus_composition_prepare(Eo *obj, void *pd);


static Eina_Bool
_elm_table_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_TABLE_EXTRA_OPS
#define ELM_TABLE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_table_homogeneous_set, _elm_table_homogeneous_set),
      EFL_OBJECT_OP_FUNC(elm_obj_table_homogeneous_get, _elm_table_homogeneous_get),
      EFL_OBJECT_OP_FUNC(elm_obj_table_padding_set, _elm_table_padding_set),
      EFL_OBJECT_OP_FUNC(elm_obj_table_padding_get, _elm_table_padding_get),
      EFL_OBJECT_OP_FUNC(elm_obj_table_align_set, _elm_table_align_set),
      EFL_OBJECT_OP_FUNC(elm_obj_table_align_get, _elm_table_align_get),
      EFL_OBJECT_OP_FUNC(elm_obj_table_clear, _elm_table_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_table_child_get, _elm_table_child_get),
      EFL_OBJECT_OP_FUNC(elm_obj_table_pack_set, _elm_table_pack_set),
      EFL_OBJECT_OP_FUNC(elm_obj_table_pack_get, _elm_table_pack_get),
      EFL_OBJECT_OP_FUNC(elm_obj_table_unpack, _elm_table_unpack),
      EFL_OBJECT_OP_FUNC(elm_obj_table_pack, _elm_table_pack),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_table_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_calculate, _elm_table_efl_canvas_group_group_calculate),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_table_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_del, _elm_table_efl_ui_widget_widget_sub_object_del),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_composition_prepare, _elm_table_efl_ui_focus_composition_prepare),
      ELM_TABLE_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"homogeneous", __eolian_elm_table_homogeneous_set_reflect, __eolian_elm_table_homogeneous_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_table_class_desc = {
   EO_VERSION,
   "Elm.Table",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _elm_table_class_initializer,
   _elm_table_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_table_class_get, &_elm_table_class_desc, EFL_UI_WIDGET_CLASS, EFL_UI_FOCUS_COMPOSITION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_table_eo.legacy.c"
