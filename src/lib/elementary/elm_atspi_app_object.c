#ifdef HAVE_CONFIG_H
  #include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED

#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"

extern Eina_List *_elm_win_list;

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

EOLIAN static Eina_List*
_elm_atspi_app_object_efl_access_object_access_children_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   Eina_List *l, *accs = NULL;
   Evas_Object *win;

   EINA_LIST_FOREACH(_elm_win_list, l, win)
     {
        Efl_Access_Type type;
        if (!efl_isa(win, EFL_ACCESS_OBJECT_CLASS))
          continue;
        type = efl_access_object_access_type_get(win);
        if (type == EFL_ACCESS_TYPE_REGULAR)
          accs = eina_list_append(accs, win);
     }

   return accs;
}

EOLIAN static const char*
_elm_atspi_app_object_efl_access_object_i18n_name_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   const char *ret;
   ret = elm_app_name_get();
   return ret;
}

EOLIAN static const char*
_elm_atspi_app_object_efl_access_object_description_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd)
{
   return _pd->descr;
}

EOLIAN static void
_elm_atspi_app_object_efl_access_object_description_set(Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED, const char *descr)
{
   eina_stringshare_replace(&_pd->descr, descr);
}

EOLIAN static Efl_Access_Role
_elm_atspi_app_object_efl_access_object_role_get(const Eo *obj EINA_UNUSED, Elm_Atspi_App_Object_Data *_pd EINA_UNUSED)
{
   return EFL_ACCESS_ROLE_APPLICATION;
}

#include "elm_atspi_app_object.eo.c"
