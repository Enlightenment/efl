#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void _changed_cb(void* data __UNUSED__, Evas_Object* obj, void* event_info)
{
   char buf[256];
   Elm_Dayselector_Day day = (Elm_Dayselector_Day) event_info;
   Eina_Bool checked = elm_dayselector_day_selected_get(obj, day);

   switch(day)
     {
      case ELM_DAYSELECTOR_SUN:
         snprintf(buf, sizeof(buf), "%s", "Sun");
         break;
      case ELM_DAYSELECTOR_MON:
         snprintf(buf, sizeof(buf), "%s", "Mon");
         break;
      case ELM_DAYSELECTOR_TUE:
         snprintf(buf, sizeof(buf), "%s", "Tue");
         break;
      case ELM_DAYSELECTOR_WED:
        snprintf(buf, sizeof(buf), "%s", "Wed");
        break;
      case ELM_DAYSELECTOR_THU:
         snprintf(buf, sizeof(buf), "%s", "Thu");
         break;
      case ELM_DAYSELECTOR_FRI:
         snprintf(buf, sizeof(buf), "%s", "Fri");
         break;
      case ELM_DAYSELECTOR_SAT:
         snprintf(buf, sizeof(buf), "%s", "Sat");
         break;
      default:
         snprintf(buf, sizeof(buf), "%s", "???");
         break;
     }
   fprintf(stderr, "%s = %d\n", buf, checked);
}

void
test_dayselector(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *dayselector, *sunday;

   win = elm_win_util_standard_add("dayselector", "Day Selector");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bx);

   // Default
   dayselector = elm_dayselector_add(win);
   evas_object_size_hint_weight_set(dayselector, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dayselector, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, dayselector);
   evas_object_show(dayselector);
   evas_object_smart_callback_add(dayselector, "dayselector,changed", _changed_cb, NULL);

   //Sunday first
   dayselector = elm_dayselector_add(win);
   evas_object_size_hint_weight_set(dayselector, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dayselector, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, dayselector);
   evas_object_show(dayselector);
   evas_object_smart_callback_add(dayselector, "dayselector,changed", _changed_cb, NULL);
   sunday = elm_object_part_content_get(dayselector, "day0");
   elm_object_signal_emit(sunday, "elm,type,weekend,style1", "");

   //Monday first
   dayselector = elm_dayselector_add(win);
   evas_object_size_hint_weight_set(dayselector, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dayselector, EVAS_HINT_FILL, 0.5);
   elm_dayselector_week_start_set(dayselector, ELM_DAYSELECTOR_MON);
   elm_box_pack_end(bx, dayselector);
   evas_object_show(dayselector);
   evas_object_smart_callback_add(dayselector, "dayselector,changed", _changed_cb, NULL);
   sunday = elm_object_part_content_get(dayselector, "day0");
   elm_object_signal_emit(sunday, "elm,type,weekend,style1", "");

   evas_object_resize(win, 350, 120);
   evas_object_show(win);
}

#endif
