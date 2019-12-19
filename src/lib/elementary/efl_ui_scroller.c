#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED
#define EFL_UI_WIDGET_FOCUS_MANAGER_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_scroller.h"
#include "elm_widget_layout.h"

#define MY_CLASS EFL_UI_SCROLLER_CLASS
#define MY_CLASS_PFX efl_ui_scroller

#define MY_CLASS_NAME "Efl.Ui.Scroller"

#define EFL_UI_SCROLLER_DATA_GET(o, sd) \
  Efl_Ui_Scroller_Data * sd = efl_data_scope_safe_get(o, EFL_UI_SCROLLER_CLASS)

#define EFL_UI_SCROLLER_DATA_GET_OR_RETURN(o, ptr, ...) \
  EFL_UI_SCROLLER_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       ERR("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return __VA_ARGS__;                                    \
    }

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {NULL, NULL}
};

static Eina_Bool
_key_action_move(Eo *obj, const char *params)
{
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd, EINA_FALSE);

   const char *dir = params;
   Efl_Ui_Focus_Direction focus_dir = 0;
   Efl_Ui_Focus_Object *focused, *next_target;
   Eina_Rect focused_geom, viewport;
   Eina_Position2D pos;
   Eina_Size2D max;
   Eina_Bool scroller_adjustment = EINA_FALSE;

   pos = efl_ui_scrollable_content_pos_get(obj);
   viewport = efl_ui_scrollable_viewport_geometry_get(obj);
   max = efl_gfx_entity_size_get(sd->content);
   if (!strcmp(dir, "prior"))
     focus_dir = EFL_UI_FOCUS_DIRECTION_PREVIOUS;
   else if (!strcmp(dir, "next"))
     focus_dir = EFL_UI_FOCUS_DIRECTION_NEXT;
   else if (!strcmp(dir, "left"))
     focus_dir = EFL_UI_FOCUS_DIRECTION_LEFT;
   else if (!strcmp(dir, "right"))
     focus_dir = EFL_UI_FOCUS_DIRECTION_RIGHT;
   else if (!strcmp(dir, "up"))
     focus_dir = EFL_UI_FOCUS_DIRECTION_UP;
   else if (!strcmp(dir, "down"))
     focus_dir = EFL_UI_FOCUS_DIRECTION_DOWN;
   else return EINA_FALSE;

   focused = efl_ui_focus_manager_focus_get(obj);
   next_target = efl_ui_focus_manager_request_move(obj, focus_dir, focused, EINA_FALSE);

   //logical movement is handled by focus directly
   if (focused &&
       (focus_dir == EFL_UI_FOCUS_DIRECTION_NEXT ||
        focus_dir == EFL_UI_FOCUS_DIRECTION_PREVIOUS))
     return EINA_FALSE;
   //check if a object that is focused is lapping out of the viewport
   // if this is the case, and the object is lapping out of the viewport in
   // the direction we want to move, then move the scroller
   if (focused)
     {
        Eina_Rectangle_Outside relative;

        focused_geom = efl_gfx_entity_geometry_get(focused);

        relative = eina_rectangle_outside_position(&viewport.rect, &focused_geom.rect);

        //now precisly check if the direction is also lapping out
        if ((focus_dir == EFL_UI_FOCUS_DIRECTION_UP && (relative & EINA_RECTANGLE_OUTSIDE_TOP)) ||
            (focus_dir == EFL_UI_FOCUS_DIRECTION_LEFT && (relative & EINA_RECTANGLE_OUTSIDE_LEFT)) ||
            (focus_dir == EFL_UI_FOCUS_DIRECTION_DOWN && (relative & EINA_RECTANGLE_OUTSIDE_BOTTOM)) ||
            (focus_dir == EFL_UI_FOCUS_DIRECTION_RIGHT && (relative & EINA_RECTANGLE_OUTSIDE_RIGHT)))
          {
             scroller_adjustment = EINA_TRUE;
          }
     }
   //check if there is a next target in the direction where we want to move
   //if not, and the scroller is not at its max in that relation,
   //then move the scroller instead of the focus
   if (!next_target)
     {
        if ((focus_dir == EFL_UI_FOCUS_DIRECTION_UP && (pos.y != 0)) ||
            (focus_dir == EFL_UI_FOCUS_DIRECTION_LEFT && (pos.x != 0)) ||
            (focus_dir == EFL_UI_FOCUS_DIRECTION_DOWN && (pos.y != max.h)) ||
            (focus_dir == EFL_UI_FOCUS_DIRECTION_RIGHT && (pos.x != max.w)))
          {
             scroller_adjustment = EINA_TRUE;
          }
     }
   if (!scroller_adjustment)
     return EINA_FALSE;

   Eina_Position2D step = efl_ui_scrollable_step_size_get(obj);

   if (!strcmp(dir, "left"))
     {
        if (pos.x <= 0) return EINA_FALSE;
        pos.x -= step.x;
     }
   else if (!strcmp(dir, "right"))
     {
        if (pos.x >= (max.w - viewport.w)) return EINA_FALSE;
        pos.x += step.x;
     }
   else if (!strcmp(dir, "up"))
     {
        if (pos.y <= 0) return EINA_FALSE;
        pos.y -= step.y;
     }
   else if (!strcmp(dir, "down"))
     {
        if (pos.y >= (max.h - viewport.h)) return EINA_FALSE;
        pos.y += step.y;
     }
   else if (!strcmp(dir, "first"))
     {
        pos.y = 0;
     }
   else if (!strcmp(dir, "last"))
     {
        pos.y = max.h - viewport.h;
     }
   else return EINA_FALSE;

   efl_ui_scrollable_scroll(obj, EINA_RECT(pos.x, pos.y, viewport.w, viewport.h), EINA_FALSE);

   return EINA_TRUE;
}


static void
_efl_ui_scroller_content_del_cb(void *data,
                                const Efl_Event *event EINA_UNUSED)
{
   efl_content_unset(data);
}

EOLIAN static Eina_Bool
_efl_ui_scroller_efl_content_content_set(Eo *obj,
                                           Efl_Ui_Scroller_Data *sd,
                                           Evas_Object *content)
{
   if (sd->content)
     {
        efl_content_set(sd->pan_obj, NULL);
        efl_event_callback_del(sd->content, EFL_EVENT_INVALIDATE,
                               _efl_ui_scroller_content_del_cb, obj);
        efl_del(sd->content);
        sd->content = NULL;
     }

   if (content && !efl_ui_widget_sub_object_add(obj, content))
     {
        efl_event_callback_call(obj, EFL_CONTENT_EVENT_CONTENT_CHANGED, NULL);
        return EINA_FALSE;
     }

   sd->content = content;
   efl_event_callback_call(obj, EFL_CONTENT_EVENT_CONTENT_CHANGED, sd->content);
   if (!content) return EINA_TRUE;

   efl_event_callback_add(sd->content, EFL_EVENT_INVALIDATE,
                          _efl_ui_scroller_content_del_cb, obj);

   efl_content_set(sd->pan_obj, content);

   efl_canvas_group_change(obj);

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_scroller_efl_content_content_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroller_Data *pd)
{
   return pd->content;
}

EOLIAN static Efl_Gfx_Entity*
_efl_ui_scroller_efl_content_content_unset(Eo *obj EINA_UNUSED, Efl_Ui_Scroller_Data *pd)
{
   Efl_Gfx_Entity *old_content = pd->content;

   efl_content_unset(pd->pan_obj);
   efl_ui_widget_sub_object_del(obj, old_content);
   efl_event_callback_del(pd->content, EFL_EVENT_INVALIDATE, _efl_ui_scroller_content_del_cb, obj);
   pd->content = NULL;
   efl_event_callback_call(obj, EFL_CONTENT_EVENT_CONTENT_CHANGED, NULL);
   if (pd->smanager)
     {
        efl_ui_scrollbar_bar_visibility_update(pd->smanager);
     }

   return old_content;
}

static void
_efl_ui_scroller_pan_resized_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   if (efl_canvas_scene_group_objects_calculating_get(evas_object_evas_get(data)))
     efl_canvas_group_calculate(data);
   else
     efl_canvas_group_change(data);
}

static void
_focused_element(void *data, const Efl_Event *event)
{
   Eina_Rect geom;
   Efl_Ui_Focus_Object *obj = data;
   Efl_Ui_Focus_Object *focus = efl_ui_focus_manager_focus_get(event->object);
   Eina_Position2D pos, pan;

   if (!focus) return;

   geom = efl_ui_focus_object_focus_geometry_get(focus);
   pos = efl_gfx_entity_position_get(obj);
   pan = efl_ui_scrollable_content_pos_get(obj);
   geom.x += pan.x - pos.x;
   geom.y += pan.y - pos.y;

   efl_ui_scrollable_scroll(obj, geom, EINA_TRUE);
}

EOLIAN static Eo *
_efl_ui_scroller_efl_object_constructor(Eo *obj,
                                        Efl_Ui_Scroller_Data *sd)
{
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "scroller");
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   sd->smanager = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj);
   efl_ui_mirrored_set(sd->smanager, efl_ui_mirrored_get(obj));
   efl_composite_attach(obj, sd->smanager);

   efl_ui_scroll_connector_bind(obj, sd->smanager);

   return obj;
}

EOLIAN static Eo *
_efl_ui_scroller_efl_object_finalize(Eo *obj,
                                     Efl_Ui_Scroller_Data *sd)
{
   obj = efl_finalize(efl_super(obj, MY_CLASS));

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   sd->pan_obj = efl_add(EFL_UI_PAN_CLASS, obj);

   efl_ui_scroll_manager_pan_set(sd->smanager, sd->pan_obj);
   edje_object_part_swallow(wd->resize_obj, "efl.content", sd->pan_obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   efl_event_callback_add(sd->pan_obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
                          _efl_ui_scroller_pan_resized_cb, obj);

   efl_event_callback_add(obj, EFL_UI_FOCUS_MANAGER_EVENT_MANAGER_FOCUS_CHANGED, _focused_element, obj);
   return obj;
}

EOLIAN static void
_efl_ui_scroller_efl_object_invalidate(Eo *obj, Efl_Ui_Scroller_Data *pd)
{
   // pan is given to edje, which reparents it, which forces us to manually deleting it
   efl_event_callback_del(pd->pan_obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
                          _efl_ui_scroller_pan_resized_cb, obj);
   efl_del(pd->pan_obj);
   pd->pan_obj = NULL;

   efl_invalidate(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_scroller_efl_object_destructor(Eo *obj,
                                       Efl_Ui_Scroller_Data *sd)
{
   efl_ui_scroll_connector_unbind(obj);
   sd->smanager = NULL;

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_scroller_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Scroller_Data *sd)
{
   Eina_Size2D min = {0, 0}, max = {0, 0}, size = {-1, -1};
   Eina_Rect view = {};
   Evas_Coord vmw = 0, vmh = 0;
   double xw = 0.0, yw = 0.0;

   efl_canvas_group_need_recalculate_set(obj, EINA_FALSE);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->content)
     {
        min = efl_gfx_hint_size_combined_min_get(sd->content);
        max = efl_gfx_hint_size_combined_max_get(sd->content);
        efl_gfx_hint_weight_get(sd->content, &xw, &yw);
     }

   if (sd->smanager)
     view = efl_ui_scrollable_viewport_geometry_get(sd->smanager);

   if (xw > 0.0)
     {
        if ((min.w > 0) && (view.w < min.w))
          view.w = min.w;
        else if ((max.w > 0) && (view.w > max.w))
          view.w = max.w;
     }
   else if (min.w > 0)
     view.w = min.w;

   if (yw > 0.0)
     {
        if ((min.h > 0) && (view.h < min.h))
          view.h = min.h;
        else if ((max.h > 0) && (view.h > max.h))
          view.h = max.h;
     }
   else if (min.h > 0)
     view.h = min.h;

   if (sd->content) efl_gfx_entity_size_set(sd->content, EINA_SIZE2D(view.w, view.h));

   edje_object_message_signal_process(wd->resize_obj);
   edje_object_size_min_calc(wd->resize_obj, &vmw, &vmh);

   if (sd->match_content_w) size.w = vmw + min.w;
   if (sd->match_content_h) size.h = vmh + min.h;

   max = efl_gfx_hint_size_max_get(obj);
   if ((max.w > 0) && (size.w > max.w)) size.w = max.w;
   if ((max.h > 0) && (size.h > max.h)) size.h = max.h;

   efl_gfx_hint_size_restricted_min_set(obj, size);
}

EOLIAN static Eina_Error
_efl_ui_scroller_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Scroller_Data *sd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   efl_ui_mirrored_set(sd->smanager, efl_ui_mirrored_get(obj));

   return int_ret;
}

EOLIAN static void
_efl_ui_scroller_efl_ui_scrollable_match_content_set(Eo *obj EINA_UNUSED,
                                                                 Efl_Ui_Scroller_Data *sd,
                                                                 Eina_Bool match_content_w,
                                                                 Eina_Bool match_content_h)
{
   sd->match_content_w = !!match_content_w;
   sd->match_content_h = !!match_content_h;

   efl_ui_scrollable_match_content_set(sd->smanager, match_content_w, match_content_h);

   efl_canvas_group_change(obj);
}

EOLIAN static Eina_Bool
_efl_ui_scroller_efl_ui_widget_focus_state_apply(Eo *obj, Efl_Ui_Scroller_Data *pd EINA_UNUSED, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect EINA_UNUSED)
{
   //undepended from logical or not we always reigster as full with ourself as redirect
   configured_state->logical = EINA_TRUE;
   return efl_ui_widget_focus_state_apply(efl_super(obj, MY_CLASS), current_state, configured_state, obj);
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_scroller_efl_ui_widget_focus_manager_focus_manager_create(Eo *obj, Efl_Ui_Scroller_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   Efl_Ui_Focus_Manager *manager;
   manager = efl_add(EFL_UI_FOCUS_MANAGER_ROOT_FOCUS_CLASS, obj,
                     efl_ui_focus_manager_root_set(efl_added, root));

   return manager;
}

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_scroller, Efl_Ui_Scroller_Data)

/* Internal EO APIs and hidden overrides */

#include "efl_ui_scroller.eo.c"
