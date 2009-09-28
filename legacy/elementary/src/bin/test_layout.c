#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
void
test_layout(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *ly, *bt;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "layout", ELM_WIN_BASIC);
   elm_win_title_set(win, "Layout");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", PACKAGE_DATA_DIR);
   elm_layout_file_set(ly, buf, "layout");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button 1");
   elm_layout_content_set(ly, "element1", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button 2");
   elm_layout_content_set(ly, "element2", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button 3");
   elm_layout_content_set(ly, "element3", bt);
   evas_object_show(bt);

   evas_object_show(win);
}
#endif
