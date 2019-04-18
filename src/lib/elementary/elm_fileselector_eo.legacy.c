
EAPI void
elm_fileselector_buttons_ok_cancel_set(Elm_Fileselector *obj, Eina_Bool visible)
{
   elm_obj_fileselector_buttons_ok_cancel_set(obj, visible);
}

EAPI Eina_Bool
elm_fileselector_buttons_ok_cancel_get(const Elm_Fileselector *obj)
{
   return elm_obj_fileselector_buttons_ok_cancel_get(obj);
}
