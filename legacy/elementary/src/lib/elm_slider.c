#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_slider.h"

EAPI const char ELM_SLIDER_SMART_NAME[] = "elm_slider";

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
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_SLIDER_SMART_NAME, _elm_slider, Elm_Slider_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static Eina_Bool
_delay_change(void *data)
{
   ELM_SLIDER_DATA_GET(data, sd);

   sd->delay = NULL;
   evas_object_smart_callback_call(data, SIG_DELAY_CHANGED, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static void
_val_fetch(Evas_Object *obj)
{
   Eina_Bool rtl;
   double posx = 0.0, posy = 0.0, pos = 0.0, val;

   ELM_SLIDER_DATA_GET(obj, sd);

   edje_object_part_drag_value_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.slider", &posx, &posy);
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
        evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
        if (sd->delay) ecore_timer_del(sd->delay);
        sd->delay = ecore_timer_add(0.2, _delay_change, obj);
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

   edje_object_part_drag_value_set
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.slider", pos, pos);
}

static void
_units_set(Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET(obj, sd);

   if (sd->units_format_func)
     {
        char *buf;

        buf = sd->units_format_func(sd->val);
        elm_layout_text_set(obj, "elm.units", buf);

        if (sd->units_format_free) sd->units_format_free(buf);
     }
   else if (sd->units)
     {
        char buf[1024];

        snprintf(buf, sizeof(buf), sd->units, sd->val);
        elm_layout_text_set(obj, "elm.units", buf);
     }
   else elm_layout_text_set(obj, "elm.units", NULL);
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
_slider_update(Evas_Object *obj)
{
   evas_object_smart_changed(obj);
}

static void
_drag(void *data,
      Evas_Object *obj __UNUSED__,
      const char *emission __UNUSED__,
      const char *source __UNUSED__)
{
   _slider_update(data);
}

static void
_drag_start(void *data,
            Evas_Object *obj __UNUSED__,
            const char *emission __UNUSED__,
            const char *source __UNUSED__)
{
   _slider_update(data);
   evas_object_smart_callback_call(data, SIG_DRAG_START, NULL);
   elm_widget_scroll_freeze_push(data);
}

static void
_drag_stop(void *data,
           Evas_Object *obj __UNUSED__,
           const char *emission __UNUSED__,
           const char *source __UNUSED__)
{
   _slider_update(data);
   evas_object_smart_callback_call(data, SIG_DRAG_STOP, NULL);
   elm_widget_scroll_freeze_pop(data);
}

static void
_drag_step(void *data,
           Evas_Object *obj __UNUSED__,
           const char *emission __UNUSED__,
           const char *source __UNUSED__)
{
   return;

   _slider_update(data);
}

static void
_drag_up(void *data,
         Evas_Object *obj __UNUSED__,
         const char *emission __UNUSED__,
         const char *source __UNUSED__)
{
   double step;

   ELM_SLIDER_DATA_GET(data, sd);
   step = 0.05;

   if (sd->inverted) step *= -1.0;

   edje_object_part_drag_step
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.slider", step, step);
}

static void
_drag_down(void *data,
           Evas_Object *obj __UNUSED__,
           const char *emission __UNUSED__,
           const char *source __UNUSED__)
{
   double step;

   ELM_SLIDER_DATA_GET(data, sd);
   step = -0.05;

   if (sd->inverted) step *= -1.0;

   edje_object_part_drag_step
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.slider", step, step);
}

static void
_popup_show(void *data,
            Evas_Object *obj __UNUSED__,
            const char *emission __UNUSED__,
            const char *source __UNUSED__)
{
   ELM_SLIDER_DATA_GET(data, sd);
   if (sd->popup)
     {
        evas_object_layer_set(sd->popup, evas_object_layer_get(data));
        evas_object_raise(sd->popup);
        evas_object_show(sd->popup);
        edje_object_signal_emit(sd->popup, "popup,show", "elm");
     }
}

static void
_popup_hide(void *data,
            Evas_Object *obj __UNUSED__,
            const char *emission __UNUSED__,
            const char *source __UNUSED__)
{
   ELM_SLIDER_DATA_GET(data, sd);
   if (sd->popup)
     {
        if (!sd->popup_hiding)
          {
             edje_object_signal_emit(sd->popup, "popup,hide", "elm");
             sd->popup_hiding = EINA_TRUE;
          }
     }
}

static void
_popup_hide_done(void *data,
                 Evas_Object *obj __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   ELM_SLIDER_DATA_GET(data, sd);
   if (sd->popup)
     {
        if (sd->popup_hiding)
          {
             evas_object_hide(sd->popup);
             sd->popup_hiding = EINA_FALSE;
          }
     }
}

static void
_popup_emit(void *data,
            Evas_Object *obj __UNUSED__,
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
_elm_slider_smart_event(Evas_Object *obj,
                        Evas_Object *src __UNUSED__,
                        Evas_Callback_Type type,
                        void *event_info)
{
   Evas_Event_Mouse_Wheel *mev;
   Evas_Event_Key_Down *ev;

   ELM_SLIDER_DATA_GET(obj, sd);

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if (type == EVAS_CALLBACK_KEY_DOWN) goto key_down;
   else if (type != EVAS_CALLBACK_MOUSE_WHEEL)
     return EINA_FALSE;

   mev = event_info;
   if (mev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (mev->z < 0) _drag_up(obj, NULL, NULL, NULL);
   else _drag_down(obj, NULL, NULL, NULL);
   mev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   goto success;

key_down:
   ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        if (!sd->horizontal) return EINA_FALSE;
        if (!sd->inverted) _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        goto success;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        if (!sd->horizontal) return EINA_FALSE;
        if (!sd->inverted) _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        goto success;
     }
   else if ((!strcmp(ev->keyname, "Up")) ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
     {
        if (sd->horizontal) return EINA_FALSE;
        if (sd->inverted) _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        goto success;
     }
   else if ((!strcmp(ev->keyname, "Down")) ||
            ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
     {
        if (sd->horizontal) return EINA_FALSE;
        if (sd->inverted) _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        goto success;
     }
   else return EINA_FALSE;

success:
   _slider_update(obj);

   return EINA_TRUE;
}

static void
_visuals_refresh(Evas_Object *obj)
{
   _val_set(obj);
   evas_object_smart_changed(obj);
}

static Eina_Bool
_elm_slider_smart_theme(Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET(obj, sd);

   if (sd->horizontal)
     {
        eina_stringshare_replace(&ELM_LAYOUT_DATA(sd)->group, "horizontal");
        if (sd->popup)
          _elm_theme_set(NULL, sd->popup,
                         "slider", "horizontal/popup",
                         elm_widget_style_get(obj));
     }
   else
     {
        eina_stringshare_replace(&ELM_LAYOUT_DATA(sd)->group, "vertical");
        if (sd->popup)
          _elm_theme_set(NULL, sd->popup,
                         "slider", "vertical/popup",
                         elm_widget_style_get(obj));
     }

   if (!ELM_WIDGET_CLASS(_elm_slider_parent_sc)->theme(obj)) return EINA_FALSE;

   if (sd->popup)
     edje_object_scale_set(sd->popup, elm_widget_scale_get(obj) *
                           elm_config_scale_get());

   if (sd->units)
     elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");

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

   _visuals_refresh(obj);

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
   if (sd->popup)
     edje_object_message_signal_process(sd->popup);

   evas_object_smart_changed(obj);

   return EINA_TRUE;
}

static void
_elm_slider_smart_sizing_eval(Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET(obj, sd);

   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_spacer_down_cb(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
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

   edje_object_part_drag_value_set
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.slider",
     button_x, button_y);
   _slider_update(data);
   evas_object_smart_callback_call(data, SIG_DRAG_START, NULL);
   elm_layout_signal_emit(data, "elm,state,indicator,show", "elm");
}

static void
_spacer_move_cb(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
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
             _slider_update(data);
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

        edje_object_part_drag_value_set
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.slider",
          button_x, button_y);

        _slider_update(data);
     }
}

static void
_spacer_up_cb(void *data,
              Evas *e __UNUSED__,
              Evas_Object *obj __UNUSED__,
              void *event_info __UNUSED__)
{
   ELM_SLIDER_DATA_GET(data, sd);

   if (!sd->spacer_down) return;
   if (sd->spacer_down) sd->spacer_down = EINA_FALSE;

   _slider_update(data);
   evas_object_smart_callback_call(data, SIG_DRAG_STOP, NULL);

   if (sd->frozen)
     {
        elm_widget_scroll_freeze_pop(data);
        sd->frozen = EINA_FALSE;
     }
   elm_layout_signal_emit(data, "elm,state,indicator,hide", "elm");
}

static void
_track_move_cb(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Evas_Coord x, y;

   ELM_SLIDER_DATA_GET(data, sd);
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(sd->popup, x, y);
}

static void
_track_resize_cb(void *data,
                 Evas *e __UNUSED__,
                 Evas_Object *obj,
                 void *event_info __UNUSED__)
{
   Evas_Coord w, h;

   ELM_SLIDER_DATA_GET(data, sd);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(sd->popup, w, h);
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
        int length = strlen(sd->units);

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

static void
_elm_slider_smart_calculate(Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET(obj, sd);

   elm_layout_freeze(obj);

   if (sd->horizontal)
     evas_object_size_hint_min_set
       (sd->spacer, (double)sd->size * elm_widget_scale_get(obj) *
       elm_config_scale_get(), 1);
   else
     evas_object_size_hint_min_set
       (sd->spacer, 1, (double)sd->size * elm_widget_scale_get(obj) *
       elm_config_scale_get());

   _val_fetch(obj);
   _units_set(obj);
   _min_max_set(obj);
   _indicator_set(obj);

   elm_layout_thaw(obj);
}

static char *
_access_info_cb(void *data __UNUSED__,
                Evas_Object *obj,
                Elm_Widget_Item *item __UNUSED__)
{
   const char *txt = elm_widget_access_info_get(obj);

   if (!txt) txt = elm_layout_text_get(obj, NULL);
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data __UNUSED__,
                 Evas_Object *obj,
                 Elm_Widget_Item *item __UNUSED__)
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
_elm_slider_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Slider_Smart_Data);

   ELM_WIDGET_CLASS(_elm_slider_parent_sc)->base.add(obj);

   priv->horizontal = EINA_TRUE;
   priv->indicator_show = EINA_TRUE;
   priv->val = 0.0;
   priv->val_min = 0.0;
   priv->val_max = 1.0;

   elm_layout_theme_set
     (obj, "slider", "horizontal", elm_widget_style_get(obj));

   elm_layout_signal_callback_add(obj, "drag", "*", _drag, obj);
   elm_layout_signal_callback_add(obj, "drag,start", "*", _drag_start, obj);
   elm_layout_signal_callback_add(obj, "drag,stop", "*", _drag_stop, obj);
   elm_layout_signal_callback_add(obj, "drag,step", "*", _drag_step, obj);
   elm_layout_signal_callback_add(obj, "drag,page", "*", _drag_stop, obj);
   elm_layout_signal_callback_add(obj, "popup,show", "elm", _popup_show, obj);
   elm_layout_signal_callback_add(obj, "popup,hide", "elm", _popup_hide, obj);
   elm_layout_signal_callback_add(obj, "*", "popup,emit", _popup_emit, obj);
   edje_object_part_drag_value_set
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm.dragable.slider", 0.0, 0.0);

   priv->spacer = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(priv->spacer, 0, 0, 0, 0);
   evas_object_pass_events_set(priv->spacer, EINA_TRUE);
   elm_layout_content_set(obj, "elm.swallow.bar", priv->spacer);

   /* if theme has an overlayed slider mode, then lets support it */
   if (edje_object_part_exists(elm_layout_edje_get(obj),
                               "elm.track.slider"))
     {
        // XXX popup needs to adapt to theme etc.
        priv->popup = edje_object_add(evas_object_evas_get(obj));
        _elm_theme_set(NULL, priv->popup,
                       "slider", "horizontal/popup",
                       elm_widget_style_get(obj));
        edje_object_scale_set(priv->popup, elm_widget_scale_get(obj) *
                              elm_config_scale_get());
        edje_object_signal_callback_add(priv->popup, "popup,hide,done", "elm",
                                        _popup_hide_done, obj);

        /* create a rectangle to track position+size of the dragable */
        priv->track = evas_object_rectangle_add(evas_object_evas_get(obj));
        evas_object_event_callback_add
          (priv->track, EVAS_CALLBACK_MOVE, _track_move_cb, obj);
        evas_object_event_callback_add
          (priv->track, EVAS_CALLBACK_RESIZE, _track_resize_cb, obj);

        evas_object_color_set(priv->track, 0, 0, 0, 0);
        evas_object_pass_events_set(priv->track, EINA_TRUE);
        elm_layout_content_set(obj, "elm.track.slider", priv->track);
     }

   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_DOWN, _spacer_down_cb, obj);
   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_MOVE, _spacer_move_cb, obj);
   evas_object_event_callback_add
     (priv->spacer, EVAS_CALLBACK_MOUSE_UP, _spacer_up_cb, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   _elm_access_object_register(obj, ELM_WIDGET_DATA(priv)->resize_obj);
   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, E_("slider"));
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_STATE, _access_state_cb, priv);
   
   evas_object_smart_changed(obj);
}

static void
_elm_slider_smart_del(Evas_Object *obj)
{
   ELM_SLIDER_DATA_GET(obj, sd);

   if (sd->indicator) eina_stringshare_del(sd->indicator);
   if (sd->units) eina_stringshare_del(sd->units);
   if (sd->delay) ecore_timer_del(sd->delay);

   if (sd->popup) evas_object_del(sd->popup);

   ELM_WIDGET_CLASS(_elm_slider_parent_sc)->base.del(obj);
}

static void
_elm_slider_smart_set_user(Elm_Slider_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_slider_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_slider_smart_del;
   ELM_WIDGET_CLASS(sc)->base.calculate = _elm_slider_smart_calculate;

   ELM_WIDGET_CLASS(sc)->theme = _elm_slider_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_slider_smart_event;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_slider_smart_sizing_eval;

   ELM_LAYOUT_CLASS(sc)->content_aliases = _content_aliases;
   ELM_LAYOUT_CLASS(sc)->text_aliases = _text_aliases;
}

EAPI const Elm_Slider_Smart_Class *
elm_slider_smart_class_get(void)
{
   static Elm_Slider_Smart_Class _sc =
     ELM_SLIDER_SMART_CLASS_INIT_NAME_VERSION(ELM_SLIDER_SMART_NAME);
   static const Elm_Slider_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_slider_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_slider_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_slider_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_slider_span_size_set(Evas_Object *obj,
                         Evas_Coord size)
{
   ELM_SLIDER_CHECK(obj);
   ELM_SLIDER_DATA_GET(obj, sd);

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

EAPI Evas_Coord
elm_slider_span_size_get(const Evas_Object *obj)
{
   ELM_SLIDER_CHECK(obj) 0;
   ELM_SLIDER_DATA_GET(obj, sd);

   return sd->size;
}

EAPI void
elm_slider_unit_format_set(Evas_Object *obj,
                           const char *units)
{
   ELM_SLIDER_CHECK(obj);
   ELM_SLIDER_DATA_GET(obj, sd);

   eina_stringshare_replace(&sd->units, units);
   if (units)
     {
        elm_layout_signal_emit(obj, "elm,state,units,visible", "elm");
        edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,units,visible", "elm");
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,state,units,hidden", "elm");
        edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,units,hidden", "elm");
     }

   evas_object_smart_changed(obj);
}

EAPI const char *
elm_slider_unit_format_get(const Evas_Object *obj)
{
   ELM_SLIDER_CHECK(obj) NULL;
   ELM_SLIDER_DATA_GET(obj, sd);

   return sd->units;
}

EAPI void
elm_slider_indicator_format_set(Evas_Object *obj,
                                const char *indicator)
{
   ELM_SLIDER_CHECK(obj);
   ELM_SLIDER_DATA_GET(obj, sd);

   eina_stringshare_replace(&sd->indicator, indicator);
   evas_object_smart_changed(obj);
}

EAPI const char *
elm_slider_indicator_format_get(const Evas_Object *obj)
{
   ELM_SLIDER_CHECK(obj) NULL;
   ELM_SLIDER_DATA_GET(obj, sd);

   return sd->indicator;
}

EAPI void
elm_slider_horizontal_set(Evas_Object *obj,
                          Eina_Bool horizontal)
{
   ELM_SLIDER_CHECK(obj);
   ELM_SLIDER_DATA_GET(obj, sd);

   horizontal = !!horizontal;
   if (sd->horizontal == horizontal) return;
   sd->horizontal = horizontal;

   ELM_WIDGET_DATA(sd)->api->theme(obj);
}

EAPI Eina_Bool
elm_slider_horizontal_get(const Evas_Object *obj)
{
   ELM_SLIDER_CHECK(obj) EINA_FALSE;
   ELM_SLIDER_DATA_GET(obj, sd);

   return sd->horizontal;
}

EAPI void
elm_slider_min_max_set(Evas_Object *obj,
                       double min,
                       double max)
{
   ELM_SLIDER_CHECK(obj);
   ELM_SLIDER_DATA_GET(obj, sd);

   if ((sd->val_min == min) && (sd->val_max == max)) return;
   sd->val_min = min;
   sd->val_max = max;
   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   _visuals_refresh(obj);
}

EAPI void
elm_slider_min_max_get(const Evas_Object *obj,
                       double *min,
                       double *max)
{
   if (min) *min = 0.0;
   if (max) *max = 0.0;

   ELM_SLIDER_CHECK(obj);
   ELM_SLIDER_DATA_GET(obj, sd);

   if (min) *min = sd->val_min;
   if (max) *max = sd->val_max;
}

EAPI void
elm_slider_value_set(Evas_Object *obj,
                     double val)
{
   ELM_SLIDER_CHECK(obj);
   ELM_SLIDER_DATA_GET(obj, sd);

   if (sd->val == val) return;
   sd->val = val;

   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   _visuals_refresh(obj);
}

EAPI double
elm_slider_value_get(const Evas_Object *obj)
{
   ELM_SLIDER_CHECK(obj) 0.0;
   ELM_SLIDER_DATA_GET(obj, sd);

   return sd->val;
}

EAPI void
elm_slider_inverted_set(Evas_Object *obj,
                        Eina_Bool inverted)
{
   ELM_SLIDER_CHECK(obj);
   ELM_SLIDER_DATA_GET(obj, sd);

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

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);

   _visuals_refresh(obj);
}

EAPI Eina_Bool
elm_slider_inverted_get(const Evas_Object *obj)
{
   ELM_SLIDER_CHECK(obj) EINA_FALSE;
   ELM_SLIDER_DATA_GET(obj, sd);

   return sd->inverted;
}

EAPI void
elm_slider_indicator_format_function_set(Evas_Object *obj,
                                         char *(*func)(double),
                                         void (*free_func)(char *))
{
   ELM_SLIDER_CHECK(obj);
   ELM_SLIDER_DATA_GET(obj, sd);

   sd->indicator_format_func = func;
   sd->indicator_format_free = free_func;
   evas_object_smart_changed(obj);
}

EAPI void
elm_slider_units_format_function_set(Evas_Object *obj,
                                     char *(*func)(double),
                                     void (*free_func)(char *))
{
   ELM_SLIDER_CHECK(obj);
   ELM_SLIDER_DATA_GET(obj, sd);

   sd->units_format_func = func;
   sd->units_format_free = free_func;

   evas_object_smart_changed(obj);
}

EAPI void
elm_slider_indicator_show_set(Evas_Object *obj,
                              Eina_Bool show)
{
   ELM_SLIDER_CHECK(obj);
   ELM_SLIDER_DATA_GET(obj, sd);

   if (show)
     {
        sd->indicator_show = EINA_TRUE;
        elm_layout_signal_emit(obj, "elm,state,val,show", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,show", "elm");
     }
   else {
        sd->indicator_show = EINA_FALSE;
        elm_layout_signal_emit(obj, "elm,state,val,hide", "elm");
        if (sd->popup)
          edje_object_signal_emit(sd->popup, "elm,state,val,hide", "elm");
     }

   evas_object_smart_changed(obj);
}

EAPI Eina_Bool
elm_slider_indicator_show_get(const Evas_Object *obj)
{
   ELM_SLIDER_CHECK(obj) EINA_FALSE;
   ELM_SLIDER_DATA_GET(obj, sd);

   return sd->indicator_show;
}
