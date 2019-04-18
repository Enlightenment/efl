EWAPI const Efl_Event_Description _ELM_ATSPI_BRIDGE_EVENT_CONNECTED =
   EFL_EVENT_DESCRIPTION("connected");
EWAPI const Efl_Event_Description _ELM_ATSPI_BRIDGE_EVENT_DISCONNECTED =
   EFL_EVENT_DESCRIPTION("disconnected");

Eina_Bool _elm_atspi_bridge_connected_get(const Eo *obj, Elm_Atspi_Bridge_Data *pd);


static Eina_Value
__eolian_elm_atspi_bridge_connected_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_atspi_bridge_connected_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_atspi_bridge_connected_get, Eina_Bool, 0);

Efl_Object *_elm_atspi_bridge_efl_object_constructor(Eo *obj, Elm_Atspi_Bridge_Data *pd);


void _elm_atspi_bridge_efl_object_destructor(Eo *obj, Elm_Atspi_Bridge_Data *pd);


static Eina_Bool
_elm_atspi_bridge_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_ATSPI_BRIDGE_EXTRA_OPS
#define ELM_ATSPI_BRIDGE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_atspi_bridge_connected_get, _elm_atspi_bridge_connected_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_atspi_bridge_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_atspi_bridge_efl_object_destructor),
      ELM_ATSPI_BRIDGE_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"connected", NULL, __eolian_elm_atspi_bridge_connected_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_atspi_bridge_class_desc = {
   EO_VERSION,
   "Elm.Atspi.Bridge",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Atspi_Bridge_Data),
   _elm_atspi_bridge_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_atspi_bridge_class_get, &_elm_atspi_bridge_class_desc, EFL_OBJECT_CLASS, NULL);

#include "elm_atspi_bridge_eo.legacy.c"
