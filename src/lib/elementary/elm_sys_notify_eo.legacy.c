
EAPI Eina_Bool
elm_sys_notify_servers_set(Elm_Sys_Notify *obj, Elm_Sys_Notify_Server servers)
{
   return elm_obj_sys_notify_servers_set(obj, servers);
}

EAPI Elm_Sys_Notify_Server
elm_sys_notify_servers_get(const Elm_Sys_Notify *obj)
{
   return elm_obj_sys_notify_servers_get(obj);
}
