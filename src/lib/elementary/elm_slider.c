#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_ACCESS_VALUE_PROTECTED
#define ELM_LAYOUT_PROTECTED
#define EFL_PART_PROTECTED
#define EFL_UI_FORMAT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_slider.h"
#include "efl_ui_slider_private.h"
#include "efl_ui_slider_interval_private.h"
#include "elm_widget_layout.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_SLIDER_CLASS

#define MY_CLASS_NAME "Elm_Slider"
#define MY_CLASS_NAME_LEGACY "elm_slider"
#define SLIDER_DELAY_CHANGED_INTERVAL 0.2
#define SLIDER_STEP 0.05

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"icon", "elm.swallow.icon"},
   {"end", "elm.swallow.end"},
   {NULL, NULL}
};

static Eina_Bool _key_action_drag(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"drag", _key_action_drag},
   {NULL, NULL}
};

static const char SIG_CHANGED[] = "changed";
static const char SIG_DELAY_CHANGED[] = "delay,changed";
static const char SIG_DRAG_START[] = "slider,drag,start";
static const char SIG_DRAG_STOP[] = "slider,drag,stop";

static const char PART_NAME_POPUP[] = "popup";
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

static Eina_Bool
_delay_change(void *data)
{
   ELM_SLIDER_DATA_GET(data, sd);

   if (!sd) return ECORE_CALLBACK_CANCEL;
   sd->delay = NULL;
   evas_object_smart_callback_call(data, SIG_DELAY_CHANGED, NULL);

   if (_elm_config->atspi_mode)
     efl_access_value_changed_signal_emit(data);

   return ECORE_CALLBACK_CANCEL;
}

static inline Eina_Bool
_is_horizontal(Efl_Ui_Layout_Orientation dir)
{
   return efl_ui_layout_orientation_is_horizontal(dir, EINA_TRUE);
}

static void
_units_set(Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET(obj, sd);

   if (sd->format_cb)
     {
        Eina_Value val;

        eina_value_setup(&val, EINA_VALUE_TYPE_DOUBLE);

        eina_strbuf_reset(sd->format_strbuf);
        if (!sd->intvl_enable)
          eina_value_set(&val, sd->val);
        else
          {
             double v1, v2;

             elm_slider_range_get(obj, &v1, &v2);
             eina_value_set(&val, v2 - v1);
          }

        sd->format_cb(sd->format_cb_data, sd->format_strbuf, val);
        elm_layout_text_set(obj, "elm.units", eina_strbuf_string_get(sd->format_strbuf));

        if (!sd->units_show)
          {
             elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");
             sd->units_show = EINA_TRUE;
          }

        eina_value_flush(&val);
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
   ELM_SLIDER_DATA_GET(obj, sd);

   Eina_Value val;
   const char *str;

   if (!sd->indi_format_cb) return;

   eina_value_setup(&val, EINA_VALUE_TYPE_DOUBLE);
   eina_strbuf_reset(sd->indi_format_strbuf);

   eina_value_set(&val, sd->val);
   sd->indi_format_cb(sd->indi_format_cb_data, sd->indi_format_strbuf, val);

   str = eina_strbuf_string_get(sd->indi_format_strbuf);

   elm_layout_text_set(obj, "elm.indicator", str);
   elm_layout_text_set(obj, "elm.dragable.slider:elm.indicator", str);
   if (sd->popup)
     edje_object_part_text_set(sd->popup, "elm.indicator", str);

   if (sd->popup2)
     {
        eina_strbuf_reset(sd->indi_format_strbuf);
        eina_value_set(&val, sd->intvl_to);
        sd->indi_format_cb(sd->indi_format_cb_data, sd->indi_format_strbuf, val);
        str = eina_strbuf_string_get(sd->indi_format_strbuf);
        elm_layout_text_set(obj, "elm.dragable2.slider:elm.indicator", str);
        edje_object_part_text_set(sd->popup2, "elm.indicator", str);
     }

    eina_value_flush(&val);
}

static void
_min_max_set(Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET(obj, sd);
   Eina_Strbuf *str;
   Eina_Value val;

   if (!sd->format_cb) return;
   eina_value_setup(&val, EINA_VALUE_TYPE_DOUBLE);

   str = eina_strbuf_new();

   eina_value_set(&val, sd->val_max);
   sd->format_cb(sd->format_cb_data, str, val);
   elm_layout_text_set(obj, "elm.units.min", eina_strbuf_string_get(str));

   eina_strbuf_reset(str);

   eina_value_set(&val, sd->val_min);
   sd->format_cb(sd->format_cb_data, str, val);
   elm_layout_text_set(obj, "elm.units.max", eina_strbuf_string_get(str));

   eina_strbuf_free(str);
   eina_value_flush(&val);
}

static void
_popup_show(void *data,
            Evas_Object *obj EINA_UNUSED,
            const char *emission EINA_UNUSED,
            const char *source EINA_UNUSED)
{
   ELM_SLIDER_DATA_GET(data, sd);
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
   ELM_SLIDER_DATA_GET(data, sd);

   if (!sd->popup_visible || !sd->popup) return;

   if (sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ALWAYS) return;
   if ((sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS) &&
       efl_ui_focus_object_focus_get(data))
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
   ELM_SLIDER_DATA_GET(data, sd);
   if (sd->popup)
     {
        if (!((efl_ui_focus_object_focus_get(data)) &&
              (sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS)))
          {
             evas_object_hide(sd->popup);
             sd->popup_visible = EINA_FALSE;
          }
     }
   if (sd->popup2)
     {
        if (!((efl_ui_focus_object_focus_get(data)) &&
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
   ELM_SLIDER_DATA_GET(data, sd);
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
_wheel_indicator_timer_cb(void *data)
{
   ELM_SLIDER_DATA_GET(data, sd);
   sd->wheel_indicator_timer = NULL;

   _popup_hide(data, NULL, NULL, NULL);
   return ECORE_CALLBACK_CANCEL;
}

static void
_val_set(Evas_Object *obj)
{
   double pos, pos2;

   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

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

   if (efl_ui_mirrored_get(obj) ^ efl_ui_layout_orientation_is_inverted(sd->dir))
     {
        pos = 1.0 - pos;
        pos2 = 1.0 - pos2;
     }

   efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                         pos, pos);
   if (sd->intvl_enable)
     efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                           pos2, pos2);

   // emit accessibility event also if value was changed by API
   if (_elm_config->atspi_mode)
     efl_access_value_changed_signal_emit(obj);

   evas_object_smart_changed(obj);
}

static void
_val_fetch(Evas_Object *obj, Eina_Bool user_event)
{
   double posx = 0.0, posy = 0.0, pos = 0.0, val;
   double posx2 = 0.0, posy2 = 0.0, pos2 = 0.0, val2 = 0.0;
   Eina_Bool inverted = EINA_FALSE;
   Eina_Bool evented = EINA_FALSE;

   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable.slider"),
                         &posx, &posy);
   if (efl_ui_layout_orientation_is_horizontal(sd->dir, EINA_TRUE)) pos = posx;
   else pos = posy;

   if (sd->intvl_enable)
     {
        efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                              &posx2, &posy2);
        if (efl_ui_layout_orientation_is_horizontal(sd->dir, EINA_TRUE)) pos2 = posx2;
        else pos2 = posy2;
     }

   if (efl_ui_mirrored_get(obj) ^ efl_ui_layout_orientation_is_inverted(sd->dir))
     {
        pos = 1.0 - pos;
        pos2 = 1.0 - pos2;
        inverted = EINA_TRUE;
     }

   val = (pos * (sd->val_max - sd->val_min)) + sd->val_min;

   if (sd->intvl_enable)
     {
        val2 = (pos2 * (sd->val_max - sd->val_min)) + sd->val_min;
        if (!inverted)
          {
             if (val > sd->intvl_to)
               {
                  val = sd->intvl_to;
                  _val_set(obj);
               }
             else if (val2 < sd->intvl_from)
               {
                  val2 = sd->intvl_from;
                  _val_set(obj);
               }
          }
        else
          {
             if (val < sd->intvl_to)
               {
                  val = sd->intvl_to;
                  _val_set(obj);
               }
             else if (val2 > sd->intvl_from)
               {
                  val2 = sd->intvl_from;
                  _val_set(obj);
               }
          }
     }
   if (fabs(val - sd->val) > DBL_EPSILON)
     {
        sd->val = val;
        sd->intvl_from = val;
        if (user_event)
          {
             evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
             ecore_timer_del(sd->delay);
             sd->delay = ecore_timer_add(SLIDER_DELAY_CHANGED_INTERVAL, _delay_change, obj);
             evented = EINA_TRUE;
          }
     }

   if (sd->intvl_enable && fabs(val2 - sd->intvl_to) > DBL_EPSILON)
     {
        sd->intvl_to = val2;
        /* avoid emitting two events and setting a timer twice */
        if (user_event && (!evented))
          {
             evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
             ecore_timer_del(sd->delay);
             sd->delay = ecore_timer_add(SLIDER_DELAY_CHANGED_INTERVAL, _delay_change, obj);
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
   if (!efl_ui_focus_object_focus_get(data))
     elm_object_focus_set(data, EINA_TRUE);
   _slider_update(data, EINA_TRUE);
   evas_object_smart_callback_call(data, SIG_DRAG_START, NULL);
   elm_widget_scroll_freeze_push(data);
}

static void
_drag_stop(void *data,
           Evas_Object *obj EINA_UNUSED,
           const char *emission EINA_UNUSED,
           const char *source EINA_UNUSED)
{
   _slider_update(data, EINA_TRUE);
   evas_object_smart_callback_call(data, SIG_DRAG_STOP, NULL);
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

   ELM_SLIDER_DATA_GET(data, sd);
   step = sd->step;

   if (efl_ui_layout_orientation_is_inverted(sd->dir)) step *= -1.0;

   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   efl_ui_drag_step_move(efl_part(wd->resize_obj, "elm.dragable.slider"),
                         step, step);
}

static void
_drag_down(void *data,
           Evas_Object *obj EINA_UNUSED,
           const char *emission EINA_UNUSED,
           const char *source EINA_UNUSED)
{
   double step;

   ELM_SLIDER_DATA_GET(data, sd);
   step = -sd->step;

   if (efl_ui_layout_orientation_is_inverted(sd->dir)) step *= -1.0;

   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   efl_ui_drag_step_move(efl_part(wd->resize_obj, "elm.dragable.slider"),
                         step, step);
}

static Eina_Bool
_key_action_drag(Evas_Object *obj, const char *params)
{
   ELM_SLIDER_DATA_GET(obj, sd);
   const char *dir = params;

   if (!strcmp(dir, "left"))
     {
        if (!_is_horizontal(sd->dir))
          return EINA_FALSE;
        if (!efl_ui_layout_orientation_is_inverted(sd->dir))
          _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "right"))
     {
        if (!_is_horizontal(sd->dir))
          return EINA_FALSE;
        if (!efl_ui_layout_orientation_is_inverted(sd->dir))
          _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "up"))
     {
        if (_is_horizontal(sd->dir))
          return EINA_FALSE;
        if (efl_ui_layout_orientation_is_inverted(sd->dir))
          _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "down"))
     {
        if (_is_horizontal(sd->dir))
          return EINA_FALSE;
        if (efl_ui_layout_orientation_is_inverted(sd->dir))
          _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
     }
   else return EINA_FALSE;

   return EINA_TRUE;
}

// _slider_efl_ui_widget_widget_input_event_handler
ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(slider, Elm_Slider_Data)

EOLIAN static Eina_Bool
_elm_slider_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Slider_Data *sd, const Efl_Event *eo_event, Evas_Object *src)
{
   Eo *ev = eo_event->info;

   if (eo_event->desc == EFL_EVENT_KEY_DOWN)
     {
        if (!_slider_efl_ui_widget_widget_input_event_handler(obj, sd, eo_event, src))
          return EINA_FALSE;
     }
   else if (eo_event->desc == EFL_EVENT_KEY_UP)
     {
        _popup_hide(obj, NULL, NULL, NULL);
        return EINA_FALSE;
     }
   else if (eo_event->desc == EFL_EVENT_POINTER_WHEEL)
     {
        if (efl_input_processed_get(ev)) return EINA_FALSE;
        if (efl_input_pointer_wheel_delta_get(ev) < 0)
          _drag_up(obj, NULL, NULL, NULL);
        else
          _drag_down(obj, NULL, NULL, NULL);
        efl_input_processed_set(ev, EINA_TRUE);
        ELM_SAFE_FREE(sd->wheel_indicator_timer, ecore_timer_del);
        sd->wheel_indicator_timer =
           ecore_timer_add(0.5, _wheel_indicator_timer_cb, obj);
     }
   else return EINA_FALSE;

   _popup_show(obj, NULL, NULL, NULL);
   _val_fetch(obj, EINA_TRUE);
   evas_object_smart_changed(obj);

   return EINA_TRUE;
}

static void
_track_move_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj,
               void *event_info EINA_UNUSED)
{
   Evas_Coord x, y;

   ELM_SLIDER_DATA_GET(data, sd);
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

   ELM_SLIDER_DATA_GET(data, sd);
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

   ELM_SLIDER_DATA_GET(data, sd);
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

   ELM_SLIDER_DATA_GET(data, sd);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(sd->popup2, w, h);
}

static void
_popup_update(Evas_Object *obj, Elm_Slider_Data *sd, Evas_Object *popup)
{
   if (elm_widget_is_legacy(obj))
     {
        if (_is_horizontal(sd->dir))
          elm_widget_theme_object_set(obj, popup, "slider", "horizontal/popup", elm_widget_style_get(obj));
        else
          elm_widget_theme_object_set(obj, popup, "slider", "vertical/popup", elm_widget_style_get(obj));
     }
   else
     elm_widget_element_update(obj, popup, PART_NAME_POPUP);
   edje_object_scale_set(popup, efl_gfx_entity_scale_get(obj) *
                         elm_config_scale_get());

   if (!efl_ui_layout_orientation_is_inverted(sd->dir))
     edje_object_signal_emit(popup, "elm,state,inverted,off", "elm");
   else
     edje_object_signal_emit(popup, "elm,state,inverted,on", "elm");

   if (sd->indicator_show)
     edje_object_signal_emit(popup, "elm,state,val,show", "elm");
   else
     edje_object_signal_emit(popup, "elm,state,val,hide", "elm");
}

static void
_popup_add(Elm_Slider_Data *sd, Eo *obj, Evas_Object **popup,
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
   edje_object_signal_callback_add(*popup, "popup,hide,done", "elm", // XXX: for compat
                                   _popup_hide_done, obj);
   edje_object_signal_callback_add(*popup, "elm,popup,hide,done", "elm",
                                   _popup_hide_done, obj);

   _popup_update(obj, sd, *popup);

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

static char *
_elm_slider_theme_group_get(Evas_Object *obj EINA_UNUSED, Elm_Slider_Data *sd)
{
   Eina_Strbuf *new_group = eina_strbuf_new();

   if (sd->intvl_enable)
     eina_strbuf_append(new_group, "range/");
   if (_is_horizontal(sd->dir))
     eina_strbuf_append(new_group, "horizontal");
   else
     eina_strbuf_append(new_group, "vertical");

   return eina_strbuf_release(new_group);
}

EOLIAN static Eina_Error
_elm_slider_efl_ui_widget_theme_apply(Eo *obj, Elm_Slider_Data *sd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_ERROR_GENERIC);
   char *group;

   group = _elm_slider_theme_group_get(obj, sd);
   if (group)
     {
        elm_widget_theme_element_set(obj, group);
        free(group);
     }

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   if (_is_horizontal(sd->dir))
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * efl_gfx_entity_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * efl_gfx_entity_scale_get(obj) *
       elm_config_scale_get());

   if (sd->intvl_enable)
     elm_layout_signal_emit(obj, "elm,slider,range,enable", "elm");
   else
     elm_layout_signal_emit(obj, "elm,slider,range,disable", "elm");

   if (efl_ui_layout_orientation_is_inverted(sd->dir))
     elm_layout_signal_emit(obj, "elm,state,inverted,on", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,inverted,off", "elm");
   if (sd->indicator_show)
     elm_layout_signal_emit(obj, "elm,state,val,show", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,val,hide", "elm");

   if (!sd->popup)
     _popup_add(sd, obj, &sd->popup, &sd->track, sd->intvl_enable);
   else
     _popup_update(obj, sd, sd->popup);

   if (sd->intvl_enable)
     {
        if (!sd->popup2)
          _popup_add(sd, obj, &sd->popup2, &sd->track2, EINA_TRUE);
        else
          _popup_update(obj, sd, sd->popup2);
     }

   _min_max_set(obj);
   _units_set(obj);
   _indicator_set(obj);
   _val_set(obj);

   edje_object_message_signal_process(wd->resize_obj);
   if (sd->popup)
     edje_object_message_signal_process(sd->popup);
   if (sd->popup2)
     edje_object_message_signal_process(sd->popup2);

   evas_object_smart_changed(obj);

   return int_ret;
}

static void
_spacer_down_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   Evas_Event_Mouse_Down *ev = event_info;
   Eina_Rect sr;
   double button_x = 0.0, button_y = 0.0;

   sd->spacer_down = EINA_TRUE;
   sr = efl_gfx_entity_geometry_get(sd->spacer);
   sd->downx = ev->canvas.x - sr.x;
   sd->downy = ev->canvas.y - sr.y;
   if (_is_horizontal(sd->dir))
     {
        button_x = ((double)ev->canvas.x - (double)sr.x) / (double)sr.w;
        if (button_x > 1) button_x = 1;
        if (button_x < 0) button_x = 0;
     }
   else
     {
        button_y = ((double)ev->canvas.y - (double)sr.y) / (double)sr.h;
        if (button_y > 1) button_y = 1;
        if (button_y < 0) button_y = 0;
     }

   if (!sd->intvl_enable)
     {
        efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                              button_x, button_y);
     }
   else
     {
        double posx = 0.0, posy = 0.0, posx2 = 0.0, posy2 = 0.0, diff1, diff2, diff3;

        efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable.slider"),
                              &posx, &posy);
        efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                              &posx2, &posy2);

        if (efl_ui_layout_orientation_is_horizontal(sd->dir, EINA_TRUE))
          {
             diff1 = fabs(button_x - posx);
             diff2 = fabs(button_x - posx2);
             diff3 = button_x - posx;
          }
        else
          {
             diff1 = fabs(button_y - posy);
             diff2 = fabs(button_y - posy2);
             diff3 = button_y - posy;
          }

        if (diff1 < diff2)
          {
             efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                                   button_x, button_y);
             sd->intvl_flag = 1;
          }
        else if (diff1 > diff2)
          {
             efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                                   button_x, button_y);
             sd->intvl_flag = 2;
          }
        else
          {
             if (diff3 < 0)
               {
                  efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                                        button_x, button_y);
                  sd->intvl_flag = 1;
               }
             else
               {
                  efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                                        button_x, button_y);
                  sd->intvl_flag = 2;
               }
          }
     }

   if (!efl_ui_focus_object_focus_get(data))
     elm_object_focus_set(data, EINA_TRUE);

   _slider_update(data, EINA_TRUE);

   elm_layout_signal_emit(data, "elm,state,indicator,show", "elm");

   evas_object_smart_callback_call(data, SIG_DRAG_START, NULL);
}

static void
_spacer_move_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   Evas_Event_Mouse_Move *ev = event_info;
   Eina_Rect sr;
   double button_x = 0.0, button_y = 0.0;

   if (sd->spacer_down)
     {
        Evas_Coord d = 0;

        sr = efl_gfx_entity_geometry_get(sd->spacer);
        if (_is_horizontal(sd->dir))
          d = abs(ev->cur.canvas.x - sr.x - sd->downx);
        else d = abs(ev->cur.canvas.y - sr.y - sd->downy);
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

             evas_object_smart_callback_call(data, SIG_DRAG_STOP, NULL);

             if (sd->frozen)
               {
                  elm_widget_scroll_freeze_pop(data);
                  sd->frozen = EINA_FALSE;
               }
             elm_layout_signal_emit(data, "elm,state,indicator,hide", "elm");
             return;
          }
        if (_is_horizontal(sd->dir))
          {
             button_x = ((double)ev->cur.canvas.x - (double)sr.x) / (double)sr.w;
             if (button_x > 1) button_x = 1;
             if (button_x < 0) button_x = 0;
          }
        else
          {
             button_y = ((double)ev->cur.canvas.y - (double)sr.y) / (double)sr.h;
             if (button_y > 1) button_y = 1;
             if (button_y < 0) button_y = 0;
          }

        if (!sd->intvl_enable)
          {
             efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                                   button_x, button_y);
          }
        else
          {
             if (sd->intvl_flag == 1)
               {
                  efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                                        button_x, button_y);
               }
             else if (sd->intvl_flag == 2)
               {
                  efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                                        button_x, button_y);
               }

          }

        _slider_update(data, EINA_TRUE);
     }
}

static void
_spacer_up_cb(void *data,
              Evas *e EINA_UNUSED,
              Evas_Object *obj EINA_UNUSED,
              void *event_info EINA_UNUSED)
{
   ELM_SLIDER_DATA_GET(data, sd);

   if (!sd->spacer_down) return;
   if (sd->spacer_down) sd->spacer_down = EINA_FALSE;

   _slider_update(data, EINA_TRUE);

   evas_object_smart_callback_call(data, SIG_DRAG_STOP, NULL);

   if (sd->frozen)
     {
        elm_widget_scroll_freeze_pop(data);
        sd->frozen = EINA_FALSE;
     }
   elm_layout_signal_emit(data, "elm,state,indicator,hide", "elm");
}

static void
_mouse_in_cb(void *data EINA_UNUSED,
              Evas *e EINA_UNUSED,
              Evas_Object *obj,
              void *event_info EINA_UNUSED)
{
   efl_ui_widget_scroll_hold_push(obj);
}

static void
_mouse_out_cb(void *data EINA_UNUSED,
              Evas *e EINA_UNUSED,
              Evas_Object *obj,
              void *event_info EINA_UNUSED)
{
   efl_ui_widget_scroll_hold_pop(obj);
}

EOLIAN static void
_elm_slider_efl_canvas_group_group_calculate(Eo *obj, Elm_Slider_Data *sd)
{
   efl_canvas_group_need_recalculate_set(obj, EINA_FALSE);
   edje_object_freeze(obj);

   if (_is_horizontal(sd->dir))
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * efl_gfx_entity_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * efl_gfx_entity_scale_get(obj) *
       elm_config_scale_get());

   _val_fetch(obj, EINA_FALSE); // need to check whether this should be called here
   _min_max_set(obj);
   _units_set(obj);
   _indicator_set(obj);

   edje_object_thaw(obj);
   efl_canvas_group_calculate(efl_super(obj, MY_CLASS));
}

static void
_on_show(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
         void *event_info EINA_UNUSED)
{
   ELM_SLIDER_DATA_GET(obj, sd);
   if (sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ALWAYS)
     _popup_show(obj, NULL, NULL, NULL);
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

EOLIAN static Eo *
_elm_slider_efl_object_constructor(Eo *obj, Elm_Slider_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   char *group;

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "slider");
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_SLIDER);
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);

   group = _elm_slider_theme_group_get(obj, priv);
   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       group,
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

   free(group);

   priv->dir = EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL;
   priv->val_max = 1.0;
   priv->step = SLIDER_STEP;
   priv->indicator_show = EINA_TRUE;
   priv->indicator_visible_mode = elm_config_slider_indicator_visible_mode_get();
   //TODO: customize this time duration from api or theme data.
   priv->wheel_indicator_duration = 0.25;

   priv->spacer = efl_add(EFL_CANVAS_RECTANGLE_CLASS, obj,
                          efl_gfx_color_set(efl_added, 0, 0, 0, 0));
   efl_content_set(efl_part(obj, "elm.swallow.bar"), priv->spacer);

   if (!priv->intvl_enable)
     _popup_add(priv, obj, &priv->popup, &priv->track, priv->intvl_enable);
   else
     _popup_add(priv, obj, &priv->popup2, &priv->track2, priv->intvl_enable);

   efl_ui_format_string_set(efl_part(obj, "indicator"), "%0.2f", EFL_UI_FORMAT_STRING_TYPE_SIMPLE);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   // accessiblity
   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("slider"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, NULL);

   // add callbacks
   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _on_show, NULL);

   elm_layout_signal_callback_add(obj, "popup,show", "elm", _popup_show, obj); // XXX: for compat
   elm_layout_signal_callback_add(obj, "popup,hide", "elm", _popup_hide, obj); // XXX: for compat
   elm_layout_signal_callback_add(obj, "elm,popup,show", "elm", _popup_show, obj);
   elm_layout_signal_callback_add(obj, "elm,popup,hide", "elm", _popup_hide, obj);
   elm_layout_signal_callback_add(obj, "*", "popup,emit", _popup_emit, obj);

   efl_layout_signal_callback_add(obj, "drag", "*", obj, _drag, NULL);
   efl_layout_signal_callback_add(obj, "drag,start", "*", obj, _drag_start, NULL);
   efl_layout_signal_callback_add(obj, "drag,stop", "*", obj, _drag_stop, NULL);
   efl_layout_signal_callback_add(obj, "drag,step", "*", obj, _drag_step, NULL);
   efl_layout_signal_callback_add(obj, "drag,page", "*", obj, _drag_stop, NULL);

   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_DOWN, _spacer_down_cb, obj);
   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_MOVE, _spacer_move_cb, obj);
   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_UP, _spacer_up_cb, obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_IN, _mouse_in_cb, obj);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_OUT, _mouse_out_cb, obj);

   evas_object_smart_changed(obj);

   return obj;
}

EOLIAN static void
_elm_slider_efl_object_destructor(Eo *obj,
                                  Elm_Slider_Data *sd)
{
   ELM_SAFE_FREE(sd->delay, ecore_timer_del);
   ELM_SAFE_FREE(sd->wheel_indicator_timer, ecore_timer_del);
   ELM_SAFE_FREE(sd->popup, evas_object_del);
   ELM_SAFE_FREE(sd->popup2, evas_object_del);

   ELM_SAFE_FREE(sd->indi_template, eina_stringshare_del);
   ELM_SAFE_FREE(sd->indi_format_strbuf, eina_strbuf_free);

   efl_ui_format_func_set(obj, NULL, NULL, NULL);
   ELM_SAFE_FREE(sd->format_strbuf, eina_strbuf_free);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Bool
_elm_slider_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Slider_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));

   if ((sd->indicator_visible_mode == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS) && efl_ui_focus_object_focus_get(obj))
     _popup_show(obj, NULL, NULL, NULL);
   else if (!efl_ui_focus_object_focus_get(obj))
     _popup_hide(obj, NULL, NULL, NULL);

   return int_ret;
}

EOLIAN static void
_elm_slider_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static void
_elm_slider_efl_ui_format_format_cb_set(Eo *obj, Elm_Slider_Data *sd, void *func_data, Efl_Ui_Format_Func func, Eina_Free_Cb func_free_cb)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->format_cb_data == func_data && sd->format_cb == func)
     return;

   if (sd->format_cb_data && sd->format_free_cb)
     sd->format_free_cb(sd->format_cb_data);

   if (efl_invalidated_get(obj)) return;

   sd->format_cb = func;
   sd->format_cb_data = func_data;
   sd->format_free_cb = func_free_cb;
   if (!sd->format_strbuf) sd->format_strbuf = eina_strbuf_new();

   elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");
   edje_object_message_signal_process(wd->resize_obj);
   if (sd->popup)
     edje_object_signal_emit(sd->popup, "elm,state,units,visible", "elm");
   if (sd->popup2)
     edje_object_signal_emit(sd->popup2, "elm,state,units,visible", "elm");

   efl_canvas_group_change(obj);
}

/* Standard widget overrides */

ELM_PART_TEXT_DEFAULT_IMPLEMENT(elm_slider, Elm_Slider_Data)
ELM_PART_MARKUP_DEFAULT_IMPLEMENT(elm_slider, Elm_Slider_Data)

static void
_slider_span_size_set(Eo *obj, Elm_Slider_Data *sd, int size)
{
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

/* Efl.Part begin */

EOLIAN static Eo *
_elm_slider_efl_part_part_get(const Eo *obj, Elm_Slider_Data *sd EINA_UNUSED, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

   if (eina_streq(part, "indicator"))
     return ELM_PART_IMPLEMENT(ELM_SLIDER_PART_INDICATOR_CLASS, obj, part);

   return efl_part_get(efl_super(obj, MY_CLASS), part);
}

EOLIAN static void
_elm_slider_part_indicator_efl_ui_format_format_cb_set(Eo *obj, void *_pd EINA_UNUSED, void *func_data, Efl_Ui_Format_Func func, Eina_Free_Cb func_free_cb)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Elm_Slider_Data *sd = efl_data_scope_get(pd->obj, ELM_SLIDER_CLASS);

   if (sd->indi_format_cb_data == func_data && sd->indi_format_cb == func)
     return;

   if (sd->indi_format_cb_data && sd->indi_format_free_cb)
     sd->indi_format_free_cb(sd->format_cb_data);

   sd->indi_format_cb = func;
   sd->indi_format_cb_data = func_data;
   sd->indi_format_free_cb = func_free_cb;
   if (!sd->indi_format_strbuf) sd->indi_format_strbuf = eina_strbuf_new();

   efl_canvas_group_change(pd->obj);
}

static Eina_Bool
_indi_default_format_cb(void *data, Eina_Strbuf *str, const Eina_Value value)
{
   const Eina_Value_Type *type = eina_value_type_get(&value);
   Elm_Slider_Data *sd = efl_data_scope_get(data, ELM_SLIDER_CLASS);
   double v;

   if (type != EINA_VALUE_TYPE_DOUBLE) return EINA_FALSE;

   eina_value_get(&value, &v);
   eina_strbuf_append_printf(str, sd->indi_template, v);

   return EINA_TRUE;
}

static void
_indi_default_format_free_cb(void *data)
{
   Elm_Slider_Data *sd = efl_data_scope_get(data, ELM_SLIDER_CLASS);

   if (sd && sd->indi_template)
     {
        eina_stringshare_del(sd->indi_template);
        sd->indi_template = NULL;
     }
}

EOLIAN static void
_elm_slider_part_indicator_efl_ui_format_format_string_set(Eo *obj, void *_pd EINA_UNUSED, const char *template, Efl_Ui_Format_String_Type type EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Elm_Slider_Data *sd = efl_data_scope_get(pd->obj, ELM_SLIDER_CLASS);

   if (!template) return;
   eina_stringshare_replace(&sd->indi_template, template);

   efl_ui_format_func_set(efl_part(pd->obj, "indicator"), pd->obj, _indi_default_format_cb, _indi_default_format_free_cb);
}

EOLIAN static void
_elm_slider_part_indicator_efl_ui_format_format_string_get(const Eo *obj, void *_pd EINA_UNUSED, const char **template, Efl_Ui_Format_String_Type *type EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Elm_Slider_Data *sd = efl_data_scope_get(pd->obj, ELM_SLIDER_CLASS);

   if (template) *template = sd->indi_template;
}

EOLIAN static void
_elm_slider_part_indicator_visible_mode_set(Eo *obj, void *_pd EINA_UNUSED, Elm_Slider_Indicator_Visible_Mode indicator_visible_mode)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Elm_Slider_Data *sd = efl_data_scope_get(pd->obj, ELM_SLIDER_CLASS);

   if (!eina_streq(pd->part, "indicator")) return;

   if (sd->indicator_visible_mode == indicator_visible_mode) return;

   sd->indicator_visible_mode = indicator_visible_mode;
}

EOLIAN static Elm_Slider_Indicator_Visible_Mode
_elm_slider_part_indicator_visible_mode_get(const Eo *obj, void *_pd EINA_UNUSED)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   Elm_Slider_Data *sd = efl_data_scope_get(pd->obj, ELM_SLIDER_CLASS);

   if (!eina_streq(pd->part, "indicator")) return ELM_SLIDER_INDICATOR_VISIBLE_MODE_NONE;
   return sd->indicator_visible_mode;
}

void _elm_slider_part_indicator_efl_ui_format_apply_formatted_value(Eo *obj EINA_UNUSED, Elm_Part_Data *pd EINA_UNUSED)
{
}

#include "elm_slider_part_indicator_eo.c"

/* Efl.Part end */

/* Legacy APIs */

EAPI Evas_Object *
elm_slider_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EAPI void
elm_slider_span_size_set(Evas_Object *obj, Evas_Coord size)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);
   _slider_span_size_set(obj, sd, size);
}

EAPI Evas_Coord
elm_slider_span_size_get(const Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd, 0);
   return sd->size;
}

EAPI void
elm_slider_unit_format_set(Evas_Object *obj, const char *units)
{
   efl_ui_format_string_set(obj, units, EFL_UI_FORMAT_STRING_TYPE_SIMPLE);
}

EAPI const char *
elm_slider_unit_format_get(const Evas_Object *obj)
{
   const char* fmt = NULL;
   efl_ui_format_string_get(obj, &fmt, NULL);
   return fmt;
}

EAPI void
elm_slider_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   Efl_Ui_Layout_Orientation dir;
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);

   dir = horizontal ? EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL : EFL_UI_LAYOUT_ORIENTATION_VERTICAL;
   dir |= (sd->dir & EFL_UI_LAYOUT_ORIENTATION_INVERTED);

   sd->dir = dir;

   efl_ui_widget_theme_apply(obj);
}

EAPI Eina_Bool
elm_slider_horizontal_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd, EINA_FALSE);

   return _is_horizontal(sd->dir);
}

EAPI void
elm_slider_step_set(Evas_Object *obj, double step)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);

   if (step <= 0)
     {
        ERR("Wrong param. The step(%lf) should be greater than 0.0", step);
        return;
     }
   if (sd->step == step) return;

   sd->step = step;
}

EAPI double
elm_slider_step_get(const Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd, 0.0);
   return sd->step;
}

EAPI void
elm_slider_value_set(Evas_Object *obj, double val)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);
   if (EINA_DBL_EQ(val, sd->val)) return;

   sd->val = val;

   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   _val_set(obj);
}

EAPI double
elm_slider_value_get(const Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd, 0.0);
   return sd->val;
}

EAPI void
elm_slider_inverted_set(Evas_Object *obj, Eina_Bool inverted)
{
   Efl_Ui_Layout_Orientation dir;
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);

   dir = sd->dir & EFL_UI_LAYOUT_ORIENTATION_AXIS_BITMASK;
   if (inverted) dir |= EFL_UI_LAYOUT_ORIENTATION_INVERTED;

   sd->dir = dir;

   efl_ui_widget_theme_apply(obj);
}

EAPI Eina_Bool
elm_slider_inverted_get(const Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd, EINA_FALSE);

   return efl_ui_layout_orientation_is_inverted(sd->dir);
}

typedef struct
{
   slider_func_type format_cb;
   slider_freefunc_type format_free_cb;
} Slider_Format_Wrapper_Data;

static Eina_Bool
_format_legacy_to_format_eo_cb(void *data, Eina_Strbuf *str, const Eina_Value value)
{
   Slider_Format_Wrapper_Data *sfwd = data;
   char *buf = NULL;
   double val = 0;
   const Eina_Value_Type *type = eina_value_type_get(&value);

   if (type == EINA_VALUE_TYPE_DOUBLE)
     eina_value_get(&value, &val);

   if (sfwd->format_cb)
     buf = sfwd->format_cb(val);
   if (buf)
     eina_strbuf_append(str, buf);
   if (sfwd->format_free_cb) sfwd->format_free_cb(buf);

   return EINA_TRUE;
}

static void
_format_legacy_to_format_eo_free_cb(void *data)
{
   Slider_Format_Wrapper_Data *sfwd = data;
   free(sfwd);
}

EAPI void
elm_slider_units_format_function_set(Evas_Object *obj, slider_func_type func, slider_freefunc_type free_func)
{
   Slider_Format_Wrapper_Data *sfwd = malloc(sizeof(Slider_Format_Wrapper_Data));

   sfwd->format_cb = func;
   sfwd->format_free_cb = free_func;

   efl_ui_format_func_set(obj, sfwd, _format_legacy_to_format_eo_cb, _format_legacy_to_format_eo_free_cb);
}

EAPI void
elm_slider_range_enabled_set(Evas_Object *obj, Eina_Bool enable)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);

   if (sd->intvl_enable == enable) return;

   sd->intvl_enable = enable;
   efl_ui_widget_theme_apply(obj);
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

EAPI Eina_Bool
elm_slider_range_enabled_get(const Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, pd, EINA_FALSE);
   return pd->intvl_enable;
}

EAPI void
elm_slider_range_set(Evas_Object *obj, double from, double to)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);
   sd->intvl_from = from;
   sd->val = from;
   sd->intvl_to = to;

   if (sd->intvl_from < sd->val_min) {
        sd->intvl_from = sd->val_min;
        sd->val = sd->val_min;
   }
   if (sd->intvl_to > sd->val_max) sd->intvl_to = sd->val_max;

   _val_set(obj);
}

EAPI void
elm_slider_range_get(const Evas_Object *obj, double *from, double *to)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);

   if (from) *from = fmin(sd->intvl_from, sd->intvl_to);
   if (to) *to = fmax(sd->intvl_from, sd->intvl_to);
}

EAPI void
elm_slider_min_max_set(Evas_Object *obj, double min, double max)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);

   if (max < min)
     {
        ERR("Wrong params. min(%lf) is greater than max(%lf).", min, max);
        return;
     }
   if (EINA_DBL_EQ(max, min))
     {
        ERR("min and max must have a different value");
        return;
     }
   if ((EINA_DBL_EQ(sd->val_min, min)) && (EINA_DBL_EQ(sd->val_max, max))) return;
   sd->val_min = min;
   sd->val_max = max;
   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   _val_set(obj);
}

EAPI void
elm_slider_min_max_get(const Evas_Object *obj, double *min, double *max)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);

   if (min) *min = sd->val_min;
   if (max) *max = sd->val_max;
}

EAPI void
elm_slider_indicator_format_set(Evas_Object *obj, const char *indicator)
{
   efl_ui_format_string_set(efl_part(obj, "indicator"), indicator, EFL_UI_FORMAT_STRING_TYPE_SIMPLE);
}

EAPI const char *
elm_slider_indicator_format_get(const Evas *obj)
{
   const char *fmt = NULL;
   efl_ui_format_string_get(efl_part(obj, "indicator"), &fmt, NULL);
   return fmt;
}

EAPI void
elm_slider_indicator_format_function_set(Evas_Object *obj, slider_func_type func, slider_freefunc_type free_func)
{
   Slider_Format_Wrapper_Data *sfwd = malloc(sizeof(Slider_Format_Wrapper_Data));

   sfwd->format_cb = func;
   sfwd->format_free_cb = free_func;

   efl_ui_format_func_set(efl_part(obj, "indicator"), sfwd,
                          _format_legacy_to_format_eo_cb,
                          _format_legacy_to_format_eo_free_cb);
}

EAPI void
elm_slider_indicator_show_on_focus_set(Evas_Object *obj, Eina_Bool flag)
{
   elm_slider_part_indicator_visible_mode_set(efl_part(obj, "indicator"),
                                       flag ? ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS
                                       : ELM_SLIDER_INDICATOR_VISIBLE_MODE_DEFAULT);
}

EAPI Eina_Bool
elm_slider_indicator_show_on_focus_get(const Evas_Object *obj)
{
   return (elm_slider_part_indicator_visible_mode_get(efl_part(obj, "indicator"))
           == ELM_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS);
}

EAPI void
elm_slider_indicator_show_set(Evas_Object *obj, Eina_Bool show)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd);

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

EAPI Eina_Bool
elm_slider_indicator_show_get(const Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET_OR_RETURN(obj, sd, EINA_FALSE);
   return sd->indicator_show;
}

EAPI void
elm_slider_indicator_visible_mode_set(Evas_Object *obj, Elm_Slider_Indicator_Visible_Mode indicator_visible_mode)
{
   elm_slider_part_indicator_visible_mode_set(efl_part(obj, "indicator"), indicator_visible_mode);
}

EAPI Elm_Slider_Indicator_Visible_Mode
elm_slider_indicator_visible_mode_get(const Evas_Object *obj)
{
   return elm_slider_part_indicator_visible_mode_get(efl_part(obj, "indicator"));
}

void _elm_slider_efl_ui_format_apply_formatted_value(Eo *obj EINA_UNUSED, Elm_Slider_Data *pd EINA_UNUSED)
{
}

/* Internal EO APIs and hidden overrides */

EFL_UI_LAYOUT_CONTENT_ALIASES_IMPLEMENT(elm_slider)

#define ELM_SLIDER_EXTRA_OPS \
   EFL_UI_LAYOUT_CONTENT_ALIASES_OPS(elm_slider)

#include "elm_slider_eo.c"
