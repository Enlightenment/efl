
void _elm_route_emap_set(Eo *obj, Elm_Route_Data *pd, void *emap);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_route_emap_set, EFL_FUNC_CALL(emap), void *emap);

void _elm_route_longitude_min_max_get(const Eo *obj, Elm_Route_Data *pd, double *min, double *max);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_route_longitude_min_max_get, EFL_FUNC_CALL(min, max), double *min, double *max);

void _elm_route_latitude_min_max_get(const Eo *obj, Elm_Route_Data *pd, double *min, double *max);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_route_latitude_min_max_get, EFL_FUNC_CALL(min, max), double *min, double *max);

Efl_Object *_elm_route_efl_object_constructor(Eo *obj, Elm_Route_Data *pd);


Eina_Error _elm_route_efl_ui_widget_theme_apply(Eo *obj, Elm_Route_Data *pd);


static Eina_Bool
_elm_route_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_ROUTE_EXTRA_OPS
#define ELM_ROUTE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_route_emap_set, _elm_route_emap_set),
      EFL_OBJECT_OP_FUNC(elm_obj_route_longitude_min_max_get, _elm_route_longitude_min_max_get),
      EFL_OBJECT_OP_FUNC(elm_obj_route_latitude_min_max_get, _elm_route_latitude_min_max_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_route_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_route_efl_ui_widget_theme_apply),
      ELM_ROUTE_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_route_class_desc = {
   EO_VERSION,
   "Elm.Route",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Route_Data),
   _elm_route_class_initializer,
   _elm_route_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_route_class_get, &_elm_route_class_desc, EFL_UI_WIDGET_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_route_eo.legacy.c"
