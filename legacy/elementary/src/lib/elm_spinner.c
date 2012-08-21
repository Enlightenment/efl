#include <Elementary.h>
//#include <ctype.h>
#include "elm_priv.h"
#include "elm_widget_spinner.h"

EAPI const char ELM_SPINNER_SMART_NAME[] = "elm_spinner";

static const char SIG_CHANGED[] = "changed";
static const char SIG_DELAY_CHANGED[] = "delay,changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_DELAY_CHANGED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_SPINNER_SMART_NAME, _elm_spinner, Elm_Spinner_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static void
_entry_show(Elm_Spinner_Smart_Data *sd)
{
   char buf[32], fmt[32] = "%0.f";

   /* try to construct just the format from given label
    * completely ignoring pre/post words
    */
   if (sd->label)
     {
        const char *start = strchr(sd->label, '%');
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
   snprintf(buf, sizeof(buf), fmt, sd->val);
   elm_object_text_set(sd->ent, buf);
}

static void
_label_write(Evas_Object *obj)
{
   Eina_List *l;
   char buf[1024];
   Elm_Spinner_Special_Value *sv;

   ELM_SPINNER_DATA_GET(obj, sd);

   EINA_LIST_FOREACH (sd->special_values, l, sv)
     {
        if (sv->value == sd->val)
          {
             snprintf(buf, sizeof(buf), "%s", sv->label);
             goto apply;
          }
     }
   if (sd->label)
     snprintf(buf, sizeof(buf), sd->label, sd->val);
   else
     snprintf(buf, sizeof(buf), "%.0f", sd->val);

apply:
   elm_layout_text_set(obj, "elm.text", buf);
   if (sd->entry_visible) _entry_show(sd);
}

static Eina_Bool
_delay_change(void *data)
{
   ELM_SPINNER_DATA_GET(data, sd);

   sd->delay = NULL;
   evas_object_smart_callback_call(data, SIG_DELAY_CHANGED, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_value_set(Evas_Object *obj,
           double new_val)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   if (sd->round > 0)
     new_val = sd->val_base +
       (double)((((int)(new_val - sd->val_base)) / sd->round) * sd->round);

   if (sd->wrap)
     {
        while (new_val < sd->val_min)
          new_val = sd->val_max + new_val + 1 - sd->val_min;
        while (new_val > sd->val_max)
          new_val = sd->val_min + new_val - sd->val_max - 1;
     }
   else
     {
        if (new_val < sd->val_min)
          new_val = sd->val_min;
        else if (new_val > sd->val_max)
          new_val = sd->val_max;
     }

   if (new_val == sd->val) return EINA_FALSE;
   sd->val = new_val;

   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
   if (sd->delay) ecore_timer_del(sd->delay);
   sd->delay = ecore_timer_add(0.2, _delay_change, obj);

   return EINA_TRUE;
}

static void
_val_set(Evas_Object *obj)
{
   double pos = 0.0;

   ELM_SPINNER_DATA_GET(obj, sd);

   if (sd->val_max > sd->val_min)
     pos = ((sd->val - sd->val_min) / (sd->val_max - sd->val_min));
   if (pos < 0.0) pos = 0.0;
   else if (pos > 1.0)
     pos = 1.0;
   edje_object_part_drag_value_set
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.slider", pos, pos);
}

static void
_drag_cb(void *data,
         Evas_Object *_obj __UNUSED__,
         const char *emission __UNUSED__,
         const char *source __UNUSED__)
{
   double pos = 0.0, offset, delta;
   Evas_Object *obj = data;

   ELM_SPINNER_DATA_GET(obj, sd);

   if (sd->entry_visible) return;
   edje_object_part_drag_value_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.slider", &pos, NULL);

   offset = sd->step * _elm_config->scale;
   delta = (pos - sd->drag_start_pos) * offset;
   /* If we are on rtl mode, change the delta to be negative on such changes */
   if (elm_widget_mirrored_get(obj)) delta *= -1;
   if (_value_set(data, sd->drag_start_pos + delta)) _label_write(data);
   sd->dragging = 1;
}

static void
_drag_start_cb(void *data,
               Evas_Object *obj __UNUSED__,
               const char *emission __UNUSED__,
               const char *source __UNUSED__)
{
   double pos;

   ELM_SPINNER_DATA_GET(data, sd);

   edje_object_part_drag_value_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.slider", &pos, NULL);
   sd->drag_start_pos = pos;
}

static void
_drag_stop_cb(void *data,
              Evas_Object *obj __UNUSED__,
              const char *emission __UNUSED__,
              const char *source __UNUSED__)
{
   ELM_SPINNER_DATA_GET(data, sd);

   sd->drag_start_pos = 0;
   edje_object_part_drag_value_set
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.slider", 0.0, 0.0);
}

static void
_entry_hide(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   elm_layout_signal_emit(obj, "elm,state,inactive", "elm");
   sd->entry_visible = EINA_FALSE;
}

static void
_reset_value(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   _entry_hide(obj);
   elm_spinner_value_set(obj, sd->orig_val);
}

static void
_entry_value_apply(Evas_Object *obj)
{
   const char *str;
   double val;
   char *end;

   ELM_SPINNER_DATA_GET(obj, sd);

   if (!sd->entry_visible) return;

   _entry_hide(obj);
   str = elm_object_text_get(sd->ent);
   if (!str) return;
   val = strtod(str, &end);
   if ((*end != '\0') && (!isspace(*end))) return;
   elm_spinner_value_set(obj, val);
}

static void
_entry_toggle_cb(void *data,
                 Evas_Object *obj __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   ELM_SPINNER_DATA_GET(data, sd);

   if (sd->dragging)
     {
        sd->dragging = 0;
        return;
     }
   if (elm_widget_disabled_get(data)) return;
   if (!sd->editable) return;
   if (sd->entry_visible) _entry_value_apply(data);
   else
     {
        sd->orig_val = sd->val;
        elm_layout_signal_emit(data, "elm,state,active", "elm");
        _entry_show(sd);
        elm_entry_select_all(sd->ent);
        elm_widget_focus_set(sd->ent, 1);
        sd->entry_visible = EINA_TRUE;
     }
}

static Eina_Bool
_spin_value(void *data)
{
   ELM_SPINNER_DATA_GET(data, sd);
   double real_speed = sd->spin_speed;

   /* Sanity check: our step size should be at least as large as our rounding value */
   if ((sd->spin_speed != 0.0) && (abs(sd->spin_speed) < sd->round))
     {
        WRN("The spinning step is smaller than the rounding value, please check your code");
        real_speed = sd->spin_speed > 0 ? sd->round : -sd->round;
     }

   if (_value_set(data, sd->val + real_speed)) _label_write(data);
   sd->interval = sd->interval / 1.05;
   ecore_timer_interval_set(sd->spin, sd->interval);

   return ECORE_CALLBACK_RENEW;
}

static void
_val_inc_start(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = sd->step;
   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = ecore_timer_add(sd->interval, _spin_value, obj);
   _spin_value(obj);
}

static void
_val_inc_stop(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = 0;
   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = NULL;
}

static void
_val_dec_start(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = -sd->step;
   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = ecore_timer_add(sd->interval, _spin_value, obj);
   _spin_value(obj);
}

static void
_val_dec_stop(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = 0;
   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = NULL;
}

static void
_button_inc_start_cb(void *data,
                     Evas_Object *obj __UNUSED__,
                     const char *emission __UNUSED__,
                     const char *source __UNUSED__)
{
   ELM_SPINNER_DATA_GET(data, sd);

   if (sd->entry_visible)
     {
        _reset_value(data);
        return;
     }
   _val_inc_start(data);
}

static void
_button_inc_stop_cb(void *data,
                    Evas_Object *obj __UNUSED__,
                    const char *emission __UNUSED__,
                    const char *source __UNUSED__)
{
   _val_inc_stop(data);
}

static void
_button_dec_start_cb(void *data,
                     Evas_Object *obj __UNUSED__,
                     const char *emission __UNUSED__,
                     const char *source __UNUSED__)
{
   ELM_SPINNER_DATA_GET(data, sd);

   if (sd->entry_visible)
     {
        _reset_value(data);
        return;
     }
   _val_dec_start(data);
}

static void
_button_dec_stop_cb(void *data,
                    Evas_Object *obj __UNUSED__,
                    const char *emission __UNUSED__,
                    const char *source __UNUSED__)
{
   _val_dec_stop(data);
}

static void
_entry_activated_cb(void *data,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   ELM_SPINNER_DATA_GET(data, sd);

   _entry_value_apply(data);
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
   if (sd->delay) ecore_timer_del(sd->delay);
   sd->delay = ecore_timer_add(0.2, _delay_change, data);
}

static void
_elm_spinner_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;

   ELM_SPINNER_DATA_GET(obj, sd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static Eina_Bool
_elm_spinner_smart_event(Evas_Object *obj,
                         Evas_Object *src __UNUSED__,
                         Evas_Callback_Type type,
                         void *event_info)
{
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (type == EVAS_CALLBACK_KEY_DOWN)
     {
        Evas_Event_Key_Down *ev = event_info;

        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
        else if (!strcmp(ev->keyname, "Left") ||
                 ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)) ||
                 !strcmp(ev->keyname, "Down") ||
                 ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
          {
             _val_dec_start(obj);
             elm_layout_signal_emit(obj, "elm,left,anim,activate", "elm");
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else if (!strcmp(ev->keyname, "Right") ||
                 ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)) ||
                 !strcmp(ev->keyname, "Up") ||
                 ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
          {
             _val_inc_start(obj);
             elm_layout_signal_emit(obj, "elm,right,anim,activate", "elm");
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
     }
   else if (type == EVAS_CALLBACK_KEY_UP)
     {
        Evas_Event_Key_Down *ev = event_info;

        if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
        if (!strcmp(ev->keyname, "Right") ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)) ||
            !strcmp(ev->keyname, "Up") ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
          _val_inc_stop(obj);
        else if (!strcmp(ev->keyname, "Left") ||
                 ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)) ||
                 !strcmp(ev->keyname, "Down") ||
                 ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
          _val_dec_stop(obj);
        else return EINA_FALSE;

        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool
_elm_spinner_smart_on_focus(Evas_Object *obj)
{
   if (!ELM_WIDGET_CLASS(_elm_spinner_parent_sc)->on_focus(obj))
     return EINA_FALSE;

   if (!elm_widget_focus_get(obj))
     _entry_value_apply(obj);

   return EINA_TRUE;
}

static char *
_access_info_cb(void *data,
                Evas_Object *obj,
                Elm_Widget_Item *item __UNUSED__)
{
   Evas_Object *spinner;
   const char *txt = elm_widget_access_info_get(obj);

   spinner = ELM_WIDGET_DATA(data)->obj;
   if (!txt) txt = elm_layout_text_get(spinner, "elm.text");
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data,
                 Evas_Object *obj __UNUSED__,
                 Elm_Widget_Item *item __UNUSED__)
{
   if (elm_widget_disabled_get(ELM_WIDGET_DATA(data)->obj))
     return strdup(E_("State: Disabled"));

   return NULL;
}

static void
_access_spinner_register(Evas_Object *obj)
{
   Elm_Access_Info *ai;
   const char* increment_part;
   const char* decrement_part;

   ELM_SPINNER_DATA_GET(obj, sd);

   if (!strcmp(elm_widget_style_get(obj), "vertical"))
     {
        increment_part = "up_bt";
        decrement_part = "down_bt";
     }
   else
     {
        increment_part = "right_bt";
        decrement_part = "left_bt";
     }

   // increment button
   sd->increment_btn_access =
     _elm_access_edje_object_part_object_register
       (obj, elm_layout_edje_get(obj), increment_part);
  
   ai = _elm_access_object_get(sd->increment_btn_access);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("spinner increment button"));

   // decrement button
   sd->decrement_btn_access =
     _elm_access_edje_object_part_object_register
       (obj, elm_layout_edje_get(obj), decrement_part);

   ai = _elm_access_object_get(sd->decrement_btn_access);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("spinner decrement button"));

   // spinner label 
   sd->access_obj = _elm_access_edje_object_part_object_register
                       (obj, elm_layout_edje_get(obj), "access_text");

   ai = _elm_access_object_get(sd->access_obj);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("spinner"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, sd);
   _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, sd);
}

static void
_elm_spinner_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Spinner_Smart_Data);

   ELM_WIDGET_CLASS(_elm_spinner_parent_sc)->base.add(obj);

   priv->val = 0.0;
   priv->val_min = 0.0;
   priv->val_max = 100.0;
   priv->wrap = 0;
   priv->step = 1.0;
   priv->first_interval = 0.85;
   priv->entry_visible = EINA_FALSE;
   priv->editable = EINA_TRUE;

   elm_layout_theme_set(obj, "spinner", "base", elm_widget_style_get(obj));
   elm_layout_signal_callback_add(obj, "drag", "*", _drag_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,start", "*", _drag_start_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,stop", "*", _drag_stop_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,step", "*", _drag_stop_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,page", "*", _drag_stop_cb, obj);

   elm_layout_signal_callback_add
     (obj, "elm,action,increment,start", "*", _button_inc_start_cb, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,increment,stop", "*", _button_inc_stop_cb, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,decrement,start", "*", _button_dec_start_cb, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,decrement,stop", "*", _button_dec_stop_cb, obj);

   edje_object_part_drag_value_set
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm.dragable.slider", 0.0, 0.0);

   priv->ent = elm_entry_add(obj);
   elm_entry_single_line_set(priv->ent, EINA_TRUE);
   evas_object_smart_callback_add
     (priv->ent, "activated", _entry_activated_cb, obj);

   elm_layout_content_set(obj, "elm.swallow.entry", priv->ent);
   elm_layout_signal_callback_add
     (obj, "elm,action,entry,toggle", "*", _entry_toggle_cb, obj);

   _label_write(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_spinner_register(obj);
}

static void
_elm_spinner_smart_del(Evas_Object *obj)
{
   Elm_Spinner_Special_Value *sv;

   ELM_SPINNER_DATA_GET(obj, sd);

   if (sd->label) eina_stringshare_del(sd->label);
   if (sd->delay) ecore_timer_del(sd->delay);
   if (sd->spin) ecore_timer_del(sd->spin);
   if (sd->special_values)
     {
        EINA_LIST_FREE (sd->special_values, sv)
          {
             eina_stringshare_del(sv->label);
             free(sv);
          }
     }

   ELM_WIDGET_CLASS(_elm_spinner_parent_sc)->base.del(obj);
}

static Eina_Bool
_elm_spinner_smart_theme(Evas_Object *obj)
{
   Eina_Bool ret;
   ret = elm_layout_theme_set(obj, "spinner", "base",
                              elm_widget_style_get(obj));

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_spinner_register(obj);

   return ret;
}

static Eina_Bool
_elm_spinner_smart_focus_next(const Evas_Object *obj,
                           Elm_Focus_Direction dir,
                           Evas_Object **next)
{
   Eina_List *items = NULL;

   ELM_SPINNER_CHECK(obj) EINA_FALSE;
   ELM_SPINNER_DATA_GET(obj, sd);

   items = eina_list_append(items, sd->access_obj);
   items = eina_list_append(items, sd->decrement_btn_access);
   items = eina_list_append(items, sd->increment_btn_access);

   return elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);
}

static void
_access_hook(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);
   
   if (is_access)
     {
        ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next =
        _elm_spinner_smart_focus_next;
        _access_spinner_register(obj);
     }
   else
     {
        ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next = NULL;
        //TODO: unregister edje part object
     }
}

static void
_elm_spinner_smart_set_user(Elm_Spinner_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_spinner_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_spinner_smart_del;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_WIDGET_CLASS(sc)->on_focus = _elm_spinner_smart_on_focus;
   ELM_WIDGET_CLASS(sc)->event = _elm_spinner_smart_event;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_spinner_smart_sizing_eval;

   ELM_WIDGET_CLASS(sc)->theme = _elm_spinner_smart_theme;

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     ELM_WIDGET_CLASS(sc)->focus_next = _elm_spinner_smart_focus_next;

   ELM_WIDGET_CLASS(sc)->access = _access_hook;
}

EAPI const Elm_Spinner_Smart_Class *
elm_spinner_smart_class_get(void)
{
   static Elm_Spinner_Smart_Class _sc =
     ELM_SPINNER_SMART_CLASS_INIT_NAME_VERSION(ELM_SPINNER_SMART_NAME);
   static const Elm_Spinner_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_spinner_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_spinner_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_spinner_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_spinner_label_format_set(Evas_Object *obj,
                             const char *fmt)
{
   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   eina_stringshare_replace(&sd->label, fmt);
   _label_write(obj);
   elm_layout_sizing_eval(obj);
}

EAPI const char *
elm_spinner_label_format_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) NULL;
   ELM_SPINNER_DATA_GET(obj, sd);

   return sd->label;
}

EAPI void
elm_spinner_min_max_set(Evas_Object *obj,
                        double min,
                        double max)
{
   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   if ((sd->val_min == min) && (sd->val_max == max)) return;
   sd->val_min = min;
   sd->val_max = max;
   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;
   _val_set(obj);
   _label_write(obj);
}

EAPI void
elm_spinner_min_max_get(const Evas_Object *obj,
                        double *min,
                        double *max)
{
   if (min) *min = 0.0;
   if (max) *max = 0.0;

   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   if (min) *min = sd->val_min;
   if (max) *max = sd->val_max;
}

EAPI void
elm_spinner_step_set(Evas_Object *obj,
                     double step)
{
   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->step = step;
}

EAPI double
elm_spinner_step_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) 0.0;
   ELM_SPINNER_DATA_GET(obj, sd);

   return sd->step;
}

EAPI void
elm_spinner_value_set(Evas_Object *obj,
                      double val)
{
   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   if (sd->val == val) return;
   sd->val = val;
   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;
   _val_set(obj);
   _label_write(obj);
}

EAPI double
elm_spinner_value_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) 0.0;
   ELM_SPINNER_DATA_GET(obj, sd);

   return sd->val;
}

EAPI void
elm_spinner_wrap_set(Evas_Object *obj,
                     Eina_Bool wrap)
{
   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->wrap = wrap;
}

EAPI Eina_Bool
elm_spinner_wrap_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) EINA_FALSE;
   ELM_SPINNER_DATA_GET(obj, sd);

   return sd->wrap;
}

EAPI void
elm_spinner_special_value_add(Evas_Object *obj,
                              double value,
                              const char *label)
{
   Elm_Spinner_Special_Value *sv;

   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   sv = calloc(1, sizeof(*sv));
   if (!sv) return;
   sv->value = value;
   sv->label = eina_stringshare_add(label);

   sd->special_values = eina_list_append(sd->special_values, sv);
   _label_write(obj);
}

EAPI void
elm_spinner_editable_set(Evas_Object *obj,
                         Eina_Bool editable)
{
   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->editable = editable;
}

EAPI Eina_Bool
elm_spinner_editable_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) EINA_FALSE;
   ELM_SPINNER_DATA_GET(obj, sd);

   return sd->editable;
}

EAPI void
elm_spinner_interval_set(Evas_Object *obj,
                         double interval)
{
   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->first_interval = interval;
}

EAPI double
elm_spinner_interval_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) 0.0;
   ELM_SPINNER_DATA_GET(obj, sd);

   return sd->first_interval;
}

EAPI void
elm_spinner_base_set(Evas_Object *obj,
                     double base)
{
   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->val_base = base;
}

EAPI double
elm_spinner_base_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) 0.0;
   ELM_SPINNER_DATA_GET(obj, sd);

   return sd->val_base;
}

EAPI void
elm_spinner_round_set(Evas_Object *obj,
                      int rnd)
{
   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->round = rnd;
}

EAPI int
elm_spinner_round_get(const Evas_Object *obj)
{
   ELM_SPINNER_CHECK(obj) 0;
   ELM_SPINNER_DATA_GET(obj, sd);

   return sd->round;
}
