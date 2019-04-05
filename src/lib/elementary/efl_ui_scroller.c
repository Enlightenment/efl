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
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(data, sd);

   sd->content = NULL;
   if (!sd->smanager) return;
   efl_ui_scrollbar_bar_visibility_update(sd->smanager);
}

EOLIAN static Eina_Bool
_efl_ui_scroller_efl_content_content_set(Eo *obj,
                                           Efl_Ui_Scroller_Data *sd,
                                           Evas_Object *content)
{
   if (sd->content)
     {
        efl_content_set(sd->pan_obj, NULL);
        efl_event_callback_del(sd->content, EFL_EVENT_DEL,
                               _efl_ui_scroller_content_del_cb, obj);
     }

   sd->content = content;
   if (!content) return EINA_TRUE;

   efl_event_callback_add(sd->content, EFL_EVENT_DEL,
                          _efl_ui_scroller_content_del_cb, obj);

   efl_content_set(sd->pan_obj, content);

   elm_layout_sizing_eval(obj);

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

   pd->content = NULL;
   if (pd->smanager)
     {
        efl_ui_scrollbar_bar_visibility_update(pd->smanager);
     }

   return old_content;
}

static void
_efl_ui_scroller_bar_read_and_update(Eo *obj)
{
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   double vx, vy;

   edje_object_part_drag_value_get
      (wd->resize_obj, "efl.dragable.vbar", NULL, &vy);
   edje_object_part_drag_value_get
      (wd->resize_obj, "efl.dragable.hbar", &vx, NULL);

   efl_ui_scrollbar_bar_position_set(sd->smanager, vx, vy);
}

static void
_efl_ui_scroller_reload_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Eo *scroller = data;
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(scroller, sd);

   efl_ui_scrollbar_bar_visibility_update(sd->smanager);
}

static void
_efl_ui_scroller_vbar_drag_cb(void *data,
                              Evas_Object *obj EINA_UNUSED,
                              const char *emission EINA_UNUSED,
                              const char *source EINA_UNUSED)
{
   _efl_ui_scroller_bar_read_and_update(data);

   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
}

static void
_efl_ui_scroller_vbar_press_cb(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               const char *emission EINA_UNUSED,
                               const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
}

static void
_efl_ui_scroller_vbar_unpress_cb(void *data,
                                 Evas_Object *obj EINA_UNUSED,
                                 const char *emission EINA_UNUSED,
                                 const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_VERTICAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
}

static void
_efl_ui_scroller_edje_drag_start_cb(void *data,
                                 Evas_Object *obj EINA_UNUSED,
                                 const char *emission EINA_UNUSED,
                                 const char *source EINA_UNUSED)
{
   Eo *scroller = data;
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(scroller, sd);

   _efl_ui_scroller_bar_read_and_update(scroller);

   sd->freeze_want = efl_ui_scrollable_scroll_freeze_get(sd->smanager);
   efl_ui_scrollable_scroll_freeze_set(sd->smanager, EINA_TRUE);
   efl_event_callback_call(scroller, EFL_UI_EVENT_SCROLL_DRAG_START, NULL);
}

static void
_efl_ui_scroller_edje_drag_stop_cb(void *data,
                                Evas_Object *obj EINA_UNUSED,
                                const char *emission EINA_UNUSED,
                                const char *source EINA_UNUSED)
{
   Eo *scroller = data;
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(scroller, sd);

   _efl_ui_scroller_bar_read_and_update(scroller);

   efl_ui_scrollable_scroll_freeze_set(sd->smanager, sd->freeze_want);
   efl_event_callback_call(scroller, EFL_UI_EVENT_SCROLL_DRAG_STOP, NULL);
}

static void
_efl_ui_scroller_edje_drag_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   _efl_ui_scroller_bar_read_and_update(data);
}

static void
_efl_ui_scroller_hbar_drag_cb(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         const char *emission EINA_UNUSED,
                         const char *source EINA_UNUSED)
{
   _efl_ui_scroller_bar_read_and_update(data);

   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_DRAG, &type);
}

static void
_efl_ui_scroller_hbar_press_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_PRESS, &type);
}

static void
_efl_ui_scroller_hbar_unpress_cb(void *data,
                            Evas_Object *obj EINA_UNUSED,
                            const char *emission EINA_UNUSED,
                            const char *source EINA_UNUSED)
{
   Efl_Ui_Scrollbar_Direction type = EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL;
   efl_event_callback_call(data, EFL_UI_SCROLLBAR_EVENT_BAR_UNPRESS, &type);
}

static void
_efl_ui_scroller_bar_size_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double width = 0.0, height = 0.0;

   edje_object_calc_force(wd->resize_obj);
   efl_ui_scrollbar_bar_size_get(sd->smanager, &width, &height);
   edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.hbar", width, 1.0);
   edje_object_part_drag_size_set(wd->resize_obj, "efl.dragable.vbar", 1.0, height);
}

static void
_efl_ui_scroller_bar_pos_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double posx = 0.0, posy = 0.0;

   efl_ui_scrollbar_bar_position_get(sd->smanager, &posx, &posy);
   edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.hbar", posx, 0.0);
   edje_object_part_drag_value_set(wd->resize_obj, "efl.dragable.vbar", 0.0, posy);
   edje_object_signal_emit(wd->resize_obj, "efl,action,scroll", "efl");
}

static void
_efl_ui_scroller_bar_show_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Scrollbar_Direction type = *(Efl_Ui_Scrollbar_Direction *)(event->info);

   if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,show,hbar", "efl");
   else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,show,vbar", "efl");
}

static void
_efl_ui_scroller_bar_hide_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Scrollbar_Direction type = *(Efl_Ui_Scrollbar_Direction *)(event->info);

   if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,hide,hbar", "efl");
   else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
     edje_object_signal_emit(wd->resize_obj, "efl,action,hide,vbar", "efl");
}

static void
_scroll_edje_object_attach(Eo *obj)
{
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);

   efl_layout_signal_callback_add
     (obj, "reload", "efl",
      obj, _efl_ui_scroller_reload_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag", "efl.dragable.vbar",
      obj, _efl_ui_scroller_vbar_drag_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag,set", "efl.dragable.vbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag,start", "efl.dragable.vbar",
      obj, _efl_ui_scroller_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag,stop", "efl.dragable.vbar",
      obj, _efl_ui_scroller_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag,step", "efl.dragable.vbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag,page", "efl.dragable.vbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "efl,vbar,press", "efl",
      obj, _efl_ui_scroller_vbar_press_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "efl,vbar,unpress", "efl",
      obj, _efl_ui_scroller_vbar_unpress_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag", "efl.dragable.hbar",
      obj, _efl_ui_scroller_hbar_drag_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag,set", "efl.dragable.hbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag,start", "efl.dragable.hbar",
      obj, _efl_ui_scroller_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag,stop", "efl.dragable.hbar",
      obj, _efl_ui_scroller_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag,step", "efl.dragable.hbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "drag,page", "efl.dragable.hbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "efl,hbar,press", "efl",
      obj, _efl_ui_scroller_hbar_press_cb, NULL);
   efl_layout_signal_callback_add
     (obj, "efl,hbar,unpress", "efl",
      obj, _efl_ui_scroller_hbar_unpress_cb, NULL);
}

static void
_scroll_edje_object_detach(Evas_Object *obj)
{
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);

   efl_layout_signal_callback_del
     (obj, "reload", "efl",
      obj, _efl_ui_scroller_reload_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag", "efl.dragable.vbar",
      obj, _efl_ui_scroller_vbar_drag_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag,set", "efl.dragable.vbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag,start", "efl.dragable.vbar",
      obj, _efl_ui_scroller_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag,stop", "efl.dragable.vbar",
      obj, _efl_ui_scroller_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag,step", "efl.dragable.vbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag,page", "efl.dragable.vbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "efl,vbar,press", "efl",
      obj, _efl_ui_scroller_vbar_press_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "efl,vbar,unpress", "efl",
      obj, _efl_ui_scroller_vbar_unpress_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag", "efl.dragable.hbar",
      obj, _efl_ui_scroller_hbar_drag_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag,set", "efl.dragable.hbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag,start", "efl.dragable.hbar",
      obj, _efl_ui_scroller_edje_drag_start_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag,stop", "efl.dragable.hbar",
      obj, _efl_ui_scroller_edje_drag_stop_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag,step", "efl.dragable.hbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "drag,page", "efl.dragable.hbar",
      obj, _efl_ui_scroller_edje_drag_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "efl,hbar,press", "efl",
      obj, _efl_ui_scroller_hbar_press_cb, NULL);
   efl_layout_signal_callback_del
     (obj, "efl,hbar,unpress", "efl",
      obj, _efl_ui_scroller_hbar_unpress_cb, NULL);
}

static void
_efl_ui_scroller_pan_resized_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

static void
_efl_ui_scroller_resized_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

static void
_efl_ui_scroller_size_hint_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
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
                                        Efl_Ui_Scroller_Data *sd EINA_UNUSED)
{
   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "scroller");
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static Eo *
_efl_ui_scroller_efl_object_finalize(Eo *obj,
                                     Efl_Ui_Scroller_Data *sd EINA_UNUSED)
{
   obj = efl_finalize(efl_super(obj, MY_CLASS));

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   sd->smanager = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj);
   efl_ui_mirrored_set(sd->smanager, efl_ui_mirrored_get(obj));
   efl_composite_attach(obj, sd->smanager);

   sd->pan_obj = efl_add(EFL_UI_PAN_CLASS, obj);

   efl_ui_scroll_manager_pan_set(sd->smanager, sd->pan_obj);
   edje_object_part_swallow(wd->resize_obj, "efl.content", sd->pan_obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   _scroll_edje_object_attach(obj);

   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED,
                          _efl_ui_scroller_bar_size_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED,
                          _efl_ui_scroller_bar_pos_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW,
                          _efl_ui_scroller_bar_show_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE,
                          _efl_ui_scroller_bar_hide_cb, obj);
   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
                          _efl_ui_scroller_resized_cb, obj);
   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED,
                          _efl_ui_scroller_size_hint_changed_cb, obj);
   efl_event_callback_add(sd->pan_obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
                          _efl_ui_scroller_pan_resized_cb, obj);

   efl_event_callback_add(obj, EFL_UI_FOCUS_MANAGER_EVENT_MANAGER_FOCUS_CHANGED, _focused_element, obj);
   return obj;
}

EOLIAN static void
_efl_ui_scroller_efl_object_destructor(Eo *obj,
                                       Efl_Ui_Scroller_Data *sd EINA_UNUSED)
{
   _scroll_edje_object_detach(obj);

   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED,
                          _efl_ui_scroller_bar_size_changed_cb, obj);
   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED,
                          _efl_ui_scroller_bar_pos_changed_cb, obj);
   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW,
                          _efl_ui_scroller_bar_show_cb, obj);
   efl_event_callback_del(obj, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE,
                          _efl_ui_scroller_bar_hide_cb, obj);
   efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
                          _efl_ui_scroller_resized_cb, obj);
   efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED,
                          _efl_ui_scroller_size_hint_changed_cb, obj);
   efl_event_callback_del(sd->pan_obj, EFL_GFX_ENTITY_EVENT_SIZE_CHANGED,
                          _efl_ui_scroller_pan_resized_cb, obj);
   efl_del(sd->pan_obj);
   sd->pan_obj = NULL;
   sd->smanager = NULL;

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_scroller_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Scroller_Data *sd)
{
   Eina_Size2D min = {0, 0}, max = {0, 0}, size = {-1, -1};
   Eina_Rect view = {};
   Evas_Coord vmw = 0, vmh = 0;
   double xw = 0.0, yw = 0.0;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->content)
     {
        min = efl_gfx_hint_size_combined_min_get(sd->content);
        max = efl_gfx_hint_size_max_get(sd->content);
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

   edje_object_size_min_calc(wd->resize_obj, &vmw, &vmh);

   if (sd->match_content_w) size.w = vmw + min.w;
   if (sd->match_content_h) size.h = vmh + min.h;

   max = efl_gfx_hint_size_max_get(obj);
   if ((max.w > 0) && (size.w > max.w)) size.w = max.w;
   if ((max.h > 0) && (size.h > max.h)) size.h = max.h;

   efl_gfx_hint_size_min_set(obj, size);
}

EOLIAN static Eina_Error
_efl_ui_scroller_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Scroller_Data *sd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   efl_ui_mirrored_set(sd->smanager, efl_ui_mirrored_get(obj));

   elm_layout_sizing_eval(obj);

   return int_ret;
}

EOLIAN static void
_efl_ui_scroller_efl_ui_scrollable_interactive_match_content_set(Eo *obj EINA_UNUSED,
                                                                 Efl_Ui_Scroller_Data *sd,
                                                                 Eina_Bool match_content_w,
                                                                 Eina_Bool match_content_h)
{
   sd->match_content_w = !!match_content_w;
   sd->match_content_h = !!match_content_h;

   efl_ui_scrollable_match_content_set(sd->smanager, match_content_w, match_content_h);

   elm_layout_sizing_eval(obj);
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

#define EFL_UI_SCROLLER_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_scroller)

#include "efl_ui_scroller.eo.c"
