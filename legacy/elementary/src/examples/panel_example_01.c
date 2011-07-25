//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` panel_example_01.c -o panel_example_01

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

EAPI int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bg, *bx, *panel, *content;

   win = elm_win_add(NULL, "panel", ELM_WIN_BASIC);
   elm_win_title_set(win, "Panel");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_FALSE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   panel = elm_panel_add(win);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_TOP);
   elm_panel_toggle(panel);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(panel);
   elm_box_pack_end(bx, panel);

   content = elm_label_add(win);
   elm_object_text_set(content, "content");
   evas_object_show(content);
   evas_object_size_hint_min_set(content, 100, 30);
   elm_panel_content_set(panel, content);

   panel = elm_panel_add(win);
   elm_panel_orient_set(panel, ELM_PANEL_ORIENT_RIGHT);
   elm_panel_hidden_set(panel, EINA_TRUE);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(panel);
   elm_box_pack_end(bx, panel);

   content = elm_label_add(win);
   elm_object_text_set(content, "content2");
   evas_object_show(content);
   evas_object_size_hint_min_set(content, 100, 30);
   elm_panel_content_set(panel, content);

   panel = elm_panel_add(win);
   evas_object_size_hint_weight_set(panel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panel, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(panel);
   elm_box_pack_end(bx, panel);

   content = elm_label_add(win);
   elm_object_text_set(content, "content2");
   evas_object_show(content);
   evas_object_size_hint_min_set(content, 100, 30);
   elm_panel_content_set(panel, content);
   

   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
