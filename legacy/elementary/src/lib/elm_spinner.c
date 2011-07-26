#include <Elementary.h>
#include "elm_priv.h"
#include <ctype.h>

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Spinner_Special_Value Elm_Spinner_Special_Value;

struct _Widget_Data
{
   Evas_Object *spinner, *ent;
   const char *label;
   double val, val_min, val_max, orig_val, step;
   double drag_start_pos, spin_speed, interval, first_interval;
   Ecore_Timer *delay, *spin;
   Eina_List *special_values;
   Eina_Bool wrap : 1;
   Eina_Bool entry_visible : 1;
   Eina_Bool dragging : 1;
   Eina_Bool editable : 1;
};

struct _Elm_Spinner_Special_Value
{
   double value;
   const char *label;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _write_label(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
//static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool _value_set(Evas_Object *obj, double delta);
static void _on_focus_hook(void *data, Evas_Object *obj);
static Eina_Bool _event_hook(Evas_Object *obj, Evas_Object *src,
                             Evas_Callback_Type type, void *event_info);

static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);

static const char SIG_CHANGED[] = "changed";
static const char SIG_DELAY_CHANGED[] = "delay,changed";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CHANGED, ""},
   {SIG_DELAY_CHANGED, ""},
   {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Elm_Spinner_Special_Value *sv;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->delay) ecore_timer_del(wd->delay);
   if (wd->spin) ecore_timer_del(wd->spin);
   if (wd->special_values)
     {
        EINA_LIST_FREE(wd->special_values, sv)
          {
             eina_stringshare_del(sv->label);
             free(sv);
          }
     }
   free(wd);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_disabled_get(obj))
      edje_object_signal_emit(wd->spinner, "elm,state,disabled", "elm");
   else
      edje_object_signal_emit(wd->spinner, "elm,state,enabled", "elm");
}

static void
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_emit(wd->spinner, emission, source);
}

static void
_signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_callback_add(wd->spinner, emission,
                                   source, func_cb, data);
}

static void
_signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_signal_callback_del_full(wd->spinner, emission, source,
                                        func_cb, data);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->spinner, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->spinner, "spinner", "base", elm_widget_style_get(obj));
   edje_object_part_swallow(wd->spinner, "elm.swallow.entry", wd->ent);
   _write_label(obj);
   if (elm_widget_focus_get(obj))
      edje_object_signal_emit(wd->spinner, "elm,action,focus", "elm");
   else
      edje_object_signal_emit(wd->spinner, "elm,action,unfocus", "elm");
   if (elm_widget_disabled_get(obj))
      edje_object_signal_emit(wd->spinner, "elm,state,disabled", "elm");
   edje_object_message_signal_process(wd->spinner);
   edje_object_scale_set(wd->spinner, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->spinner, "elm,action,focus", "elm");
        evas_object_focus_set(wd->spinner, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit(wd->spinner, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->spinner, EINA_FALSE);
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
_entry_show(Widget_Data *wd)
{
   char buf[32], fmt[32] = "%0.f";

   /* try to construct just the format from given label
    * completely ignoring pre/post words
    */
   if (wd->label)
     {
        const char *start = strchr(wd->label, '%');
        while (start)
          {
             /* handle %% */
             if (start[1] != '%')
               break;
             else
               start = strchr(start + 2, '%');
          }

        if (start)
          {
             const char *itr, *end = NULL;
             for (itr = start + 1; *itr != '\0'; itr++)
               {
                  /* allowing '%d' is quite dangerous, remove it? */
                  if ((*itr == 'd') || (*itr == 'f'))
                    {
                       end = itr + 1;
                       break;
                    }
               }

             if ((end) && ((size_t)(end - start + 1) < sizeof(fmt)))
               {
                  memcpy(fmt, start, end - start);
                  fmt[end - start] = '\0';
               }
          }
     }
   snprintf(buf, sizeof(buf), fmt, wd->val);
   elm_entry_entry_set(wd->ent, buf);
}

static void
_write_label(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Spinner_Special_Value *sv;
   Widget_Data *wd = elm_widget_data_get(obj);
   char buf[1024];
   if (!wd) return;
   EINA_LIST_FOREACH(wd->special_values, l, sv)
     {
        if (sv->value == wd->val)
          {
             snprintf(buf, sizeof(buf), "%s", sv->label);
             goto apply;
          }
     }
   if (wd->label)
     snprintf(buf, sizeof(buf), wd->label, wd->val);
   else
     snprintf(buf, sizeof(buf), "%.0f", wd->val);

apply:
   edje_object_part_text_set(wd->spinner, "elm.text", buf);
   if (wd->entry_visible) _entry_show(wd);
}

static Eina_Bool
_value_set(Evas_Object *obj, double delta)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   double new_val;
   if (!wd) return EINA_FALSE;
   new_val = wd->val + delta;
   if (wd->wrap)
     {
        while (new_val < wd->val_min)
          new_val = wd->val_max + new_val + 1 - wd->val_min;
        while (new_val > wd->val_max)
          new_val = wd->val_min + new_val - wd->val_max - 1;
     }
   else
     {
        if (new_val < wd->val_min)
          new_val = wd->val_min;
        else if (new_val > wd->val_max)
          new_val = wd->val_max;
     }

   if (new_val == wd->val) return EINA_FALSE;
   wd->val = new_val;

   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
   if (wd->delay) ecore_timer_del(wd->delay);
   wd->delay = ecore_timer_add(0.2, _delay_change, obj);

   return EINA_TRUE;
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->spinner, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

/*
   static void
   _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
   {
   _sizing_eval(data);
   }
 */

static void
_val_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   double pos = 0.0;
   if (!wd) return;
   if (wd->val_max > wd->val_min)
     pos = ((wd->val - wd->val_min) / (wd->val_max - wd->val_min));
   if (pos < 0.0) pos = 0.0;
   else if (pos > 1.0) pos = 1.0;
   edje_object_part_drag_value_set(wd->spinner, "elm.dragable.slider",
                                   pos, pos);
}

static void
_drag(void *data, Evas_Object *_obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *obj = data;
   Widget_Data *wd = elm_widget_data_get(obj);
   double pos = 0.0, offset, delta;
   if (!wd) return;
   if (wd->entry_visible) return;
   edje_object_part_drag_value_get(wd->spinner, "elm.dragable.slider",
                                   &pos, NULL);
   offset = wd->step;
   delta = (pos - wd->drag_start_pos) * offset;
   /* If we are on rtl mode, change the delta to be negative on such changes */
   if (elm_widget_mirrored_get(obj))
     delta *= -1;
   if (_value_set(data, delta)) _write_label(data);
   wd->drag_start_pos = pos;
   wd->dragging = 1;
}

static void
_drag_start(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   double pos;
   if (!wd) return;
   edje_object_part_drag_value_get(wd->spinner, "elm.dragable.slider",
                                   &pos, NULL);
   wd->drag_start_pos = pos;
}

static void
_drag_stop(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->drag_start_pos = 0;
   edje_object_part_drag_value_set(wd->spinner, "elm.dragable.slider", 0.0, 0.0);
}

static void
_hide_entry(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_emit(wd->spinner, "elm,state,inactive", "elm");
   wd->entry_visible = 0;
}

static void
_reset_value(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _hide_entry(obj);
   elm_spinner_value_set(obj, wd->orig_val);
}

static void
_apply_entry_value(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *str;
   char *end;
   double val;

   if (!wd) return;
   _hide_entry(obj);
   str = elm_entry_entry_get(wd->ent);
   if (!str) return;
   val = strtod(str, &end);
   if ((*end != '\0') && (!isspace(*end))) return;
   elm_spinner_value_set(obj, val);
}

static void
_toggle_entry(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd->dragging)
     {
        wd->dragging = 0;
        return;
     }
   if (elm_widget_disabled_get(data)) return;
   if (!wd->editable) return;
   if (wd->entry_visible) _apply_entry_value(data);
   else
     {
        wd->orig_val = wd->val;
        edje_object_signal_emit(wd->spinner, "elm,state,active", "elm");
        _entry_show(wd);
        elm_entry_select_all(wd->ent);
        elm_widget_focus_set(wd->ent, 1);
        wd->entry_visible = 1;
     }
}

static Eina_Bool
_spin_value(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return ECORE_CALLBACK_CANCEL;
   if (_value_set(data, wd->spin_speed)) _write_label(data);
   wd->interval = wd->interval / 1.05;
   ecore_timer_interval_set(wd->spin, wd->interval);
   return ECORE_CALLBACK_RENEW;
}

static void
_val_inc_start(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->interval = wd->first_interval;
   wd->spin_speed = wd->step;
   if (wd->spin) ecore_timer_del(wd->spin);
   wd->spin = ecore_timer_add(wd->interval, _spin_value, obj);
   _spin_value(obj);
}

static void
_val_inc_stop(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->interval = wd->first_interval;
   wd->spin_speed = 0;
   if (wd->spin) ecore_timer_del(wd->spin);
   wd->spin = NULL;
}

static void
_val_dec_start(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->interval = wd->first_interval;
   wd->spin_speed = -wd->step;
   if (wd->spin) ecore_timer_del(wd->spin);
   wd->spin = ecore_timer_add(wd->interval, _spin_value, obj);
   _spin_value(obj);
}

static void
_val_dec_stop(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->interval = wd->first_interval;
   wd->spin_speed = 0;
   if (wd->spin) ecore_timer_del(wd->spin);
   wd->spin = NULL;
}

static void
_button_inc_start(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd->entry_visible)
     {
        _reset_value(data);
        return;
     }
   _val_inc_start(data);
}

static void
_button_inc_stop(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   _val_inc_stop(data);
}

static void
_button_dec_start(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   if (wd->entry_visible)
     {
        _reset_value(data);
        return;
     }
   _val_dec_start(data);
}

static void
_button_dec_stop(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   _val_dec_stop(data);
}

static void
_entry_activated(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   _apply_entry_value(data);
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
   if (wd->delay) ecore_timer_del(wd->delay);
   wd->delay = ecore_timer_add(0.2, _delay_change, data);
}

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (type == EVAS_CALLBACK_KEY_DOWN)
     {
        Evas_Event_Key_Down *ev = event_info;
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
        else if (!strcmp(ev->keyname, "Left") || !strcmp(ev->keyname, "KP_Left")
                 || !strcmp(ev->keyname, "Down") || !strcmp(ev->keyname, "KP_Down"))
          {
             _val_dec_start(obj);
             edje_object_signal_emit(wd->spinner, "elm,left,anim,activate", "elm");
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else if (!strcmp(ev->keyname, "Right") || !strcmp(ev->keyname, "KP_Right")
                 || !strcmp(ev->keyname, "Up") || !strcmp(ev->keyname, "KP_Up"))
          {
             _val_inc_start(obj);
             edje_object_signal_emit(wd->spinner, "elm,right,anim,activate", "elm");
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
     }
   else if (type == EVAS_CALLBACK_KEY_UP)
     {
        Evas_Event_Key_Down *ev = event_info;
        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
        if (!strcmp(ev->keyname, "Right") || !strcmp(ev->keyname, "KP_Right")
            || !strcmp(ev->keyname, "Up") || !strcmp(ev->keyname, "KP_Up"))
          _val_inc_stop(obj);
        else if (!strcmp(ev->keyname, "Left") || !strcmp(ev->keyname, "KP_Left")
                 || !strcmp(ev->keyname, "Down") || !strcmp(ev->keyname, "KP_Down"))
          _val_dec_stop(obj);
        else  return EINA_FALSE;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

EAPI Evas_Object *
elm_spinner_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "spinner");
   elm_widget_type_set(obj, "spinner");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);

   wd->val = 0.0;
   wd->val_min = 0.0;
   wd->val_max = 100.0;
   wd->wrap = 0;
   wd->step = 1.0;
   wd->first_interval = 0.85;
   wd->entry_visible = 0;
   wd->editable = EINA_TRUE;

   wd->spinner = edje_object_add(e);
   _elm_theme_object_set(obj, wd->spinner, "spinner", "base", "default");
   elm_widget_resize_object_set(obj, wd->spinner);
   edje_object_signal_callback_add(wd->spinner, "drag", "*", _drag, obj);
   edje_object_signal_callback_add(wd->spinner, "drag,start", "*",
                                   _drag_start, obj);
   edje_object_signal_callback_add(wd->spinner, "drag,stop", "*",
                                   _drag_stop, obj);
   edje_object_signal_callback_add(wd->spinner, "drag,step", "*",
                                   _drag_stop, obj);
   edje_object_signal_callback_add(wd->spinner, "drag,page", "*",
                                   _drag_stop, obj);

   edje_object_signal_callback_add(wd->spinner, "elm,action,increment,start",
                                   "*", _button_inc_start, obj);
   edje_object_signal_callback_add(wd->spinner, "elm,action,increment,stop",
                                   "*", _button_inc_stop, obj);
   edje_object_signal_callback_add(wd->spinner, "elm,action,decrement,start",
                                   "*", _button_dec_start, obj);
   edje_object_signal_callback_add(wd->spinner, "elm,action,decrement,stop",
                                   "*", _button_dec_stop, obj);
   edje_object_part_drag_value_set(wd->spinner, "elm.dragable.slider",
                                   0.0, 0.0);

   wd->ent = elm_entry_add(obj);
   elm_entry_single_line_set(wd->ent, 1);
   evas_object_smart_callback_add(wd->ent, "activated", _entry_activated, obj);
   edje_object_part_swallow(wd->spinner, "elm.swallow.entry", wd->ent);
   edje_object_signal_callback_add(wd->spinner, "elm,action,entry,toggle",
                                   "*", _toggle_entry, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _write_label(obj);
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_spinner_label_format_set(Evas_Object *obj, const char *fmt)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   eina_stringshare_replace(&wd->label, fmt);
   _write_label(obj);
   _sizing_eval(obj);
}

EAPI const char *
elm_spinner_label_format_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->label;
}

EAPI void
elm_spinner_min_max_set(Evas_Object *obj, double min, double max)
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
   _write_label(obj);
}

EAPI void
elm_spinner_min_max_get(const Evas_Object *obj, double *min, double *max)
{
   if (min) *min = 0.0;
   if (max) *max = 0.0;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (min) *min = wd->val_min;
   if (max) *max = wd->val_max;
}

EAPI void
elm_spinner_step_set(Evas_Object *obj, double step)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->step = step;
}

EAPI double
elm_spinner_step_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0.0;
   return wd->step;
}

EAPI void
elm_spinner_value_set(Evas_Object *obj, double val)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->val == val) return;
   wd->val = val;
   if (wd->val < wd->val_min) wd->val = wd->val_min;
   if (wd->val > wd->val_max) wd->val = wd->val_max;
   _val_set(obj);
   _write_label(obj);
}

EAPI double
elm_spinner_value_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0.0;
   return wd->val;
}

EAPI void
elm_spinner_wrap_set(Evas_Object *obj, Eina_Bool wrap)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->wrap = wrap;
}

EAPI Eina_Bool
elm_spinner_wrap_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->wrap;
}

EAPI void
elm_spinner_special_value_add(Evas_Object *obj, double value, const char *label)
{
   Elm_Spinner_Special_Value *sv;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   sv = calloc(1, sizeof(*sv));
   if (!sv) return;
   sv->value = value;
   sv->label = eina_stringshare_add(label);

   wd->special_values = eina_list_append(wd->special_values, sv);
   _write_label(obj);
}

EAPI void
elm_spinner_editable_set(Evas_Object *obj, Eina_Bool editable)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->editable = editable;
}

EAPI Eina_Bool
elm_spinner_editable_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->editable;
}

EAPI void
elm_spinner_interval_set(Evas_Object *obj, double interval)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->first_interval = interval;
}

EAPI double
elm_spinner_interval_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0.0;
   return wd->first_interval;
}
