#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED
#define ELM_INTERFACE_ATSPI_VALUE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_slider_private.h"
#include "elm_widget_layout.h"

#include "efl_ui_slider_internal_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_SLIDER_CLASS

#define MY_CLASS_NAME "Efl.Ui.Slider"
#define MY_CLASS_NAME_LEGACY "elm_slider"
#define SLIDER_DELAY_CHANGED_INTERVAL 0.2
#define SLIDER_STEP 0.05

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.icon"},
   {"end", "elm.swallow.end"},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"default", "elm.text"},
   {NULL, NULL}
};

static const char SIG_CHANGED[] = "changed";
static const char SIG_DELAY_CHANGED[] = "delay,changed";
static const char SIG_DRAG_START[] = "slider,drag,start";
static const char SIG_DRAG_STOP[] = "slider,drag,stop";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_DELAY_CHANGED, ""},
   {SIG_DRAG_START, ""},
   {SIG_DRAG_STOP, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static void _min_max_set(Evas_Object *obj);

static Eina_Bool _key_action_drag(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"drag", _key_action_drag},
   {NULL, NULL}
};

static Eina_Bool
_delay_change(void *data)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);

   sd->delay = NULL;
   efl_event_callback_legacy_call(data, EFL_UI_SLIDER_EVENT_DELAY_CHANGED, NULL);

   if (_elm_config->atspi_mode)
     elm_interface_atspi_accessible_value_changed_signal_emit(data);

   return ECORE_CALLBACK_CANCEL;
}

static inline Eina_Bool
_is_horizontal(Efl_Orient orientation)
{
   if (orientation == EFL_ORIENT_LEFT ||
       orientation == EFL_ORIENT_RIGHT)
     return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
_is_inverted(Efl_Orient orientation)
{
   if (orientation == EFL_ORIENT_LEFT ||
       orientation == EFL_ORIENT_UP)
     return EINA_TRUE;

   return EINA_FALSE;
}

static Efl_Orient
_orientation_get(Eina_Bool horizontal, Eina_Bool inverted)
{
   if (horizontal)
     {
        if (inverted)
          return EFL_ORIENT_LEFT;
        else
          return EFL_ORIENT_RIGHT;
     }
   else
     {
        if (inverted)
          return EFL_ORIENT_UP;
        else
          return EFL_ORIENT_DOWN;
     }
}

static void
_val_fetch(Evas_Object *obj, Eina_Bool user_event)
{
   Eina_Bool rtl;
   double posx = 0.0, posy = 0.0, pos = 0.0, val;
   double posx2 = 0.0, posy2 = 0.0, pos2 = 0.0, val2;

   EFL_UI_SLIDER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.dragable.slider", &posx, &posy);
   if (_is_horizontal(sd->orientation)) pos = posx;
   else pos = posy;

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.dragable2.slider", &posx2, &posy2);
   if (_is_horizontal(sd->orientation)) pos2 = posx2;
   else pos2 = posy2;

   rtl = efl_ui_mirrored_get(obj);
   if ((!rtl && _is_inverted(sd->orientation)) ||
       (rtl && ((sd->orientation == EFL_ORIENT_UP) ||
                (sd->orientation == EFL_ORIENT_RIGHT))))
     {
        pos = 1.0 - pos;
        pos2 = 1.0 - pos2;
     }

   val = (pos * (sd->val_max - sd->val_min)) + sd->val_min;
   val2 = (pos2 * (sd->val_max - sd->val_min)) + sd->val_min;

   if (fabs(val - sd->val) > DBL_EPSILON)
     {
        sd->val = val;
        sd->intvl_from = val;
        if (user_event)
          {
             efl_event_callback_legacy_call(obj, EFL_UI_SLIDER_EVENT_CHANGED, NULL);
             ecore_timer_del(sd->delay);
             sd->delay = ecore_timer_add(SLIDER_DELAY_CHANGED_INTERVAL, _delay_change, obj);
          }
     }
   if (fabs(val2 - sd->intvl_to) > DBL_EPSILON)
     {
        sd->intvl_to = val2;
        if (user_event)
          {
             efl_event_callback_legacy_call(obj, EFL_UI_SLIDER_EVENT_CHANGED, NULL);
             ecore_timer_del(sd->delay);
             sd->delay = ecore_timer_add(SLIDER_DELAY_CHANGED_INTERVAL, _delay_change, obj);
          }
     }
}

static void
_val_set(Evas_Object *obj)
{
   Eina_Bool rtl;
   double pos, pos2;

   EFL_UI_SLIDER_DATA_GET(obj, sd);

   if (sd->val_max > sd->val_min)
     {
        pos = (sd->val - sd->val_min) / (sd->val_max - sd->val_min);
        pos2 = (sd->intvl_to - sd->val_min) / (sd->val_max - sd->val_min);
     }
   else
     {
        pos = 0.0;
        pos2 = 0.0;
     }

   if (pos < 0.0) pos = 0.0;
   else if (pos > 1.0)
     pos = 1.0;
   if (pos2 < 0.0) pos2 = 0.0;
   else if (pos2 > 1.0)
     pos2 = 1.0;

   rtl = efl_ui_mirrored_get(obj);
   if ((!rtl && _is_inverted(sd->orientation)) ||
       (rtl && ((sd->orientation == EFL_ORIENT_UP) ||
                (sd->orientation == EFL_ORIENT_RIGHT))))
     {
        pos = 1.0 - pos;
        pos2 = 1.0 - pos2;
     }

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.slider", pos, pos);

   if (sd->intvl_enable)
     edje_object_part_drag_value_set
        (wd->resize_obj, "elm.dragable2.slider", pos2, pos2);

   // emit accessiblity event also if value was chagend by API
   if (_elm_config->atspi_mode)
     elm_interface_atspi_accessible_value_changed_signal_emit(obj);
}

static void
_units_set(Evas_Object *obj)
{
   EFL_UI_SLIDER_DATA_GET(obj, sd);

   if (sd->units_format_func)
     {
        char *buf;

        buf = sd->units_format_func(sd->val);
        if (buf)
          {
             elm_layout_text_set(obj, "elm.units", buf);
             if (!sd->units_show)
               {
                  elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");
                  sd->units_show = EINA_TRUE;
               }
          }
        else
          {
             elm_layout_text_set(obj, "elm.units", NULL);
             if (sd->units_show)
               {
                  elm_layout_signal_emit(obj, "elm,state,units,hidden", "elm");
                  sd->units_show = EINA_FALSE;
               }
          }
        if (sd->units_format_free) sd->units_format_free(buf);
     }
   else if (sd->units)
     {
        char buf[1024];

        if (!sd->intvl_enable)
          snprintf(buf, sizeof(buf), sd->units, sd->val);
        else
          {
             double v1, v2;

             efl_ui_range_interval_get(obj, &v1, &v2);
             snprintf(buf, sizeof(buf), sd->units, v2 - v1);
          }

        elm_layout_text_set(obj, "elm.units", buf);
        if (!sd->units_show)
          {
             elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");
             sd->units_show = EINA_TRUE;
          }
     }
   else
     {
        elm_layout_text_set(obj, "elm.units", NULL);
        if (sd->units_show)
          {
             elm_layout_signal_emit(obj, "elm,state,units,hidden", "elm");
             sd->units_show = EINA_FALSE;
          }
     }
}

static void
_indicator_set(Evas_Object *obj)
{
   EFL_UI_SLIDER_DATA_GET(obj, sd);

   if (sd->indicator_format_func)
     {
        char *buf;

        buf = sd->indicator_format_func(sd->val);
        elm_layout_text_set(obj, "elm.indicator", buf);
        elm_layout_text_set(obj, "elm.dragable.slider:elm.indicator", buf);
        if (sd->popup)
          edje_object_part_text_set(sd->popup, "elm.indicator", buf);
        if (sd->popup2)
          {
             if (sd->indicator_format_free) sd->indicator_format_free(buf);
             buf = sd->indicator_format_func(sd->intvl_to);
             elm_layout_text_set(obj, "elm.dragable2.slider:elm.indicator", buf);
             edje_object_part_text_set(sd->popup2, "elm.indicator", buf);
          }

        if (sd->indicator_format_free) sd->indicator_format_free(buf);
     }
   else if (sd->indicator)
     {
        char buf[1024];

        snprintf(buf, sizeof(buf), sd->indicator, sd->val);
        elm_layout_text_set(obj, "elm.indicator", buf);
        elm_layout_text_set(obj, "elm.dragable.slider:elm.indicator", buf);
        if (sd->popup)
          edje_object_part_text_set(sd->popup, "elm.indicator", buf);
        if (sd->popup2)
          {
             memset(buf, 0, 1024);
             snprintf(buf, sizeof(buf), sd->indicator, sd->intvl_to);
             elm_layout_text_set(obj, "elm.dragable2.slider:elm.indicator", buf);
             edje_object_part_text_set(sd->popup2, "elm.indicator", buf);
          }
     }
   else
     {
        elm_layout_text_set(obj, "elm.indicator", NULL);
        elm_layout_text_set(obj, "elm.dragable.slider:elm.indicator", NULL);
        if (sd->popup)
          edje_object_part_text_set(sd->popup, "elm.indicator", NULL);
        if (sd->popup2)
          {
             elm_layout_text_set(obj, "elm.dragable2.slider:elm.indicator", NULL);
             edje_object_part_text_set(sd->popup2, "elm.indicator", NULL);
          }
     }
}

static void
_slider_update(Evas_Object *obj, Eina_Bool user_event)
{
   _val_fetch(obj, user_event);
   evas_object_smart_changed(obj);
}

static void
_drag(void *data,
      Evas_Object *obj EINA_UNUSED,
      const char *emission EINA_UNUSED,
      const char *source EINA_UNUSED)
{
   _slider_update(data, EINA_TRUE);
}

static void
_drag_start(void *data,
            Evas_Object *obj EINA_UNUSED,
            const char *emission EINA_UNUSED,
            const char *source EINA_UNUSED)
{
   if (!elm_widget_focus_get(data))
     elm_object_focus_set(data, EINA_TRUE);
   _slider_update(data, EINA_TRUE);
   efl_event_callback_legacy_call(data, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_START, NULL);
   elm_widget_scroll_freeze_push(data);
}

static void
_drag_stop(void *data,
           Evas_Object *obj EINA_UNUSED,
           const char *emission EINA_UNUSED,
           const char *source EINA_UNUSED)
{
   _slider_update(data, EINA_TRUE);
   efl_event_callback_legacy_call(data, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_STOP, NULL);
   elm_widget_scroll_freeze_pop(data);
}

static void
_drag_step(void *data,
           Evas_Object *obj EINA_UNUSED,
           const char *emission EINA_UNUSED,
           const char *source EINA_UNUSED)
{
   _slider_update(data, EINA_TRUE);
}

static void
_drag_up(void *data,
         Evas_Object *obj EINA_UNUSED,
         const char *emission EINA_UNUSED,
         const char *source EINA_UNUSED)
{
   double step;

   EFL_UI_SLIDER_DATA_GET(data, sd);
   step = sd->step;

   if (_is_inverted(sd->orientation)) step *= -1.0;

   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   edje_object_part_drag_step
     (wd->resize_obj, "elm.dragable.slider", step, step);
}

static void
_drag_down(void *data,
           Evas_Object *obj EINA_UNUSED,
           const char *emission EINA_UNUSED,
           const char *source EINA_UNUSED)
{
   double step;

   EFL_UI_SLIDER_DATA_GET(data, sd);
   step = -sd->step;

   if (_is_inverted(sd->orientation)) step *= -1.0;

   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   edje_object_part_drag_step
     (wd->resize_obj, "elm.dragable.slider", step, step);
}

static void
_popup_show(void *data,
            Evas_Object *obj EINA_UNUSED,
            const char *emission EINA_UNUSED,
            const char *source EINA_UNUSED)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);
   if (sd->popup &&
       (sd->indicator_visible_mode != ELM_SLIDER_INDICATOR_VISIBLE_MODE_NONE))
     {
        evas_object_raise(sd->popup);
        evas_object_show(sd->popup);
        sd->popup_visible = EINA_TRUE;
        edje_object_signal_emit(sd->popup, "popup,show", "elm"); // XXX: for compat
        edje_object_signal_emit(sd->popup, "elm,popup,show", "elm");
     }
   if (sd->popup2 &&
       (sd->indicator_visible_mode != ELM_SLIDER_INDICATOR_VISIBLE_MODE_NONE))
     {
        evas_object_raise(sd->popup2);
        evas_object_show(sd->popup2);
        edje_object_signal_emit(sd->popup2, "popup,show", "elm"); // XXX: for compat
        edje_object_signal_emit(sd->popup2, "elm,popup,show", "elm");
     }
   ELM_SAFE_FREE(sd->wheel_indicator_timer, ecore_timer_del);
}

static void
_popup_hide(void *data,
            Evas_Object *obj EINA_UNUSED,
            const char *emission EINA_UNUSED,
            const char *source EINA_UNUSED)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);

   if (!sd->popup_visible || !sd->popup) return;

   if (sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ALWAYS) return;
   if ((sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS) &&
       elm_widget_focus_get(data))
     return;

   edje_object_signal_emit(sd->popup, "popup,hide", "elm"); // XXX: for compat
   edje_object_signal_emit(sd->popup, "elm,popup,hide", "elm");

   if (sd->popup2)
     {
        edje_object_signal_emit(sd->popup2, "popup,hide", "elm"); // XXX: for compat
        edje_object_signal_emit(sd->popup2, "elm,popup,hide", "elm");
     }
}

static void
_popup_hide_done(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 const char *emission EINA_UNUSED,
                 const char *source EINA_UNUSED)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);
   if (sd->popup)
     {
        if (!((elm_widget_focus_get(data)) &&
              (sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS)))
          {
             evas_object_hide(sd->popup);
             sd->popup_visible = EINA_FALSE;
          }
     }
   if (sd->popup2)
     {
        if (!((elm_widget_focus_get(data)) &&
              (sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS)))
          {
             evas_object_hide(sd->popup2);
          }
     }
}

static void
_popup_emit(void *data,
            Evas_Object *obj EINA_UNUSED,
            const char *emission,
            const char *source)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);
   if (sd->popup)
     {
        edje_object_signal_emit(sd->popup, emission, source);
     }
   if (sd->popup2)
     {
        edje_object_signal_emit(sd->popup2, emission, source);
     }
}

static Eina_Bool
_key_action_drag(Evas_Object *obj, const char *params)
{
   EFL_UI_SLIDER_DATA_GET(obj, sd);
   const char *dir = params;

   if (!strcmp(dir, "left"))
     {
        if (!_is_horizontal(sd->orientation))
          return EINA_FALSE;
        if (!_is_inverted(sd->orientation))
          _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "right"))
     {
        if (!_is_horizontal(sd->orientation))
          return EINA_FALSE;
        if (!_is_inverted(sd->orientation))
          _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "up"))
     {
        if (_is_horizontal(sd->orientation))
          return EINA_FALSE;
        if (_is_inverted(sd->orientation))
          _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "down"))
     {
        if (_is_horizontal(sd->orientation))
          return EINA_FALSE;
        if (_is_inverted(sd->orientation))
          _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
     }
   else return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_wheel_indicator_timer_cb(void *data)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);
   sd->wheel_indicator_timer = NULL;

   _popup_hide(data, NULL, NULL, NULL);
   return ECORE_CALLBACK_CANCEL;
}

EOLIAN static Eina_Bool
_efl_ui_slider_elm_widget_widget_event(Eo *obj, Efl_Ui_Slider_Data *sd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void) src;

   if (type == EVAS_CALLBACK_KEY_DOWN)
     {
        Evas_Event_Key_Down *ev = event_info;
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

        if (!_elm_config_key_binding_call(obj, MY_CLASS_NAME, ev, key_actions))
          return EINA_FALSE;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
     }
   else if (type == EVAS_CALLBACK_KEY_UP)
     {
          _popup_hide(obj, NULL, NULL, NULL);
        return EINA_FALSE;
     }
   else if (type == EVAS_CALLBACK_MOUSE_WHEEL)
     {
        Evas_Event_Mouse_Wheel *mev = event_info;
        if (mev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

        if (mev->z < 0) _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
        mev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        _popup_show(obj, NULL, NULL, NULL);
        _slider_update(obj, EINA_TRUE);
        sd->wheel_indicator_timer =
           ecore_timer_add(0.5, _wheel_indicator_timer_cb, obj);
        return EINA_TRUE;

     }
   else return EINA_FALSE;

   _popup_show(obj, NULL, NULL, NULL);
   _slider_update(obj, EINA_TRUE);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_slider_elm_widget_activate(Eo *obj, Efl_Ui_Slider_Data *sd, Elm_Activate act)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (act == ELM_ACTIVATE_DEFAULT) return EINA_FALSE;

   if ((act == ELM_ACTIVATE_UP) ||
       (act == ELM_ACTIVATE_RIGHT))
     {
        if (!_is_inverted(sd->orientation))
          _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
     }
   else if ((act == ELM_ACTIVATE_DOWN) ||
            (act == ELM_ACTIVATE_LEFT))
     {
        if (!_is_inverted(sd->orientation))
          _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
     }

   _slider_update(obj, EINA_TRUE);

   return EINA_TRUE;
}

static void
_visuals_refresh(Evas_Object *obj)
{
   _val_set(obj);
   evas_object_smart_changed(obj);
}

static void
_track_move_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj,
               void *event_info EINA_UNUSED)
{
   Evas_Coord x, y;

   EFL_UI_SLIDER_DATA_GET(data, sd);
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(sd->popup, x, y);
}

static void
_track2_move_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj,
               void *event_info EINA_UNUSED)
{
   Evas_Coord x, y;

   EFL_UI_SLIDER_DATA_GET(data, sd);
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(sd->popup2, x, y);
}

static void
_track_resize_cb(void *data,
                 Evas *e EINA_UNUSED,
                 Evas_Object *obj,
                 void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;

   EFL_UI_SLIDER_DATA_GET(data, sd);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(sd->popup, w, h);
}

static void
_track2_resize_cb(void *data,
                 Evas *e EINA_UNUSED,
                 Evas_Object *obj,
                 void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;

   EFL_UI_SLIDER_DATA_GET(data, sd);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(sd->popup2, w, h);
}

static void
_popup_add(Efl_Ui_Slider_Data *sd, Eo *obj, Evas_Object **popup,
           Evas_Object **track, Eina_Bool is_range)
{
   /* if theme has an overlayed slider mode, then lets support it */
   if (!is_range
       && !edje_object_part_exists(elm_layout_edje_get(obj), "elm.track.slider"))
     return;
   else if (is_range
            && !edje_object_part_exists(elm_layout_edje_get(obj), "elm.track2.slider"))
     return;

   // XXX popup needs to adapt to theme etc.
   *popup = edje_object_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(*popup, obj);
   if (_is_horizontal(sd->orientation))
     _elm_theme_set(elm_widget_theme_get(obj), *popup, "slider", "horizontal/popup", elm_widget_style_get(obj));
   else
     _elm_theme_set(elm_widget_theme_get(obj), *popup, "slider", "vertical/popup", elm_widget_style_get(obj));
   edje_object_scale_set(*popup, efl_ui_scale_get(obj) *
                         elm_config_scale_get());
   edje_object_signal_callback_add(*popup, "popup,hide,done", "elm", // XXX: for compat
                                   _popup_hide_done, obj);
   edje_object_signal_callback_add(*popup, "elm,popup,hide,done", "elm",
                                   _popup_hide_done, obj);

   /* create a rectangle to track position+size of the dragable */
   *track = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(*track, 0, 0, 0, 0);
   evas_object_pass_events_set(*track, EINA_TRUE);
   if (!is_range)
     {
        evas_object_event_callback_add
           (*track, EVAS_CALLBACK_MOVE, _track_move_cb, obj);
        evas_object_event_callback_add
           (*track, EVAS_CALLBACK_RESIZE, _track_resize_cb, obj);
        elm_layout_content_set(obj, "elm.track.slider", *track);
     }
   else
     {
        evas_object_event_callback_add
           (*track, EVAS_CALLBACK_MOVE, _track2_move_cb, obj);
        evas_object_event_callback_add
           (*track, EVAS_CALLBACK_RESIZE, _track2_resize_cb, obj);
        elm_layout_content_set(obj, "elm.track2.slider", *track);
     }
}

EOLIAN static Elm_Theme_Apply
_efl_ui_slider_elm_widget_theme_apply(Eo *obj, Efl_Ui_Slider_Data *sd)
{
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;
   ELM_LAYOUT_DATA_GET(obj, ld);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ELM_THEME_APPLY_FAILED);

   if (_is_horizontal(sd->orientation))
     {
        if (!sd->intvl_enable)
          eina_stringshare_replace(&ld->group, "horizontal");
        else
          eina_stringshare_replace(&ld->group, "range/horizontal");
        if (sd->popup)
          _elm_theme_set(elm_widget_theme_get(obj), sd->popup,
                         "slider", "horizontal/popup",
                         elm_widget_style_get(obj));
        if (sd->popup2)
          _elm_theme_set(elm_widget_theme_get(obj), sd->popup2,
                         "slider", "horizontal/popup",
                         elm_widget_style_get(obj));
     }
   else
     {
        if (!sd->intvl_enable)
          eina_stringshare_replace(&ld->group, "vertical");
        else
          eina_stringshare_replace(&ld->group, "range/vertical");

        if (sd->popup)
          _elm_theme_set(elm_widget_theme_get(obj), sd->popup,
                         "slider", "vertical/popup",
                         elm_widget_style_get(obj));
        if (sd->popup2)
          _elm_theme_set(elm_widget_theme_get(obj), sd->popup2,
                         "slider", "vertical/popup",
                         elm_widget_style_get(obj));
     }

   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

   if (sd->popup)
     {
        edje_object_scale_set(sd->popup, efl_ui_scale_get(obj) *
                              elm_config_scale_get());
        if (sd->intvl_enable && sd->popup2)
          edje_object_scale_set(sd->popup2, efl_ui_scale_get(obj) *
                                elm_config_scale_get());
        else if (sd->intvl_enable && !sd->popup2)
          _popup_add(sd, obj, &sd->popup2, &sd->track2, EINA_TRUE);
     }
   else
     {
        _popup_add(sd, obj, &sd->popup, &sd->track, EINA_FALSE);
        if (sd->intvl_enable && !sd->popup2)
          _popup_add(sd, obj, &sd->popup2, &sd->track2, EINA_TRUE);
     }

   if (_is_horizontal(sd->orientation))
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * efl_ui_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * efl_ui_scale_get(obj) *
       elm_config_scale_get());

   if (sd->intvl_enable)
     elm_layout_signal_emit(obj, "elm,slider,range,enable", "elm");
   else
     elm_layout_signal_emit(obj, "elm,slider,range,disable", "elm");

   if (_is_inverted(sd->orientation))
     {
        elm_layout_signal_emit(obj, "elm,state,inverted,on", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,inverted,on", "elm");
        if (sd->popup2)
          edje_object_signal_emit(sd->popup2, "elm,state,inverted,on", "elm");
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,state,inverted,off", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,inverted,off", "elm");
     }

   if (sd->indicator_show)
     {
        elm_layout_signal_emit(obj, "elm,state,val,show", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,show", "elm");
        if (sd->popup2)
          edje_object_signal_emit(sd->popup2, "elm,state,val,show", "elm");
     }

   _min_max_set(obj);
   _units_set(obj);
   _indicator_set(obj);
   _visuals_refresh(obj);

   edje_object_message_signal_process(wd->resize_obj);
   if (sd->popup)
     edje_object_message_signal_process(sd->popup);
   if (sd->popup2)
     edje_object_message_signal_process(sd->popup2);

   evas_object_smart_changed(obj);

   return int_ret;
}

EOLIAN static void
_efl_ui_slider_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Slider_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_move_knob_on_mouse(Evas_Object *obj, double button_x, double button_y)
{
   EFL_UI_SLIDER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!sd->intvl_enable)
     {
        edje_object_part_drag_value_set
           (wd->resize_obj, "elm.dragable.slider",
            button_x, button_y);
     }
   else
     {
        double posx = 0.0, posy = 0.0, posx2 = 0.0, posy2 = 0.0, diff1, diff2;

        edje_object_part_drag_value_get
           (wd->resize_obj, "elm.dragable.slider", &posx, &posy);
        edje_object_part_drag_value_get
           (wd->resize_obj, "elm.dragable2.slider", &posx2, &posy2);

        if (_is_horizontal(sd->orientation))
          {
             diff1 = fabs(button_x - posx);
             diff2 = fabs(button_x - posx2);
          }
        else
          {
             diff1 = fabs(button_y - posy);
             diff2 = fabs(button_y - posy2);
          }

        if (diff1 < diff2)
          {
             edje_object_part_drag_value_set
                (wd->resize_obj, "elm.dragable.slider",
                 button_x, button_y);
          }
        else
          {
             edje_object_part_drag_value_set
                (wd->resize_obj, "elm.dragable2.slider",
                 button_x, button_y);
          }
     }

}

static void
_spacer_down_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);

   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w, h;
   double button_x = 0.0, button_y = 0.0;

   sd->spacer_down = EINA_TRUE;
   sd->val2 = sd->val;
   evas_object_geometry_get(sd->spacer, &x, &y, &w, &h);
   sd->downx = ev->canvas.x - x;
   sd->downy = ev->canvas.y - y;
   if (_is_horizontal(sd->orientation))
     {
        button_x = ((double)ev->canvas.x - (double)x) / (double)w;
        if (button_x > 1) button_x = 1;
        if (button_x < 0) button_x = 0;
     }
   else
     {
        button_y = ((double)ev->canvas.y - (double)y) / (double)h;
        if (button_y > 1) button_y = 1;
        if (button_y < 0) button_y = 0;
     }

   _move_knob_on_mouse(data, button_x, button_y);

   if (!elm_widget_focus_get(data))
     elm_object_focus_set(data, EINA_TRUE);
   _slider_update(data, EINA_TRUE);
   efl_event_callback_legacy_call(data, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_START, NULL);
   elm_layout_signal_emit(data, "elm,state,indicator,show", "elm");
}

static void
_spacer_move_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);

   Evas_Coord x, y, w, h;
   double button_x = 0.0, button_y = 0.0;
   Evas_Event_Mouse_Move *ev = event_info;

   if (sd->spacer_down)
     {
        Evas_Coord d = 0;

        evas_object_geometry_get(sd->spacer, &x, &y, &w, &h);
        if (_is_horizontal(sd->orientation))
          d = abs(ev->cur.canvas.x - x - sd->downx);
        else d = abs(ev->cur.canvas.y - y - sd->downy);
        if (d > (_elm_config->thumbscroll_threshold - 1))
          {
             if (!sd->frozen)
               {
                  elm_widget_scroll_freeze_push(data);
                  sd->frozen = EINA_TRUE;
               }
             ev->event_flags &= ~EVAS_EVENT_FLAG_ON_HOLD;
          }

        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
          {
             if (sd->spacer_down) sd->spacer_down = EINA_FALSE;
             _slider_update(data, EINA_TRUE);
             efl_event_callback_legacy_call
               (data, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_STOP, NULL);
             if (sd->frozen)
               {
                  elm_widget_scroll_freeze_pop(data);
                  sd->frozen = EINA_FALSE;
               }
             elm_layout_signal_emit(data, "elm,state,indicator,hide", "elm");
             elm_slider_value_set(data, sd->val2);
             return;
          }
        if (_is_horizontal(sd->orientation))
          {
             button_x = ((double)ev->cur.canvas.x - (double)x) / (double)w;
             if (button_x > 1) button_x = 1;
             if (button_x < 0) button_x = 0;
          }
        else
          {
             button_y = ((double)ev->cur.canvas.y - (double)y) / (double)h;
             if (button_y > 1) button_y = 1;
             if (button_y < 0) button_y = 0;
          }

        _move_knob_on_mouse(data, button_x, button_y);
        _slider_update(data, EINA_TRUE);
     }
}

static void
_spacer_up_cb(void *data,
              Evas *e EINA_UNUSED,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   EFL_UI_SLIDER_DATA_GET(data, sd);

   if (!sd->spacer_down) return;
   if (sd->spacer_down) sd->spacer_down = EINA_FALSE;

   _slider_update(data, EINA_TRUE);
   efl_event_callback_legacy_call(data, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_STOP, NULL);

   if (sd->frozen)
     {
        elm_widget_scroll_freeze_pop(data);
        sd->frozen = EINA_FALSE;
     }
   elm_layout_signal_emit(data, "elm,state,indicator,hide", "elm");
}

static void
_min_max_set(Evas_Object *obj)
{
   char *buf_min = NULL;
   char *buf_max = NULL;

   EFL_UI_SLIDER_DATA_GET(obj, sd);

   if (sd->units_format_func)
     {
        buf_min = sd->units_format_func(sd->val_min);
        buf_max = sd->units_format_func(sd->val_max);
     }
   else if (sd->units)
     {
        int length = eina_stringshare_strlen(sd->units);

        buf_min = alloca(length + 128);
        buf_max = alloca(length + 128);

        snprintf((char *)buf_min, length + 128, sd->units, sd->val_min);
        snprintf((char *)buf_max, length + 128, sd->units, sd->val_max);
     }

   elm_layout_text_set(obj, "elm.units.min", buf_min);
   elm_layout_text_set(obj, "elm.units.max", buf_max);

   if (sd->units_format_func && sd->units_format_free)
     {
        sd->units_format_free(buf_min);
        sd->units_format_free(buf_max);
     }
}

EOLIAN static void
_efl_ui_slider_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Slider_Data *sd)
{
   elm_layout_freeze(obj);

   if (_is_horizontal(sd->orientation))
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * efl_ui_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * efl_ui_scale_get(obj) *
       elm_config_scale_get());

   _val_fetch(obj, EINA_FALSE);
   _min_max_set(obj);
   _units_set(obj);
   _indicator_set(obj);

   elm_layout_thaw(obj);
}

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   const char *txt = elm_widget_access_info_get(obj);

   if (!txt) txt = elm_layout_text_get(obj, NULL);
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   char *ret;
   Eina_Strbuf *buf = eina_strbuf_new();
   const char *txt = elm_layout_text_get(obj, "elm.units");

   if (txt) eina_strbuf_append(buf, txt);

   if (elm_widget_disabled_get(obj))
     eina_strbuf_append(buf, " state: disabled");

   if (eina_strbuf_length_get(buf))
     {
        ret = eina_strbuf_string_steal(buf);
        eina_strbuf_free(buf);
        return ret;
     }

   eina_strbuf_free(buf);
   return NULL;
}

static void
_on_show(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
         void *event_info EINA_UNUSED)
{
   EFL_UI_SLIDER_DATA_GET(obj, sd);
   if (sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ALWAYS)
     _popup_show(obj, NULL, NULL, NULL);
}

EOLIAN static void
_efl_ui_slider_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Slider_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->orientation = EFL_ORIENT_RIGHT;
   priv->indicator_show = EINA_TRUE;
   priv->indicator_visible_mode = elm_config_slider_indicator_visible_mode_get();
   priv->val_max = 1.0;
   //TODO: customize this time duration from api or theme data.
   priv->wheel_indicator_duration = 0.25;
   priv->step = SLIDER_STEP;

   if (!elm_layout_theme_set
       (obj, "slider", "horizontal", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_signal_callback_add(obj, "drag", "*", _drag, obj);
   elm_layout_signal_callback_add(obj, "drag,start", "*", _drag_start, obj);
   elm_layout_signal_callback_add(obj, "drag,stop", "*", _drag_stop, obj);
   elm_layout_signal_callback_add(obj, "drag,step", "*", _drag_step, obj);
   elm_layout_signal_callback_add(obj, "drag,page", "*", _drag_stop, obj);
   elm_layout_signal_callback_add(obj, "popup,show", "elm", _popup_show, obj); // XXX: for compat
   elm_layout_signal_callback_add(obj, "popup,hide", "elm", _popup_hide, obj); // XXX: for compat
   elm_layout_signal_callback_add(obj, "elm,popup,show", "elm", _popup_show, obj);
   elm_layout_signal_callback_add(obj, "elm,popup,hide", "elm", _popup_hide, obj);
   elm_layout_signal_callback_add(obj, "*", "popup,emit", _popup_emit, obj);
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.slider", 0.0, 0.0);

   priv->spacer = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(priv->spacer, 0, 0, 0, 0);
   evas_object_pass_events_set(priv->spacer, EINA_TRUE);
   elm_layout_content_set(obj, "elm.swallow.bar", priv->spacer);

   if (!priv->intvl_enable)
     _popup_add(priv, obj, &priv->popup, &priv->track, priv->intvl_enable);
   else
     _popup_add(priv, obj, &priv->popup2, &priv->track2, priv->intvl_enable);

   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_DOWN, _spacer_down_cb, obj);
   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_MOVE, _spacer_move_cb, obj);
   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_UP, _spacer_up_cb, obj);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _on_show, NULL);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("slider"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, NULL);

   evas_object_smart_changed(obj);
}

EOLIAN static void
_efl_ui_slider_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Slider_Data *sd)
{
   eina_stringshare_del(sd->indicator);
   eina_stringshare_del(sd->units);
   ecore_timer_del(sd->delay);
   ecore_timer_del(sd->wheel_indicator_timer);
   evas_object_del(sd->popup);
   evas_object_del(sd->popup2);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_efl_ui_slider_elm_layout_text_aliases_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *_pd EINA_UNUSED)
{
   return _text_aliases;
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_efl_ui_slider_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

EOLIAN static Eina_Bool
_efl_ui_slider_efl_ui_range_range_interval_enabled_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *pd)
{
   return pd->intvl_enable;
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_range_interval_enabled_set(Eo *obj, Efl_Ui_Slider_Data *sd, Eina_Bool enable)
{
   if (sd->intvl_enable == enable) return;

   sd->intvl_enable = enable;

   elm_obj_widget_theme_apply(obj);
   if (sd->intvl_enable)
     {
        elm_layout_signal_emit(obj, "elm,slider,range,enable", "elm");
        if (sd->indicator_show)
          edje_object_signal_emit(sd->popup2, "elm,state,val,show", "elm");
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,slider,range,disable", "elm");
        ELM_SAFE_FREE(sd->popup2, evas_object_del);
     }
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_range_interval_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *pd, double *from, double *to)
{
   if (from) *from = fmin(pd->intvl_from, pd->intvl_to);
   if (to) *to = fmax(pd->intvl_from, pd->intvl_to);
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_range_interval_set(Eo *obj, Efl_Ui_Slider_Data *pd, double from, double to)
{
   pd->intvl_from = from;
   //TODO: remove val later
   pd->val = from;
   pd->intvl_to = to;

   if (pd->intvl_from < pd->val_min) pd->intvl_from = pd->val_min;
   if (pd->intvl_to > pd->val_max) pd->intvl_to = pd->val_max;

   _visuals_refresh(obj);
}

EOLIAN static Eo *
_efl_ui_slider_efl_object_constructor(Eo *obj, Efl_Ui_Slider_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_SLIDER);

   return obj;
}

EOLIAN static void
_efl_ui_slider_efl_orientation_orientation_set(Eo *obj, Efl_Ui_Slider_Data *sd, Efl_Orient dir)
{
   sd->orientation = dir;

   elm_obj_widget_theme_apply(obj);
}

EOLIAN static Efl_Orient
_efl_ui_slider_efl_orientation_orientation_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->orientation;
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_span_size_set(Eo *obj, Efl_Ui_Slider_Data *sd, Evas_Coord size)
{
   if (_is_horizontal(sd->orientation))
     efl_gfx_size_hint_min_set(efl_part(obj, "elm.swallow.bar"), size, 1);
   else
     efl_gfx_size_hint_min_set(efl_part(obj, "elm.swallow.bar"), 1, size);
}

EOLIAN static Evas_Coord
_efl_ui_slider_efl_ui_range_span_size_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->size;
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_range_unit_format_set(Eo *obj, Efl_Ui_Slider_Data *sd, const char *units)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eina_stringshare_replace(&sd->units, units);
   if (units)
     {
        elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");
        edje_object_message_signal_process(wd->resize_obj);
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,units,visible", "elm");
        if (sd->popup2)
          edje_object_signal_emit(sd->popup2, "elm,state,units,visible", "elm");
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,state,units,hidden", "elm");
        edje_object_message_signal_process(wd->resize_obj);
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,units,hidden", "elm");
        if (sd->popup2)
          edje_object_signal_emit(sd->popup2, "elm,state,units,hidden", "elm");
     }
   evas_object_smart_changed(obj);
}

EOLIAN static const char *
_efl_ui_slider_efl_ui_range_range_unit_format_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->units;
}

EOLIAN static void
_efl_ui_slider_indicator_format_set(Eo *obj, Efl_Ui_Slider_Data *sd, const char *indicator)
{
   eina_stringshare_replace(&sd->indicator, indicator);
   evas_object_smart_changed(obj);
}

EOLIAN static const char*
_efl_ui_slider_indicator_format_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->indicator;
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_range_min_max_set(Eo *obj, Efl_Ui_Slider_Data *sd, double min, double max)
{
   if ((sd->val_min == min) && (sd->val_max == max)) return;
   sd->val_min = min;
   sd->val_max = max;
   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   _visuals_refresh(obj);
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_range_min_max_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd, double *min, double *max)
{
   if (min) *min = sd->val_min;
   if (max) *max = sd->val_max;
}

EOLIAN static void
_efl_ui_slider_efl_ui_range_range_value_set(Eo *obj, Efl_Ui_Slider_Data *sd, double val)
{
   if (sd->val == val) return;
   sd->val = val;
   sd->intvl_from = val;

   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   _visuals_refresh(obj);
}

EOLIAN static double
_efl_ui_slider_efl_ui_range_range_value_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->val;
}

EOLIAN static void
_efl_ui_slider_indicator_format_function_set(Eo *obj, Efl_Ui_Slider_Data *sd, slider_func_type func, slider_freefunc_type free_func)
{
   sd->indicator_format_func = func;
   sd->indicator_format_free = free_func;
   evas_object_smart_changed(obj);
}

EOLIAN static void
_efl_ui_slider_indicator_show_set(Eo *obj, Efl_Ui_Slider_Data *sd, Eina_Bool show)
{
   if (show)
     {
        sd->indicator_show = EINA_TRUE;
        elm_layout_signal_emit(obj, "elm,state,val,show", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,show", "elm");
        if (sd->popup2)
          edje_object_signal_emit(sd->popup2, "elm,state,val,show", "elm");
     }
   else
     {
        sd->indicator_show = EINA_FALSE;
        elm_layout_signal_emit(obj, "elm,state,val,hide", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,hide", "elm");
        if (sd->popup2)
          edje_object_signal_emit(sd->popup2, "elm,state,val,hide", "elm");
     }

   evas_object_smart_changed(obj);
}

EOLIAN static Eina_Bool
_efl_ui_slider_indicator_show_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->indicator_show;
}

EOLIAN static void
_efl_ui_slider_step_set(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd, double step)
{
   if (sd->step == step) return;

   if (step < 0.0) step = 0.0;
   else if (step > 1.0) step = 1.0;

   sd->step = step;
}

EOLIAN static double
_efl_ui_slider_step_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->step;
}

EOLIAN static void
_efl_ui_slider_indicator_show_on_focus_set(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd, Eina_Bool flag)
{
   if (flag)
     sd->indicator_visible_mode = ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS;
   else
     sd->indicator_visible_mode = ELM_SLIDER_INDICATOR_VISIBLE_MODE_DEFAULT;
}

EOLIAN static Eina_Bool
_efl_ui_slider_indicator_show_on_focus_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return (sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS);
}

EOLIAN static void
_efl_ui_slider_indicator_visible_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd, Elm_Slider_Indicator_Visible_Mode indicator_visible_mode)
{
   if (sd->indicator_visible_mode == indicator_visible_mode) return;
   sd->indicator_visible_mode = indicator_visible_mode;
}

EOLIAN static Elm_Slider_Indicator_Visible_Mode
_efl_ui_slider_indicator_visible_mode_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->indicator_visible_mode;
}

EOLIAN static Eina_Bool
_efl_ui_slider_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_slider_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_efl_ui_slider_elm_widget_on_focus(Eo *obj, Efl_Ui_Slider_Data *sd EINA_UNUSED, Elm_Object_Item *item EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = elm_obj_widget_on_focus(efl_super(obj, MY_CLASS), NULL);

   if ((sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS) && elm_widget_focus_get(obj))
     _popup_show(obj, NULL, NULL, NULL);
   else if (!elm_widget_focus_get(obj))
     _popup_hide(obj, NULL, NULL, NULL);

   return int_ret;
}

EOLIAN static void
_efl_ui_slider_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

// A11Y Accessibility

EOLIAN static void
_efl_ui_slider_elm_interface_atspi_value_value_and_text_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd, double *value, const char **text)
{
   if (value) *value = sd->val;
   if (text) *text = NULL;
}

EOLIAN static Eina_Bool
_efl_ui_slider_elm_interface_atspi_value_value_and_text_set(Eo *obj, Efl_Ui_Slider_Data *sd, double value, const char *text EINA_UNUSED)
{
   double oldval = sd->val;

   if (value < sd->val_min) value = sd->val_min;
   if (value > sd->val_max) value = sd->val_max; 

   efl_event_callback_legacy_call(obj, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_START, NULL);
   sd->val = value;
   _visuals_refresh(obj);
   sd->val = oldval;
   _slider_update(obj, EINA_TRUE);
   efl_event_callback_legacy_call(obj, EFL_UI_SLIDER_EVENT_SLIDER_DRAG_STOP, NULL);

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_slider_elm_interface_atspi_value_range_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd, double *lower, double *upper, const char **descr)
{
   if (lower) *lower = sd->val_min;
   if (upper) *upper = sd->val_max;
   if (descr) *descr = NULL;
}

EOLIAN static double
_efl_ui_slider_elm_interface_atspi_value_increment_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *sd)
{
   return sd->step;
}

EOLIAN const Elm_Atspi_Action *
_efl_ui_slider_elm_interface_atspi_widget_action_elm_actions_get(Eo *obj EINA_UNUSED, Efl_Ui_Slider_Data *pd EINA_UNUSED)
{
   static Elm_Atspi_Action atspi_actions[] = {
          { "drag,left", "drag", "left", _key_action_drag},
          { "drag,right", "drag", "right", _key_action_drag},
          { "drag,up", "drag", "up", _key_action_drag},
          { "drag,down", "drag", "down", _key_action_drag},
          { NULL, NULL, NULL, NULL}
   };
   return &atspi_actions[0];
}

// A11Y Accessibility - END

/* Efl.Part begin */
ELM_PART_OVERRIDE(efl_ui_slider, EFL_UI_SLIDER, ELM_LAYOUT, Efl_Ui_Slider_Data, Elm_Part_Data)

static Eina_Bool
_efl_ui_slider_content_set(Eo *obj, Efl_Ui_Slider_Data *_pd EINA_UNUSED, const char *part, Evas_Object *content)
{
   Eina_Bool int_ret;

   int_ret = efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
   if (!int_ret) return EINA_FALSE;

   return EINA_TRUE;
}

static EOLIAN Eina_Bool
_efl_ui_slider_internal_part_efl_container_content_set(Eo *obj, Elm_Part_Data *_pd EINA_UNUSED, Efl_Gfx *content)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, ELM_LAYOUT_INTERNAL_PART_CLASS);
   Efl_Ui_Slider_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_SLIDER_CLASS);
   ELM_PART_RETURN_VAL(_efl_ui_slider_content_set(pd->obj, sd, pd->part, content));
}

static void
_span_size_set(Eo *obj, Efl_Ui_Slider_Data *sd, int w, int h)
{
   Evas_Coord size;

   if (_is_horizontal(sd->orientation))
     size = w;
   else
     size = h;

   if (sd->size == size) return;
   sd->size = size;

   if (sd->indicator_show)
     {
        elm_layout_signal_emit(obj, "elm,state,val,show", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,show", "elm");
        if (sd->popup2)
          edje_object_signal_emit(sd->popup2, "elm,state,val,show", "elm");
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,state,val,hide", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,hide", "elm");
        if (sd->popup2)
          edje_object_signal_emit(sd->popup2, "elm,state,val,hide", "elm");
     }

   evas_object_smart_changed(obj);
}

EOLIAN void
_efl_ui_slider_internal_part_efl_gfx_size_hint_hint_min_set(Eo *obj, Elm_Part_Data *_pd EINA_UNUSED, int w, int h)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, ELM_LAYOUT_INTERNAL_PART_CLASS);
   Efl_Ui_Slider_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_SLIDER_CLASS);

   _span_size_set(pd->obj, sd, w, h);
}

EOLIAN void
_efl_ui_slider_internal_part_efl_gfx_size_hint_hint_min_get(Eo *obj, Elm_Part_Data *_pd EINA_UNUSED, int *w, int *h)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, ELM_LAYOUT_INTERNAL_PART_CLASS);
   Efl_Ui_Slider_Data *sd = efl_data_scope_get(pd->obj, EFL_UI_SLIDER_CLASS);

   if (_is_horizontal(sd->orientation))
     {
        if (w) *w = sd->size;
        if (h) *h = 1;
     }
   else
     {
        if (w) *w = 1;
        if (h) *h = sd->size;
     }
}

#include "efl_ui_slider_internal_part.eo.c"
/* Efl.Part end */

/* Legacy APIs */

EAPI Evas_Object *
elm_slider_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = efl_add(MY_CLASS, parent);
   return obj;
}

EAPI void
elm_slider_span_size_set(Evas_Object *obj, Evas_Coord size)
{
   efl_ui_range_span_size_set(obj, size);
}

EAPI Evas_Coord
elm_slider_span_size_get(const Evas_Object *obj)
{
   return efl_ui_range_span_size_get(obj);
}

EAPI void
elm_slider_unit_format_set(Evas_Object *obj, const char *units)
{
   efl_ui_range_unit_format_set(obj, units);
}

EAPI const char *
elm_slider_unit_format_get(const Evas_Object *obj)
{
   return efl_ui_range_unit_format_get(obj);
}

EAPI void
elm_slider_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   Efl_Orient dir;
   EFL_UI_SLIDER_DATA_GET(obj, sd);

   dir = _orientation_get(horizontal, _is_inverted(sd->orientation));

   efl_orientation_set(obj, dir);
}

EAPI Eina_Bool
elm_slider_horizontal_get(const Evas_Object *obj)
{
   Efl_Orient dir;
   dir = efl_orientation_get(obj);

   return _is_horizontal(dir);
}

EAPI void
elm_slider_value_set(Evas_Object *obj, double val)
{
   efl_ui_range_value_set(obj, val);
}

EAPI double
elm_slider_value_get(const Evas_Object *obj)
{
   return efl_ui_range_value_get(obj);
}

EAPI void
elm_slider_inverted_set(Evas_Object *obj, Eina_Bool inverted)
{
   Efl_Orient dir;
   EFL_UI_SLIDER_DATA_GET(obj, sd);

   dir = _orientation_get(_is_horizontal(sd->orientation), inverted);

   efl_orientation_set(obj, dir);
}

EAPI Eina_Bool
elm_slider_inverted_get(const Evas_Object *obj)
{
   Efl_Orient dir;
   dir = efl_orientation_get(obj);

   return _is_inverted(dir);
}

EAPI void
elm_slider_units_format_function_set(Evas_Object *obj, slider_func_type func, slider_freefunc_type free_func)
{
   EFL_UI_SLIDER_DATA_GET(obj, sd);

   sd->units_format_func = func;
   sd->units_format_free = free_func;

   evas_object_smart_changed(obj);
}

EAPI void
elm_slider_range_enabled_set(Evas_Object *obj, Eina_Bool enable)
{
   efl_ui_range_interval_enabled_set(obj, enable);
}

EAPI Eina_Bool
elm_slider_range_enabled_get(const Evas_Object *obj)
{
   return efl_ui_range_interval_enabled_get(obj);
}

EAPI void
elm_slider_range_set(Evas_Object *obj, double from, double to)
{
   efl_ui_range_interval_set(obj, from, to);
}

EAPI void
elm_slider_range_get(const Evas_Object *obj, double *from, double *to)
{
   efl_ui_range_interval_get(obj, from, to);
}

EAPI void
elm_slider_min_max_set(Evas_Object *obj, double min, double max)
{
   efl_ui_range_min_max_set(obj, min, max);
}

EAPI void
elm_slider_min_max_get(const Evas_Object *obj, double *min, double *max)
{
   efl_ui_range_min_max_get(obj, min, max);
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_SLIDER_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_slider)

#include "efl_ui_slider.eo.c"
