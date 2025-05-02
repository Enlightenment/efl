#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
my_fl_1(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_ROTATE_Y_CENTER_AXIS);
}

void
my_fl_2(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_ROTATE_X_CENTER_AXIS);
}

void
my_fl_3(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_ROTATE_XZ_CENTER_AXIS);
}

void
my_fl_4(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_ROTATE_YZ_CENTER_AXIS);
}

void
my_fl_5(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_CUBE_LEFT);
}

void
my_fl_6(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_CUBE_RIGHT);
}

void
my_fl_7(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_CUBE_UP);
}

void
my_fl_8(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_CUBE_DOWN);
}

void
my_fl_9(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_CROSS_FADE);
}

void
my_fl_front(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = data;
   elm_flip_go_to(fl, EINA_TRUE, ELM_FLIP_ROTATE_X_CENTER_AXIS);
}

void
my_fl_back(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = data;
   elm_flip_go_to(fl, EINA_FALSE, ELM_FLIP_ROTATE_X_CENTER_AXIS);
}

static void
_animations_ck_changed(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Eo *fl = data;
   char buf[PATH_MAX];
   if (elm_check_state_get(obj))
     {
        Eo *ly = elm_layout_add(fl), *bt;
        snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
        elm_layout_file_set(ly, buf, "layout");
        evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_object_part_content_set(fl, "back", ly);
        evas_object_show(ly);

        bt = elm_button_add(ly);
        elm_object_text_set(bt, "Button 1");
        elm_object_part_content_set(ly, "element1", bt);

        bt = elm_button_add(ly);
        elm_object_text_set(bt, "Button 2");
        elm_object_part_content_set(ly, "element2", bt);

        bt = elm_button_add(ly);
        elm_object_text_set(bt, "Button 3");
        elm_object_part_content_set(ly, "element3", bt);
     }
   else
     {
        Eo *o = elm_bg_add(fl);
        evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
        elm_bg_file_set(o, buf, NULL);
        elm_object_part_content_set(fl, "back", o);
        evas_object_show(o);
     }
}

void
test_flip(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bx2, *fl, *o, *bt, *ck;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("flip", "Flip");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fl = elm_flip_add(win);
   evas_object_size_hint_align_set(fl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(fl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, fl);

   o = elm_bg_add(win);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), "sky_01.jpg");
   elm_bg_file_set(o, buf, NULL);
   elm_object_part_content_set(fl, "front", o);
   evas_object_show(o);

   evas_object_show(fl);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "1");
   evas_object_smart_callback_add(bt, "clicked", my_fl_1, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "2");
   evas_object_smart_callback_add(bt, "clicked", my_fl_2, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "3");
   evas_object_smart_callback_add(bt, "clicked", my_fl_3, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "4");
   evas_object_smart_callback_add(bt, "clicked", my_fl_4, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "5");
   evas_object_smart_callback_add(bt, "clicked", my_fl_5, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "6");
   evas_object_smart_callback_add(bt, "clicked", my_fl_6, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "7");
   evas_object_smart_callback_add(bt, "clicked", my_fl_7, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "8");
   evas_object_smart_callback_add(bt, "clicked", my_fl_8, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "9");
   evas_object_smart_callback_add(bt, "clicked", my_fl_9, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);

   ck = elm_check_add(bx2);
   elm_object_style_set(ck, "toggle");
   elm_object_text_set(ck, "Allow animations");
   elm_check_state_set(ck, EINA_TRUE);
   evas_object_smart_callback_add(ck, "changed", _animations_ck_changed, fl);
   elm_box_pack_end(bx2, ck);
   evas_object_show(ck);
   _animations_ck_changed(fl, ck, NULL);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320 * elm_config_scale_get(),
                           480 * elm_config_scale_get());
   evas_object_show(win);
}

void
test_flip2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *bx, *bx2, *fl, *o, *bt, *tb, *li, *en;
   char buf[4096];

   win = elm_win_add(NULL, "flip2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Flip 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), "sky_01.jpg");
   elm_bg_file_set(bg, buf, NULL);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fl = elm_flip_add(win);
   evas_object_size_hint_align_set(fl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(fl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, fl);

   o = elm_box_add(win);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   tb = elm_toolbar_add(win);
   elm_toolbar_homogeneous_set(tb, EINA_FALSE);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);

   elm_toolbar_item_append(tb, "arrow_left", "Hello", NULL, NULL);
   elm_toolbar_item_append(tb, "arrow_right", "Out", NULL, NULL);
   elm_toolbar_item_append(tb, "home", "There", NULL, NULL);
   elm_toolbar_item_append(tb, "refresh", "World", NULL, NULL);

   elm_box_pack_end(o, tb);
   evas_object_show(tb);

   li = elm_list_add(win);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   elm_list_item_append(li, "This is a list", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Second item", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "3rd", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Fourth", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Number five is alive!", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "The quick brown fox jumps over the lazy dog", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Booyah", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "And another item", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "More of them", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Get with it", NULL, NULL,  NULL, NULL);

   elm_list_go(li);

   elm_box_pack_end(o, li);
   evas_object_show(li);

   elm_object_part_content_set(fl, "front", o);
   evas_object_show(o);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_NONE);
   snprintf(buf, sizeof(buf),
            "This is an entry widget in this window that<br/>"
            "uses markup <b>like this</> for styling and<br/>"
            "formatting <em>like this</>, as well as<br/>"
            "<a href=X><link>links in the text</></a>, so enter text<br/>"
            "in here to edit it. By the way, links are<br/>"
            "called <a href=anc-02>Anchors</a> so you will need<br/>"
            "to refer to them this way.<br/>"
            "<br/>"

            "Also you can stick in items with (relsize + ascent): "
            "<item relsize=16x16 vsize=ascent href=emoticon/evil-laugh></item>"
            " (full) "
            "<item relsize=16x16 vsize=full href=emoticon/guilty-smile></item>"
            " (to the left)<br/>"

            "Also (size + ascent): "
            "<item size=16x16 vsize=ascent href=emoticon/haha></item>"
            " (full) "
            "<item size=16x16 vsize=full href=emoticon/happy-panting></item>"
            " (before this)<br/>"

            "And as well (absize + ascent): "
            "<item absize=64x64 vsize=ascent href=emoticon/knowing-grin></item>"
            " (full) "
            "<item absize=64x64 vsize=full href=emoticon/not-impressed></item>"
            " or even paths to image files on disk too like: "
            "<item absize=96x128 vsize=full href=file://%s/images/sky_01.jpg></item>"
            " ... end."
            , elm_app_data_dir_get()
           );
   elm_object_text_set(en, buf);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   elm_object_part_content_set(fl, "back", en);
   evas_object_show(en);

   evas_object_show(fl);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "1");
   evas_object_smart_callback_add(bt, "clicked", my_fl_1, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "2");
   evas_object_smart_callback_add(bt, "clicked", my_fl_2, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "3");
   evas_object_smart_callback_add(bt, "clicked", my_fl_3, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "4");
   evas_object_smart_callback_add(bt, "clicked", my_fl_4, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "5");
   evas_object_smart_callback_add(bt, "clicked", my_fl_5, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "6");
   evas_object_smart_callback_add(bt, "clicked", my_fl_6, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320 * elm_config_scale_get(),
                           480 * elm_config_scale_get());
   evas_object_show(win);
}

void
test_flip3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bx2, *fl, *fl_f, *fl_b, *o, *bt, *fr;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("flip3", "Flip Flip");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fl = elm_flip_add(win);
   evas_object_size_hint_align_set(fl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(fl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, fl);
   evas_object_show(fl);

   fr = elm_frame_add(win);
   elm_object_text_set(fr, "Front");
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_part_content_set(fl, "front", fr);
   evas_object_show(fr);

   fl_f = elm_flip_add(win);
   evas_object_size_hint_align_set(fl_f, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(fl_f, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_set(fr, fl_f);
   evas_object_show(fl_f);

   o = elm_bg_add(win);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), "sky_01.jpg");
   elm_bg_file_set(o, buf, NULL);
   elm_object_part_content_set(fl_f, "front", o);
   evas_object_show(o);

   o = elm_bg_add(win);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), "sky_02.jpg");
   elm_bg_file_set(o, buf, NULL);
   elm_object_part_content_set(fl_f, "back", o);
   evas_object_show(o);

   fr = elm_frame_add(win);
   elm_object_text_set(fr, "Back");
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_part_content_set(fl, "back", fr);
   evas_object_show(fr);

   fl_b = elm_flip_add(win);
   evas_object_size_hint_align_set(fl_b, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(fl_b, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_set(fr, fl_b);
   evas_object_show(fl_b);

   o = elm_bg_add(win);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), "sky_03.jpg");
   elm_bg_file_set(o, buf, NULL);
   elm_object_part_content_set(fl_b, "front", o);
   evas_object_show(o);

   o = elm_bg_add(win);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), "sky_04.jpg");
   elm_bg_file_set(o, buf, NULL);
   elm_object_part_content_set(fl_b, "back", o);
   evas_object_show(o);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Main Flip");
   evas_object_smart_callback_add(bt, "clicked", my_fl_6, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Front Flip");
   evas_object_smart_callback_add(bt, "clicked", my_fl_6, fl_f);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Back Flip");
   evas_object_smart_callback_add(bt, "clicked", my_fl_6, fl_b);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320 * elm_config_scale_get(),
                           480 * elm_config_scale_get());
   evas_object_show(win);
}

static void
my_fl_go(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;

   Evas_Object *rdg = evas_object_data_get(win, "rdg");
   Evas_Object *rd = elm_radio_selected_object_get(rdg);
   Evas_Object *fl = evas_object_data_get(win, "fl");

   const char *text = elm_object_text_get(rd);
   if (!text) return;

   if (!strcmp(text, "Rotate"))
     elm_flip_go(fl, ELM_FLIP_ROTATE_Y_CENTER_AXIS);
   else if (!strcmp(text, "Cube"))
     elm_flip_go(fl, ELM_FLIP_CUBE_RIGHT);
   else if (!strcmp(text, "Page"))
     elm_flip_go(fl, ELM_FLIP_PAGE_RIGHT);
}

static void
my_fl_ch(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *fl = evas_object_data_get(win, "fl");
   Evas_Object *rdg = evas_object_data_get(win, "rdg");
   elm_flip_interaction_set(fl, elm_radio_value_get(rdg));
}

void
test_flip_interactive(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *fl, *im, *li, *bt, *rd, *rdg;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("flip-interactive", "Flip Interactive");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fl = elm_flip_add(win);
   evas_object_size_hint_align_set(fl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(fl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, fl);
   evas_object_data_set(win, "fl", fl);

   elm_flip_interaction_set(fl, ELM_FLIP_INTERACTION_NONE);
   elm_flip_interaction_direction_enabled_set(fl, ELM_FLIP_DIRECTION_UP, EINA_TRUE);
   elm_flip_interaction_direction_enabled_set(fl, ELM_FLIP_DIRECTION_DOWN, EINA_TRUE);
   elm_flip_interaction_direction_enabled_set(fl, ELM_FLIP_DIRECTION_LEFT, EINA_TRUE);
   elm_flip_interaction_direction_enabled_set(fl, ELM_FLIP_DIRECTION_RIGHT, EINA_TRUE);
   elm_flip_interaction_direction_hitsize_set(fl, ELM_FLIP_DIRECTION_UP, 0.25);
   elm_flip_interaction_direction_hitsize_set(fl, ELM_FLIP_DIRECTION_DOWN, 0.25);
   elm_flip_interaction_direction_hitsize_set(fl, ELM_FLIP_DIRECTION_LEFT, 0.25);
   elm_flip_interaction_direction_hitsize_set(fl, ELM_FLIP_DIRECTION_RIGHT, 0.25);
   evas_object_show(fl);

   im = evas_object_image_filled_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(im, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   snprintf(buf, sizeof(buf), "%s/images/%s",
            elm_app_data_dir_get(), "twofish.jpg");
   evas_object_image_file_set(im, buf, NULL);
   elm_object_part_content_set(fl, "front", im);
   evas_object_show(im);

#if 0
   im = evas_object_image_filled_add(evas_object_evas_get(win));
   evas_object_size_hint_weight_set(im, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   snprintf(buf, sizeof(buf), "%s/images/%s",
            elm_app_data_dir_get(), "sky_04.jpg");
   evas_object_image_file_set(im, buf, NULL);
   elm_object_part_content_set(fl, "back", im);
   evas_object_show(im);
#else
   li = elm_list_add(win);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_list_item_append(li, "Item 0", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Item 1", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Item 2", NULL, NULL, NULL, NULL);
   elm_list_item_append(li, "Item 3 (Which is very long just for testing purposes)", NULL, NULL, NULL, NULL);
   elm_list_go(li);
   elm_object_part_content_set(fl, "back", li);
   evas_object_show(li);
#endif

   rd = elm_radio_add(win);
   evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   elm_radio_state_value_set(rd, ELM_FLIP_INTERACTION_NONE);
   elm_object_text_set(rd, "None");
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", my_fl_ch, win);
   rdg = rd;
   evas_object_data_set(win, "rdg", rdg);

   rd = elm_radio_add(win);
   evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   elm_radio_state_value_set(rd, ELM_FLIP_INTERACTION_ROTATE);
   elm_object_text_set(rd, "Rotate");
   elm_radio_group_add(rd, rdg);
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", my_fl_ch, win);

   rd = elm_radio_add(win);
   evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   elm_radio_state_value_set(rd, ELM_FLIP_INTERACTION_CUBE);
   elm_object_text_set(rd, "Cube");
   elm_radio_group_add(rd, rdg);
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", my_fl_ch, win);

   rd = elm_radio_add(win);
   evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, 0.0);
   elm_radio_state_value_set(rd, ELM_FLIP_INTERACTION_PAGE);
   elm_object_text_set(rd, "Page");
   elm_radio_group_add(rd, rdg);
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);
   evas_object_smart_callback_add(rd, "changed", my_fl_ch, win);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Flip Go");
   evas_object_smart_callback_add(bt, "clicked", my_fl_go, win);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320 * elm_config_scale_get(),
                           480 * elm_config_scale_get());
   evas_object_show(win);
}

void
test_flip_to(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bx2, *fl, *o, *bt, *ly;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("flip-to", "Flip To");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fl = elm_flip_add(win);
   evas_object_size_hint_align_set(fl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(fl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, fl);

   o = elm_bg_add(win);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   snprintf(buf, sizeof(buf), "%s/images/%s", elm_app_data_dir_get(), "sky_01.jpg");
   elm_bg_file_set(o, buf, NULL);
   elm_object_part_content_set(fl, "front", o);
   evas_object_show(o);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "layout");
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_part_content_set(fl, "back", ly);
   evas_object_show(ly);

   bt = elm_button_add(ly);
   elm_object_text_set(bt, "Button 1");
   elm_object_part_content_set(ly, "element1", bt);

   bt = elm_button_add(ly);
   elm_object_text_set(bt, "Button 2");
   elm_object_part_content_set(ly, "element2", bt);

   bt = elm_button_add(ly);
   elm_object_text_set(bt, "Button 3");
   elm_object_part_content_set(ly, "element3", bt);

   evas_object_show(fl);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Show Front");
   evas_object_smart_callback_add(bt, "clicked", my_fl_front, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Show Back");
   evas_object_smart_callback_add(bt, "clicked", my_fl_back, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320 * elm_config_scale_get(),
                           480 * elm_config_scale_get());
   evas_object_show(win);
}
