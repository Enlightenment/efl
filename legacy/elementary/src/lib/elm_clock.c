#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_clock.h"

EAPI const char ELM_CLOCK_SMART_NAME[] = "elm_clock";

#define DEFAULT_FIRST_INTERVAL 0.85

static void _time_update(Evas_Object *obj);

static const char SIG_CHANGED[] = "changed";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_CLOCK_SMART_NAME, _elm_clock, Elm_Clock_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static Eina_Bool
_on_clock_val_up(void *data)
{
   ELM_CLOCK_DATA_GET(data, sd);

   if (!sd->edit) goto clock_val_up_cancel;
   if (!sd->sel_obj) goto clock_val_up_cancel;
   if (sd->sel_obj == sd->digit[0])
     {
        sd->hrs = sd->hrs + 10;
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
        sd->hrs = sd->hrs - 10;
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
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   ELM_CLOCK_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   sd->sel_obj = obj;
   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = ecore_timer_add(sd->interval, _on_clock_val_up, data);

   _on_clock_val_up(data);
}

static void
_on_clock_val_down_start(void *data,
                         Evas_Object *obj,
                         const char *emission __UNUSED__,
                         const char *source __UNUSED__)
{
   ELM_CLOCK_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   sd->sel_obj = obj;
   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = ecore_timer_add(sd->interval, _on_clock_val_down, data);

   _on_clock_val_down(data);
}

static void
_on_clock_val_change_stop(void *data,
                          Evas_Object *obj __UNUSED__,
                          const char *emission __UNUSED__,
                          const char *source __UNUSED__)
{
   Elm_Clock_Smart_Data *sd = data;

   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = NULL;
   sd->sel_obj = NULL;
}

static void
_access_activate_cb(void *data,
                    Evas_Object *part_obj,
                    Elm_Widget_Item *item __UNUSED__)
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
             _elm_access_text_set(_elm_access_object_get(ao),
               ELM_ACCESS_TYPE, eina_strbuf_string_get(strbuf));
             _elm_access_activate_callback_set
               (_elm_access_object_get(ao), _access_activate_cb, obj);

             /* decrement button */
             ao = _elm_access_edje_object_part_object_register
                    (obj, sd->digit[i], "access.b");

             eina_strbuf_replace(strbuf, "increment", "decrement", 1);
             _elm_access_text_set(_elm_access_object_get(ao),
               ELM_ACCESS_TYPE, eina_strbuf_string_get(strbuf));
             _elm_access_activate_callback_set
               (_elm_access_object_get(ao), _access_activate_cb, obj);

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
        _elm_access_text_set(_elm_access_object_get(ao),
          ELM_ACCESS_TYPE, E_("clock increment button for am,pm"));
        _elm_access_activate_callback_set
          (_elm_access_object_get(ao), _access_activate_cb, obj);

        /* decrement button */
        ao = _elm_access_edje_object_part_object_register
               (obj, sd->am_pm_obj, "access.b");
        _elm_access_text_set(_elm_access_object_get(ao),
          ELM_ACCESS_TYPE, E_("clock decrement button for am,pm"));
        _elm_access_activate_callback_set
          (_elm_access_object_get(ao), _access_activate_cb, obj);

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

   Edje_Message_Int msg;
   int ampm = 0;
   const char *style = elm_widget_style_get(obj);

   if ((sd->cur.seconds != sd->seconds) || (sd->cur.am_pm != sd->am_pm) ||
       (sd->cur.edit != sd->edit) || (sd->cur.digedit != sd->digedit))
     {
        int i;
        Evas_Coord mw, mh;

        for (i = 0; i < 6; i++)
          {
             if (sd->digit[i])
               {
                  evas_object_del(sd->digit[i]);
                  sd->digit[i] = NULL;
               }
          }
        if (sd->am_pm_obj)
          {
             evas_object_del(sd->am_pm_obj);
             sd->am_pm_obj = NULL;
          }

        if ((sd->seconds) && (sd->am_pm))
          elm_layout_theme_set(obj, "clock", "base-all", style);
        else if (sd->seconds)
          elm_layout_theme_set(obj, "clock", "base-seconds", style);
        else if (sd->am_pm)
          elm_layout_theme_set(obj, "clock", "base-am_pm", style);
        else
          elm_layout_theme_set(obj, "clock", "base", style);

        edje_object_scale_set
          (ELM_WIDGET_DATA(sd)->resize_obj, elm_widget_scale_get(obj) *
          elm_config_scale_get());

        for (i = 0; i < 6; i++)
          {
             char buf[16];

             if ((!sd->seconds) && (i >= 4)) break;
             sd->digit[i] = edje_object_add
                 (evas_object_evas_get(ELM_WIDGET_DATA(sd)->resize_obj));
             elm_widget_theme_object_set
               (obj, sd->digit[i], "clock", "flipdigit", style);
             edje_object_scale_set
               (sd->digit[i], elm_widget_scale_get(obj) *
               elm_config_scale_get());

             if ((sd->edit) && (sd->digedit & (1 << i)))
               edje_object_signal_emit
                 (sd->digit[i], "elm,state,edit,on", "elm");
             edje_object_signal_callback_add
               (sd->digit[i], "elm,action,up,start", "",
               _on_clock_val_up_start, obj);
             edje_object_signal_callback_add
               (sd->digit[i], "elm,action,up,stop", "",
               _on_clock_val_change_stop, sd);
             edje_object_signal_callback_add
               (sd->digit[i], "elm,action,down,start", "",
               _on_clock_val_down_start, obj);
             edje_object_signal_callback_add
               (sd->digit[i], "elm,action,down,stop", "",
               _on_clock_val_change_stop, sd);

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
               edje_object_add(evas_object_evas_get(ELM_WIDGET_DATA(sd)->resize_obj));
             elm_widget_theme_object_set
               (obj, sd->am_pm_obj, "clock", "flipampm", style);
             edje_object_scale_set(sd->am_pm_obj, elm_widget_scale_get(obj) *
                                   _elm_config->scale);
             if (sd->edit)
               edje_object_signal_emit
                 (sd->am_pm_obj, "elm,state,edit,on", "elm");
             edje_object_signal_callback_add
               (sd->am_pm_obj, "elm,action,up,start", "",
               _on_clock_val_up_start, obj);
             edje_object_signal_callback_add
               (sd->am_pm_obj, "elm,action,up,stop", "",
               _on_clock_val_change_stop, sd);
             edje_object_signal_callback_add
               (sd->am_pm_obj, "elm,action,down,start", "",
               _on_clock_val_down_start, obj);
             edje_object_signal_callback_add
               (sd->am_pm_obj, "elm,action,down,stop", "",
               _on_clock_val_change_stop, sd);

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

        edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &mw, &mh);
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
             if (hrs >= 12)
               {
                  if (hrs > 12) hrs -= 12;
                  ampm = 1;
               }
             else if (!hrs)
               hrs = 12;
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
        if (sd->hrs >= 12) ampm = 1;
        if (ampm != sd->cur.ampm)
          {
             if (sd->cur.ampm != ampm)
               {
                  msg.val = ampm;
                  edje_object_message_send
                    (sd->am_pm_obj, EDJE_MESSAGE_INT, 1, &msg);
               }
             sd->cur.ampm = ampm;
          }
     }
   else
     sd->cur.ampm = -1;
}

static Eina_Bool
_elm_clock_smart_theme(Evas_Object *obj)
{
   if (!ELM_WIDGET_CLASS(_elm_clock_parent_sc)->theme(obj)) return EINA_FALSE;

   _time_update(obj);

   return EINA_TRUE;
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
_access_info_cb(void *data __UNUSED__,
                Evas_Object *obj,
                Elm_Widget_Item *item __UNUSED__)
{
   int hrs;
   char *ret;
   char *ampm = NULL;
   Eina_Strbuf *buf;

   ELM_CLOCK_DATA_GET(obj, sd);

   buf = eina_strbuf_new();

   hrs = sd->hrs;

   if (sd->am_pm)
     {
        if (hrs >= 12)
          {
             if (hrs > 12) hrs -= 12;
             ampm = "PM";
          }
        else ampm = "AM";
     }

   eina_strbuf_append_printf(buf, (ampm) ? ("%d, %d, %s") : ("%d, %d"),
                             hrs, sd->min, ampm);

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static char *
_access_state_cb(void *data __UNUSED__,
                 Evas_Object *obj,
                 Elm_Widget_Item *item __UNUSED__)
{
   ELM_CLOCK_DATA_GET(obj, sd);
   if (sd->edit)
     return strdup(E_("State: Editable"));

   return NULL;
}

static void
_elm_clock_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Clock_Smart_Data);

   ELM_WIDGET_CLASS(_elm_clock_parent_sc)->base.add(obj);

   priv->cur.ampm = -1;
   priv->cur.seconds = EINA_TRUE;
   priv->cur.am_pm = EINA_TRUE;
   priv->cur.edit = EINA_TRUE;
   priv->cur.digedit = ELM_CLOCK_EDIT_DEFAULT;
   priv->first_interval = DEFAULT_FIRST_INTERVAL;
   priv->timediff = 0;

   elm_widget_can_focus_set(obj, EINA_TRUE);

   _time_update(obj);
   _ticker(obj);

   /* access */
   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
     {
        evas_object_propagate_events_set(obj, EINA_FALSE);
        edje_object_signal_emit(ELM_WIDGET_DATA(priv)->resize_obj,
          "elm,state,access,on", "elm");
     }

   _elm_access_object_register(obj, ELM_WIDGET_DATA(priv)->resize_obj);
   _elm_access_text_set
     (_elm_access_object_get(obj), ELM_ACCESS_TYPE, E_("Clock"));
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_INFO, _access_info_cb, NULL);
   evas_object_propagate_events_set(obj, EINA_FALSE);
   _elm_access_callback_set
     (_elm_access_object_get(obj), ELM_ACCESS_STATE, _access_state_cb, NULL);
}

static void
_elm_clock_smart_del(Evas_Object *obj)
{
   ELM_CLOCK_DATA_GET(obj, sd);

   if (sd->ticker) ecore_timer_del(sd->ticker);
   if (sd->spin) ecore_timer_del(sd->spin);

   /* NB: digits are killed for being sub objects, automatically */

   ELM_WIDGET_CLASS(_elm_clock_parent_sc)->base.del(obj);
}

static Eina_Bool
_elm_clock_smart_focus_next(const Evas_Object *obj,
                            Elm_Focus_Direction dir,
                            Evas_Object **next)
{
   Evas_Object *ao, *po;
   Eina_List *items = NULL;

   ELM_CLOCK_DATA_GET(obj, sd);

   if (!sd->edit)
     {
        *next = (Evas_Object *)obj;
        return !elm_widget_focus_get(obj);
     }
   else if (!elm_widget_focus_get(obj))
     {
        *next = (Evas_Object *)obj;
        return EINA_TRUE;
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

   return elm_widget_focus_list_next_get
           (obj, items, eina_list_data_get, dir, next);
}

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_CLOCK_DATA_GET(obj, sd);

   /* clock object */
   evas_object_propagate_events_set(obj, !is_access);

   if (is_access)
     edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj,
       "elm,state,access,on", "elm");
   else
     edje_object_signal_emit(ELM_WIDGET_DATA(sd)->resize_obj,
       "elm,state,access,off", "elm");

    /* clock time objects */
    _access_time_register(obj, is_access);
}

static void
_elm_clock_smart_access(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_CLOCK_CHECK(obj);
   ELM_CLOCK_DATA_GET(obj, sd);

   if (is_access)
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next =
       _elm_clock_smart_focus_next;
   else
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next = NULL;
   _access_obj_process(obj, is_access);
}

static void
_elm_clock_smart_set_user(Elm_Clock_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_clock_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_clock_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_clock_smart_theme;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   /* access */
   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
     ELM_WIDGET_CLASS(sc)->focus_next = _elm_clock_smart_focus_next;

   ELM_WIDGET_CLASS(sc)->access = _elm_clock_smart_access;
}

EAPI const Elm_Clock_Smart_Class *
elm_clock_smart_class_get(void)
{
   static Elm_Clock_Smart_Class _sc =
     ELM_CLOCK_SMART_CLASS_INIT_NAME_VERSION(ELM_CLOCK_SMART_NAME);
   static const Elm_Clock_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_clock_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_clock_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_clock_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
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
   ELM_CLOCK_DATA_GET(obj, sd);

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
   ELM_CLOCK_DATA_GET(obj, sd);

   if (hrs) *hrs = sd->hrs;
   if (min) *min = sd->min;
   if (sec) *sec = sd->sec;
}

EAPI void
elm_clock_edit_set(Evas_Object *obj,
                   Eina_Bool edit)
{
   ELM_CLOCK_CHECK(obj);
   ELM_CLOCK_DATA_GET(obj, sd);

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
   ELM_CLOCK_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->edit;
}

EAPI void
elm_clock_edit_mode_set(Evas_Object *obj,
                        Elm_Clock_Edit_Mode digedit)
{
   ELM_CLOCK_CHECK(obj);
   ELM_CLOCK_DATA_GET(obj, sd);

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
   ELM_CLOCK_DATA_GET_OR_RETURN_VAL(obj, sd, 0);

   return sd->digedit;
}

EAPI void
elm_clock_show_am_pm_set(Evas_Object *obj,
                         Eina_Bool am_pm)
{
   ELM_CLOCK_CHECK(obj);
   ELM_CLOCK_DATA_GET(obj, sd);

   sd->am_pm = !!am_pm;
   _time_update(obj);
}

EAPI Eina_Bool
elm_clock_show_am_pm_get(const Evas_Object *obj)
{
   ELM_CLOCK_CHECK(obj) EINA_FALSE;
   ELM_CLOCK_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->am_pm;
}

EAPI void
elm_clock_show_seconds_set(Evas_Object *obj,
                           Eina_Bool seconds)
{
   ELM_CLOCK_CHECK(obj);
   ELM_CLOCK_DATA_GET(obj, sd);

   sd->seconds = !!seconds;
   _time_update(obj);
}

EAPI Eina_Bool
elm_clock_show_seconds_get(const Evas_Object *obj)
{
   ELM_CLOCK_CHECK(obj) EINA_FALSE;
   ELM_CLOCK_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->seconds;
}

EAPI void
elm_clock_first_interval_set(Evas_Object *obj,
                             double interval)
{
   ELM_CLOCK_CHECK(obj);
   ELM_CLOCK_DATA_GET(obj, sd);

   sd->first_interval = !!interval;
}

EAPI double
elm_clock_first_interval_get(const Evas_Object *obj)
{
   ELM_CLOCK_CHECK(obj) 0.0;
   ELM_CLOCK_DATA_GET_OR_RETURN_VAL(obj, sd, 0.0);

   return sd->first_interval;
}
