//Compile with:
//gcc -o bg_example_01 bg_example_01.c -g `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static void
on_done(void *data, Evas_Object *obj, void *event_info)
{
   /* quit the mainloop (elm_run) */
   elm_exit();
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg;

   win = elm_win_add(NULL, "bg-plain", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bg Plain");
   evas_object_smart_callback_add(win, "delete,request", on_done, NULL);
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   /* allow bg to expand in x & y */
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   /* and now just resize the window to a size you want. normally widgets
    * will determine the initial size though */
   evas_object_resize(win, 320, 320);
   /* and show the window */
   evas_object_show(win);

   elm_run(); /* and run the program now, starting to handle all
               * events, etc. */
   elm_shutdown(); /* clean up and shut down */

   /* exit code */
   return 0;
}
ELM_MAIN()
