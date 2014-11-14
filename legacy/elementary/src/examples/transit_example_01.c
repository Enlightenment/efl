//Compile with:
//gcc -o transit_example_01 transit_example_01.c `pkg-config --cflags --libs elementary`

#include <Elementary.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bt;
   Elm_Transit *trans;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("transit-basic", "Transit - Basic");
   elm_win_autodel_set(win, EINA_TRUE);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Resizing Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 100);
   evas_object_resize(bt, 100, 50);

   trans = elm_transit_add();
   elm_transit_object_add(trans, bt);

   elm_transit_effect_resizing_add(trans, 100, 50, 300, 150);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
