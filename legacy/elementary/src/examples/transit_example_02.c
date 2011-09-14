//Compile with:
//gcc -g -DPACKAGE_DATA_DIR="\"<directory>\"" `pkg-config --cflags --libs elementary` transit_example_02.c -o transit_example_02
// where directory is the a path where images/plant_01.jpg can be found.

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static void
on_done(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   /* quit the mainloop (elm_run) */
   elm_exit();
}

int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *bt, *bt2, *bt3, *bt4;
   Elm_Transit *trans, *trans2, *trans3, *trans4;

   win = elm_win_add(NULL, "transit9", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 9");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Chain 1");
   evas_object_resize(bt, 100, 100);
   evas_object_move(bt, 0, 0);
   evas_object_show(bt);

   bt2 = elm_button_add(win);
   elm_object_text_set(bt2, "Chain 2");
   evas_object_resize(bt2, 100, 100);
   evas_object_move(bt2, 300, 0);
   evas_object_show(bt2);

   bt3 = elm_button_add(win);
   elm_object_text_set(bt3, "Chain 3");
   evas_object_resize(bt3, 100, 100);
   evas_object_move(bt3, 300, 300);
   evas_object_show(bt3);

   bt4 = elm_button_add(win);
   elm_object_text_set(bt4, "Chain 4");
   evas_object_resize(bt4, 100, 100);
   evas_object_move(bt4, 0, 300);
   evas_object_show(bt4);

   trans = elm_transit_add();
   elm_transit_tween_mode_set(trans, ELM_TRANSIT_TWEEN_MODE_ACCELERATE);
   elm_transit_effect_translation_add(trans, 0, 0, 300, 0);
   elm_transit_object_add(trans, bt);
   elm_transit_duration_set(trans, 1);
   elm_transit_objects_final_state_keep_set(trans, EINA_TRUE);
   elm_transit_go(trans);

   trans2 = elm_transit_add();
   elm_transit_tween_mode_set(trans2, ELM_TRANSIT_TWEEN_MODE_ACCELERATE);
   elm_transit_effect_translation_add(trans2, 0, 0, 0, 300);
   elm_transit_object_add(trans2, bt2);
   elm_transit_duration_set(trans2, 1);
   elm_transit_objects_final_state_keep_set(trans2, EINA_TRUE);
   elm_transit_chain_transit_add(trans, trans2);

   trans3 = elm_transit_add();
   elm_transit_tween_mode_set(trans3, ELM_TRANSIT_TWEEN_MODE_ACCELERATE);
   elm_transit_effect_translation_add(trans3, 0, 0, -300, 0);
   elm_transit_object_add(trans3, bt3);
   elm_transit_duration_set(trans3, 1);
   elm_transit_objects_final_state_keep_set(trans3, EINA_TRUE);
   elm_transit_chain_transit_add(trans2, trans3);

   trans4 = elm_transit_add();
   elm_transit_tween_mode_set(trans4, ELM_TRANSIT_TWEEN_MODE_ACCELERATE);
   elm_transit_effect_translation_add(trans4, 0, 0, 0, -300);
   elm_transit_object_add(trans4, bt4);
   elm_transit_duration_set(trans4, 1);
   elm_transit_objects_final_state_keep_set(trans4, EINA_TRUE);
   elm_transit_chain_transit_add(trans3, trans4);

   elm_run();

   return 0;
}

ELM_MAIN()
