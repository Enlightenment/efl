
EAPI Efl_Canvas_Object *
elm_plug_image_object_get(const Elm_Plug *obj)
{
   return elm_obj_plug_image_object_get(obj);
}

EAPI Eina_Bool
elm_plug_connect(Elm_Plug *obj, const char *svcname, int svcnum, Eina_Bool svcsys)
{
   return elm_obj_plug_connect(obj, svcname, svcnum, svcsys);
}
