
void _ecore_con_eet_base_server_set(Eo *obj, Ecore_Con_Eet_Base_Data *pd, Ecore_Con_Server *data);

EOAPI EFL_VOID_FUNC_BODYV(ecore_con_eet_base_server_set, EFL_FUNC_CALL(data), Ecore_Con_Server *data);

Ecore_Con_Server *_ecore_con_eet_base_server_get(const Eo *obj, Ecore_Con_Eet_Base_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(ecore_con_eet_base_server_get, Ecore_Con_Server *, NULL);

void _ecore_con_eet_base_data_callback_set(Eo *obj, Ecore_Con_Eet_Base_Data *pd, const char *name, Ecore_Con_Eet_Data_Cb func, const void *data);

EOAPI EFL_VOID_FUNC_BODYV(ecore_con_eet_base_data_callback_set, EFL_FUNC_CALL(name, func, data), const char *name, Ecore_Con_Eet_Data_Cb func, const void *data);

void _ecore_con_eet_base_raw_data_callback_set(Eo *obj, Ecore_Con_Eet_Base_Data *pd, const char *name, Ecore_Con_Eet_Raw_Data_Cb func, const void *data);

EOAPI EFL_VOID_FUNC_BODYV(ecore_con_eet_base_raw_data_callback_set, EFL_FUNC_CALL(name, func, data), const char *name, Ecore_Con_Eet_Raw_Data_Cb func, const void *data);

void _ecore_con_eet_base_data_callback_del(Eo *obj, Ecore_Con_Eet_Base_Data *pd, const char *name);

EOAPI EFL_VOID_FUNC_BODYV(ecore_con_eet_base_data_callback_del, EFL_FUNC_CALL(name), const char *name);

void _ecore_con_eet_base_raw_data_callback_del(Eo *obj, Ecore_Con_Eet_Base_Data *pd, const char *name);

EOAPI EFL_VOID_FUNC_BODYV(ecore_con_eet_base_raw_data_callback_del, EFL_FUNC_CALL(name), const char *name);

void _ecore_con_eet_base_register(Eo *obj, Ecore_Con_Eet_Base_Data *pd, const char *name, Eet_Data_Descriptor *edd);

EOAPI EFL_VOID_FUNC_BODYV(ecore_con_eet_base_register, EFL_FUNC_CALL(name, edd), const char *name, Eet_Data_Descriptor *edd);

void _ecore_con_eet_base_send(Eo *obj, Ecore_Con_Eet_Base_Data *pd, Ecore_Con_Reply *reply, const char *name, void *value);

EOAPI EFL_VOID_FUNC_BODYV(ecore_con_eet_base_send, EFL_FUNC_CALL(reply, name, value), Ecore_Con_Reply *reply, const char *name, void *value);

void _ecore_con_eet_base_raw_send(Eo *obj, Ecore_Con_Eet_Base_Data *pd, Ecore_Con_Reply *reply, const char *protocol_name, const char *section, Eina_Binbuf *section_data);

EOAPI EFL_VOID_FUNC_BODYV(ecore_con_eet_base_raw_send, EFL_FUNC_CALL(reply, protocol_name, section, section_data), Ecore_Con_Reply *reply, const char *protocol_name, const char *section, Eina_Binbuf *section_data);

Efl_Object *_ecore_con_eet_base_efl_object_constructor(Eo *obj, Ecore_Con_Eet_Base_Data *pd);


void _ecore_con_eet_base_efl_object_destructor(Eo *obj, Ecore_Con_Eet_Base_Data *pd);


Efl_Object *_ecore_con_eet_base_efl_object_finalize(Eo *obj, Ecore_Con_Eet_Base_Data *pd);


static Eina_Bool
_ecore_con_eet_base_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ECORE_CON_EET_BASE_EXTRA_OPS
#define ECORE_CON_EET_BASE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(ecore_con_eet_base_server_set, _ecore_con_eet_base_server_set),
      EFL_OBJECT_OP_FUNC(ecore_con_eet_base_server_get, _ecore_con_eet_base_server_get),
      EFL_OBJECT_OP_FUNC(ecore_con_eet_base_data_callback_set, _ecore_con_eet_base_data_callback_set),
      EFL_OBJECT_OP_FUNC(ecore_con_eet_base_raw_data_callback_set, _ecore_con_eet_base_raw_data_callback_set),
      EFL_OBJECT_OP_FUNC(ecore_con_eet_base_data_callback_del, _ecore_con_eet_base_data_callback_del),
      EFL_OBJECT_OP_FUNC(ecore_con_eet_base_raw_data_callback_del, _ecore_con_eet_base_raw_data_callback_del),
      EFL_OBJECT_OP_FUNC(ecore_con_eet_base_register, _ecore_con_eet_base_register),
      EFL_OBJECT_OP_FUNC(ecore_con_eet_base_send, _ecore_con_eet_base_send),
      EFL_OBJECT_OP_FUNC(ecore_con_eet_base_raw_send, _ecore_con_eet_base_raw_send),
      EFL_OBJECT_OP_FUNC(efl_constructor, _ecore_con_eet_base_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _ecore_con_eet_base_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_finalize, _ecore_con_eet_base_efl_object_finalize),
      ECORE_CON_EET_BASE_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _ecore_con_eet_base_class_desc = {
   EO_VERSION,
   "Ecore.Con.Eet.Base",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Ecore_Con_Eet_Base_Data),
   _ecore_con_eet_base_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(ecore_con_eet_base_class_get, &_ecore_con_eet_base_class_desc, EFL_OBJECT_CLASS, NULL);

#include "ecore_con_eet_base_eo.legacy.c"
