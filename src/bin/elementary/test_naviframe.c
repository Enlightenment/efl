#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define BUTTON_TEXT_SET(BT, TEXT) \
   elm_object_text_set((BT), (TEXT)); \
   elm_object_tooltip_text_set((BT), (TEXT)); \
   elm_object_tooltip_window_mode_set((BT), EINA_TRUE)

static char img1[PATH_MAX];
static char img2[PATH_MAX];
static char img3[PATH_MAX];
static char img4[PATH_MAX];
static char img5[PATH_MAX];
static char img6[PATH_MAX];
static char img7[PATH_MAX];

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
_navi_pop(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_naviframe_item_pop(data);
}

void
_navi_it_del(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_object_item_del(data);
}

void
_title_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("Title Clicked!\n");
}

void
_item_activated(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *it = event_info;
   printf("Item(%p) is activated! The Title is \"%s\"\n", it, elm_object_item_text_get(it));
}

void
_title_visible(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_naviframe_item_title_enabled_set(data,
                                        !elm_naviframe_item_title_enabled_get(data),
                                        EINA_TRUE);
}

void
_promote(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_naviframe_item_promote(data);
}

Eina_Bool
_pop_cb(void *data EINA_UNUSED, Elm_Object_Item *it)
{
   elm_object_item_del(it);

   /* If EINA_TRUE is returned, pop transition effect happens and then the item
    * is automatically deleted.
    * If EINA_FALSE is returned, pop transition effect does not happen and the
    * item is not automatically deleted.
    */
   return EINA_FALSE;
}

void
_page9(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *bt2, *nf = data;
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt, "Page 8");

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt2, "Page 1");
   evas_object_smart_callback_add(bt2, "clicked", _promote,
                                  evas_object_data_get(nf, "page1"));

   it = elm_naviframe_item_push(nf, "Page 9", bt, bt2, NULL, NULL);
   elm_object_item_part_text_set(it, "subtitle", "Callback for naviframe item pop is set");

   elm_naviframe_item_pop_cb_set(it, _pop_cb, NULL);

   evas_object_smart_callback_add(bt, "clicked", _navi_pop, nf);
}

void
_page8(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *bt2, *content, *nf = data;
   Elm_Object_Item *it;

   snprintf(img6, sizeof(img6), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt, "Page 7");

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt2, "Page 9");
   evas_object_smart_callback_add(bt2, "clicked", _page9, nf);

   content = _content_new(nf, img6);
   it = elm_naviframe_item_push(nf, "Page 8", bt, bt2, content, NULL);
   elm_object_item_part_text_set(it, "subtitle", "Overlap style!");

   evas_object_smart_callback_add(bt, "clicked", _navi_pop, nf);
}

static void
_page7_btn_down_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj,
                   void *event_info EINA_UNUSED)
{
   evas_object_color_set(obj, 100, 0, 0, 100);
   _page8(data, NULL, NULL);
}

static void
_page7_btn_up_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
                 void *event_info EINA_UNUSED)
{
   evas_object_color_set(obj, 255, 255, 255, 255);
   printf("Page7 Button Mouse Up!\n");
}

Evas_Object *
_page7_content_new(Evas_Object *nf)
{
   Evas_Object *bt;

   bt = elm_button_add(nf);
   elm_object_text_set(bt, "Page 8");
   evas_object_event_callback_add(bt, EVAS_CALLBACK_MOUSE_DOWN,
                                  _page7_btn_down_cb, nf);
   evas_object_event_callback_add(bt, EVAS_CALLBACK_MOUSE_UP,
                                  _page7_btn_up_cb, NULL);

   return bt;
}

void
_page7(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *bt2, *content, *nf = data;
   Elm_Object_Item *it;

   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt, "Page 6");

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt2, "Page 8");
   evas_object_smart_callback_add(bt2, "clicked", _page8, nf);
   content = _page7_content_new(nf);
   it = elm_naviframe_item_push(nf, "Page 7", bt, bt2, content, "overlap");
   elm_object_item_part_text_set(it, "subtitle", "Overlap style!");

   evas_object_smart_callback_add(bt, "clicked", _navi_pop, nf);
}

void
_page6(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *bt2, *content, *nf = data;
   Elm_Object_Item *it;

   snprintf(img7, sizeof(img7), "%s/images/sky_03.jpg", elm_app_data_dir_get());
   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt, "Page 5");

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt2, "Page 7");
   evas_object_smart_callback_add(bt2, "clicked", _page7, nf);

   content = _content_new(nf, img7);
   it = elm_naviframe_item_push(nf, "Page 6", bt, bt2, content, "overlap");
   elm_object_item_part_text_set(it, "subtitle", "Overlap style!");

   evas_object_smart_callback_add(bt, "clicked", _navi_pop, nf);
}

void
_page5(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *bt2, *content, *nf = data;
   Elm_Object_Item *it;

   snprintf(img5, sizeof(img5), "%s/images/sky_01.jpg", elm_app_data_dir_get());
   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt, "Page 4");

   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt2, "Page 6");
   evas_object_smart_callback_add(bt2, "clicked", _page6, nf);

   content = _content_new(nf, img5);
   it = elm_naviframe_item_insert_after(nf,
                                        elm_naviframe_top_item_get(nf),
                                        "Page 5",
                                        bt,
                                        bt2,
                                        content,
                                        NULL);
   elm_object_item_part_text_set(it, "subtitle", "This page is inserted after top item without transition");
   evas_object_smart_callback_add(bt, "clicked", _navi_it_del, it);
}

void
_page4(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *ic, *content, *nf = data;
   char buf[PATH_MAX];
   Elm_Object_Item *it;

   snprintf(img4, sizeof(img4), "%s/images/rock_02.jpg", elm_app_data_dir_get());
   ic = elm_icon_add(nf);
   elm_icon_standard_set(ic, "go-right");

   bt = elm_button_add(nf);
   evas_object_smart_callback_add(bt, "clicked", _page5, nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_content_set(bt, "icon", ic);

   content = _content_new(nf, img4);

   it = elm_naviframe_item_push(nf,
                                "Page 4",
                                NULL,
                                bt,
                                content,
                                NULL);
   elm_object_item_part_text_set(it, "subtitle", "Title area visibility test");

   ic = elm_icon_add(nf);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png",
            elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_object_item_part_content_set(it, "icon", ic);
   elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
   evas_object_smart_callback_add(content, "clicked", _title_visible, it);
}

void
_page3(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt2, *content, *nf = data;

   snprintf(img3, sizeof(img3), "%s/images/rock_01.jpg", elm_app_data_dir_get());
   bt2 = elm_button_add(nf);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   BUTTON_TEXT_SET(bt2, "Next");
   evas_object_smart_callback_add(bt2, "clicked", _page4, nf);

   content = _content_new(nf, img3);

   elm_naviframe_item_push(nf,
                           "Page 3",
                           NULL,
                           bt2,
                           content,
                           NULL);
}

void
_page2(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bt, *content, *ic, *nf = data;
   Elm_Object_Item *it;

   snprintf(img2, sizeof(img2), "%s/images/plant_01.jpg", elm_app_data_dir_get());
   bt = elm_button_add(nf);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(bt, "clicked", _page3, nf);

   ic = elm_icon_add(nf);
   elm_icon_standard_set(ic, "arrow_right");
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_layout_content_set(bt, "icon", ic);

   content = _content_new(nf, img2);

   it = elm_naviframe_item_push(nf, "Page 2 - Long Title Here",
                                NULL, bt, content,  NULL);
   elm_object_item_part_text_set(it, "subtitle", "Here is sub-title part!");
}

void
test_naviframe(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *nf, *btn, *content;
   Elm_Object_Item *it;

   snprintf(img1, sizeof(img1), "%s/images/logo.png", elm_app_data_dir_get());
   win = elm_win_util_standard_add("naviframe", "Naviframe");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);

   nf = elm_naviframe_add(win);
   evas_object_size_hint_weight_set(nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, nf);
   evas_object_show(nf);
   evas_object_smart_callback_add(nf, "title,clicked", _title_clicked, 0);
   evas_object_smart_callback_add(nf, "item,activated", _item_activated, NULL);

   btn = elm_button_add(nf);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(btn, "clicked", _page2, nf);
   BUTTON_TEXT_SET(btn, "Next");
   evas_object_show(btn);

   content = _content_new(nf, img1);
   it = elm_naviframe_item_push(nf, "Page 1", NULL, btn, content, NULL);
   evas_object_data_set(nf, "page1", it);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

void
test_naviframe2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *nf, *sc, *btn, *ico, *content;
   Elm_Object_Item *it;

   snprintf(img1, sizeof(img1), "%s/images/logo.png", elm_app_data_dir_get());
   win = elm_win_util_standard_add("naviframe", "Naviframe");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);

   nf = elm_naviframe_add(win);
   evas_object_size_hint_weight_set(nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, nf);
   evas_object_show(nf);

   sc = elm_segment_control_add(nf);
   elm_segment_control_item_add(sc, NULL, "Show All");
   elm_segment_control_item_add(sc, NULL, "Just Filtered");

   btn = elm_button_add(nf);
   ico = elm_icon_add(btn);
   elm_icon_standard_set(ico, "refresh");
   elm_layout_content_set(btn, "icon", ico);

   content = _content_new(nf, img1);
   it = elm_naviframe_item_push(nf, NULL, NULL, btn, content, NULL);
   evas_object_data_set(nf, "page1", it);

   elm_object_item_part_content_set(it, "icon", sc);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

static void
_bt_pop_all(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it = evas_object_data_get(obj, "root");

   while (elm_naviframe_top_item_get(data) != it)
     elm_naviframe_item_pop(data);
}

void
test_naviframe3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *tb, *bxh, *nf, *btn, *content;
   Elm_Object_Item *it;

   snprintf(img1, sizeof(img1), "%s/images/logo.png", elm_app_data_dir_get());
   win = elm_win_util_standard_add("naviframe", "Naviframe");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);

   bxh = elm_box_add(win);
   evas_object_size_hint_weight_set(bxh, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(bxh, EINA_TRUE);
   elm_win_resize_object_add(win, bxh);
   evas_object_show(bxh);

   tb = elm_toolbar_add(win);
   evas_object_size_hint_weight_set(tb, 0.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_toolbar_horizontal_set(tb, EINA_FALSE);
   elm_toolbar_select_mode_set(tb, ELM_OBJECT_SELECT_MODE_ALWAYS);
   elm_box_pack_end(bxh, tb);
   evas_object_show(tb);

   nf = elm_naviframe_add(win);
   evas_object_size_hint_weight_set(nf, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(nf, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bxh, nf);
   evas_object_show(nf);

   btn = elm_button_add(nf);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(btn, "clicked", _page2, nf);
   BUTTON_TEXT_SET(btn, "Next");
   evas_object_show(btn);

   content = _content_new(nf, img1);
   it = elm_naviframe_item_push(nf, "Page 1", NULL, btn, content, NULL);
   evas_object_data_set(nf, "page1", it);

   evas_object_data_set(tb, "root", it);
   elm_toolbar_item_append(tb, NULL, "Pop all", _bt_pop_all, nf);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
