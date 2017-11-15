#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_widget_scroller.h"
#include "elm_widget_layout.h"

#define MY_CLASS EFL_UI_SCROLLER_CLASS
#define MY_CLASS_NAME "Efl_Ui_Scroller"

#define EFL_UI_SCROLLER_DATA_GET(o, sd) \
  Efl_Ui_Scroller_Data * sd = efl_data_scope_safe_get(o, EFL_UI_SCROLLER_CLASS)

#define EFL_UI_SCROLLER_DATA_GET_OR_RETURN(o, ptr, ...) \
  EFL_UI_SCROLLER_DATA_GET(o, ptr);                         \
  if (EINA_UNLIKELY(!ptr))                            \
    {                                                 \
       CRI("No widget data for object %p (%s)",       \
           o, evas_object_type_get(o));               \
       return __VA_ARGS__;                                    \
    }
static void
_efl_ui_scroller_content_del_cb(void *data,
                                const Efl_Event *event EINA_UNUSED)
{
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(data, sd);

   sd->content = NULL;
   if (!sd->smanager) return;
   efl_ui_scrollbar_visibility_update(sd->smanager);
}

EOLIAN static Eina_Bool
_efl_ui_scroller_efl_container_content_set(Eo *obj,
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

   return EINA_TRUE;
}

static void
_efl_ui_scroller_bar_read_and_update(Eo *obj)
{
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   double vx, vy;

   edje_object_part_drag_value_get
      (wd->resize_obj, "elm.dragable.vbar", NULL, &vy);
   edje_object_part_drag_value_get
      (wd->resize_obj, "elm.dragable.hbar", &vx, NULL);

   efl_ui_scrollbar_position_set(sd->smanager, vx, vy);
}

static void
_efl_ui_scroller_reload_cb(void *data,
                           Evas_Object *obj EINA_UNUSED,
                           const char *emission EINA_UNUSED,
                           const char *source EINA_UNUSED)
{
   Eo *scroller = data;
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(scroller, sd);

   efl_ui_scrollbar_visibility_update(sd->smanager);
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

   sd->freeze_want = efl_ui_scrollable_freeze_get(sd->smanager);
   efl_ui_scrollable_freeze_set(sd->smanager, EINA_TRUE);
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

   efl_ui_scrollable_freeze_set(sd->smanager, sd->freeze_want);
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

   efl_ui_scrollbar_size_get(sd->smanager, &width, &height);
   edje_object_part_drag_size_set(wd->resize_obj, "elm.dragable.hbar", width, 1.0);
   edje_object_part_drag_size_set(wd->resize_obj, "elm.dragable.vbar", 1.0, height);
}

static void
_efl_ui_scroller_bar_pos_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double posx = 0.0, posy = 0.0;

   efl_ui_scrollbar_position_get(sd->smanager, &posx, &posy);
   edje_object_part_drag_value_set(wd->resize_obj, "elm.dragable.hbar", posx, 0.0);
   edje_object_part_drag_value_set(wd->resize_obj, "elm.dragable.vbar", 0.0, posy);
}

static void
_efl_ui_scroller_bar_show_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Scrollbar_Direction type = *(Efl_Ui_Scrollbar_Direction *)(event->info);

   if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
     edje_object_signal_emit(wd->resize_obj, "elm,action,show,hbar", "elm");
   else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
     edje_object_signal_emit(wd->resize_obj, "elm,action,show,vbar", "elm");
}

static void
_efl_ui_scroller_bar_hide_cb(void *data, const Efl_Event *event)
{
   Eo *obj = data;
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Efl_Ui_Scrollbar_Direction type = *(Efl_Ui_Scrollbar_Direction *)(event->info);

   if (type == EFL_UI_SCROLLBAR_DIRECTION_HORIZONTAL)
     edje_object_signal_emit(wd->resize_obj, "elm,action,hide,hbar", "elm");
   else if (type == EFL_UI_SCROLLBAR_DIRECTION_VERTICAL)
     edje_object_signal_emit(wd->resize_obj, "elm,action,hide,vbar", "elm");
}

static void
_scroll_edje_object_attach(Eo *obj)
{
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);

   efl_canvas_layout_signal_callback_add
     (obj, "reload", "elm", _efl_ui_scroller_reload_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag", "elm.dragable.vbar", _efl_ui_scroller_vbar_drag_cb,
     obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag,set", "elm.dragable.vbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag,start", "elm.dragable.vbar",
     _efl_ui_scroller_edje_drag_start_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag,stop", "elm.dragable.vbar",
     _efl_ui_scroller_edje_drag_stop_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag,step", "elm.dragable.vbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag,page", "elm.dragable.vbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "elm,vbar,press", "elm",
     _efl_ui_scroller_vbar_press_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "elm,vbar,unpress", "elm",
     _efl_ui_scroller_vbar_unpress_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag", "elm.dragable.hbar", _efl_ui_scroller_hbar_drag_cb,
     obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag,set", "elm.dragable.hbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag,start", "elm.dragable.hbar",
     _efl_ui_scroller_edje_drag_start_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag,stop", "elm.dragable.hbar",
     _efl_ui_scroller_edje_drag_stop_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag,step", "elm.dragable.hbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "drag,page", "elm.dragable.hbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "elm,hbar,press", "elm",
     _efl_ui_scroller_hbar_press_cb, obj);
   efl_canvas_layout_signal_callback_add
     (obj, "elm,hbar,unpress", "elm",
     _efl_ui_scroller_hbar_unpress_cb, obj);
}

static void
_scroll_edje_object_detach(Evas_Object *obj)
{
   EFL_UI_SCROLLER_DATA_GET_OR_RETURN(obj, sd);

   efl_canvas_layout_signal_callback_del
     (obj, "reload", "elm", _efl_ui_scroller_reload_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag", "elm.dragable.vbar", _efl_ui_scroller_vbar_drag_cb,
     obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag,set", "elm.dragable.vbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag,start", "elm.dragable.vbar",
     _efl_ui_scroller_edje_drag_start_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag,stop", "elm.dragable.vbar",
     _efl_ui_scroller_edje_drag_stop_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag,step", "elm.dragable.vbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag,page", "elm.dragable.vbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "elm,vbar,press", "elm",
     _efl_ui_scroller_vbar_press_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "elm,vbar,unpress", "elm",
     _efl_ui_scroller_vbar_unpress_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag", "elm.dragable.hbar", _efl_ui_scroller_hbar_drag_cb,
     obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag,set", "elm.dragable.hbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag,start", "elm.dragable.hbar",
     _efl_ui_scroller_edje_drag_start_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag,stop", "elm.dragable.hbar",
     _efl_ui_scroller_edje_drag_stop_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag,step", "elm.dragable.hbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "drag,page", "elm.dragable.hbar",
     _efl_ui_scroller_edje_drag_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "elm,hbar,press", "elm",
     _efl_ui_scroller_hbar_press_cb, obj);
   efl_canvas_layout_signal_callback_del
     (obj, "elm,hbar,unpress", "elm",
     _efl_ui_scroller_hbar_unpress_cb, obj);
}

EOLIAN static Eo *
_efl_ui_scroller_efl_object_constructor(Eo *obj,
                                        Efl_Ui_Scroller_Data *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static Eo *
_efl_ui_scroller_efl_object_finalize(Eo *obj,
                                     Efl_Ui_Scroller_Data *sd EINA_UNUSED)
{
   obj = efl_finalize(efl_super(obj, MY_CLASS));

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   efl_ui_layout_theme_set(obj, "scroller", "base", efl_ui_widget_style_get(obj));

   sd->smanager = efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj);
   efl_ui_scroll_manager_mirrored_set(sd->smanager, efl_ui_mirrored_get(obj));

   sd->pan_obj = efl_add(EFL_UI_PAN_CLASS, obj);

   efl_ui_scroll_manager_pan_set(sd->smanager, sd->pan_obj);
   edje_object_part_swallow(wd->resize_obj, "elm.swallow.content", sd->pan_obj);

   evas_object_raise(edje_object_part_object_get(wd->resize_obj, "elm.dragable.hbar"));
   evas_object_raise(edje_object_part_object_get(wd->resize_obj, "elm.dragable.vbar"));

   _scroll_edje_object_attach(obj);

   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SIZE_CHANGED,
                          _efl_ui_scroller_bar_size_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_POS_CHANGED,
                          _efl_ui_scroller_bar_pos_changed_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_SHOW,
                          _efl_ui_scroller_bar_show_cb, obj);
   efl_event_callback_add(obj, EFL_UI_SCROLLBAR_EVENT_BAR_HIDE,
                          _efl_ui_scroller_bar_hide_cb, obj);
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

   efl_del(sd->pan_obj);
   sd->pan_obj = NULL;
   efl_del(sd->smanager);
   sd->smanager = NULL;

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Ui_Theme_Apply
_efl_ui_scroller_elm_widget_theme_apply(Eo *obj, Efl_Ui_Scroller_Data *sd)
{
   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;

   efl_ui_scroll_manager_mirrored_set(sd->smanager, efl_ui_mirrored_get(obj));

   return int_ret;
}

EOLIAN static Eina_Size2D
_efl_ui_scroller_efl_ui_scrollable_interactive_content_size_get(Eo *obj EINA_UNUSED,
                                                            Efl_Ui_Scroller_Data *sd)
{
   return efl_ui_scrollable_content_size_get(sd->smanager);
}

EOLIAN static Eina_Rect
_efl_ui_scroller_efl_ui_scrollable_interactive_viewport_geometry_get(Eo *obj EINA_UNUSED,
                                                                 Efl_Ui_Scroller_Data *sd)
{
   return efl_ui_scrollable_viewport_geometry_get(sd->smanager);
}

EOLIAN static void
_efl_ui_scroller_efl_ui_scrollable_interactive_bounce_enabled_set(Eo *obj EINA_UNUSED,
                                                              Efl_Ui_Scroller_Data *sd,
                                                              Eina_Bool horiz,
                                                              Eina_Bool vert)
{
   efl_ui_scrollable_bounce_enabled_set(sd->smanager, horiz, vert);
}

EOLIAN static void
_efl_ui_scroller_efl_ui_scrollable_interactive_bounce_enabled_get(Eo *obj EINA_UNUSED,
                                                    Efl_Ui_Scroller_Data *sd,
                                                    Eina_Bool *horiz,
                                                    Eina_Bool *vert)
{
   efl_ui_scrollable_bounce_enabled_get(sd->smanager, horiz, vert);
}

EOLIAN static Eina_Bool
_efl_ui_scroller_efl_ui_scrollable_interactive_hold_get(Eo *obj EINA_UNUSED,
                                            Efl_Ui_Scroller_Data *sd)
{
   return efl_ui_scrollable_hold_get(sd->smanager);
}

EOLIAN static void
_efl_ui_scroller_efl_ui_scrollable_interactive_hold_set(Eo *obj EINA_UNUSED,
                                            Efl_Ui_Scroller_Data *sd,
                                            Eina_Bool hold)
{
   efl_ui_scrollable_hold_set(sd->smanager, hold);
}

EOLIAN static Eina_Bool
_efl_ui_scroller_efl_ui_scrollable_interactive_freeze_get(Eo *obj EINA_UNUSED,
                                              Efl_Ui_Scroller_Data *sd)
{
   return efl_ui_scrollable_freeze_get(sd->smanager);
}

EOLIAN static void
_efl_ui_scroller_efl_ui_scrollable_interactive_freeze_set(Eo *obj EINA_UNUSED,
                                                    Efl_Ui_Scroller_Data *sd,
                                                    Eina_Bool freeze)
{
   efl_ui_scrollable_freeze_set(sd->smanager, freeze);
}

EOLIAN static void
_efl_ui_scroller_efl_ui_scrollbar_mode_set(Eo *obj EINA_UNUSED,
                                                      Efl_Ui_Scroller_Data *sd,
                                                      Efl_Ui_Scrollbar_Mode hmode,
                                                      Efl_Ui_Scrollbar_Mode vmode)
{
   efl_ui_scrollbar_mode_set(sd->smanager, hmode, vmode);
}

EOLIAN static void
_efl_ui_scroller_efl_ui_scrollbar_mode_get(Eo *obj EINA_UNUSED,
                                                      Efl_Ui_Scroller_Data *sd,
                                                      Efl_Ui_Scrollbar_Mode *hmode,
                                                      Efl_Ui_Scrollbar_Mode *vmode)
{
   efl_ui_scrollbar_mode_get(sd->smanager, hmode, vmode);
}

EOLIAN static void
_efl_ui_scroller_efl_ui_scrollable_interactive_show(Eo *obj EINA_UNUSED,
                                                Efl_Ui_Scroller_Data *sd,
                                                Eina_Rect rc)
{
   efl_ui_scrollable_show(sd->smanager, rc);
}

EOLIAN static void
_efl_ui_scroller_efl_ui_scrollable_interactive_scroll(Eo *obj EINA_UNUSED,
                                            Efl_Ui_Scroller_Data *sd,
                                            Eina_Rect rc)
{
   efl_ui_scrollable_scroll(sd->smanager, rc);
}

#include "efl_ui_scroller.eo.c"
