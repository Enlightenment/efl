
void _evas_line_xy_set(Eo *obj, Evas_Line_Data *pd, int x1, int y1, int x2, int y2);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_line_xy_set, EFL_FUNC_CALL(x1, y1, x2, y2), int x1, int y1, int x2, int y2);

void _evas_line_xy_get(const Eo *obj, Evas_Line_Data *pd, int *x1, int *y1, int *x2, int *y2);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_obj_line_xy_get, EFL_FUNC_CALL(x1, y1, x2, y2), int *x1, int *y1, int *x2, int *y2);

Efl_Object *_evas_line_efl_object_constructor(Eo *obj, Evas_Line_Data *pd);


static Eina_Bool
_evas_line_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef EVAS_LINE_EXTRA_OPS
#define EVAS_LINE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(evas_obj_line_xy_set, _evas_line_xy_set),
      EFL_OBJECT_OP_FUNC(evas_obj_line_xy_get, _evas_line_xy_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _evas_line_efl_object_constructor),
      EVAS_LINE_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _evas_line_class_desc = {
   EO_VERSION,
   "Evas.Line",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Evas_Line_Data),
   _evas_line_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(evas_line_class_get, &_evas_line_class_desc, EFL_CANVAS_OBJECT_CLASS, NULL);

#include "evas_line_eo.legacy.c"
