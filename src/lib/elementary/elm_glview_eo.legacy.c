
EAPI Eina_Bool
elm_glview_resize_policy_set(Elm_Glview *obj, Elm_GLView_Resize_Policy policy)
{
   return elm_obj_glview_resize_policy_set(obj, policy);
}

EAPI Eina_Bool
elm_glview_render_policy_set(Elm_Glview *obj, Elm_GLView_Render_Policy policy)
{
   return elm_obj_glview_render_policy_set(obj, policy);
}

EAPI Eina_Bool
elm_glview_mode_set(Elm_Glview *obj, Elm_GLView_Mode mode)
{
   return elm_obj_glview_mode_set(obj, mode);
}

EAPI Evas_GL_API *
elm_glview_gl_api_get(const Elm_Glview *obj)
{
   return elm_obj_glview_gl_api_get(obj);
}

EAPI Evas_GL *
elm_glview_evas_gl_get(const Elm_Glview *obj)
{
   return elm_obj_glview_evas_gl_get(obj);
}

EAPI int
elm_glview_rotation_get(const Elm_Glview *obj)
{
   return elm_obj_glview_rotation_get(obj);
}
