
void _evas_grid_grid_size_set(Eo *obj, Evas_Grid_Data *pd, int w, int h);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_grid_size_set, EFL_FUNC_CALL(w, h), int w, int h);

void _evas_grid_grid_size_get(const Eo *obj, Evas_Grid_Data *pd, int *w, int *h);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_obj_grid_size_get, EFL_FUNC_CALL(w, h), int *w, int *h);

Eina_List *_evas_grid_children_get(const Eo *obj, Evas_Grid_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_grid_children_get, Eina_List *, NULL);

Eina_Accessor *_evas_grid_accessor_new(const Eo *obj, Evas_Grid_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_grid_accessor_new, Eina_Accessor *, NULL);

void _evas_grid_clear(Eo *obj, Evas_Grid_Data *pd, Eina_Bool clear);

EOAPI EFL_VOID_FUNC_BODYV(evas_obj_grid_clear, EFL_FUNC_CALL(clear), Eina_Bool clear);

Eina_Iterator *_evas_grid_iterator_new(const Eo *obj, Evas_Grid_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_obj_grid_iterator_new, Eina_Iterator *, NULL);

Efl_Canvas_Object *_evas_grid_add_to(Eo *obj, Evas_Grid_Data *pd);

EOAPI EFL_FUNC_BODY(evas_obj_grid_add_to, Efl_Canvas_Object *, NULL);

Eina_Bool _evas_grid_unpack(Eo *obj, Evas_Grid_Data *pd, Efl_Canvas_Object *child);

EOAPI EFL_FUNC_BODYV(evas_obj_grid_unpack, Eina_Bool, 0, EFL_FUNC_CALL(child), Efl_Canvas_Object *child);

Eina_Bool _evas_grid_pack_get(const Eo *obj, Evas_Grid_Data *pd, Efl_Canvas_Object *child, int *x, int *y, int *w, int *h);

EOAPI EFL_FUNC_BODYV_CONST(evas_obj_grid_pack_get, Eina_Bool, 0, EFL_FUNC_CALL(child, x, y, w, h), Efl_Canvas_Object *child, int *x, int *y, int *w, int *h);

Eina_Bool _evas_grid_pack(Eo *obj, Evas_Grid_Data *pd, Efl_Canvas_Object *child, int x, int y, int w, int h);

EOAPI EFL_FUNC_BODYV(evas_obj_grid_pack, Eina_Bool, 0, EFL_FUNC_CALL(child, x, y, w, h), Efl_Canvas_Object *child, int x, int y, int w, int h);

Efl_Object *_evas_grid_efl_object_constructor(Eo *obj, Evas_Grid_Data *pd);


void _evas_grid_efl_ui_i18n_mirrored_set(Eo *obj, Evas_Grid_Data *pd, Eina_Bool rtl);


Eina_Bool _evas_grid_efl_ui_i18n_mirrored_get(const Eo *obj, Evas_Grid_Data *pd);


static Eina_Bool
_evas_grid_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef EVAS_GRID_EXTRA_OPS
#define EVAS_GRID_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(evas_obj_grid_size_set, _evas_grid_grid_size_set),
      EFL_OBJECT_OP_FUNC(evas_obj_grid_size_get, _evas_grid_grid_size_get),
      EFL_OBJECT_OP_FUNC(evas_obj_grid_children_get, _evas_grid_children_get),
      EFL_OBJECT_OP_FUNC(evas_obj_grid_accessor_new, _evas_grid_accessor_new),
      EFL_OBJECT_OP_FUNC(evas_obj_grid_clear, _evas_grid_clear),
      EFL_OBJECT_OP_FUNC(evas_obj_grid_iterator_new, _evas_grid_iterator_new),
      EFL_OBJECT_OP_FUNC(evas_obj_grid_add_to, _evas_grid_add_to),
      EFL_OBJECT_OP_FUNC(evas_obj_grid_unpack, _evas_grid_unpack),
      EFL_OBJECT_OP_FUNC(evas_obj_grid_pack_get, _evas_grid_pack_get),
      EFL_OBJECT_OP_FUNC(evas_obj_grid_pack, _evas_grid_pack),
      EFL_OBJECT_OP_FUNC(efl_constructor, _evas_grid_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_mirrored_set, _evas_grid_efl_ui_i18n_mirrored_set),
      EFL_OBJECT_OP_FUNC(efl_ui_mirrored_get, _evas_grid_efl_ui_i18n_mirrored_get),
      EVAS_GRID_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _evas_grid_class_desc = {
   EO_VERSION,
   "Evas.Grid",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Evas_Grid_Data),
   _evas_grid_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(evas_grid_class_get, &_evas_grid_class_desc, EFL_CANVAS_GROUP_CLASS, NULL);

#include "evas_grid_eo.legacy.c"
