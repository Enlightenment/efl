/**
 * Simple Elementary's <b>dayselector widget</b> example, illustrating its
 * usage and API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g dayselector_example.c -o dayselector_example `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

/* Callback function when a day is clicked. */
static void _changed_cb(void* data, Evas_Object* obj, void* event_info)
{
   char buf[256];
   Elm_Dayselector_Day day;
   Eina_Bool selected;

   /* get which day's state is changed */
   day = (Elm_Dayselector_Day) event_info;
   /* get the state of corresponding day's check object */
   selected = elm_dayselector_day_selected_get(obj, day);
   switch(day)
     {
      case ELM_DAYSELECTOR_SUN: snprintf(buf, sizeof(buf), "%s", "Sun");   break;
      case ELM_DAYSELECTOR_MON: snprintf(buf, sizeof(buf), "%s", "Mon");   break;
      case ELM_DAYSELECTOR_TUE: snprintf(buf, sizeof(buf), "%s", "Tue");   break;
      case ELM_DAYSELECTOR_WED: snprintf(buf, sizeof(buf), "%s", "Wed");   break;
      case ELM_DAYSELECTOR_THU: snprintf(buf, sizeof(buf), "%s", "Thu");   break;
      case ELM_DAYSELECTOR_FRI: snprintf(buf, sizeof(buf), "%s", "Fri");   break;
      case ELM_DAYSELECTOR_SAT: snprintf(buf, sizeof(buf), "%s", "Sat");   break;
      default: snprintf(buf, sizeof(buf), "%s", "???"); break;
     }
   fprintf(stderr, "%s state is %d\n", buf, selected);
}
/* End of clicked callback */

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bx, *dayselector;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("dayselector", "Dayselector Example");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   /* add a dayselector widget showing weekdays starting from Sunday */
   dayselector = elm_dayselector_add(win);
   evas_object_size_hint_weight_set(dayselector, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dayselector, EVAS_HINT_FILL, 0.5);
   evas_object_smart_callback_add(dayselector, "dayselector,changed", _changed_cb, NULL);
   elm_box_pack_end(bx, dayselector);
   evas_object_show(dayselector);

   /* add a dayselector widget showing weekdays starting from Monday */
   dayselector = elm_dayselector_add(win);
   evas_object_size_hint_weight_set(dayselector, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(dayselector, EVAS_HINT_FILL, 0.5);
   evas_object_smart_callback_add(dayselector, "dayselector,changed", _changed_cb, NULL);
   elm_object_style_set(dayselector, "mon_first");
   elm_box_pack_end(bx, dayselector);
   evas_object_show(dayselector);

   evas_object_resize(win, 350, 120);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
