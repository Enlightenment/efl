#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Slider Slider
 *
 * The slider adds a dragable “slider” widget for selecting the value of
 * something within a range.
 *
 *
 * A slider can be horizontal or vertical. It can contain an Icon and has a
 * primary label as well as a units label (that is formatted with floating
 * point values and thus accepts a printf-style format string, like
 * “%1.2f units”. There is also an indicator string that may be somewhere
 * else (like on the slider itself) that also accepts a format string like
 * units. Label, Icon Unit and Indicator strings/objects are optional.
 *
 * A slider may be inverted which means values invert, with high vales being
 * on the left or top and low values on the right or bottom (as opposed to
 * normally being low on the left or top and high on the bottom and right).
 *
 * The slider should have its minimum and maximum values set by the
 * application with  elm_slider_min_max_set() and value should also be set by
 * the application before use with  elm_slider_value_set(). The span of the
 * slider is its length (horizontally or vertically). This will be scaled by
 * the object or applications scaling factor. At any point code can query the
 * slider for its value with elm_slider_value_get().
 *
 * Signals that you can add callbacks for are:
 *
 * "changed" - Whenever the slider value is changed by the user.
 * "slider,drag,start" - dragging the slider indicator around has started
 * "slider,drag,stop" - dragging the slider indicator around has stopped
 * "delay,changed" - A short time after the value is changed by the user.
 *                   This will be called only when the user stops dragging for
 *                   a very short period or when they release their
 *                   finger/mouse, so it avoids possibly expensive reactions to
 *                   the value change.
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *slider;
   Evas_Object *icon;
   Evas_Object *end;
   Evas_Object *spacer;

   Ecore_Timer *delay;

   const char *label;
   const char *units;
   const char *indicator;

   const char *(*indicator_format_func)(double val);
   void (*indicator_format_free)(const char *str);

   const char *(*units_format_func)(double val);
   void (*units_format_free)(const char *str);

   double val, val_min, val_max;
   Evas_Coord size;

   Eina_Bool horizontal : 1;
   Eina_Bool inverted : 1;
   Eina_Bool indicator_show : 1;
};

#define ELM_SLIDER_INVERTED_FACTOR (-1.0)

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _units_set(Evas_Object *obj);
static void _val_set(Evas_Object *obj);
static void _indicator_set(Evas_Object *obj);
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _drag_up(void *data, Evas_Object *obj,
                    const char *emission, const char *source);
static void _drag_down(void *data, Evas_Object *obj,
                    const char *emission, const char *source);
static Eina_Bool _event_hook(Evas_Object *obj, Evas_Object *src,
                             Evas_Callback_Type type, void *event_info);
static void _spacer_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);

static const char SIG_CHANGED[] = "changed";
static const char SIG_DELAY_CHANGED[] = "delay,changed";
static const char SIG_DRAG_START[] = "slider,drag,start";
static const char SIG_DRAG_STOP[] = "slider,drag,stop";
static const Evas_Smart_Cb_Description _signals[] = {
  {SIG_CHANGED, ""},
  {SIG_DELAY_CHANGED, ""},
  {SIG_DRAG_START, ""},
  {SIG_DRAG_STOP, ""},
  {NULL, NULL}
};

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Mouse_Wheel *mev;
   Evas_Event_Key_Down *ev;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   if (type == EVAS_CALLBACK_KEY_DOWN) goto key_down;
   else if (type != EVAS_CALLBACK_MOUSE_WHEEL) return EINA_FALSE;

   mev = event_info;
   if (mev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if (mev->z < 0) _drag_up(obj, NULL, NULL, NULL);
   else _drag_down(obj, NULL, NULL, NULL);
   mev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;

  key_down:
   ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if ((!strcmp(ev->keyname, "Left"))
       || (!strcmp(ev->keyname, "KP_Left")))
     {
        if (!wd->horizontal) return EINA_FALSE;
        if (!wd->inverted) _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "Right"))
            || (!strcmp(ev->keyname, "KP_Right")))
     {
        if (!wd->horizontal) return EINA_FALSE;
        if (!wd->inverted) _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "Up")) || (!strcmp(ev->keyname, "KP_Up")))
     {
        if (wd->horizontal) return EINA_FALSE;
        if (wd->inverted) _drag_up(obj, NULL, NULL, NULL);
        else _drag_down(obj, NULL, NULL, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "Down")) || (!strcmp(ev->keyname, "KP_Down")))
     {
        if (wd->horizontal) return EINA_FALSE;
        if (wd->inverted) _drag_down(obj, NULL, NULL, NULL);
        else _drag_up(obj, NULL, NULL, NULL);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else return EINA_FALSE;
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->indicator) eina_stringshare_del(wd->units);
   if (wd->delay) ecore_timer_del(wd->delay);
   free(wd);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->slider, "elm,action,focus", "elm");
        evas_object_focus_set(wd->slider, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit(wd->slider, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->slider, EINA_FALSE);
     }
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->slider, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   if (wd->horizontal)
     _elm_theme_object_set(obj, wd->slider, "slider", "horizontal", elm_widget_style_get(obj));
   else
     _elm_theme_object_set(obj, wd->slider, "slider", "vertical", elm_widget_style_get(obj));
   if (wd->icon)
     {
        edje_object_part_swallow(wd->slider, "elm.swallow.content", wd->icon);
        edje_object_signal_emit(wd->slider, "elm,state,icon,visible", "elm");
     }
   if (wd->end)
     edje_object_signal_emit(wd->slider, "elm,state,end,visible", "elm");
   else
     edje_object_signal_emit(wd->slider, "elm,state,end,hidden", "elm");
   if (wd->label)
     {
        edje_object_part_text_set(wd->slider, "elm.text", wd->label);
        edje_object_signal_emit(wd->slider, "elm,state,text,visible", "elm");
     }

   if (wd->units)
     edje_object_signal_emit(wd->slider, "elm,state,units,visible", "elm");

   if (wd->horizontal)
     evas_object_size_hint_min_set(wd->spacer, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale, 1);
   else
     evas_object_size_hint_min_set(wd->spacer, 1, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale);

   if (wd->inverted)
     edje_object_signal_emit(wd->slider, "elm,state,inverted,on", "elm");

   edje_object_part_swallow(wd->slider, "elm.swallow.bar", wd->spacer);
   _units_set(obj);
   _indicator_set(obj);
   edje_object_message_signal_process(wd->slider);
   edje_object_scale_set(wd->slider, elm_widget_scale_get(obj) * _elm_config->scale);
   _val_set(obj);
   _sizing_eval(obj);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->slider, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(wd->slider, "elm,state,enabled", "elm");
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->slider, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if ((obj != wd->icon) && (obj != wd->end)) return;
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   if (sub == wd->icon)
     {
        edje_object_signal_emit(wd->slider, "elm,state,icon,hidden", "elm");
        evas_object_event_callback_del_full
           (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints, obj);
        wd->icon = NULL;
        edje_object_message_signal_process(wd->slider);
        _sizing_eval(obj);
     }
   if (sub == wd->end)
     {
        edje_object_signal_emit(wd->slider, "elm,state,end,hidden", "elm");
        evas_object_event_callback_del_full(sub,
                                            EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
        wd->end = NULL;
        edje_object_message_signal_process(wd->slider);
        _sizing_eval(obj);
     }
}

static Eina_Bool
_delay_change(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return ECORE_CALLBACK_CANCEL;
   wd->delay = NULL;
   evas_object_smart_callback_call(data, SIG_DELAY_CHANGED, NULL);
   return ECORE_CALLBACK_CANCEL;
}

static void
_val_fetch(Evas_Object *obj)
{
   Eina_Bool rtl;
   Widget_Data *wd = elm_widget_data_get(obj);
   double posx = 0.0, posy = 0.0, pos = 0.0, val;
   if (!wd) return;
   edje_object_part_drag_value_get(wd->slider, "elm.dragable.slider",
                                   &posx, &posy);
   if (wd->horizontal) pos = posx;
   else pos = posy;

   rtl = elm_widget_mirrored_get(obj);
   if ((!rtl && wd->inverted) || (rtl &&
                                  ((!wd->horizontal && wd->inverted) ||
                                   (wd->horizontal && !wd->inverted)))) pos = 1.0 - pos;
   val = (pos * (wd->val_max - wd->val_min)) + wd->val_min;
   if (val != wd->val)
     {
        wd->val = val;
        evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
        if (wd->delay) ecore_timer_del(wd->delay);
        wd->delay = ecore_timer_add(0.2, _delay_change, obj);
     }
}

static void
_val_set(Evas_Object *obj)
{
   Eina_Bool rtl;
   Widget_Data *wd = elm_widget_data_get(obj);
   double pos;
   if (!wd) return;
   if (wd->val_max > wd->val_min)
     pos = (wd->val - wd->val_min) / (wd->val_max - wd->val_min);
   else
     pos = 0.0;
   if (pos < 0.0) pos = 0.0;
   else if (pos > 1.0) pos = 1.0;

   rtl = elm_widget_mirrored_get(obj);
   if ((!rtl && wd->inverted) || (rtl &&
                                  ((!wd->horizontal && wd->inverted) ||
                                   (wd->horizontal && !wd->inverted)))) pos = 1.0 - pos;
   edje_object_part_drag_value_set(wd->slider, "elm.dragable.slider", pos, pos);
}

static void
_units_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->units_format_func)
     {
        const char *buf;
        buf = wd->units_format_func(wd->val);
        edje_object_part_text_set(wd->slider, "elm.units", buf);
        if (wd->units_format_free) wd->units_format_free(buf);
     }
   else if (wd->units)
     {
        char buf[1024];

        snprintf(buf, sizeof(buf), wd->units, wd->val);
        edje_object_part_text_set(wd->slider, "elm.units", buf);
     }
   else
     edje_object_part_text_set(wd->slider, "elm.units", NULL);
}

static void
_indicator_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->indicator_format_func)
     {
        const char *buf;
        buf = wd->indicator_format_func(wd->val);
        edje_object_part_text_set(wd->slider, "elm.dragable.slider:elm.indicator", buf);
        if (wd->indicator_format_free) wd->indicator_format_free(buf);
     }
   else if (wd->indicator)
     {
        char buf[1024];
        snprintf(buf, sizeof(buf), wd->indicator, wd->val);
        edje_object_part_text_set(wd->slider, "elm.dragable.slider:elm.indicator", buf);
     }
   else
     edje_object_part_text_set(wd->slider, "elm.dragable.slider:elm.indicator", NULL);
}

static void
_drag(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _val_fetch(data);
   _units_set(data);
   _indicator_set(data);
}

static void
_drag_start(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _val_fetch(data);
   evas_object_smart_callback_call(data, SIG_DRAG_START, NULL);
   _units_set(data);
   _indicator_set(data);
   elm_widget_scroll_freeze_push(data);
}

static void
_drag_stop(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _val_fetch(data);
   evas_object_smart_callback_call(data, SIG_DRAG_STOP, NULL);
   _units_set(data);
   _indicator_set(data);
   elm_widget_scroll_freeze_pop(data);
}

static void
_drag_step(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _val_fetch(data);
   _units_set(data);
   _indicator_set(data);
}

static void
_drag_up(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   double step;
   Widget_Data *wd;

   wd = elm_widget_data_get(data);
   step = 0.05;

   if (wd->inverted) step *= ELM_SLIDER_INVERTED_FACTOR;

   edje_object_part_drag_step(wd->slider, "elm.dragable.slider", step, step);
}

static void
_drag_down(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   double step;
   Widget_Data *wd;

   wd = elm_widget_data_get(data);
   step = -0.05;

   if (wd->inverted) step *= ELM_SLIDER_INVERTED_FACTOR;

   edje_object_part_drag_step(wd->slider, "elm.dragable.slider", step, step);
}

static void
_spacer_cb(void *data, Evas *e, Evas_Object *obj __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w, h;
   double button_x, button_y;

   evas_object_geometry_get(wd->spacer, &x, &y, &w, &h);
   edje_object_part_drag_value_get(wd->slider, "elm.dragable.slider", &button_x, &button_y);
   if (wd->horizontal)
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
   edje_object_part_drag_value_set(wd->slider, "elm.dragable.slider", button_x, button_y);
   evas_event_feed_mouse_cancel(e, 0, NULL);
   evas_event_feed_mouse_down(e, 1, EVAS_BUTTON_NONE, 0, NULL);
}

static void
_elm_slider_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return;
   if (!wd) return;
   eina_stringshare_replace(&wd->label, label);
   if (label)
     {
        edje_object_signal_emit(wd->slider, "elm,state,text,visible", "elm");
        edje_object_message_signal_process(wd->slider);
     }
   else
     {
        edje_object_signal_emit(wd->slider, "elm,state,text,hidden", "elm");
        edje_object_message_signal_process(wd->slider);
     }
   edje_object_part_text_set(wd->slider, "elm.text", label);
   _sizing_eval(obj);
}

static const char *
_elm_slider_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return NULL;
   if (!wd) return NULL;
   return wd->label;
}

/**
 * Add a new slider to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Slider
 */
EAPI Evas_Object *
elm_slider_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "slider");
   elm_widget_type_set(obj, "slider");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);
   elm_widget_text_set_hook_set(obj, _elm_slider_label_set);
   elm_widget_text_get_hook_set(obj, _elm_slider_label_get);

   wd->horizontal = EINA_TRUE;
   wd->indicator_show = EINA_TRUE;
   wd->val = 0.0;
   wd->val_min = 0.0;
   wd->val_max = 1.0;

   wd->slider = edje_object_add(e);
   _elm_theme_object_set(obj, wd->slider, "slider", "horizontal", "default");
   elm_widget_resize_object_set(obj, wd->slider);
   edje_object_signal_callback_add(wd->slider, "drag", "*", _drag, obj);
   edje_object_signal_callback_add(wd->slider, "drag,start", "*", _drag_start, obj);
   edje_object_signal_callback_add(wd->slider, "drag,stop", "*", _drag_stop, obj);
   edje_object_signal_callback_add(wd->slider, "drag,step", "*", _drag_step, obj);
   edje_object_signal_callback_add(wd->slider, "drag,page", "*", _drag_stop, obj);
   //   edje_object_signal_callback_add(wd->slider, "drag,set", "*", _drag_stop, obj);
   edje_object_part_drag_value_set(wd->slider, "elm.dragable.slider", 0.0, 0.0);

   wd->spacer = evas_object_rectangle_add(e);
   evas_object_color_set(wd->spacer, 0, 0, 0, 0);
   evas_object_pass_events_set(wd->spacer, EINA_TRUE);
   elm_widget_sub_object_add(obj, wd->spacer);
   edje_object_part_swallow(wd->slider, "elm.swallow.bar", wd->spacer);
   evas_object_event_callback_add(wd->spacer, EVAS_CALLBACK_MOUSE_DOWN, _spacer_cb, obj);
   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);

   // TODO: convert Elementary to subclassing of Evas_Smart_Class
   // TODO: and save some bytes, making descriptions per-class and not instance!
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

/**
 * Set the label of the slider
 *
 * @param obj The slider object
 * @param label The text label string in UTF-8
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_label_set(Evas_Object *obj, const char *label)
{
   _elm_slider_label_set(obj, NULL, label);
}

/**
 * Get the label of the slider
 *
 * @param obj The slider object
 * @return The text label string in UTF-8
 *
 * @ingroup Slider
 */
EAPI const char *
elm_slider_label_get(const Evas_Object *obj)
{
   return _elm_slider_label_get(obj, NULL);
}

/**
 * Set the icon object (leftmost widget) of the slider object.
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_slider_icon_unset() function.
 *
 * @param obj The slider object
 * @param icon The icon object
 *
 * @note If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->icon == icon) return;
   if (wd->icon) evas_object_del(wd->icon);
   wd->icon = icon;
   if (icon)
     {
        elm_widget_sub_object_add(obj, icon);
        evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_part_swallow(wd->slider, "elm.swallow.icon", icon);
        edje_object_signal_emit(wd->slider, "elm,state,icon,visible", "elm");
        edje_object_message_signal_process(wd->slider);
     }
   _sizing_eval(obj);
}

/**
 * Unset the leftmost widget of the slider, unparenting and
 * returning it.
 *
 * @param obj The slider object
 * @return the previously set icon sub-object of this slider, on
 * success.
 *
 * @see elm_slider_icon_set()
 *
 * @ingroup Slider
 */
EAPI Evas_Object *
elm_slider_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *ret = NULL;
   if (!wd) return NULL;
   if (wd->icon)
     {
        elm_widget_sub_object_del(obj, wd->icon);
        ret = wd->icon;
        edje_object_part_unswallow(wd->slider, wd->icon);
        edje_object_signal_emit(wd->slider, "elm,state,icon,hidden", "elm");
        wd->icon = NULL;
        _sizing_eval(obj);
     }
   return ret;
}

/**
 * Get the icon object of the slider object. This object is owned by
 * the scrolled entry and should not be modified.
 *
 * @param obj The slider object
 * @return The icon object
 *
 * @ingroup Slider
 */
EAPI Evas_Object *
elm_slider_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}

/**
 * Set the length of the dragable region of the slider
 *
 * This sets the minimum width or height (depending on orientation) of the
 * area of the slider that allows the slider to be dragged around. This in
 * turn affects the objects minimum size (along with icon label and unit
 * text). Note that this will also get multiplied by the scale factor.
 *
 * @param obj The slider object
 * @param size The length of the slider area
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_span_size_set(Evas_Object *obj, Evas_Coord size)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->size == size) return;
   wd->size = size;
   if (wd->horizontal)
     evas_object_size_hint_min_set(wd->spacer, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale, 1);
   else
     evas_object_size_hint_min_set(wd->spacer, 1, (double)wd->size * elm_widget_scale_get(obj) * _elm_config->scale);
   if (wd->indicator_show)
     edje_object_signal_emit(wd->slider, "elm,state,val,show", "elm");
   else
     edje_object_signal_emit(wd->slider, "elm,state,val,hide", "elm");
   edje_object_part_swallow(wd->slider, "elm.swallow.bar", wd->spacer);
   _sizing_eval(obj);
}

/**
 * Get the length of the dragable region of the slider
 *
 * This gets the minimum width or height (depending on orientation) of
 * the area of the slider that allows the slider to be dragged
 * around. Note that this will also get multiplied by the scale
 * factor.
 *
 * @param obj The slider object
 * @return The length of the slider area
 *
 * @ingroup Slider
 */
EAPI Evas_Coord
elm_slider_span_size_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->size;
}

/**
 * Set the format string of the unit area
 *
 * If NULL, this disabls the unit area display. If not it sets the format
 * string for the unit text. The unit text is provided a floating point
 * value, so the unit text can display up to 1 floating point value. Note that
 * this is optional. Use a format string such as "%1.2f meters" for example.
 *
 * @param obj The slider object
 * @param units The format string for the units display
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_unit_format_set(Evas_Object *obj, const char *units)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->units, units);
   if (units)
     {
        edje_object_signal_emit(wd->slider, "elm,state,units,visible", "elm");
        edje_object_message_signal_process(wd->slider);
     }
   else
     {
        edje_object_signal_emit(wd->slider, "elm,state,units,hidden", "elm");
        edje_object_message_signal_process(wd->slider);
     }
   _units_set(obj);
   _sizing_eval(obj);
}

/**
 * Get the format string for the unit area
 *
 * The slider may also display a value (the value of the slider) somewhere
 * (for example above the slider knob that is dragged around). This sets the
 * format string for this. See elm_slider_unit_format_set() for more
 * information on how this works.
 *
 * @param obj The slider object
 * @return The format string for the unit display.
 *
 * @ingroup Slider
 */
EAPI const char *
elm_slider_unit_format_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->units;
}

/**
 * Set the format string for the indicator area
 *
 * The slider may also display a value (the value of the slider) somewhere
 * (for example above the slider knob that is dragged around). This sets the
 * format string for this. See elm_slider_unit_format_set() for more
 * information on how this works.
 *
 * @param obj The slider object
 * @param indicator The format string for the indicator display
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_indicator_format_set(Evas_Object *obj, const char *indicator)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->indicator, indicator);
   _indicator_set(obj);
}

/**
 * Get the format string for the indicator area
 *
 * The slider may also display a value (the value of the slider) somewhere
 * (for example above the slider knob that is dragged around). This sets the
 * format string for this. See elm_slider_indicator_format_set() for more
 * information on how this works.
 *
 * @param obj The slider object
 * @return The format string for the indicator display.
 *
 * @ingroup Slider
 */
EAPI const char *
elm_slider_indicator_format_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->indicator;
}

/**
 * Set orientation of the slider
 *
 * @param obj The slider object
 * @param horizontal If set, the slider will be horizontal
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   horizontal = !!horizontal;
   if (wd->horizontal == horizontal) return;
   wd->horizontal = horizontal;
   _theme_hook(obj);
}

/**
 * Get orientation of the slider
 *
 * @param obj The slider object
 * @return If @c EINA_TRUE the slider will be horizontal, else it is
 *         vertical.
 * @ingroup Slider
 */
EAPI Eina_Bool
elm_slider_horizontal_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->horizontal;
}

/**
 * Set the minimum and maximum values for the slider
 *
 * Maximum mut be greater than minimum.
 *
 * @param obj The slider object
 * @param min The minimum value
 * @param max The maximum value
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_min_max_set(Evas_Object *obj, double min, double max)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((wd->val_min == min) && (wd->val_max == max)) return;
   wd->val_min = min;
   wd->val_max = max;
   if (wd->val < wd->val_min) wd->val = wd->val_min;
   if (wd->val > wd->val_max) wd->val = wd->val_max;
   _val_set(obj);
   _units_set(obj);
   _indicator_set(obj);
}

/**
 * Get the minimum and maximum values for the slider
 *
 * @param obj The slider object
 * @param min The pointer to store minimum value, may be @c NULL.
 * @param max The pointer to store maximum value, may be @c NULL.
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_min_max_get(const Evas_Object *obj, double *min, double *max)
{
   if (min) *min = 0.0;
   if (max) *max = 0.0;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (min) *min = wd->val_min;
   if (max) *max = wd->val_max;
}

/**
 * Set the value the slider indicates
 *
 * @param obj The slider object
 * @param val The value (must be between min and max for the slider)
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_value_set(Evas_Object *obj, double val)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->val == val) return;
   wd->val = val;
   if (wd->val < wd->val_min) wd->val = wd->val_min;
   if (wd->val > wd->val_max) wd->val = wd->val_max;
   _val_set(obj);
   _units_set(obj);
   _indicator_set(obj);
}

/**
 * Get the value the slider has
 *
 * @param obj The slider object
 * @return The value of the slider
 *
 * @ingroup Slider
 */
EAPI double
elm_slider_value_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0.0;
   return wd->val;
}

/**
 * Invert the slider display
 *
 * Normally the slider will display and interpret values from low to high
 * and when horizontal that is left to right. When vertical that is top
 * to bottom. This inverts this (so from right to left or bottom to top) if
 * inverted is set to 1.
 *
 * @param obj The slider object
 * @param inverted The inverted flag. 1 == inverted, 0 == normal
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_inverted_set(Evas_Object *obj, Eina_Bool inverted)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   inverted = !!inverted;
   if (wd->inverted == inverted) return;
   wd->inverted = inverted;
   if (wd->inverted)
     edje_object_signal_emit(wd->slider, "elm,state,inverted,on", "elm");
   else
     edje_object_signal_emit(wd->slider, "elm,state,inverted,off", "elm");
   edje_object_message_signal_process(wd->slider);
   _val_set(obj);
   _units_set(obj);
   _indicator_set(obj);
}

/**
 * Get if the slider display is inverted (backwards)
 *
 * @param obj The slider object
 * @return If @c EINA_TRUE the slider will be inverted.
 * @ingroup Slider
 */
EAPI Eina_Bool
elm_slider_inverted_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->inverted;
}

/**
 * Set the format function pointer for the indicator area
 *
 * Set the callback function to format the indicator string.
 * See elm_slider_indicator_format_set() for more info on how this works.
 *
 * @param obj The slider object
 * @param indicator The format string for the indicator display
 * @param func The indicator format function
 * @param free_func The freeing function for the format string
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_indicator_format_function_set(Evas_Object *obj, const char *(*func)(double val), void (*free_func)(const char *str))
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->indicator_format_func = func;
   wd->indicator_format_free = free_func;
   _indicator_set(obj);
}

/**
 * Set the format function pointer for the units area
 *
 * Set the callback function to format the indicator string.
 * See elm_slider_units_format_set() for more info on how this works.
 *
 * @param obj The slider object
 * @param indicator The format string for the units display
 * @param func The units format function
 * @param free_func The freeing function for the format string
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_units_format_function_set(Evas_Object *obj, const char *(*func)(double val), void (*free_func)(const char *str))
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->units_format_func = func;
   wd->units_format_free = free_func;
   _indicator_set(obj);
}

/**
 * Set the end object (rightmost widget) of the slider object.
 *
 * Once the end object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_button_end_unset() function.
 *
 * @param obj The slider object
 * @param end The end object
 *
 * @note If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_end_set(Evas_Object *obj, Evas_Object *end)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->end == end) return;
   if (wd->end) evas_object_del(wd->end);
   wd->end = end;
   if (end)
     {
        elm_widget_sub_object_add(obj, end);
        evas_object_event_callback_add(end, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_part_swallow(wd->slider, "elm.swallow.end", end);
        edje_object_signal_emit(wd->slider, "elm,state,end,visible", "elm");
        edje_object_message_signal_process(wd->slider);
     }
   _sizing_eval(obj);
}

/**
 * Unset the rightmost widget of the slider, unparenting and
 * returning it.
 *
 * @param obj The slider object
 * @return the previously set end sub-object of this slider, on
 * success.
 *
 * @see elm_slider_end_set()
 *
 * @ingroup Slider
 */
EAPI Evas_Object *
elm_slider_end_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *ret = NULL;
   if (!wd) return NULL;
   if (wd->end)
     {
        elm_widget_sub_object_del(obj, wd->end);
        ret = wd->end;
        edje_object_part_unswallow(wd->slider, wd->end);
        edje_object_signal_emit(wd->slider, "elm,state,end,hidden", "elm");
        wd->end = NULL;
        _sizing_eval(obj);
     }
   return ret;
}

/**
 * Get the end icon object of the slider object. This object is owned
 * by the scrolled entry and should not be modified.
 *
 * @param obj The slider object
 * @return The end icon object
 *
 * @ingroup Slider
 */
EAPI Evas_Object *
elm_slider_end_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->end;
}

/**
 * Set whether to the slider indicator (augmented knob) at all.
 *
 * @param obj The slider object
 * @param show @c EINA_TRUE will make it show it, @c EINA_FALSE will
 * let the knob alwayes at default size.
 *
 * @note It will conflict with elm_slider_indicator_format_set(), if
 * you wanted those effects.
 *
 * @ingroup Slider
 */
EAPI void
elm_slider_indicator_show_set(Evas_Object *obj, Eina_Bool show)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (show) {
        wd->indicator_show = EINA_TRUE;
        edje_object_signal_emit(wd->slider, "elm,state,val,show", "elm");
   }
   else {
        wd->indicator_show = EINA_FALSE;
        edje_object_signal_emit(wd->slider, "elm,state,val,hide", "elm");
   }
}

/**
 * Get the state of indicator in the slider (if it's being shown or
 * not).
 *
 * @param obj The slider object
 * @return @c EINA_TRUE if the indicator is being shown, @c EINA_FALSE
 * otherwise.
 *
 *  @ingroup Slider
 */
EAPI Eina_Bool
elm_slider_indicator_show_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->indicator_show;
}

