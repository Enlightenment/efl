#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

#include <Elementary.h>

/* FIXME: it shouldn't be required. For unknown reason map won't call
 * pan_calculate until shot delay time, but then it will take a screenshot
 * when the map isn't loaded yet (actually it won't be downloaded, because
 * after the SS it will kill the preview). */
static Eina_Bool
_nasty_hack(void *data)
{
   Evas_Object *o = data;
   Evas *e = evas_object_evas_get(o);
   evas_smart_objects_calculate(e);
   return ECORE_CALLBACK_CANCEL;
}

EAPI_MAIN int
elm_main(int argc __UNUSED__, char **argv)
{
   Evas_Object *win, *bg;
   Evas_Coord w, h;
   Evas_Object *o;

   w = atoi(argv[1]);
   h = atoi(argv[2]);

   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   o = elm_map_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, o);
   evas_object_show(o);

   ecore_timer_add(0.5, _nasty_hack, win);

#include "widget_preview_tmpl_foot.c"
