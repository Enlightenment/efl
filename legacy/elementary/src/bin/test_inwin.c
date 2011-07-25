#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
void
test_inwin(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *inwin, *lb;

   win = elm_win_add(NULL, "inwin", ELM_WIN_BASIC);
   elm_win_title_set(win, "Inwin");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   inwin = elm_win_inwin_add(win);
   evas_object_show(inwin);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
		       "This is an \"inwin\" - a window in a<br>"
		       "window. This is handy for quick popups<br>"
		       "you want centered, taking over the window<br>"
		       "until dismissed somehow. Unlike hovers they<br>"
		       "don't hover over their target.");
   elm_win_inwin_content_set(inwin, lb);
   evas_object_show(lb);

   evas_object_resize(win, 320, 240);
   evas_object_show(win);
}

void
test_inwin2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *inwin, *lb;

   win = elm_win_add(NULL, "inwin2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Inwin 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   inwin = elm_win_inwin_add(win);
   elm_object_style_set(inwin, "minimal_vertical");
   evas_object_show(inwin);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
		       "This is an \"inwin\" - a window in a<br>"
		       "window. This is handy for quick popups<br>"
		       "you want centered, taking over the window<br>"
		       "until dismissed somehow. Unlike hovers they<br>"
		       "don't hover over their target.<br>"
		       "<br>"
		       "This inwin style compacts itself vertically<br>"
		       "to the size of its contents minimum size.");
   elm_win_inwin_content_set(inwin, lb);
   evas_object_show(lb);

   evas_object_resize(win, 320, 240);
   evas_object_show(win);
}
#endif
