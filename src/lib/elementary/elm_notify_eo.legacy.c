
EAPI void
elm_notify_align_set(Elm_Notify *obj, double horizontal, double vertical)
{
   elm_obj_notify_align_set(obj, horizontal, vertical);
}

EAPI void
elm_notify_align_get(const Elm_Notify *obj, double *horizontal, double *vertical)
{
   elm_obj_notify_align_get(obj, horizontal, vertical);
}

EAPI void
elm_notify_allow_events_set(Elm_Notify *obj, Eina_Bool allow)
{
   elm_obj_notify_allow_events_set(obj, allow);
}

EAPI Eina_Bool
elm_notify_allow_events_get(const Elm_Notify *obj)
{
   return elm_obj_notify_allow_events_get(obj);
}

EAPI void
elm_notify_timeout_set(Elm_Notify *obj, double timeout)
{
   elm_obj_notify_timeout_set(obj, timeout);
}

EAPI double
elm_notify_timeout_get(const Elm_Notify *obj)
{
   return elm_obj_notify_timeout_get(obj);
}

EAPI void
elm_notify_dismiss(Elm_Notify *obj)
{
   elm_obj_notify_dismiss(obj);
}
