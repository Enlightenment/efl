//Compile with:
//gcc -o check_example_02  check_example_02.c -g `pkg-config --cflags --libs elementary` && ./check_example_02

#include <Elementary.h>

Evas_Object *cb, *cb1, *cb2, *cb3;

static void
_print(void *data, Evas_Object *obj, void *event_info)
{
   if (!elm_check_three_state_mode_get(cb))
     {
        printf("check0 %smarked\n", *((Eina_Bool*)data) ? "" : "un");
     }
   else
     {
        printf("check0 elm_check_state_get() value is %d (%s)\n", elm_check_state_get(cb), *((Eina_Bool*)data) ? "EINA_TRUE" : "EINA_FALSE");
     }
}

static void
_print1(void *data, Evas_Object *obj, void *event_info)
{
   Check_State st = elm_check_state_get(cb1);
   if (st == _CHECK_STATE_CHECKED)
     printf("check1 elm_check_state_get() value is %d (%s)\n", st, "_CHECK_STATE_CHECKED");
   else if (st == _CHECK_STATE_INDETERMINATE)
     printf("check1 elm_check_state_get() value is %d (%s)\n", st, "_CHECK_STATE_INDETERMINATE");
   else
     printf("check1 elm_check_state_get() value is %d (%s)\n", st, "_CHECK_STATE_UNCHECKED");
}

static void
_print2(void *data, Evas_Object *obj, void *event_info)
{
   printf("Currently three state mode is: %s\n", elm_check_three_state_mode_get(cb) ? "On" : "Off");
   elm_check_three_state_mode_set(cb, elm_check_state_get(cb2));
   elm_check_three_state_mode_set(cb1, elm_check_state_get(cb2));
   printf("Three state mode is changed to: %s\n", elm_check_three_state_mode_get(cb) ? "On" : "Off");
}

static void
_print3(void *data, Evas_Object *obj, void *event_info)
{
   elm_object_disabled_set(cb, elm_check_state_get(cb3));
   elm_object_disabled_set(cb1, elm_check_state_get(cb3));
   printf("Checkbox disable mode is : %s\n", elm_check_state_get(cb3) ? "Enabled" : "Disabled");
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *icon;
   Eina_Bool value, *value2, *value3;
   Check_State *value1;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("check", "Check");
   elm_win_autodel_set(win, EINA_TRUE);

   cb = elm_check_add(win);
   elm_object_text_set(cb, "check0 using bool");
   elm_check_state_pointer_set(cb, &value);
   elm_check_state_set(cb, EINA_FALSE);
   evas_object_smart_callback_add(cb, "changed", _print, &value);
   evas_object_move(cb, 10, 10);
   evas_object_resize(cb, 250, 30);
   evas_object_show(cb);

   cb1 = elm_check_add(win);
   elm_object_text_set(cb1, "check1 using enum");
   elm_check_state_pointer_set(cb1, (Check_State *)&value1);
   elm_check_state_set(cb1, (Check_State)_CHECK_STATE_CHECKED);
   evas_object_smart_callback_add(cb1, "changed", _print1, &value1);
   evas_object_move(cb1, 10, 50);
   evas_object_resize(cb1, 250, 30);
   evas_object_show(cb1);

   icon = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(icon, 0, 255, 0, 255);
   evas_object_resize(icon, 20, 20);
   evas_object_show(icon);

   cb2 = elm_check_add(win);
   elm_object_text_set(cb2, "Enable three state mode in above checkboxes");
   elm_check_state_pointer_set(cb2, (Check_State *)&value2);
   elm_check_state_set(cb2, (Check_State)_CHECK_STATE_UNCHECKED);
   evas_object_smart_callback_add(cb2, "changed", _print2, &value2);
   elm_object_part_content_set(cb2, "icon", icon);
   evas_object_move(cb2, 10, 90);
   evas_object_resize(cb2, 250, 30);
   evas_object_show(cb2);

   cb3 = elm_check_add(win);
   elm_object_text_set(cb3, "Enable/Disable checkboxes");
   elm_check_state_pointer_set(cb3, (Check_State *)&value3);
   elm_check_state_set(cb3, (Check_State)_CHECK_STATE_UNCHECKED);
   evas_object_smart_callback_add(cb3, "changed", _print3, &value3);
   evas_object_move(cb3, 10, 130);
   evas_object_resize(cb3, 250, 30);
   evas_object_show(cb3);

   evas_object_resize(win, 200, 190);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
