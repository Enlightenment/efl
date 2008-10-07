#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *clk;
   Evas_Bool seconds : 1;
   Evas_Bool am_pm : 1;
   Evas_Bool edit : 1;
   int hrs, min, sec;
   Evas_Object *digit[6];
   Evas_Object *ampm;
   Ecore_Timer *ticker;
   struct {
      int hrs, min, sec;
      char ampm;
      char seconds;
      char am_pm;
      char edit;
   } cur;
};

static void _del_hook(Evas_Object *obj);
static int _ticker(void *data);
static void _signal_clock_val_up(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _signal_clock_val_down(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _time_update(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   int i;
   for (i = 0; i < 6; i++)
     {
	if (wd->digit[i]) evas_object_del(wd->digit[i]);
     }
   if (wd->ampm) evas_object_del(wd->ampm);
   if (wd->ticker) ecore_timer_del(wd->ticker);
   free(wd);
}

static int
_ticker(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   double          t;
   struct timeval  timev;
   struct tm      *tm;
   time_t          tt;

   gettimeofday(&timev, NULL);
   t = ((double)(1000000 - timev.tv_usec)) / 1000000.0;
   wd->ticker = ecore_timer_add(t, _ticker, data);
   if (!wd->edit)
     {
	tt = (time_t)(timev.tv_sec);
	tzset();
	tm = localtime(&tt);
	if (tm)
	  {
	     wd->hrs = tm->tm_hour;
	     wd->min = tm->tm_min;
	     wd->sec = tm->tm_sec;
	     _time_update(data);
	  }
     }
   return 0;
}

static void
_signal_clock_val_up(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd->edit) return;
   if (obj == wd->digit[0])
     {
	wd->hrs = wd->hrs + 10;
	if (wd->hrs >= 24) wd->hrs -= 24;
     }
   if (obj == wd->digit[1])
     {
	wd->hrs = wd->hrs + 1;
	if (wd->hrs >= 24) wd->hrs -= 24;
     }
   if (obj == wd->digit[2])
     {
	wd->min = wd->min + 10;
	if (wd->min >= 60) wd->min -= 60;
     }
   if (obj == wd->digit[3])
     {
	wd->min = wd->min + 1;
	if (wd->min >= 60) wd->min -= 60;
     }
   if (obj == wd->digit[4])
     {
	wd->sec = wd->sec + 10;
	if (wd->sec >= 60) wd->sec -= 60;
     }
   if (obj == wd->digit[5])
     {
	wd->sec = wd->sec + 1;
	if (wd->sec >= 60) wd->sec -= 60;
     }
   if (obj == wd->ampm)
     {
	wd->hrs = wd->hrs + 12;
	if (wd->hrs > 23) wd->hrs -= 24;
     }
   _time_update(data);
   evas_object_smart_callback_call(data, "changed", NULL);
}

static void
_signal_clock_val_down(void *data, Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd->edit) return;
   if (obj == wd->digit[0])
     {
	wd->hrs = wd->hrs - 10;
	if (wd->hrs < 0) wd->hrs += 24;
     }
   if (obj == wd->digit[1])
     {
	wd->hrs = wd->hrs - 1;
	if (wd->hrs < 0) wd->hrs += 24;
     }
   if (obj == wd->digit[2])
     {
	wd->min = wd->min - 10;
	if (wd->min < 0) wd->min += 60;
     }
   if (obj == wd->digit[3])
     {
	wd->min = wd->min - 1;
	if (wd->min < 0) wd->min += 60;
     }
   if (obj == wd->digit[4])
     {
	wd->sec = wd->sec - 10;
	if (wd->sec < 0) wd->sec += 60;
     }
   if (obj == wd->digit[5])
     {
	wd->sec = wd->sec - 1;
	if (wd->sec < 0) wd->sec += 60;
     }
   if (obj == wd->ampm)
     {
	wd->hrs = wd->hrs - 12;
	if (wd->hrs < 0) wd->hrs += 24;
     }
   _time_update(data);
   evas_object_smart_callback_call(data, "changed", NULL);
}

static void
_time_update(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Edje_Message_Int msg;
   int ampm = 0;

   if ((wd->cur.seconds != wd->seconds) || (wd->cur.am_pm != wd->am_pm) ||
       (wd->cur.edit != wd->edit))
     {
	int i;
	Evas_Coord mw, mh;

	for (i = 0; i < 6; i++)
	  {
	     if (wd->digit[i])
	       {
		  evas_object_del(wd->digit[i]);
		  wd->digit[i] = NULL;
	       }
	  }
	if (wd->ampm)
	  {
	     evas_object_del(wd->ampm);
	     wd->ampm = NULL;
	  }
	
	if ((wd->seconds) && (wd->am_pm))
	  _elm_theme_set(wd->clk, "clock", "clock/all");
	else if (wd->seconds)
	  _elm_theme_set(wd->clk, "clock", "clock/seconds");
	else if (wd->am_pm)
	  _elm_theme_set(wd->clk, "clock", "clock/am_pm");
	else
	  _elm_theme_set(wd->clk, "clock", "clock");
	
	for (i = 0; i < 6; i++)
	  {
	     char buf[16];

	     if ((!wd->seconds) && (i >= 4)) break;
	     wd->digit[i] = edje_object_add(evas_object_evas_get(wd->clk));
	     _elm_theme_set(wd->digit[i], "clock", "flipdigit");
	     if (wd->edit)
	       edje_object_signal_emit(wd->digit[i], "elm,state,edit,on", "elm");
	     edje_object_signal_callback_add(wd->digit[i], "elm,action,up", "",
					     _signal_clock_val_up, obj);
	     edje_object_signal_callback_add(wd->digit[i], "elm,action,down", "",
					     _signal_clock_val_down, obj);
	     edje_object_size_min_calc(wd->digit[i], &mw, &mh);
	     edje_extern_object_min_size_set(wd->digit[i], mw, mh);
	     snprintf(buf, sizeof(buf), "d%i", i);
	     edje_object_part_swallow(wd->clk , buf, wd->digit[i]);
	     evas_object_show(wd->digit[i]);
	  }
	if (wd->am_pm)
	  {
	     wd->ampm = edje_object_add(evas_object_evas_get(wd->clk));
	     _elm_theme_set(wd->ampm, "clock", "flipampm");
	     if (wd->edit)
	       edje_object_signal_emit(wd->ampm, "elm,state,edit,on", "elm");
	     edje_object_signal_callback_add(wd->ampm, "elm,action,up", "",
					     _signal_clock_val_up, obj);
	     edje_object_signal_callback_add(wd->ampm, "elm,action,down", "",
					     _signal_clock_val_down, obj);
	     edje_object_size_min_calc(wd->ampm, &mw, &mh);
	     edje_extern_object_min_size_set(wd->ampm, mw, mh);
	     edje_object_part_swallow(wd->clk , "ampm", wd->ampm);
	     evas_object_show(wd->ampm);
	  }
	
	edje_object_size_min_calc(wd->clk, &mw, &mh);
	evas_object_size_hint_min_set(obj, mw, mh);

	wd->cur.hrs = 0;
	wd->cur.min = 0;
	wd->cur.sec = 0;
	wd->cur.ampm = -1;
	wd->cur.seconds = wd->seconds;
	wd->cur.am_pm = wd->am_pm;
	wd->cur.edit = wd->edit;
     }
   if (wd->hrs != wd->cur.hrs)
     {
	int hrs;
	int d1, d2, dc1, dc2;

	hrs = wd->hrs;
	if (wd->am_pm)
	  {
	     if (hrs >= 12)
	       {
		  if (hrs > 12) hrs -= 12;
		  ampm = 1;
	       }
	     else if (hrs == 0) hrs = 12;
	  }
	d1 = hrs / 10;
	d2 = hrs % 10;
	dc1 = wd->cur.hrs / 10;
	dc2 = wd->cur.hrs % 10;
	if (d1 != dc1)
	  {
	     msg.val = d1;
	     edje_object_message_send(wd->digit[0], EDJE_MESSAGE_INT, 1, &msg);
	  }
	if (d2 != dc2)
	  {
	     msg.val = d2;
	     edje_object_message_send(wd->digit[1], EDJE_MESSAGE_INT, 1, &msg);
	  }
	wd->cur.hrs = hrs;
     }
   if (wd->min != wd->cur.min)
     {
	int d1, d2, dc1, dc2;
	
	d1 = wd->min / 10;
	d2 = wd->min % 10;
	dc1 = wd->cur.min / 10;
	dc2 = wd->cur.min % 10;
	if (d1 != dc1)
	  {
	     msg.val = d1;
	     edje_object_message_send(wd->digit[2], EDJE_MESSAGE_INT, 1, &msg);
	  }
	if (d2 != dc2)
	  {
	     msg.val = d2;
	     edje_object_message_send(wd->digit[3], EDJE_MESSAGE_INT, 1, &msg);
	  }
	wd->cur.min = wd->min;
     }
   if (wd->seconds)
     {
	if (wd->sec != wd->cur.sec)
	  {
	     int d1, d2, dc1, dc2;
	     
	     d1 = wd->sec / 10;
	     d2 = wd->sec % 10;
	     dc1 = wd->cur.sec / 10;
	     dc2 = wd->cur.sec % 10;
	     if (d1 != dc1)
	       {
		  msg.val = d1;
		  edje_object_message_send(wd->digit[4], EDJE_MESSAGE_INT, 1, &msg);
	       }
	     if (d2 != dc2)
	       {
		  msg.val = d2;
		  edje_object_message_send(wd->digit[5], EDJE_MESSAGE_INT, 1, &msg);
	       }
	     wd->cur.sec = wd->sec;
	  }
     }
   else
     wd->cur.sec = -1; 
   
   if (wd->am_pm)
     {
	if (wd->hrs >= 12) ampm = 1;
	if (ampm != wd->cur.ampm)
	  {
	     int d1, d2, dc1, dc2;
	     
	     if (wd->cur.ampm != ampm)
	       {
		  msg.val = ampm;
		  edje_object_message_send(wd->ampm, EDJE_MESSAGE_INT, 1, &msg);
	       }
	     wd->cur.ampm = ampm;
	  }
     }
   else
     wd->cur.ampm = -1;
}

EAPI Evas_Object *
elm_clock_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   
   wd->clk = edje_object_add(e);
   elm_widget_resize_object_set(obj, wd->clk);

   wd->cur.ampm = -1;
   wd->cur.seconds = -1;
   wd->cur.am_pm = -1;
   wd->cur.edit = -1;
   
   _time_update(obj);
   _ticker(obj);
   
   return obj;
}

EAPI void
elm_clock_time_set(Evas_Object *obj, int hrs, int min, int sec)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->hrs = hrs;
   wd->min = min;
   wd->sec = sec;
   _time_update(obj);
}

EAPI void
elm_clock_time_get(Evas_Object *obj, int *hrs, int *min, int *sec)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (hrs) *hrs = wd->hrs;
   if (min) *min = wd->min;
   if (sec) *sec = wd->sec;
}

EAPI void
elm_clock_edit_set(Evas_Object *obj, Evas_Bool edit)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->edit = edit;
   _time_update(obj);
}

EAPI void
elm_clock_show_am_pm_set(Evas_Object *obj, Evas_Bool am_pm)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->am_pm = am_pm;
   _time_update(obj);
}

EAPI void
elm_clock_show_seconds_set(Evas_Object *obj, Evas_Bool seconds)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->seconds = seconds;
   _time_update(obj);
}
