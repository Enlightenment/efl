#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

void
_navi_pop(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_naviframe_item_pop(data);
}

void
_title_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("Title Clicked!\n");
}

void
_title_visible(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_naviframe_item_title_visible_set(data,
                               !elm_naviframe_item_title_visible_get(data));
}

void
_page4(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *bt, *bt2, *ic, *nf = data;
   char buf[PATH_MAX];
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/icon_right_arrow.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_button_icon_set(bt, ic);

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(bt2, "Naviframe Test");

   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   it = elm_naviframe_item_push(nf,
                                "Page 4",
                                NULL,
                                bt,
                                bt2,
                                NULL);
   elm_naviframe_item_subtitle_label_set(it, "Here is sub-title part!");
   elm_naviframe_item_icon_set(it, ic);
   elm_naviframe_item_title_visible_set(it, EINA_FALSE);
   evas_object_smart_callback_add(bt2, "clicked", _title_visible, it);
}

void
_page3(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *bt, *bt2, *bt3, *ic, *nf = data;
   char buf[PATH_MAX];
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(bt, "Prev");
   evas_object_smart_callback_add(bt, "clicked", _navi_pop, nf);

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(bt2, "Next");
   evas_object_smart_callback_add(bt2, "clicked", _page4, nf);

   bt3 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt3, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(bt3, "Naviframe Test");

   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   it = elm_naviframe_item_push(nf,
                                "Page 3",
                                bt,
                                bt2,
                                bt3,
                                NULL);
   elm_naviframe_item_subtitle_label_set(it, "Here is sub-title part!");
   elm_naviframe_item_icon_set(it, ic);
}

void
_page2(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *bt, *bt2, *ic, *nf = data;
   char buf[PATH_MAX];
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(bt, "clicked", _page3, nf);

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(bt2, "Naviframe Test");

   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/icon_right_arrow.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_button_icon_set(bt, ic);

   it = elm_naviframe_item_push(nf,
                                "Page 2",
                                NULL,
                                bt,
                                bt2,
                                NULL);
   elm_naviframe_item_subtitle_label_set(it, "Here is sub-title part!");
}

void
test_naviframe(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *nf, *btn;

   win = elm_win_add(NULL, "naviframe", ELM_WIN_BASIC);
   elm_win_title_set(win, "Naviframe");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   nf = elm_naviframe_add(win);
   elm_win_resize_object_add(win, nf);
   evas_object_show(nf);
   evas_object_smart_callback_add(nf, "title,clicked", _title_clicked, 0);

   btn = elm_button_add(nf);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(btn, "clicked", _page2, nf);
   elm_object_text_set(btn, "Naviframe Test");
   evas_object_show(btn);

   elm_naviframe_item_push(nf,
                           "Page 1",
                           NULL,
                           NULL,
                           btn,
                           NULL);

   evas_object_resize(win, 400, 600);
   evas_object_show(win);
}
#endif
