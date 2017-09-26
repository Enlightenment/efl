#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_WIDGET_PROTECTED
#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_inwin.eo.h"
#include "elm_widget_inwin.h"
#include "elm_widget_layout.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_INWIN_CLASS
#define MY_CLASS_PFX elm_inwin

#define MY_CLASS_NAME "Elm_Inwin"
#define MY_CLASS_NAME_LEGACY "elm_inwin"

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"default", "elm.swallow.content"},
   {NULL, NULL}
};

typedef struct {
   Efl_Ui_Focus_Manager *manager, *registered_manager;
   Eina_Bool registered;
} Elm_Inwin_Data;

EOLIAN static void
_elm_inwin_elm_layout_sizing_eval(Eo *obj, Elm_Inwin_Data *pd EINA_UNUSED)
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

EOLIAN static void
_elm_inwin_efl_canvas_group_group_add(Eo *obj, Elm_Inwin_Data *pd EINA_UNUSED)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_TRUE);

   evas_object_size_hint_weight_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   if (!elm_layout_theme_set(obj, "win", "inwin", elm_object_style_get(obj)))
     CRI("Failed to set layout!");
}

EOLIAN static void
_elm_inwin_elm_widget_widget_parent_set(Eo *obj, Elm_Inwin_Data *pd EINA_UNUSED, Evas_Object *parent)
{
   elm_win_resize_object_add(parent, obj);

   elm_layout_sizing_eval(obj);
}

EAPI Evas_Object *
elm_win_inwin_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return efl_add(MY_CLASS, parent, efl_canvas_object_legacy_ctor(efl_added));
}


EOLIAN static Efl_Ui_Focus_Manager*
_elm_inwin_elm_widget_focus_manager_create(Eo *obj EINA_UNUSED, Elm_Inwin_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   Efl_Ui_Focus_Manager *manager;

   manager = efl_add(EFL_UI_FOCUS_MANAGER_ROOT_FOCUS_CLASS, obj,
     efl_ui_focus_manager_root_set(efl_added, root)
   );

   return manager;
}


EOLIAN static Eo *
_elm_inwin_efl_object_constructor(Eo *obj, Elm_Inwin_Data *pd EINA_UNUSED)
{
   Evas_Object *parent = NULL;

   parent = efl_parent_get(obj);

   if (parent && !efl_isa(parent, EFL_UI_WIN_CLASS))
     {
        ERR("Failed");
        return NULL;
     }

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_GLASS_PANE);

   pd->manager = elm_obj_widget_focus_manager_create(obj, obj);

   efl_composite_attach(obj, pd->manager);

   return obj;
}

EOLIAN static void
_elm_inwin_activate(Eo *obj, Elm_Inwin_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (elm_widget_disabled_get(obj)) return;

   evas_object_raise(obj);
   evas_object_show(obj);
   edje_object_signal_emit
     (wd->resize_obj, "elm,action,show", "elm");
   elm_object_focus_set(obj, EINA_TRUE);
}

EOLIAN static void
_elm_inwin_efl_gfx_visible_set(Eo *obj, Elm_Inwin_Data *pd, Eina_Bool v)
{
   efl_gfx_visible_set(efl_super(obj, MY_CLASS), v);

   if (v && !pd->registered)
     {
        pd->registered_manager = efl_ui_focus_user_manager_get(obj);

        efl_ui_focus_manager_redirect_set(pd->registered_manager, obj);
        efl_ui_focus_manager_focus_set(pd->manager, obj);
        pd->registered = EINA_TRUE;
     }
   else if (!v && pd->registered)
     {
        efl_ui_focus_manager_redirect_set(pd->registered_manager, NULL);
        pd->registered = EINA_FALSE;
     }
}

EOLIAN static Efl_Ui_Focus_Object*
_elm_inwin_efl_ui_focus_manager_move(Eo *obj, Elm_Inwin_Data *pd, Efl_Ui_Focus_Direction direction)
{
   Eo *ret = efl_ui_focus_manager_move(pd->manager , direction);

   if (ret)
     return ret;

   if ((direction == EFL_UI_FOCUS_DIRECTION_PREV) || (direction == EFL_UI_FOCUS_DIRECTION_NEXT))
     efl_ui_focus_manager_focus_set(pd->manager, obj);

   return efl_ui_focus_manager_focus_get(obj);
}

EOLIAN static Eina_Bool
_elm_inwin_efl_container_content_set(Eo *obj, Elm_Inwin_Data *pd EINA_UNUSED, Efl_Gfx *content)
{
   return efl_content_set(efl_part(obj, _content_aliases[0].real_part), content);
}

EOLIAN static Efl_Gfx *
_elm_inwin_efl_container_content_get(Eo *obj, Elm_Inwin_Data *pd EINA_UNUSED)
{
   return efl_content_get(efl_part(obj, _content_aliases[0].real_part));
}

EOLIAN static Efl_Gfx *
_elm_inwin_efl_container_content_unset(Eo *obj, Elm_Inwin_Data *pd EINA_UNUSED)
{
   return efl_content_unset(efl_part(obj, _content_aliases[0].real_part));
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

/* Internal EO APIs and hidden overrides */

ELM_LAYOUT_CONTENT_ALIASES_IMPLEMENT(elm_inwin)
ELM_PART_CONTENT_DEFAULT_SET(elm_inwin, _content_aliases[0].real_part)

#define ELM_INWIN_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(elm_inwin), \
   ELM_LAYOUT_SIZING_EVAL_OPS(elm_inwin), \
   ELM_LAYOUT_CONTENT_ALIASES_OPS(elm_inwin), \
   ELM_PART_CONTENT_DEFAULT_OPS(elm_inwin)

#include "elm_inwin.eo.c"
