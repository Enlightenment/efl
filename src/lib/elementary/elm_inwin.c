#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_WIDGET_PROTECTED
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_inwin.h"
#include "elm_widget_layout.h"

#define MY_CLASS ELM_INWIN_CLASS

#define MY_CLASS_NAME "Elm_Inwin"
#define MY_CLASS_NAME_LEGACY "elm_inwin"

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"default", "elm.swallow.content"},
   {NULL, NULL}
};

EOLIAN static void
_elm_inwin_elm_layout_sizing_eval(Eo *obj, void *_pd EINA_UNUSED)
{
   Evas_Object *content;
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   content = elm_layout_content_get(obj, NULL);

   if (!content) return;

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

EOLIAN static Eina_Bool
_elm_inwin_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_inwin_elm_widget_focus_next(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, Elm_Focus_Direction dir, Evas_Object **next, Elm_Object_Item **next_item)
{
   Evas_Object *content;

   content = elm_layout_content_get(obj, NULL);

   /* attempt to follow focus cycle into sub-object */
   if (content)
     {
        elm_widget_focus_next_get(content, dir, next, next_item);
        if (*next) return EINA_TRUE;
     }

   *next = (Evas_Object *)obj;

   return EINA_FALSE;
}

EOLIAN static void
_elm_inwin_efl_canvas_group_group_add(Eo *obj, void *_pd EINA_UNUSED)
{
   efl_canvas_group_add(eo_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_TRUE);

   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   if (!elm_layout_theme_set(obj, "win", "inwin", elm_object_style_get(obj)))
     CRI("Failed to set layout!");
}

EOLIAN static void
_elm_inwin_elm_widget_widget_parent_set(Eo *obj, void *_pd EINA_UNUSED, Evas_Object *parent)
{
   elm_win_resize_object_add(parent, obj);

   elm_layout_sizing_eval(obj);
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_inwin_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   return _content_aliases;
}

EAPI Evas_Object *
elm_win_inwin_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static Eo *
_elm_inwin_efl_object_constructor(Eo *obj, void *_pd EINA_UNUSED)
{
   Evas_Object *parent = NULL;

   parent = efl_parent_get(obj);

   if (parent && !eo_isa(parent, EFL_UI_WIN_CLASS))
     {
        ERR("Failed");
        return NULL;
     }

   obj = efl_constructor(eo_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_GLASS_PANE);

   return obj;
}

EOLIAN static void
_elm_inwin_activate(Eo *obj, void *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (elm_widget_disabled_get(obj)) return;

   evas_object_raise(obj);
   evas_object_show(obj);
   edje_object_signal_emit
     (wd->resize_obj, "elm,action,show", "elm");
   elm_object_focus_set(obj, EINA_TRUE);
}

EAPI void
elm_win_inwin_content_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_INWIN_CHECK(obj);
   efl_content_set(obj, content);
}

EAPI Evas_Object *
elm_win_inwin_content_get(const Evas_Object *obj)
{
   ELM_INWIN_CHECK(obj) NULL;
   return efl_content_get(obj);
}

EAPI Evas_Object *
elm_win_inwin_content_unset(Evas_Object *obj)
{
   ELM_INWIN_CHECK(obj) NULL;
   return efl_content_unset(obj);
}

static void
_elm_inwin_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_inwin.eo.c"
