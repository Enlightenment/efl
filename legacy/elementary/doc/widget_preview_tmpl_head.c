#include <Elementary.h>

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win;
   Evas_Coord w, h;
   
   w = atoi(argv[1]);
   h = atoi(argv[2]);
   
   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   elm_win_alpha_set(win, EINA_TRUE);
   
