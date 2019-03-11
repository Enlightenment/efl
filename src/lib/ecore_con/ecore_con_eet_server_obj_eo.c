
Efl_Object *_ecore_con_eet_server_obj_efl_object_constructor(Eo *obj, Ecore_Con_Eet_Server_Obj_Data *pd);


void _ecore_con_eet_server_obj_efl_object_destructor(Eo *obj, Ecore_Con_Eet_Server_Obj_Data *pd);


static Eina_Bool
_ecore_con_eet_server_obj_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ECORE_CON_EET_SERVER_OBJ_EXTRA_OPS
#define ECORE_CON_EET_SERVER_OBJ_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _ecore_con_eet_server_obj_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _ecore_con_eet_server_obj_efl_object_destructor),
      ECORE_CON_EET_SERVER_OBJ_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _ecore_con_eet_server_obj_class_desc = {
   EO_VERSION,
   "Ecore.Con.Eet.Server.Obj",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Ecore_Con_Eet_Server_Obj_Data),
   _ecore_con_eet_server_obj_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(ecore_con_eet_server_obj_class_get, &_ecore_con_eet_server_obj_class_desc, ECORE_CON_EET_BASE_CLASS, NULL);
