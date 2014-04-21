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
static Eo *_atspi_root;
static int _init;

EOLIAN static Eina_List*
_elm_atspi_app_object_elm_interface_atspi_accessible_children_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   Eina_List *l, *accs = NULL;
   Evas_Object *win;

   EINA_LIST_FOREACH(_elm_win_list, l, win)
     {
        if (!win) continue;
        if (eo_isa(win, ELM_INTERFACE_ATSPI_ACCESSIBLE_CLASS))
          accs = eina_list_append(accs, win);
     }

   return accs;
}

EOLIAN static const char*
_elm_atspi_app_object_elm_interface_atspi_accessible_name_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return elm_app_name_get();
}

EOLIAN static const char*
_elm_atspi_app_object_elm_interface_atspi_accessible_description_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return NULL;
}

EOLIAN static Elm_Atspi_Role
_elm_atspi_app_object_elm_interface_atspi_accessible_role_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return ELM_ATSPI_ROLE_APPLICATION;
}

EAPI void
_elm_atspi_init(void)
{
   if (!_init)
     {
        _atspi_root = eo_add(ELM_ATSPI_APP_OBJECT_CLASS, NULL);
        _init = 1;
     }
}

EAPI void
_elm_atspi_shutdown(void)
{
   if (_init)
     {
        eo_unref(_atspi_root);
        _atspi_root = NULL;
        _init = 0;
     }
}

EAPI Eo*
_elm_atspi_root_get(void)
{
   return _atspi_root;
}

#include "elm_atspi_app_object.eo.c"
