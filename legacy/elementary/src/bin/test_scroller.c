#include <Elementary.h>

void
test_scroller(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *tb, *bt, *sc;
   int i, j;
   char buf[64];

   win = elm_win_add(NULL, "scroller", ELM_WIN_BASIC);
   elm_win_title_set(win, "Scroller");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, 1.0, 1.0);

   for (j = 0; j < 24; j++)
     {
        for (i = 0; i < 12; i++)
          {
             snprintf(buf, sizeof(buf), "[%02i, %02i]\n", i, j);
             bt = elm_button_add(win);
             elm_button_label_set(bt, buf);
             evas_object_size_hint_weight_set(bt, 1.0, 1.0);
             evas_object_size_hint_align_set(bt, -1.0, -1.0);
             elm_table_pack(tb, bt, i, j, 1, 1);
             evas_object_show(bt);
          }
     }
   
   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   elm_win_resize_object_add(win, sc);

   elm_scroller_content_set(sc, tb);
   evas_object_show(tb);

   elm_scroller_page_relative_set(sc, 1.0, 1.0);
//   elm_scroller_page_size_set(sc, 200, 200);
   evas_object_show(sc);
   
   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}
