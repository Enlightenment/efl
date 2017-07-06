#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_separator.eo.h"
#include "elm_widget_separator.h"
#include "elm_widget_layout.h"

#define MY_CLASS ELM_SEPARATOR_CLASS

#define MY_CLASS_NAME "Elm_Separator"
#define MY_CLASS_NAME_LEGACY "elm_separator"

EOLIAN static Elm_Theme_Apply
_elm_separator_elm_widget_theme_apply(Eo *obj, Elm_Separator_Data *sd EINA_UNUSED)
{
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;
   ELM_LAYOUT_DATA_GET(obj, ld);

   if (sd->orientation == EFL_ORIENT_HORIZONTAL)
     eina_stringshare_replace(&ld->group, "horizontal");
   else
     eina_stringshare_replace(&ld->group, "vertical");

   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

   return int_ret;
}

EOLIAN static void
_elm_separator_elm_layout_sizing_eval(Eo *obj, Elm_Separator_Data *sd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
}

EOLIAN static void
_elm_separator_efl_canvas_group_group_add(Eo *obj, Elm_Separator_Data *sd EINA_UNUSED)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   sd->orientation = EFL_ORIENT_VERTICAL;

   if (!elm_layout_theme_set
       (obj, "separator", "vertical", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_sizing_eval(obj);
}

EAPI Evas_Object *
elm_separator_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return efl_add(MY_CLASS, parent, efl_canvas_object_legacy_ctor(efl_added));
}

EAPI void
elm_separator_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   Efl_Orient orient;

   if (horizontal)
     orient = EFL_ORIENT_HORIZONTAL;
   else
     orient = EFL_ORIENT_VERTICAL;

   if (orient == efl_orientation_get(obj)) return;

   efl_orientation_set(obj, orient);
}

EAPI Eina_Bool
elm_separator_horizontal_get(const Evas_Object *obj)
{
   ELM_SEPARATOR_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   if (sd->orientation == EFL_ORIENT_VERTICAL)
     return EINA_FALSE;
   else
     return EINA_TRUE;
}

EOLIAN static Eo *
_elm_separator_efl_object_constructor(Eo *obj, Elm_Separator_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_SEPARATOR);

   return obj;
}

EOLIAN static void
_elm_separator_efl_orientation_orientation_set(Eo *obj, Elm_Separator_Data *sd, Efl_Orient dir)
{
   if (dir != EFL_ORIENT_VERTICAL &&
       dir != EFL_ORIENT_HORIZONTAL)
     return;

   if (sd->orientation == dir) return;

   sd->orientation = dir;

   elm_obj_widget_theme_apply(obj);
}

EOLIAN static Efl_Orient
_elm_separator_efl_orientation_orientation_get(Eo *obj EINA_UNUSED, Elm_Separator_Data *sd)
{
   return sd->orientation;
}

EOLIAN static Eina_Bool
_elm_separator_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Separator_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_separator_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Separator_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_elm_separator_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Internal EO APIs and hidden overrides */

#define ELM_SEPARATOR_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(elm_separator)

#include "elm_separator.eo.c"
