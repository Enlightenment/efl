
EAPI void
elm_systray_id_set(Elm_Systray *obj, const char *id)
{
   elm_obj_systray_id_set(obj, id);
}

EAPI const char *
elm_systray_id_get(const Elm_Systray *obj)
{
   return elm_obj_systray_id_get(obj);
}

EAPI void
elm_systray_category_set(Elm_Systray *obj, Elm_Systray_Category cat)
{
   elm_obj_systray_category_set(obj, cat);
}

EAPI Elm_Systray_Category
elm_systray_category_get(const Elm_Systray *obj)
{
   return elm_obj_systray_category_get(obj);
}

EAPI void
elm_systray_icon_theme_path_set(Elm_Systray *obj, const char *icon_theme_path)
{
   elm_obj_systray_icon_theme_path_set(obj, icon_theme_path);
}

EAPI const char *
elm_systray_icon_theme_path_get(const Elm_Systray *obj)
{
   return elm_obj_systray_icon_theme_path_get(obj);
}

EAPI void
elm_systray_menu_set(Elm_Systray *obj, const Efl_Object *menu)
{
   elm_obj_systray_menu_set(obj, menu);
}

EAPI const Efl_Object *
elm_systray_menu_get(const Elm_Systray *obj)
{
   return elm_obj_systray_menu_get(obj);
}

EAPI void
elm_systray_att_icon_name_set(Elm_Systray *obj, const char *att_icon_name)
{
   elm_obj_systray_att_icon_name_set(obj, att_icon_name);
}

EAPI const char *
elm_systray_att_icon_name_get(const Elm_Systray *obj)
{
   return elm_obj_systray_att_icon_name_get(obj);
}

EAPI void
elm_systray_status_set(Elm_Systray *obj, Elm_Systray_Status st)
{
   elm_obj_systray_status_set(obj, st);
}

EAPI Elm_Systray_Status
elm_systray_status_get(const Elm_Systray *obj)
{
   return elm_obj_systray_status_get(obj);
}

EAPI void
elm_systray_icon_name_set(Elm_Systray *obj, const char *icon_name)
{
   elm_obj_systray_icon_name_set(obj, icon_name);
}

EAPI const char *
elm_systray_icon_name_get(const Elm_Systray *obj)
{
   return elm_obj_systray_icon_name_get(obj);
}

EAPI void
elm_systray_title_set(Elm_Systray *obj, const char *title)
{
   elm_obj_systray_title_set(obj, title);
}

EAPI const char *
elm_systray_title_get(const Elm_Systray *obj)
{
   return elm_obj_systray_title_get(obj);
}

EAPI Eina_Bool
elm_systray_register(Elm_Systray *obj)
{
   return elm_obj_systray_register(obj);
}
