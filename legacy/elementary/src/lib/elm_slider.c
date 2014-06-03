#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_slider.h"
#include "elm_widget_layout.h"

#define ELM_INTERFACE_ATSPI_VALUE_PROTECTED
#include "elm_interface_atspi_value.eo.h"

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
   ELM_SLIDER_DATA_GET(data, sd);

   sd->delay = NULL;
   evas_object_smart_callback_call(data, SIG_DELAY_CHANGED, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_val_fetch(Evas_Object *obj, Eina_Bool user_event)
{
   Eina_Bool rtl;
   double posx = 0.0, posy = 0.0, pos = 0.0, val;

   ELM_SLIDER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.dragable.slider", &posx, &posy);
   if (sd->horizontal) pos = posx;
   else pos = posy;

   rtl = elm_widget_mirrored_get(obj);
   if ((!rtl && sd->inverted) ||
       (rtl && ((!sd->horizontal && sd->inverted) ||
                (sd->horizontal && !sd->inverted))))
     pos = 1.0 - pos;

   val = (pos * (sd->val_max - sd->val_min)) + sd->val_min;
   if (val != sd->val)
     {
        sd->val = val;
        if (user_event)
          {
             evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
             ecore_timer_del(sd->delay);
             sd->delay = ecore_timer_add(SLIDER_DELAY_CHANGED_INTERVAL, _delay_change, obj);
          }
     }
}

static void
_val_set(Evas_Object *obj)
{
   Eina_Bool rtl;
   double pos;

   ELM_SLIDER_DATA_GET(obj, sd);

   if (sd->val_max > sd->val_min)
     pos = (sd->val - sd->val_min) / (sd->val_max - sd->val_min);
   else pos = 0.0;

   if (pos < 0.0) pos = 0.0;
   else if (pos > 1.0)
     pos = 1.0;

   rtl = elm_widget_mirrored_get(obj);
   if ((!rtl && sd->inverted) ||
       (rtl && ((!sd->horizontal && sd->inverted) ||
                (sd->horizontal && !sd->inverted))))
     pos = 1.0 - pos;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.slider", pos, pos);
}

static void
_units_set(Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET(obj, sd);

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

        snprintf(buf, sizeof(buf), sd->units, sd->val);
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
   ELM_SLIDER_DATA_GET(obj, sd);

   if (sd->indicator_format_func)
     {
        char *buf;

        buf = sd->indicator_format_func(sd->val);
        elm_layout_text_set(obj, "elm.indicator", buf);
        elm_layout_text_set(obj, "elm.dragable.slider:elm.indicator", buf);
        if (sd->popup)
          edje_object_part_text_set(sd->popup, "elm.indicator", buf);

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
     }
   else
     {
        elm_layout_text_set(obj, "elm.indicator", NULL);
        elm_layout_text_set(obj, "elm.dragable.slider:elm.indicator", NULL);
        if (sd->popup)
          edje_object_part_text_set(sd->popup, "elm.indicator", NULL);
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

   if (sd->inverted) step *= -1.0;

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

   ELM_SLIDER_DATA_GET(data, sd);
   step = -sd->step;

   if (sd->inverted) step *= -1.0;

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
   ELM_SLIDER_DATA_GET(data, sd);
   if (sd->popup)
     {
        evas_object_layer_set(sd->popup, evas_object_layer_get(data));
        evas_object_raise(sd->popup);
        evas_object_show(sd->popup);
        sd->popup_visible = EINA_TRUE;
        edje_object_signal_emit(sd->popup, "popup,show", "elm"); // XXX: for compat
        edje_object_signal_emit(sd->popup, "elm,popup,show", "elm");
     }
}

static void
_popup_hide(void *data,
            Evas_Object *obj EINA_UNUSED,
            const char *emission EINA_UNUSED,
            const char *source EINA_UNUSED)
{
   ELM_SLIDER_DATA_GET(data, sd);

   if (!sd->popup_visible || !sd->popup) return;

   if (!(elm_widget_focus_get(data) && sd->always_popup_show))
     {
        // XXX: for compat
        edje_object_signal_emit(sd->popup, "popup,hide", "elm");
        edje_object_signal_emit(sd->popup, "elm,popup,hide", "elm");
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
        if (!(elm_widget_focus_get(data) && sd->always_popup_show))
          {
             evas_object_hide(sd->popup);
             sd->popup_visible = EINA_FALSE;
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
}

static Eina_Bool
_key_action_drag(Evas_Object *obj, const char *params)
{
   ELM_SLIDER_DATA_GET(obj, sd);
   const char *dir = params;

   if (!strcmp(dir, "left"))
     {
        if (!sd->horizontal) return EINA_FALSE;
        if (!sd->inverted) _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "right"))
     {
        if (!sd->horizontal) return EINA_FALSE;
        if (!sd->inverted) _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "up"))
     {
        if (sd->horizontal) return EINA_FALSE;
        if (sd->inverted) _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
     }
   else if (!strcmp(dir, "down"))
     {
        if (sd->horizontal) return EINA_FALSE;
        if (sd->inverted) _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
     }
   else return EINA_FALSE;

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_slider_elm_widget_event(Eo *obj, Elm_Slider_Data *sd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void) src;

   if (type == EVAS_CALLBACK_KEY_DOWN)
     {
        Evas_Event_Key_Down *ev = event_info;
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

        if (!_elm_config_key_binding_call(obj, ev, key_actions))
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
     }
   else return EINA_FALSE;

   _popup_show(obj, NULL, NULL, NULL);
   _slider_update(obj, EINA_TRUE);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_slider_elm_widget_activate(Eo *obj, Elm_Slider_Data *sd, Elm_Activate act)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (act == ELM_ACTIVATE_DEFAULT) return EINA_FALSE;

   if ((act == ELM_ACTIVATE_UP) ||
       (act == ELM_ACTIVATE_RIGHT))
     {
        if (!sd->inverted) _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
     }
   else if ((act == ELM_ACTIVATE_DOWN) ||
            (act == ELM_ACTIVATE_LEFT))
     {
        if (!sd->inverted) _drag_down(obj, NULL, NULL, NULL);
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

   ELM_SLIDER_DATA_GET(data, sd);
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(sd->popup, x, y);
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
_popup_add(Elm_Slider_Data *sd, Eo *obj)
{
   /* if theme has an overlayed slider mode, then lets support it */
   if (!edje_object_part_exists(elm_layout_edje_get(obj), "elm.track.slider")) return;

   // XXX popup needs to adapt to theme etc.
   sd->popup = edje_object_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(sd->popup, obj);
   if (sd->horizontal)
     _elm_theme_set(NULL, sd->popup, "slider", "horizontal/popup", elm_widget_style_get(obj));
   else
     _elm_theme_set(NULL, sd->popup, "slider", "vertical/popup", elm_widget_style_get(obj));
   edje_object_scale_set(sd->popup, elm_widget_scale_get(obj) *
                         elm_config_scale_get());
   edje_object_signal_callback_add(sd->popup, "popup,hide,done", "elm", // XXX: for compat
                                   _popup_hide_done, obj);
   edje_object_signal_callback_add(sd->popup, "elm,popup,hide,done", "elm",
                                   _popup_hide_done, obj);

   /* create a rectangle to track position+size of the dragable */
   sd->track = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_event_callback_add
     (sd->track, EVAS_CALLBACK_MOVE, _track_move_cb, obj);
   evas_object_event_callback_add
     (sd->track, EVAS_CALLBACK_RESIZE, _track_resize_cb, obj);

   evas_object_color_set(sd->track, 0, 0, 0, 0);
   evas_object_pass_events_set(sd->track, EINA_TRUE);
   elm_layout_content_set(obj, "elm.track.slider", sd->track);
}

EOLIAN static Eina_Bool
_elm_slider_elm_widget_theme_apply(Eo *obj, Elm_Slider_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;
   ELM_LAYOUT_DATA_GET(obj, ld);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   if (sd->horizontal)
     {
        eina_stringshare_replace(&ld->group, "horizontal");
        if (sd->popup)
          _elm_theme_set(NULL, sd->popup,
                         "slider", "horizontal/popup",
                         elm_widget_style_get(obj));
     }
   else
     {
        eina_stringshare_replace(&ld->group, "vertical");
        if (sd->popup)
          _elm_theme_set(NULL, sd->popup,
                         "slider", "vertical/popup",
                         elm_widget_style_get(obj));
     }

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   if (sd->popup)
     edje_object_scale_set(sd->popup, elm_widget_scale_get(obj) *
                           elm_config_scale_get());
   else
     _popup_add(sd, obj);

   if (sd->horizontal)
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * elm_widget_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * elm_widget_scale_get(obj) *
       elm_config_scale_get());

   if (sd->inverted)
     {
        elm_layout_signal_emit(obj, "elm,state,inverted,on", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,inverted,on", "elm");
     }
   if (sd->indicator_show)
     {
        elm_layout_signal_emit(obj, "elm,state,val,show", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,show", "elm");
     }
   _min_max_set(obj);
   _units_set(obj);
   _indicator_set(obj);
   _visuals_refresh(obj);

   edje_object_message_signal_process(wd->resize_obj);
   if (sd->popup)
     edje_object_message_signal_process(sd->popup);

   evas_object_smart_changed(obj);

   return EINA_TRUE;
}

EOLIAN static void
_elm_slider_elm_layout_sizing_eval(Eo *obj, Elm_Slider_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_spacer_down_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   ELM_SLIDER_DATA_GET(data, sd);

   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w, h;
   double button_x = 0.0, button_y = 0.0;

   sd->spacer_down = EINA_TRUE;
   sd->val2 = sd->val;
   evas_object_geometry_get(sd->spacer, &x, &y, &w, &h);
   sd->downx = ev->canvas.x - x;
   sd->downy = ev->canvas.y - y;
   if (sd->horizontal)
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

   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.slider",
     button_x, button_y);
   _slider_update(data, EINA_TRUE);
   evas_object_smart_callback_call(data, SIG_DRAG_START, NULL);
   elm_layout_signal_emit(data, "elm,state,indicator,show", "elm");
}

static void
_spacer_move_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   ELM_SLIDER_DATA_GET(data, sd);

   Evas_Coord x, y, w, h;
   double button_x = 0.0, button_y = 0.0;
   Evas_Event_Mouse_Move *ev = event_info;

   if (sd->spacer_down)
     {
        Evas_Coord d = 0;

        evas_object_geometry_get(sd->spacer, &x, &y, &w, &h);
        if (sd->horizontal) d = abs(ev->cur.canvas.x - x - sd->downx);
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
             evas_object_smart_callback_call(data, SIG_DRAG_STOP, NULL);
             if (sd->frozen)
               {
                  elm_widget_scroll_freeze_pop(data);
                  sd->frozen = EINA_FALSE;
               }
             elm_layout_signal_emit(data, "elm,state,indicator,hide", "elm");
             elm_slider_value_set(data, sd->val2);
             return;
          }
        if (sd->horizontal)
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

        ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
        edje_object_part_drag_value_set
          (wd->resize_obj, "elm.dragable.slider",
          button_x, button_y);
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
_min_max_set(Evas_Object *obj)
{
   char *buf_min = NULL;
   char *buf_max = NULL;

   ELM_SLIDER_DATA_GET(obj, sd);

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
_elm_slider_evas_object_smart_calculate(Eo *obj, Elm_Slider_Data *sd)
{
   elm_layout_freeze(obj);

   if (sd->horizontal)
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * elm_widget_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * elm_widget_scale_get(obj) *
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

EOLIAN static void
_elm_slider_evas_object_smart_add(Eo *obj, Elm_Slider_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->horizontal = EINA_TRUE;
   priv->indicator_show = EINA_TRUE;
   priv->val_max = 1.0;
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

   _popup_add(priv, obj);

   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_DOWN, _spacer_down_cb, obj);
   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_MOVE, _spacer_move_cb, obj);
   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_UP, _spacer_up_cb, obj);

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
_elm_slider_evas_object_smart_del(Eo *obj, Elm_Slider_Data *sd)
{
   eina_stringshare_del(sd->indicator);
   eina_stringshare_del(sd->units);
   ecore_timer_del(sd->delay);
   evas_object_del(sd->popup);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_slider_elm_layout_text_aliases_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *_pd EINA_UNUSED)
{
   return _text_aliases;
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_slider_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

EAPI Evas_Object *
elm_slider_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_slider_eo_base_constructor(Eo *obj, Elm_Slider_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));
}

EOLIAN static void
_elm_slider_span_size_set(Eo *obj, Elm_Slider_Data *sd, Evas_Coord size)
{
   if (sd->size == size) return;
   sd->size = size;

   if (sd->indicator_show)
     {
        elm_layout_signal_emit(obj, "elm,state,val,show", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,show", "elm");
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,state,val,hide", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,hide", "elm");
     }

   evas_object_smart_changed(obj);
}

EOLIAN static Evas_Coord
_elm_slider_span_size_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd)
{
   return sd->size;
}

EOLIAN static void
_elm_slider_unit_format_set(Eo *obj, Elm_Slider_Data *sd, const char *units)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eina_stringshare_replace(&sd->units, units);
   if (units)
     {
        elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");
        edje_object_message_signal_process(wd->resize_obj);
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,units,visible", "elm");
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,state,units,hidden", "elm");
        edje_object_message_signal_process(wd->resize_obj);
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,units,hidden", "elm");
     }
   evas_object_smart_changed(obj);
}

EOLIAN static const char*
_elm_slider_unit_format_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd)
{
   return sd->units;
}

EOLIAN static void
_elm_slider_indicator_format_set(Eo *obj, Elm_Slider_Data *sd, const char *indicator)
{
   eina_stringshare_replace(&sd->indicator, indicator);
   evas_object_smart_changed(obj);
}

EOLIAN static const char*
_elm_slider_indicator_format_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd)
{
   return sd->indicator;
}

EOLIAN static void
_elm_slider_horizontal_set(Eo *obj, Elm_Slider_Data *sd, Eina_Bool horizontal)
{
   horizontal = !!horizontal;
   if (sd->horizontal == horizontal) return;
   sd->horizontal = horizontal;

   eo_do(obj, elm_obj_widget_theme_apply());
}

EOLIAN static Eina_Bool
_elm_slider_horizontal_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd)
{
   return sd->horizontal;
}

EOLIAN static void
_elm_slider_min_max_set(Eo *obj, Elm_Slider_Data *sd, double min, double max)
{
   if ((sd->val_min == min) && (sd->val_max == max)) return;
   sd->val_min = min;
   sd->val_max = max;
   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   _visuals_refresh(obj);
}

EOLIAN static void
_elm_slider_min_max_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd, double *min, double *max)
{
   if (min) *min = 0.0;
   if (max) *max = 0.0;


   if (min) *min = sd->val_min;
   if (max) *max = sd->val_max;
}

EOLIAN static void
_elm_slider_value_set(Eo *obj, Elm_Slider_Data *sd, double val)
{
   if (sd->val == val) return;
   sd->val = val;

   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   _visuals_refresh(obj);
}

EOLIAN static double
_elm_slider_value_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd)
{
   return sd->val;
}

EOLIAN static void
_elm_slider_inverted_set(Eo *obj, Elm_Slider_Data *sd, Eina_Bool inverted)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   inverted = !!inverted;
   if (sd->inverted == inverted) return;
   sd->inverted = inverted;

   if (sd->inverted)
     {
        elm_layout_signal_emit(obj, "elm,state,inverted,on", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,inverted,on", "elm");
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,state,inverted,off", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,inverted,off", "elm");
     }

   edje_object_message_signal_process(wd->resize_obj);

   _visuals_refresh(obj);
}

EOLIAN static Eina_Bool
_elm_slider_inverted_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd)
{
   return sd->inverted;
}

EOLIAN static void
_elm_slider_indicator_format_function_set(Eo *obj, Elm_Slider_Data *sd, slider_func_type func, slider_freefunc_type free_func)
{
   sd->indicator_format_func = func;
   sd->indicator_format_free = free_func;
   evas_object_smart_changed(obj);
}

EOLIAN static void
_elm_slider_units_format_function_set(Eo *obj, Elm_Slider_Data *sd, slider_func_type func, slider_freefunc_type free_func)
{
   sd->units_format_func = func;
   sd->units_format_free = free_func;

   evas_object_smart_changed(obj);
}

EOLIAN static void
_elm_slider_indicator_show_set(Eo *obj, Elm_Slider_Data *sd, Eina_Bool show)
{
   if (show)
     {
        sd->indicator_show = EINA_TRUE;
        elm_layout_signal_emit(obj, "elm,state,val,show", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,show", "elm");
     }
   else
     {
        sd->indicator_show = EINA_FALSE;
        elm_layout_signal_emit(obj, "elm,state,val,hide", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,hide", "elm");
     }

   evas_object_smart_changed(obj);
}

EOLIAN static Eina_Bool
_elm_slider_indicator_show_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd)
{
   return sd->indicator_show;
}

EOLIAN static void
_elm_slider_step_set(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd, double step)
{
   if (sd->step == step) return;

   if (step < 0.0) step = 0.0;
   else if (step > 1.0) step = 1.0;

   sd->step = step;
}

EOLIAN static double
_elm_slider_step_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd)
{
   return sd->step;
}

EOLIAN static void
_elm_slider_indicator_show_on_focus_set(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd, Eina_Bool flag)
{
   if (flag)
     sd->always_popup_show = EINA_TRUE;
   else
     sd->always_popup_show = EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_slider_indicator_show_on_focus_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd)
{
   return sd->always_popup_show;
}

EOLIAN static Eina_Bool
_elm_slider_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Slider_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_slider_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Slider_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_slider_elm_widget_on_focus(Eo *obj, Elm_Slider_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_on_focus());

   if (sd->always_popup_show && elm_widget_focus_get(obj))
     _popup_show(obj, NULL, NULL, NULL);
   else
     _popup_hide(obj, NULL, NULL, NULL);

   return int_ret;
}

EOLIAN static void
_elm_slider_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

// A11Y Accessibility

EOLIAN static void
_elm_slider_elm_interface_atspi_value_value_and_text_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd, double *value, const char **text)
{
   if (value) *value = sd->val;
   if (text) *text = NULL;
}

EOLIAN static Eina_Bool
_elm_slider_elm_interface_atspi_value_value_and_text_set(Eo *obj, Elm_Slider_Data *sd, double value, const char *text EINA_UNUSED)
{
   double oldval = sd->val;

   if (sd->val_min > value) return EINA_FALSE;
   if (sd->val_max < value) return EINA_FALSE;

   evas_object_smart_callback_call(obj, SIG_DRAG_START, NULL);
   sd->val = value;
   _visuals_refresh(obj);
   sd->val = oldval;
   _slider_update(obj, EINA_TRUE);
   evas_object_smart_callback_call(obj, SIG_DRAG_STOP, NULL);

   return EINA_TRUE;
}

EOLIAN static void
_elm_slider_elm_interface_atspi_value_range_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd, double *lower, double *upper, const char **descr)
{
   if (lower) *lower = sd->val_min;
   if (upper) *upper = sd->val_max;
   if (descr) *descr = NULL;
}

EOLIAN static double
_elm_slider_elm_interface_atspi_value_increment_get(Eo *obj EINA_UNUSED, Elm_Slider_Data *sd)
{
   return sd->step;
}

// A11Y Accessibility - END

#include "elm_slider.eo.c"
