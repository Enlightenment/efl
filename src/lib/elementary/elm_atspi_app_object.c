#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

typedef struct _Elm_Atspi_App_Object_Data Elm_Atspi_App_Object_Data;

struct _Elm_Atspi_App_Object_Data
{
   const char *descr;
};

EOLIAN static void
_elm_atspi_app_object_efl_object_destructor(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd)
{
   if (_pd->descr) eina_stringshare_del(_pd->descr);

   efl_destructor(efl_super(obj, ELM_ATSPI_APP_OBJECT_CLASS));
}

EOLIAN static const char*
_elm_atspi_app_object_elm_interface_atspi_accessible_name_get(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   const char *ret;
   ret = elm_app_name_get();
   return ret;
}

EOLIAN static const char*
_elm_atspi_app_object_elm_interface_atspi_accessible_description_get(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd)
{
   return _pd->descr;
}

EOLIAN static void
_elm_atspi_app_object_elm_interface_atspi_accessible_description_set(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED, const char *descr)
{
   eina_stringshare_replace(&_pd->descr, descr);
}

EOLIAN static Elm_Atspi_Role
_elm_atspi_app_object_elm_interface_atspi_accessible_role_get(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   return ELM_ATSPI_ROLE_APPLICATION;
}

#include "elm_atspi_app_object.eo.c"
