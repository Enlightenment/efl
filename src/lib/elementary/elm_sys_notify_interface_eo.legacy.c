
EAPI void
elm_sys_notify_interface_send(const Elm_Sys_Notify_Interface *obj, unsigned int replaces_id, const char *icon, const char *summary, const char *body, Elm_Sys_Notify_Urgency urgency, int timeout, Elm_Sys_Notify_Send_Cb cb, const void *cb_data)
{
   elm_obj_sys_notify_interface_send(obj, replaces_id, icon, summary, body, urgency, timeout, cb, cb_data);
}

EAPI void
elm_sys_notify_interface_simple_send(const Elm_Sys_Notify_Interface *obj, const char *icon, const char *summary, const char *body)
{
   elm_obj_sys_notify_interface_simple_send(obj, icon, summary, body);
}

EAPI void
elm_sys_notify_interface_close(const Elm_Sys_Notify_Interface *obj, unsigned int id)
{
   elm_obj_sys_notify_interface_close(obj, id);
}
