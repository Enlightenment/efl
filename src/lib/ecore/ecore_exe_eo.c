EWAPI const Efl_Event_Description _ECORE_EXE_EVENT_DATA_GET =
   EFL_EVENT_DESCRIPTION("data,get");
EWAPI const Efl_Event_Description _ECORE_EXE_EVENT_DATA_ERROR =
   EFL_EVENT_DESCRIPTION("data,error");

void _ecore_exe_command_set(Eo *obj, Ecore_Exe_Data *pd, const char *exe_cmd, Ecore_Exe_Flags flags);

EOAPI EFL_VOID_FUNC_BODYV(ecore_obj_exe_command_set, EFL_FUNC_CALL(exe_cmd, flags), const char *exe_cmd, Ecore_Exe_Flags flags);

void _ecore_exe_command_get(const Eo *obj, Ecore_Exe_Data *pd, const char **exe_cmd, Ecore_Exe_Flags *flags);

EOAPI EFL_VOID_FUNC_BODYV_CONST(ecore_obj_exe_command_get, EFL_FUNC_CALL(exe_cmd, flags), const char **exe_cmd, Ecore_Exe_Flags *flags);

void _ecore_exe_efl_object_destructor(Eo *obj, Ecore_Exe_Data *pd);


Efl_Object *_ecore_exe_efl_object_finalize(Eo *obj, Ecore_Exe_Data *pd);


void _ecore_exe_efl_control_suspend_set(Eo *obj, Ecore_Exe_Data *pd, Eina_Bool suspend);


static Eina_Bool
_ecore_exe_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ECORE_EXE_EXTRA_OPS
#define ECORE_EXE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(ecore_obj_exe_command_set, _ecore_exe_command_set),
      EFL_OBJECT_OP_FUNC(ecore_obj_exe_command_get, _ecore_exe_command_get),
      EFL_OBJECT_OP_FUNC(efl_destructor, _ecore_exe_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_finalize, _ecore_exe_efl_object_finalize),
      EFL_OBJECT_OP_FUNC(efl_control_suspend_set, _ecore_exe_efl_control_suspend_set),
      ECORE_EXE_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _ecore_exe_class_desc = {
   EO_VERSION,
   "Ecore.Exe",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Ecore_Exe_Data),
   _ecore_exe_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(ecore_exe_class_get, &_ecore_exe_class_desc, EFL_OBJECT_CLASS, EFL_CONTROL_INTERFACE, NULL);
