EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_sys_notify_interface_send, EFL_FUNC_CALL(replaces_id, icon, summary, body, urgency, timeout, cb, cb_data), unsigned int replaces_id, const char *icon, const char *summary, const char *body, Elm_Sys_Notify_Urgency urgency, int timeout, Elm_Sys_Notify_Send_Cb cb, const void *cb_data);
EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_sys_notify_interface_simple_send, EFL_FUNC_CALL(icon, summary, body), const char *icon, const char *summary, const char *body);
EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_sys_notify_interface_close, EFL_FUNC_CALL(id), unsigned int id);

static Eina_Bool
_elm_sys_notify_interface_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_SYS_NOTIFY_INTERFACE_EXTRA_OPS
#define ELM_SYS_NOTIFY_INTERFACE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_sys_notify_interface_send, NULL),
      EFL_OBJECT_OP_FUNC(elm_obj_sys_notify_interface_simple_send, NULL),
      EFL_OBJECT_OP_FUNC(elm_obj_sys_notify_interface_close, NULL),
      ELM_SYS_NOTIFY_INTERFACE_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_sys_notify_interface_class_desc = {
   EO_VERSION,
   "Elm.Sys_Notify_Interface",
   EFL_CLASS_TYPE_INTERFACE,
   0,
   _elm_sys_notify_interface_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_sys_notify_interface_interface_get, &_elm_sys_notify_interface_class_desc, NULL, NULL);

#include "elm_sys_notify_interface_eo.legacy.c"
