#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

void 
test_panel(void *data, Evas_Object *obj, void *event_info) 
{
   Evas_Object *win, *bg, *panel;

   win = elm_win_add(NULL, "panel", ELM_WIN_BASIC);
   elm_win_title_set(win, "Panel");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   panel = elm_panel_add(win);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_LEFT);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(panel);

   evas_object_resize(win, 300, 300);
   evas_object_show(win);
}

#endif
