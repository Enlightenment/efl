#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"
#include "elm_atspi_app_object.eo.h"

extern Eina_List *_elm_win_list;

typedef struct _Elm_Atspi_App_Object_Data Elm_Atspi_App_Object_Data;

struct _Elm_Atspi_App_Object_Data
{
   const char *descr;
};

EOLIAN static void
_elm_atspi_app_object_eo_base_destructor(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd)
{
   if (_pd->descr) eina_stringshare_del(_pd->descr);

   eo_do_super(obj, ELM_ATSPI_APP_OBJECT_CLASS, eo_destructor());
}

EOLIAN static Eina_List*
_elm_atspi_app_object_elm_interface_atspi_accessible_children_get(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   Eina_List *l, *accs = NULL;
   Evas_Object *win;

   EINA_LIST_FOREACH(_elm_win_list, l, win)
     {
        if (!win) continue;
        if (eo_isa(win, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN))
          accs = eina_list_append(accs, win);
     }

   return accs;
}

EOLIAN static const char*
_elm_atspi_app_object_elm_interface_atspi_accessible_name_get(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   return elm_app_name_get();
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
