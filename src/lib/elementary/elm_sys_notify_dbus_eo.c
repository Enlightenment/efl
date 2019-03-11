
Efl_Object *_elm_sys_notify_dbus_efl_object_constructor(Eo *obj, void *pd);


void _elm_sys_notify_dbus_efl_object_destructor(Eo *obj, void *pd);


void _elm_sys_notify_dbus_elm_sys_notify_interface_send(const Eo *obj, void *pd, unsigned int replaces_id, const char *icon, const char *summary, const char *body, Elm_Sys_Notify_Urgency urgency, int timeout, Elm_Sys_Notify_Send_Cb cb, const void *cb_data);


void _elm_sys_notify_dbus_elm_sys_notify_interface_simple_send(const Eo *obj, void *pd, const char *icon, const char *summary, const char *body);


void _elm_sys_notify_dbus_elm_sys_notify_interface_close(const Eo *obj, void *pd, unsigned int id);


static Eina_Bool
_elm_sys_notify_dbus_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_SYS_NOTIFY_DBUS_EXTRA_OPS
#define ELM_SYS_NOTIFY_DBUS_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_sys_notify_dbus_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_sys_notify_dbus_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(elm_obj_sys_notify_interface_send, _elm_sys_notify_dbus_elm_sys_notify_interface_send),
      EFL_OBJECT_OP_FUNC(elm_obj_sys_notify_interface_simple_send, _elm_sys_notify_dbus_elm_sys_notify_interface_simple_send),
      EFL_OBJECT_OP_FUNC(elm_obj_sys_notify_interface_close, _elm_sys_notify_dbus_elm_sys_notify_interface_close),
      ELM_SYS_NOTIFY_DBUS_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_sys_notify_dbus_class_desc = {
   EO_VERSION,
   "Elm.Sys_Notify.Dbus",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _elm_sys_notify_dbus_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_sys_notify_dbus_class_get, &_elm_sys_notify_dbus_class_desc, EFL_OBJECT_CLASS, ELM_SYS_NOTIFY_INTERFACE_INTERFACE, NULL);
