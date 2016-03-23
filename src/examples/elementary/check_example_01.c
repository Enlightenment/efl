//Compile with:
//gcc -o check_example_01  check_example_01.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void
_print(void *data, Evas_Object *obj, void *event_info)
{
   printf("check %smarked\n", *((Eina_Bool*)data) ? "" : "un");
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *cb, *cb2, *icon;
   Eina_Bool value;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("check", "Check");
   elm_win_autodel_set(win, EINA_TRUE);

   cb = elm_check_add(win);
   elm_object_text_set(cb, "checkbox");
   elm_check_state_pointer_set(cb, &value);
   evas_object_smart_callback_add(cb, "changed", _print, &value);
   evas_object_move(cb, 10, 10);
   evas_object_resize(cb, 200, 30);
   evas_object_show(cb);

   icon = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(icon, 0, 255, 0, 255);
   evas_object_resize(icon, 20, 20);
   evas_object_show(icon);

   cb2 = elm_check_add(win);
   elm_object_text_set(cb2, "another checkbox");
   elm_check_state_set(cb2, EINA_TRUE);
   elm_object_part_content_set(cb2, "icon", icon);
   evas_object_move(cb2, 10, 50);
   evas_object_resize(cb2, 200, 30);
   evas_object_show(cb2);

   evas_object_resize(win, 200, 100);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
