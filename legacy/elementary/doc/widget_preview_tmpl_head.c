#include <Elementary.h>

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg;
   Evas_Coord w, h;
   
   w = atoi(argv[1]);
   h = atoi(argv[2]);
   
   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);
