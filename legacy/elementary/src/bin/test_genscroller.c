#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

void
test_genscroller(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gl, *bx;

   win = elm_win_add(NULL, "genscroller", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genscroller");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genscroller_add(win);
   elm_genscroller_world_size_set(gl, 2000, 2000);
   evas_object_size_hint_weight_set(gl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(gl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, gl);
   evas_object_show(gl);

   evas_object_resize(win, 480, 800);
   evas_object_show(win);
}
#endif
