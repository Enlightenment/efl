#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#include "atspi/atspi-constants.h"

#define ATSPI_REGISTRYD_ROOT_ID ATSPI_DBUS_NAME_REGISTRY ":" ATSPI_DBUS_PATH_ROOT

typedef struct _Elm_Atspi_App_Object_Data Elm_Atspi_App_Object_Data;

struct _Elm_Atspi_App_Object_Data
{
   Elm_Atspi_Proxy *desktop_proxy;
   const char *descr;
};

EOLIAN static void
_elm_atspi_app_object_efl_object_destructor(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd)
{
   if (_pd->descr) eina_stringshare_del(_pd->descr);
   efl_destructor(efl_super(obj, ELM_ATSPI_APP_OBJECT_CLASS));
}

EOLIAN static Eo*
_elm_atspi_app_object_efl_object_constructor(Eo *obj, Elm_Atspi_App_Object_Data *_pd)
{
   efl_constructor(efl_super(obj, ELM_ATSPI_APP_OBJECT_CLASS));
   _pd->desktop_proxy = efl_add(ELM_ATSPI_PROXY_CLASS, obj, elm_atspi_proxy_id_constructor(efl_added, ATSPI_REGISTRYD_ROOT_ID));

   elm_interface_atspi_accessible_parent_set(obj, _pd->desktop_proxy);

   return obj;
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

EOLIAN static void
_elm_atspi_app_object_elm_atspi_plug_on_connected(Eo *obj, Elm_Atspi_App_Object_Data *_pd)
{
   elm_atspi_plug_embed_by(obj, _pd->desktop_proxy);
}

EOLIAN static void
_elm_atspi_app_object_elm_atspi_plug_on_disconnected(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   elm_atspi_plug_unembed_by(obj, _pd->desktop_proxy);
}

EOLIAN static void
_elm_atspi_app_object_elm_atspi_plug_on_embedded(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED, Elm_Atspi_Proxy *proxy EINA_UNUSED)
{
}

EOLIAN static void
_elm_atspi_app_object_elm_atspi_plug_on_unembedded(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED, Elm_Atspi_Proxy *proxy EINA_UNUSED)
{
}

EOLIAN static int
_elm_atspi_app_object_elm_interface_atspi_accessible_index_in_parent_get(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   return -1;
}

EOLIAN static void
_elm_atspi_app_object_elm_atspi_plug_embed_by(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED, Elm_Atspi_Proxy *proxy EINA_UNUSED)
{
}

EOLIAN static void
_elm_atspi_app_object_elm_atspi_plug_unembed_by(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED, Elm_Atspi_Proxy *proxy EINA_UNUSED)
{
}

#include "elm_atspi_app_object.eo.c"
