#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_POPUP_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_private.h"

#define MY_CLASS EFL_UI_POPUP_CLASS
#define MY_CLASS_NAME "Efl.Ui.Popup"
#define MY_CLASS_NAME_LEGACY "elm_popup"

static void
_bg_clicked_cb(void *data,
                  Evas_Object *o EINA_UNUSED,
                  const char *emission EINA_UNUSED,
                  const char *source EINA_UNUSED)
{
   Evas_Object *obj = data;
   efl_event_callback_legacy_call(obj, EFL_UI_POPUP_EVENT_BG_CLICKED, NULL);
}

EOLIAN static void
_efl_ui_popup_efl_gfx_position_set(Eo *obj, Efl_Ui_Popup_Data *pd EINA_UNUSED, Eina_Position2D pos)
{
   efl_gfx_position_set(efl_super(obj, MY_CLASS), pos);
}

static void
_parent_geom_cb(void *data, const Efl_Event *ev)
{
   Evas_Object *event_bg = data;
   Evas_Coord x, y, w, h;
   evas_object_geometry_get(ev->object, &x, &y, &w, &h);

   if (efl_isa(ev->object, EFL_UI_WIN_CLASS))
     {
        x = 0;
        y = 0;
     }

   evas_object_move(event_bg, x, y);
   evas_object_resize(event_bg, w, h);
}

EOLIAN static void
_efl_ui_popup_elm_widget_widget_parent_set(Eo *obj, Efl_Ui_Popup_Data *pd EINA_UNUSED, Evas_Object *parent EINA_UNUSED)
{
   Evas_Coord x, y, w, h;
   pd->win_parent = efl_provider_find(obj, EFL_UI_WIN_CLASS);
   if (!pd->win_parent)
     {
        ERR("Cannot find window parent");
        return;
     }

   evas_object_geometry_get(pd->win_parent, &x, &y, &w, &h);
   evas_object_move(pd->event_bg, x, y);
   evas_object_resize(pd->event_bg, w, h);

   efl_event_callback_add(pd->win_parent, EFL_GFX_EVENT_RESIZE, _parent_geom_cb, pd->event_bg);
   efl_event_callback_add(pd->win_parent, EFL_GFX_EVENT_MOVE, _parent_geom_cb, pd->event_bg);
}

EOLIAN static Eina_Bool
_efl_ui_popup_efl_container_content_set(Eo *obj, Efl_Ui_Popup_Data *pd EINA_UNUSED, Evas_Object *content)
{
   return efl_content_set(efl_part(obj, "elm.swallow.content"), content);
}

EOLIAN static Evas_Object*
_efl_ui_popup_efl_container_content_get(Eo *obj, Efl_Ui_Popup_Data *pd EINA_UNUSED)
{
   return efl_content_get(efl_part(obj, "elm.swallow.content"));
}

EOLIAN static Evas_Object*
_efl_ui_popup_efl_container_content_unset(Eo *obj, Efl_Ui_Popup_Data *pd EINA_UNUSED)
{
   return efl_content_unset(efl_part(obj, "elm.swallow.content"));
}

EOLIAN static Efl_Canvas_Object *
_efl_ui_popup_parent_window_get(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Data *pd)
{
   return pd->win_parent;
}

EOLIAN static void
_efl_ui_popup_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_layout_theme_set(obj, "popup", "base", "view");

   pd->event_bg = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set(obj, pd->event_bg, "popup", "base", "event_bg");
   evas_object_smart_member_add(pd->event_bg, obj);
   evas_object_stack_below(pd->event_bg, wd->resize_obj);

   edje_object_signal_callback_add(pd->event_bg, "elm,action,clicked", "*", _bg_clicked_cb, obj);
}

EOLIAN static void
_efl_ui_popup_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Popup_Data *pd)
{
   ELM_SAFE_FREE(pd->event_bg, evas_object_del);
   efl_event_callback_del(pd->win_parent, EFL_GFX_EVENT_RESIZE, _parent_geom_cb, pd->event_bg);
   efl_event_callback_del(pd->win_parent, EFL_GFX_EVENT_MOVE, _parent_geom_cb, pd->event_bg);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_popup_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Popup_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
}

EOLIAN static void
_efl_ui_popup_bg_set(Eo *obj, Efl_Ui_Popup_Data *pd, const char* file, const char* group)
{
   Evas_Object *prev_obj = edje_object_part_swallow_get(pd->event_bg, "elm.swallow.image");
   if (prev_obj)
   {
     edje_object_part_unswallow(pd->event_bg, prev_obj);
     evas_object_del(prev_obj);
   }

   Evas_Object *image = elm_image_add(obj);
   Eina_Bool ret = elm_image_file_set(image, file, group);
   if (!ret)
   {
      edje_object_signal_emit(pd->event_bg, "elm,state,image,hidden", "elm");
      evas_object_del(image);
      return;
   }
   edje_object_part_swallow(pd->event_bg, "elm.swallow.image", image);
   edje_object_signal_emit(pd->event_bg, "elm,state,image,visible", "elm");
}

EOLIAN static void
_efl_ui_popup_bg_repeat_events_set(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Data *pd, Eina_Bool repeat)
{
   pd->bg_repeat_events = repeat;

   evas_object_repeat_events_set(pd->event_bg, repeat);
}

EOLIAN static Eina_Bool
_efl_ui_popup_bg_repeat_events_get(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Data *pd)
{
  return pd->bg_repeat_events;
}

EOLIAN static void
_efl_ui_popup_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME, klass);
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_POPUP_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_popup), \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_popup)

#include "efl_ui_popup.eo.c"
