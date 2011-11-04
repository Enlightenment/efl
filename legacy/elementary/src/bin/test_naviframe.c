#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static const char *img1 = PACKAGE_DATA_DIR"/images/logo.png";
static const char *img2 = PACKAGE_DATA_DIR"/images/plant_01.jpg";
static const char *img3 = PACKAGE_DATA_DIR"/images/rock_01.jpg";
static const char *img4 = PACKAGE_DATA_DIR"/images/rock_02.jpg";
static const char *img5 = PACKAGE_DATA_DIR"/images/sky_01.jpg";

Evas_Object *
_content_new(Evas_Object *parent, const char *img)
{
   Evas_Object *photo = elm_photo_add(parent);
   elm_photo_file_set(photo, img);
   elm_photo_fill_inside_set(photo, EINA_TRUE);
   elm_object_style_set(photo, "shadow");
   return photo;
}

void
_navi_pop(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_naviframe_item_pop(data);
}

void
_navi_it_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_naviframe_item_del(data);
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
_page5(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *bt, *en, *content, *nf = data;
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(bt, "Prev");

   content = _content_new(nf, img5);
   it = elm_naviframe_item_insert_after(elm_naviframe_top_item_get(nf),
                                        "Page 5",
                                        bt,
                                        NULL,
                                        content,
                                        NULL);

   evas_object_smart_callback_add(bt, "clicked", _navi_it_del, it);
}

void
_page4(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *bt, *ic, *content, *nf = data;
   char buf[PATH_MAX];
   Elm_Object_Item *it;

   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/icon_right_arrow.png",
            elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bt = elm_button_add(nf);
   evas_object_smart_callback_add(bt, "clicked", _page5, nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(bt, ic);

   content = _content_new(nf, img4);

   it = elm_naviframe_item_push(nf,
                                "Page 4",
                                NULL,
                                bt,
                                content,
                                NULL);
   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_object_item_content_part_set(it, ELM_NAVIFRAME_ITEM_CONTENT_ICON, ic);
   elm_naviframe_item_title_visible_set(it, EINA_FALSE);
   evas_object_smart_callback_add(content, "clicked", _title_visible, it);
}

void
_page3(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *bt, *bt2, *ic, *content, *nf = data;
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

   content = _content_new(nf, img3);

   it = elm_naviframe_item_push(nf,
                                "Page 3",
                                bt,
                                bt2,
                                content,
                                NULL);
   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   elm_object_item_content_part_set(it, ELM_NAVIFRAME_ITEM_CONTENT_ICON, ic);
}

void
_page2(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *bt, *content, *ic, *nf = data;
   char buf[PATH_MAX];
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(bt, "clicked", _page3, nf);

   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/icon_right_arrow.png",
            elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_object_content_set(bt, ic);

   content = _content_new(nf, img2);

   it = elm_naviframe_item_push(nf, "Page 2", NULL, bt, content,  NULL);
   elm_object_item_text_part_set(it, ELM_NAVIFRAME_ITEM_TEXT_SUBTITLE,
                                 "Here is sub-title part!");
}

void
test_naviframe(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *nf, *btn, *content;

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
   evas_object_size_hint_weight_set(nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(nf);
   evas_object_smart_callback_add(nf, "title,clicked", _title_clicked, 0);

   btn = elm_button_add(nf);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(btn, "clicked", _page2, nf);
   elm_object_text_set(btn, "Next");
   evas_object_show(btn);

   content = _content_new(nf, img1);
   elm_naviframe_item_push(nf, "Page 1", NULL, btn, content, NULL);

   evas_object_resize(win, 400, 600);
   evas_object_show(win);
}
#endif
