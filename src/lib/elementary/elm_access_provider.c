#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#include <Elementary.h>
#include <elm_priv.h>

static Efl_Access_Object*
_access_object_create(Efl_Object *obj)
{
   if (efl_isa(obj, EFL_UI_WIDGET_CLASS))
     return efl_add(EFL_ACCESS_WIDGET_CLASS, obj);

   return NULL;
}

static EOLIAN Efl_Access_Object*
_elm_access_provider_efl_access_provider_access_object_get(const Elm_Access_Provider *self EINA_UNUSED, void *ptr EINA_UNUSED, Efl_Object *object)
{
   if (!efl_isa(object, EFL_UI_WIDGET_CLASS))
     return NULL;

   Efl_Access_Object *access = efl_ui_widget_access_object_get(object);
   if (!access)
     {
        access = _access_object_create(object);
        efl_ui_widget_access_object_set(object, access);
        efl_access_widget_real_widget_set(access, object);
     }

   return access;
}

#include "elm_access_provider.eo.c"
