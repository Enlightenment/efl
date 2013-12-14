#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#endif
#include <Elementary.h>

static void _changed_cb(void* data EINA_UNUSED, Evas_Object* obj, void* event_info)
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
test_dayselector(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *dayselector, *monday;
   Eina_List *weekdays_list;
   const char *weekday;
   const char *weekdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
   const char *weekdays2[] = {"S", "M", "T", "W", "T", "F", "S"};

   win = elm_win_util_standard_add("dayselector", "Day Selector");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   // Default
   dayselector = elm_dayselector_add(bx);
   evas_object_size_hint_weight_set(dayselector, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dayselector, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, dayselector);
   evas_object_show(dayselector);
   evas_object_smart_callback_add(dayselector, "dayselector,changed",
                                  _changed_cb, NULL);

   //Sunday first
   dayselector = elm_dayselector_add(bx);
   evas_object_size_hint_weight_set(dayselector, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dayselector, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, dayselector);
   evas_object_show(dayselector);
   evas_object_smart_callback_add(dayselector, "dayselector,changed",
                                  _changed_cb, NULL);
   elm_dayselector_weekdays_names_set(dayselector, weekdays);
   elm_dayselector_week_start_set(dayselector, ELM_DAYSELECTOR_SUN);

   //Special Style
   dayselector = elm_dayselector_add(bx);
   evas_object_size_hint_weight_set(dayselector, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dayselector, EVAS_HINT_FILL, 0.5);
   elm_dayselector_week_start_set(dayselector, ELM_DAYSELECTOR_MON);
   elm_box_pack_end(bx, dayselector);
   evas_object_show(dayselector);
   evas_object_smart_callback_add(dayselector, "dayselector,changed",
                                  _changed_cb, NULL);
   monday = elm_object_part_content_get(dayselector, "day0");
   elm_object_signal_emit(monday, "elm,type,weekend,style1", "");

   //Setting weekday name
   dayselector = elm_dayselector_add(bx);
   evas_object_size_hint_weight_set(dayselector, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dayselector, EVAS_HINT_FILL, 0.5);
   elm_dayselector_weekdays_names_set(dayselector, weekdays2);
   elm_box_pack_end(bx, dayselector);
   evas_object_show(dayselector);
   evas_object_smart_callback_add(dayselector, "dayselector,changed",
                                  _changed_cb, NULL);

   weekdays_list = elm_dayselector_weekdays_names_get(dayselector);

   fprintf(stderr, "User set weekday names to: ");
   EINA_LIST_FREE(weekdays_list, weekday)
     {
        fprintf(stderr, "%s\n", weekday);
        eina_stringshare_del(weekday);
     }

   //Disabled
   dayselector = elm_dayselector_add(bx);
   evas_object_size_hint_weight_set(dayselector, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dayselector, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, dayselector);
   evas_object_show(dayselector);
   elm_object_disabled_set(dayselector, EINA_TRUE);

   evas_object_resize(win, 350, 150);
   evas_object_show(win);
}
