//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` bg_example_03.c -o bg_example_03

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
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
   Evas_Object *win, *bg;

   win = elm_win_add(NULL, "bg-plain", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bg Plain");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   /* allow bg to expand in x & y */
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   /* set size hints. a minimum size for the bg. this should propagate back
    * to the window thus limiting its size based off the bg as the bg is one
    * of the window's resize objects. */
   evas_object_size_hint_min_set(bg, 160, 160);
   /* and set a maximum size. not needed very often. normally used together
    * with evas_object_size_hint_min_set() at the same size to make a
    * window not resizable */
   evas_object_size_hint_max_set(bg, 640, 640);
   /* and now just resize the window to a size you want. normally widgets
    * will determine the initial size though */
   evas_object_resize(win, 320, 320);
   /* and show the window */
   evas_object_show(win);

   elm_run();

   return 0;
}

ELM_MAIN()
