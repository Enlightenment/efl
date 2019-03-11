
void _elm_grid_grid_size_set(Eo *obj, void *pd, int w, int h);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_grid_size_set, EFL_FUNC_CALL(w, h), int w, int h);

void _elm_grid_grid_size_get(const Eo *obj, void *pd, int *w, int *h);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_grid_size_get, EFL_FUNC_CALL(w, h), int *w, int *h);

Eina_List *_elm_grid_children_get(const Eo *obj, void *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_grid_children_get, Eina_List *, NULL);

void _elm_grid_clear(Eo *obj, void *pd, Eina_Bool clear);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_grid_clear, EFL_FUNC_CALL(clear), Eina_Bool clear);

void _elm_grid_unpack(Eo *obj, void *pd, Efl_Canvas_Object *subobj);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_grid_unpack, EFL_FUNC_CALL(subobj), Efl_Canvas_Object *subobj);

void _elm_grid_pack(Eo *obj, void *pd, Efl_Canvas_Object *subobj, int x, int y, int w, int h);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_grid_pack, EFL_FUNC_CALL(subobj, x, y, w, h), Efl_Canvas_Object *subobj, int x, int y, int w, int h);

Efl_Object *_elm_grid_efl_object_constructor(Eo *obj, void *pd);


Eina_Error _elm_grid_efl_ui_widget_theme_apply(Eo *obj, void *pd);


void _elm_grid_efl_ui_focus_composition_prepare(Eo *obj, void *pd);


static Eina_Bool
_elm_grid_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_GRID_EXTRA_OPS
#define ELM_GRID_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_grid_size_set, _elm_grid_grid_size_set),
      EFL_OBJECT_OP_FUNC(elm_obj_grid_size_get, _elm_grid_grid_size_get),
      EFL_OBJECT_OP_FUNC(elm_obj_grid_children_get, _elm_grid_children_get),
      EFL_OBJECT_OP_FUNC(elm_obj_grid_clear, _elm_grid_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_grid_unpack, _elm_grid_unpack),
      EFL_OBJECT_OP_FUNC(elm_obj_grid_pack, _elm_grid_pack),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_grid_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_grid_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_composition_prepare, _elm_grid_efl_ui_focus_composition_prepare),
      ELM_GRID_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_grid_class_desc = {
   EO_VERSION,
   "Elm.Grid",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _elm_grid_class_initializer,
   _elm_grid_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_grid_class_get, &_elm_grid_class_desc, EFL_UI_WIDGET_CLASS, EFL_UI_FOCUS_COMPOSITION_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_grid_eo.legacy.c"
