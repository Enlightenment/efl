#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_clock.h"

EAPI Eo_Op ELM_OBJ_CLOCK_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_CLOCK_CLASS

#define MY_CLASS_NAME "Elm_Clock"
#define MY_CLASS_NAME_LEGACY "elm_clock"

#define DEFAULT_FIRST_INTERVAL 0.85
static void _time_update(Evas_Object *obj);

static const char SIG_CHANGED[] = "changed";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static Eina_Bool
_on_clock_val_up(void *data)
{
   ELM_CLOCK_DATA_GET(data, sd);

   if (!sd->edit) goto clock_val_up_cancel;
   if (!sd->sel_obj) goto clock_val_up_cancel;
   if (sd->sel_obj == sd->digit[0])
     {
        sd->hrs = sd->hrs + 12;
        if (sd->hrs >= 24) sd->hrs -= 24;
     }
   if (sd->sel_obj == sd->digit[1])
     {
        sd->hrs = sd->hrs + 1;
        if (sd->hrs >= 24) sd->hrs -= 24;
     }
   if (sd->sel_obj == sd->digit[2])
     {
        sd->min = sd->min + 10;
        if (sd->min >= 60) sd->min -= 60;
     }
   if (sd->sel_obj == sd->digit[3])
     {
        sd->min = sd->min + 1;
        if (sd->min >= 60) sd->min -= 60;
     }
   if (sd->sel_obj == sd->digit[4])
     {
        sd->sec = sd->sec + 10;
        if (sd->sec >= 60) sd->sec -= 60;
     }
   if (sd->sel_obj == sd->digit[5])
     {
        sd->sec = sd->sec + 1;
        if (sd->sec >= 60) sd->sec -= 60;
     }
   if (sd->sel_obj == sd->am_pm_obj)
     {
        sd->hrs = sd->hrs + 12;
        if (sd->hrs > 23) sd->hrs -= 24;
     }

   sd->interval = sd->interval / 1.05;
   ecore_timer_interval_set(sd->spin, sd->interval);
   _time_update(data);
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
   return ECORE_CALLBACK_RENEW;

clock_val_up_cancel:

   sd->spin = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_on_clock_val_down(void *data)
{
   ELM_CLOCK_DATA_GET(data, sd);

   if (!sd->edit) goto clock_val_down_cancel;
   if (!sd->sel_obj) goto clock_val_down_cancel;
   if (sd->sel_obj == sd->digit[0])
     {
        sd->hrs = sd->hrs - 12;
        if (sd->hrs < 0) sd->hrs += 24;
     }
   if (sd->sel_obj == sd->digit[1])
     {
        sd->hrs = sd->hrs - 1;
        if (sd->hrs < 0) sd->hrs += 24;
     }
   if (sd->sel_obj == sd->digit[2])
     {
        sd->min = sd->min - 10;
        if (sd->min < 0) sd->min += 60;
     }
   if (sd->sel_obj == sd->digit[3])
     {
        sd->min = sd->min - 1;
        if (sd->min < 0) sd->min += 60;
     }
   if (sd->sel_obj == sd->digit[4])
     {
        sd->sec = sd->sec - 10;
        if (sd->sec < 0) sd->sec += 60;
     }
   if (sd->sel_obj == sd->digit[5])
     {
        sd->sec = sd->sec - 1;
        if (sd->sec < 0) sd->sec += 60;
     }
   if (sd->sel_obj == sd->am_pm_obj)
     {
        sd->hrs = sd->hrs - 12;
        if (sd->hrs < 0) sd->hrs += 24;
     }
   sd->interval = sd->interval / 1.05;
   ecore_timer_interval_set(sd->spin, sd->interval);
   _time_update(data);
   evas_object_smart_callback_call(data, SIG_CHANGED, NULL);
   return ECORE_CALLBACK_RENEW;

clock_val_down_cancel:
   sd->spin = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_clock_val_up_start(void *data,
                       Evas_Object *obj,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   ELM_CLOCK_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   sd->sel_obj = obj;
   ecore_timer_del(sd->spin);
   sd->spin = ecore_timer_add(sd->interval, _on_clock_val_up, data);

   _on_clock_val_up(data);
}

static void
_on_clock_val_down_start(void *data,
                         Evas_Object *obj,
                         const char *emission EINA_UNUSED,
                         const char *source EINA_UNUSED)
{
   ELM_CLOCK_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   sd->sel_obj = obj;
   ecore_timer_del(sd->spin);
   sd->spin = ecore_timer_add(sd->interval, _on_clock_val_down, data);

   _on_clock_val_down(data);
}

static void
_on_clock_val_change_stop(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   ELM_CLOCK_DATA_GET(data, sd);

   ELM_SAFE_FREE(sd->spin, ecore_timer_del);
   sd->sel_obj = NULL;
}

static void
_access_activate_cb(void *data,
                    Evas_Object *part_obj,
                    Elm_Object_Item *item EINA_UNUSED)
{
   Evas_Object *digit, *inc_btn;
   ELM_CLOCK_DATA_GET(data, sd);

   digit = evas_object_smart_parent_get(part_obj);
   if (!digit) return;

   inc_btn = (Evas_Object *)edje_object_part_object_get(digit, "access.t");

   if (part_obj != inc_btn)
     _on_clock_val_down_start(data, digit, NULL, NULL);
   else
     _on_clock_val_up_start(data, digit, NULL, NULL);

   _on_clock_val_change_stop(sd, NULL, NULL, NULL);
}

static void
_access_time_register(Evas_Object *obj, Eina_Bool is_access)
{
   Evas_Object *ao, *po;

   ELM_CLOCK_DATA_GET(obj, sd);

   if (!sd->edit) return;

   /* hour, min, sec edit button */
   int i;
   for (i = 0; i < 6; i++)
     {
        if (is_access && (sd->digedit & (1 << i)))
          {
             char *digit = NULL;

             switch (1 << i)
               {
                case ELM_CLOCK_EDIT_HOUR_DECIMAL:
                  digit = "hour decimal";
                  break;
                case ELM_CLOCK_EDIT_HOUR_UNIT:
                  digit = "hour unit";
                  break;
                case ELM_CLOCK_EDIT_MIN_DECIMAL:
                  digit = "minute decimal";
                  break;
                case ELM_CLOCK_EDIT_MIN_UNIT:
                  digit = "minute unit";
                  break;
                case ELM_CLOCK_EDIT_SEC_DECIMAL:
                  digit = "second decimal";
                  break;
                case ELM_CLOCK_EDIT_SEC_UNIT:
                  digit = "second unit";
                  break;
               }

             Eina_Strbuf *strbuf;
             strbuf = eina_strbuf_new();

             /* increment button */
             ao = _elm_access_edje_object_part_object_register
                    (obj, sd->digit[i], "access.t");

             eina_strbuf_append_printf(strbuf,
               "clock increment button for %s", digit);
             _elm_access_text_set(_elm_access_info_get(ao),
               ELM_ACCESS_TYPE, eina_strbuf_string_get(strbuf));
             _elm_access_activate_callback_set
               (_elm_access_info_get(ao), _access_activate_cb, obj);

             /* decrement button */
             ao = _elm_access_edje_object_part_object_register
                    (obj, sd->digit[i], "access.b");

             eina_strbuf_replace(strbuf, "increment", "decrement", 1);
             _elm_access_text_set(_elm_access_info_get(ao),
               ELM_ACCESS_TYPE, eina_strbuf_string_get(strbuf));
             _elm_access_activate_callback_set
               (_elm_access_info_get(ao), _access_activate_cb, obj);

             eina_strbuf_free(strbuf);

             edje_object_signal_emit
               (sd->digit[i], "elm,state,access,edit,on", "elm");
          }
        else if (!is_access && (sd->digedit & (1 << i)))
          {
             _elm_access_edje_object_part_object_unregister
               (obj, sd->digit[i], "access.t");

             _elm_access_edje_object_part_object_unregister
               (obj, sd->digit[i], "access.b");

             edje_object_signal_emit
               (sd->digit[i], "elm,state,access,edit,off", "elm");
          }

        /* no need to propagate mouse event with acess */
        po = (Evas_Object *)edje_object_part_object_get
               (sd->digit[i], "access.t");
        evas_object_propagate_events_set(po, !is_access);

        po = (Evas_Object *)edje_object_part_object_get
               (sd->digit[i], "access.b");
        evas_object_propagate_events_set(po, !is_access);

     }

   /* am, pm edit button  */
   if (is_access && sd->am_pm)
     {
        /* increment button */
        ao = _elm_access_edje_object_part_object_register
               (obj, sd->am_pm_obj, "access.t");
        _elm_access_text_set(_elm_access_info_get(ao),
          ELM_ACCESS_TYPE, E_("clock increment button for am,pm"));
        _elm_access_activate_callback_set
          (_elm_access_info_get(ao), _access_activate_cb, obj);

        /* decrement button */
        ao = _elm_access_edje_object_part_object_register
               (obj, sd->am_pm_obj, "access.b");
        _elm_access_text_set(_elm_access_info_get(ao),
          ELM_ACCESS_TYPE, E_("clock decrement button for am,pm"));
        _elm_access_activate_callback_set
          (_elm_access_info_get(ao), _access_activate_cb, obj);

         edje_object_signal_emit
           (sd->am_pm_obj, "elm,state,access,edit,on", "elm");
     }
   else if (!is_access && sd->am_pm)
     {
        _elm_access_edje_object_part_object_register
          (obj, sd->am_pm_obj, "access.t");

        _elm_access_edje_object_part_object_register
          (obj, sd->am_pm_obj, "access.b");

         edje_object_signal_emit
           (sd->am_pm_obj, "elm,state,access,edit,off", "elm");
     }

    /* no need to propagate mouse event with access */
    po = (Evas_Object *)edje_object_part_object_get
           (sd->am_pm_obj, "access.t");
    evas_object_propagate_events_set(po, !is_access);

    po = (Evas_Object *)edje_object_part_object_get
           (sd->am_pm_obj, "access.b");
    evas_object_propagate_events_set(po, !is_access);

}

static void
_time_update(Evas_Object *obj)
{
   ELM_CLOCK_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Edje_Message_Int msg;
   const char *style = elm_widget_style_get(obj);

   if ((sd->cur.seconds != sd->seconds) || (sd->cur.am_pm != sd->am_pm) ||
       (sd->cur.edit != sd->edit) || (sd->cur.digedit != sd->digedit))
     {
        int i;
        Evas_Coord mw, mh;

        for (i = 0; i < 6; i++)
          ELM_SAFE_FREE(sd->digit[i], evas_object_del);
        ELM_SAFE_FREE(sd->am_pm_obj, evas_object_del);

        if ((sd->seconds) && (sd->am_pm))
          {
             if (!elm_layout_theme_set(obj, "clock", "base-all", style))
               CRI("Failed to set layout!");
          }
        else if (sd->seconds)
          {
             if (!elm_layout_theme_set(obj, "clock", "base-seconds", style))
               CRI("Failed to set layout!");
          }
        else if (sd->am_pm)
          {
             if (!elm_layout_theme_set(obj, "clock", "base-am_pm", style))
               CRI("Failed to set layout!");
          }
        else
          {
             if (!elm_layout_theme_set(obj, "clock", "base", style))
               CRI("Failed to set layout!");
          }

        edje_object_scale_set
          (wd->resize_obj, elm_widget_scale_get(obj) *
          elm_config_scale_get());

        for (i = 0; i < 6; i++)
          {
             char buf[16];

             if ((!sd->seconds) && (i >= 4)) break;
             sd->digit[i] = edje_object_add
                 (evas_object_evas_get(wd->resize_obj));
             elm_widget_theme_object_set
               (obj, sd->digit[i], "clock", "flipdigit", style);
             edje_object_scale_set
               (sd->digit[i], elm_widget_scale_get(obj) *
               elm_config_scale_get());

             if ((sd->edit) && (sd->digedit & (1 << i)))
               edje_object_signal_emit
                 (sd->digit[i], "elm,state,edit,on", "elm");
             edje_object_signal_callback_add
               (sd->digit[i], "elm,action,up,start", "*",
               _on_clock_val_up_start, obj);
             edje_object_signal_callback_add
               (sd->digit[i], "elm,action,up,stop", "*",
               _on_clock_val_change_stop, obj);
             edje_object_signal_callback_add
               (sd->digit[i], "elm,action,down,start", "*",
               _on_clock_val_down_start, obj);
             edje_object_signal_callback_add
               (sd->digit[i], "elm,action,down,stop", "*",
               _on_clock_val_change_stop, obj);

             mw = mh = -1;
             elm_coords_finger_size_adjust(1, &mw, 2, &mh);
             edje_object_size_min_restricted_calc
               (sd->digit[i], &mw, &mh, mw, mh);
             elm_coords_finger_size_adjust(1, &mw, 2, &mh);
             edje_extern_object_min_size_set(sd->digit[i], mw, mh);
             snprintf(buf, sizeof(buf), "d%i", i);
             elm_layout_content_set(obj, buf, sd->digit[i]);
             evas_object_show(sd->digit[i]);
          }
        if (sd->am_pm)
          {
             sd->am_pm_obj =
               edje_object_add(evas_object_evas_get(wd->resize_obj));
             elm_widget_theme_object_set
               (obj, sd->am_pm_obj, "clock", "flipampm", style);
             edje_object_scale_set(sd->am_pm_obj, elm_widget_scale_get(obj) *
                                   _elm_config->scale);
             if (sd->edit)
               edje_object_signal_emit
                 (sd->am_pm_obj, "elm,state,edit,on", "elm");
             edje_object_signal_callback_add
               (sd->am_pm_obj, "elm,action,up,start", "*",
               _on_clock_val_up_start, obj);
             edje_object_signal_callback_add
               (sd->am_pm_obj, "elm,action,up,stop", "*",
               _on_clock_val_change_stop, obj);
             edje_object_signal_callback_add
               (sd->am_pm_obj, "elm,action,down,start", "*",
               _on_clock_val_down_start, obj);
             edje_object_signal_callback_add
               (sd->am_pm_obj, "elm,action,down,stop", "*",
               _on_clock_val_change_stop, obj);

             mw = mh = -1;
             elm_coords_finger_size_adjust(1, &mw, 2, &mh);
             edje_object_size_min_restricted_calc
               (sd->am_pm_obj, &mw, &mh, mw, mh);
             elm_coords_finger_size_adjust(1, &mw, 2, &mh);
             edje_extern_object_min_size_set(sd->am_pm_obj, mw, mh);
             elm_layout_content_set(obj, "ampm", sd->am_pm_obj);
             evas_object_show(sd->am_pm_obj);
          }

        /* access */
        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
          _access_time_register(obj, EINA_TRUE);

        edje_object_size_min_calc(wd->resize_obj, &mw, &mh);
        evas_object_size_hint_min_set(obj, mw, mh);

        sd->cur.hrs = 0;
        sd->cur.min = 0;
        sd->cur.sec = 0;
        sd->cur.ampm = -1;
        sd->cur.seconds = sd->seconds;
        sd->cur.am_pm = sd->am_pm;
        sd->cur.edit = sd->edit;
        sd->cur.digedit = sd->digedit;
     }
   if (sd->hrs != sd->cur.hrs)
     {
        int hrs;
        int d1, d2, dc1, dc2;

        hrs = sd->hrs;
        if (sd->am_pm)
          {
             if (hrs > 12) hrs -= 12;
             else if (!hrs) hrs = 12;
          }
        d1 = hrs / 10;
        d2 = hrs % 10;
        dc1 = sd->cur.hrs / 10;
        dc2 = sd->cur.hrs % 10;
        if (d1 != dc1)
          {
             msg.val = d1;
             edje_object_message_send(sd->digit[0], EDJE_MESSAGE_INT, 1, &msg);
          }
        if (d2 != dc2)
          {
             msg.val = d2;
             edje_object_message_send(sd->digit[1], EDJE_MESSAGE_INT, 1, &msg);
          }
        sd->cur.hrs = hrs;
     }
   if (sd->min != sd->cur.min)
     {
        int d1, d2, dc1, dc2;

        d1 = sd->min / 10;
        d2 = sd->min % 10;
        dc1 = sd->cur.min / 10;
        dc2 = sd->cur.min % 10;
        if (d1 != dc1)
          {
             msg.val = d1;
             edje_object_message_send(sd->digit[2], EDJE_MESSAGE_INT, 1, &msg);
          }
        if (d2 != dc2)
          {
             msg.val = d2;
             edje_object_message_send(sd->digit[3], EDJE_MESSAGE_INT, 1, &msg);
          }
        sd->cur.min = sd->min;
     }
   if (sd->seconds)
     {
        if (sd->sec != sd->cur.sec)
          {
             int d1, d2, dc1, dc2;

             d1 = sd->sec / 10;
             d2 = sd->sec % 10;
             dc1 = sd->cur.sec / 10;
             dc2 = sd->cur.sec % 10;
             if (d1 != dc1)
               {
                  msg.val = d1;
                  edje_object_message_send
                    (sd->digit[4], EDJE_MESSAGE_INT, 1, &msg);
               }
             if (d2 != dc2)
               {
                  msg.val = d2;
                  edje_object_message_send
                    (sd->digit[5], EDJE_MESSAGE_INT, 1, &msg);
               }
             sd->cur.sec = sd->sec;
          }
     }
   else
     sd->cur.sec = -1;

   if (sd->am_pm)
     {
        int ampm = 0;
        if (sd->hrs >= 12) ampm = 1;
        if (ampm != sd->cur.ampm)
          {
             msg.val = ampm;
             edje_object_message_send
               (sd->am_pm_obj, EDJE_MESSAGE_INT, 1, &msg);
             sd->cur.ampm = ampm;
          }
     }
   else
     sd->cur.ampm = -1;
}

static void
_elm_clock_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   _time_update(obj);

   if (ret) *ret = EINA_TRUE;
}

static Eina_Bool
_ticker(void *data)
{
   ELM_CLOCK_DATA_GET(data, sd);

   double t;
   struct timeval timev;
   struct tm *tm;
   time_t tt;

   gettimeofday(&timev, NULL);
   t = ((double)(1000000 - timev.tv_usec)) / 1000000.0;

   sd->ticker = ecore_timer_add(t, _ticker, data);
   if (!sd->edit)
     {
        tt = (time_t)(timev.tv_sec) + sd->timediff;
        tzset();
        tm = localtime(&tt);
        if (tm)
          {
             sd->hrs = tm->tm_hour;
             sd->min = tm->tm_min;
             sd->sec = tm->tm_sec;
             _time_update(data);
          }
     }

   return ECORE_CALLBACK_CANCEL;
}

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   int hrs;
   char *ret;
   Eina_Strbuf *buf;

   ELM_CLOCK_DATA_GET(obj, sd);

   buf = eina_strbuf_new();

   hrs = sd->hrs;

   if (sd->am_pm)
     {
        char *ampm = NULL;
        if (hrs >= 12)
          {
             if (hrs > 12) hrs -= 12;
             ampm = "PM";
          }
        else ampm = "AM";

        eina_strbuf_append_printf(buf, "%d, %d, %s", hrs, sd->min, ampm);
     }
   else
     {
        eina_strbuf_append_printf(buf, "%d, %d", hrs, sd->min);
     }

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static char *
_access_state_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   ELM_CLOCK_DATA_GET(obj, sd);
   if (sd->edit)
     return strdup(E_("State: Editable"));

   return NULL;
}

static void
_elm_clock_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Clock_Smart_Data *priv = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->cur.ampm = -1;
   priv->cur.seconds = EINA_TRUE;
   priv->cur.am_pm = EINA_TRUE;
   priv->cur.edit = EINA_TRUE;
   priv->cur.digedit = ELM_CLOCK_EDIT_DEFAULT;
   priv->first_interval = DEFAULT_FIRST_INTERVAL;

   elm_widget_can_focus_set(obj, EINA_TRUE);

   _time_update(obj);
   _ticker(obj);

   /* access */
   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
     {
        evas_object_propagate_events_set(obj, EINA_FALSE);
        edje_object_signal_emit(wd->resize_obj,
          "elm,state,access,on", "elm");
     }

   _elm_access_object_register(obj, wd->resize_obj);
   _elm_access_text_set
     (_elm_access_info_get(obj), ELM_ACCESS_TYPE, E_("Clock"));
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   evas_object_propagate_events_set(obj, EINA_FALSE);
   _elm_access_callback_set
     (_elm_access_info_get(obj), ELM_ACCESS_STATE, _access_state_cb, NULL);
}

static void
_elm_clock_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Clock_Smart_Data *sd = _pd;

   ecore_timer_del(sd->ticker);
   ecore_timer_del(sd->spin);

   /* NB: digits are killed for being sub objects, automatically */

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

static Eina_Bool _elm_clock_smart_focus_next_enable = EINA_FALSE;

static void
_elm_clock_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = _elm_clock_smart_focus_next_enable;
}

static void
_elm_clock_smart_focus_next(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Evas_Object *ao, *po;
   Eina_List *items = NULL;

   Elm_Clock_Smart_Data *sd = _pd;

   if (!sd->edit)
     {
        *next = (Evas_Object *)obj;
        if (ret) *ret = !elm_widget_highlight_get(obj);
        return;
     }
   else if (!elm_widget_highlight_get(obj))
     {
        *next = (Evas_Object *)obj;
        if (ret) *ret = EINA_TRUE;
        return;
     }

   int i;
   for (i = 0; i < 6; i++)
     {
        if ((!sd->seconds) && (i >= 4)) break;
        if (sd->digedit & (1 << i))
          {
             po = (Evas_Object *)edje_object_part_object_get
                    (sd->digit[i], "access.t");
             ao = evas_object_data_get(po, "_part_access_obj");
             items = eina_list_append(items, ao);

             po = (Evas_Object *)edje_object_part_object_get
                    (sd->digit[i], "access.b");
             ao = evas_object_data_get(po, "_part_access_obj");
             items = eina_list_append(items, ao);
          }
     }

   if (sd->am_pm)
     {
        po = (Evas_Object *)edje_object_part_object_get
               (sd->am_pm_obj, "access.t");
        ao = evas_object_data_get(po, "_part_access_obj");
        items = eina_list_append(items, ao);

        po = (Evas_Object *)edje_object_part_object_get
               (sd->am_pm_obj, "access.b");
        ao = evas_object_data_get(po, "_part_access_obj");
        items = eina_list_append(items, ao);
     }

   if (ret) *ret = elm_widget_focus_list_next_get
           (obj, items, eina_list_data_get, dir, next);
}

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   /* clock object */
   evas_object_propagate_events_set(obj, !is_access);

   if (is_access)
     edje_object_signal_emit(wd->resize_obj,
       "elm,state,access,on", "elm");
   else
     edje_object_signal_emit(wd->resize_obj,
       "elm,state,access,off", "elm");

    /* clock time objects */
    _access_time_register(obj, is_access);
}

static void
_elm_clock_smart_access(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   _elm_clock_smart_focus_next_enable = va_arg(*list, int);
   _access_obj_process(obj, _elm_clock_smart_focus_next_enable);
}

EAPI Evas_Object *
elm_clock_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

static void
_timediff_set(Elm_Clock_Smart_Data *sd)
{
   struct timeval timev;
   struct tm *tm;
   time_t tt;

   gettimeofday(&timev, NULL);
   tt = (time_t)(timev.tv_sec);
   tzset();
   tm = localtime(&tt);
   sd->timediff = (((sd->hrs - tm->tm_hour) * 60 +
                    sd->min - tm->tm_min) * 60) + sd->sec - tm->tm_sec;
}

EAPI void
elm_clock_time_set(Evas_Object *obj,
                   int hrs,
                   int min,
                   int sec)
{
   ELM_CLOCK_CHECK(obj);
   eo_do(obj, elm_obj_clock_time_set(hrs, min, sec));
}

static void
_time_set(Eo *obj, void *_pd, va_list *list)
{
   int hrs = va_arg(*list, int);
   int min = va_arg(*list, int);
   int sec = va_arg(*list, int);
   Elm_Clock_Smart_Data *sd = _pd;

   sd->hrs = hrs;
   sd->min = min;
   sd->sec = sec;

   _timediff_set(sd);
   _time_update(obj);
}

EAPI void
elm_clock_time_get(const Evas_Object *obj,
                   int *hrs,
                   int *min,
                   int *sec)
{
   ELM_CLOCK_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_clock_time_get(hrs, min, sec));
}

static void
_time_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *hrs = va_arg(*list, int *);
   int *min = va_arg(*list, int *);
   int *sec = va_arg(*list, int *);
   Elm_Clock_Smart_Data *sd = _pd;

   if (hrs) *hrs = sd->hrs;
   if (min) *min = sd->min;
   if (sec) *sec = sd->sec;
}

EAPI void
elm_clock_edit_set(Evas_Object *obj,
                   Eina_Bool edit)
{
   ELM_CLOCK_CHECK(obj);
   eo_do(obj, elm_obj_clock_edit_set(edit));
}

static void
_edit_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool edit = va_arg(*list, int);
   Elm_Clock_Smart_Data *sd = _pd;

   sd->edit = edit;
   if (!edit)
     _timediff_set(sd);
   if ((edit) && (sd->digedit == ELM_CLOCK_EDIT_DEFAULT))
     elm_clock_edit_mode_set(obj, ELM_CLOCK_EDIT_ALL);
   else
     _time_update(obj);
}

EAPI Eina_Bool
elm_clock_edit_get(const Evas_Object *obj)
{
   ELM_CLOCK_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_clock_edit_get(&ret));
   return ret;
}

static void
_edit_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Clock_Smart_Data *sd = _pd;

   *ret = sd->edit;
}

EAPI void
elm_clock_edit_mode_set(Evas_Object *obj,
                        Elm_Clock_Edit_Mode digedit)
{
   ELM_CLOCK_CHECK(obj);
   eo_do(obj, elm_obj_clock_edit_mode_set(digedit));
}

static void
_edit_mode_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Clock_Edit_Mode digedit = va_arg(*list, Elm_Clock_Edit_Mode);
   Elm_Clock_Smart_Data *sd = _pd;

   sd->digedit = digedit;
   if (digedit == ELM_CLOCK_EDIT_DEFAULT)
     elm_clock_edit_set(obj, EINA_FALSE);
   else
     _time_update(obj);
}

EAPI Elm_Clock_Edit_Mode
elm_clock_edit_mode_get(const Evas_Object *obj)
{
   ELM_CLOCK_CHECK(obj) 0;
   Elm_Clock_Edit_Mode ret = 0;
   eo_do((Eo *) obj, elm_obj_clock_edit_mode_get(&ret));
   return ret;
}

static void
_edit_mode_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Clock_Edit_Mode *ret = va_arg(*list, Elm_Clock_Edit_Mode *);
   Elm_Clock_Smart_Data *sd = _pd;

   *ret = sd->digedit;
}

EAPI void
elm_clock_show_am_pm_set(Evas_Object *obj,
                         Eina_Bool am_pm)
{
   ELM_CLOCK_CHECK(obj);
   eo_do(obj, elm_obj_clock_show_am_pm_set(am_pm));
}

static void
_show_am_pm_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool am_pm = va_arg(*list, int);
   Elm_Clock_Smart_Data *sd = _pd;

   sd->am_pm = !!am_pm;
   _time_update(obj);
}

EAPI Eina_Bool
elm_clock_show_am_pm_get(const Evas_Object *obj)
{
   ELM_CLOCK_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_clock_show_am_pm_get(&ret));
   return ret;
}

static void
_show_am_pm_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Clock_Smart_Data *sd = _pd;

   *ret = sd->am_pm;
}

EAPI void
elm_clock_show_seconds_set(Evas_Object *obj,
                           Eina_Bool seconds)
{
   ELM_CLOCK_CHECK(obj);
   eo_do(obj, elm_obj_clock_show_seconds_set(seconds));
}

static void
_show_seconds_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool seconds = va_arg(*list, int);
   Elm_Clock_Smart_Data *sd = _pd;

   sd->seconds = !!seconds;
   _time_update(obj);
}

EAPI Eina_Bool
elm_clock_show_seconds_get(const Evas_Object *obj)
{
   ELM_CLOCK_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_clock_show_seconds_get(&ret));
   return ret;
}

static void
_show_seconds_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Clock_Smart_Data *sd = _pd;

   *ret = sd->seconds;
}

EAPI void
elm_clock_first_interval_set(Evas_Object *obj,
                             double interval)
{
   ELM_CLOCK_CHECK(obj);
   eo_do(obj, elm_obj_clock_first_interval_set(interval));
}

static void
_first_interval_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double interval = va_arg(*list, double);
   Elm_Clock_Smart_Data *sd = _pd;

   sd->first_interval = interval;
}

EAPI double
elm_clock_first_interval_get(const Evas_Object *obj)
{
   ELM_CLOCK_CHECK(obj) 0.0;
   double ret = 0.0;
   eo_do((Eo *) obj, elm_obj_clock_first_interval_get(&ret));
   return ret;
}

static void
_first_interval_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Clock_Smart_Data *sd = _pd;

   *ret = sd->first_interval;
}

EAPI void
elm_clock_pause_set(Evas_Object *obj, Eina_Bool paused)
{
   ELM_CLOCK_CHECK(obj);
   eo_do(obj, elm_obj_clock_pause_set(paused));
}

static void
_pause_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool paused = va_arg(*list, int);
   Elm_Clock_Smart_Data *sd = _pd;
   paused = !!paused;
   if (sd->paused == paused)
     return;
   sd->paused = paused;
   if (paused)
     ecore_timer_freeze(sd->ticker);
   else
     {
        _timediff_set(sd);
        ecore_timer_thaw(sd->ticker);
     }
}

EAPI Eina_Bool
elm_clock_pause_get(const Evas_Object *obj)
{
   ELM_CLOCK_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_clock_pause_get(&ret));
   return ret;
}

static void
_pause_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Clock_Smart_Data *sd = _pd;

   *ret = sd->paused;
}

static void
_elm_clock_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_clock_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_clock_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_clock_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ACCESS), _elm_clock_smart_access),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_clock_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_clock_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_clock_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_TIME_SET), _time_set),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_TIME_GET), _time_get),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_EDIT_SET), _edit_set),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_EDIT_GET), _edit_get),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_EDIT_MODE_SET), _edit_mode_set),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_EDIT_MODE_GET), _edit_mode_get),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_SHOW_AM_PM_SET), _show_am_pm_set),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_SHOW_AM_PM_GET), _show_am_pm_get),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_SHOW_SECONDS_SET), _show_seconds_set),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_SHOW_SECONDS_GET), _show_seconds_get),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_FIRST_INTERVAL_SET), _first_interval_set),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_FIRST_INTERVAL_GET), _first_interval_get),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_PAUSE_SET), _pause_set),
        EO_OP_FUNC(ELM_OBJ_CLOCK_ID(ELM_OBJ_CLOCK_SUB_ID_PAUSE_GET), _pause_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);

   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
      _elm_clock_smart_focus_next_enable = EINA_TRUE;
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_TIME_SET, "Set a clock widget's time, programmatically."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_TIME_GET, "Get a clock widget's time values."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_EDIT_SET, "Set whether a given clock widget is under <b>edition mode</b> or."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_EDIT_GET, "Retrieve whether a given clock widget is under editing mode."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_EDIT_MODE_SET, "Set what digits of the given clock widget should be editable."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_EDIT_MODE_GET, "Retrieve what digits of the given clock widget should be."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_SHOW_AM_PM_SET, "Set if the given clock widget must show hours in military or."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_SHOW_AM_PM_GET, "Get if the given clock widget shows hours in military or am/pm."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_SHOW_SECONDS_SET, "Set if the given clock widget must show time with seconds or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_SHOW_SECONDS_GET, "Get whether the given clock widget is showing time with seconds."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_FIRST_INTERVAL_SET, "Set the first interval on time updates for a user mouse button hold."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_FIRST_INTERVAL_GET, "Get the first interval on time updates for a user mouse button hold."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_PAUSE_SET, "Set whether the given clock widget should be paused or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_CLOCK_SUB_ID_PAUSE_GET, "Get if the given clock widget is paused."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_CLOCK_BASE_ID, op_desc, ELM_OBJ_CLOCK_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Clock_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_clock_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
