#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

void
my_fl_1(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_ROTATE_Y_CENTER_AXIS);
}

void
my_fl_2(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_ROTATE_X_CENTER_AXIS);
}

void
my_fl_3(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_ROTATE_XZ_CENTER_AXIS);
}

void
my_fl_4(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *fl = data;
   elm_flip_go(fl, ELM_FLIP_ROTATE_YZ_CENTER_AXIS);
}

void
test_flip(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bx2, *fl, *o, *bt, *ly;
   char buf[PATH_MAX];
   
   win = elm_win_add(NULL, "flip", ELM_WIN_BASIC);
   elm_win_title_set(win, "Flip");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

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
   snprintf(buf, sizeof(buf), "%s/images/%s", PACKAGE_DATA_DIR, "sky_01.jpg");
   elm_bg_file_set(o, buf, NULL);
   elm_flip_content_front_set(fl, o);
   evas_object_show(o);
   
   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", PACKAGE_DATA_DIR);
   elm_layout_file_set(ly, buf, "layout");
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_flip_content_back_set(fl, ly);
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

   evas_object_show(fl);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "1");
   evas_object_smart_callback_add(bt, "clicked", my_fl_1, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "2");
   evas_object_smart_callback_add(bt, "clicked", my_fl_2, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "3");
   evas_object_smart_callback_add(bt, "clicked", my_fl_3, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "4");
   evas_object_smart_callback_add(bt, "clicked", my_fl_4, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);
   
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

void
test_flip2(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bx2, *fl, *o, *bt, *tb, *ic, *li;
   char buf[PATH_MAX];
   
   win = elm_win_add(NULL, "flip2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Flip 2");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/%s", PACKAGE_DATA_DIR, "sky_01.jpg");
   elm_bg_file_set(bg, buf, NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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
   elm_toolbar_homogenous_set(tb, 0);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, 0.0);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "Hello", NULL, NULL);
   
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "Out", NULL, NULL);
   
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "There", NULL, NULL);
   
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "World", NULL, NULL);
   
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
   
   elm_flip_content_front_set(fl, o);
   evas_object_show(o);
   
   li = elm_list_add(win);
   evas_object_size_hint_align_set(li, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(li, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   
   elm_list_item_append(li, "This is a list", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Second item", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "3rd", NULL, NULL,  NULL, NULL);
   
   elm_list_go(li);
   
   elm_flip_content_back_set(fl, li);
   evas_object_show(li);

   evas_object_show(fl);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "1");
   evas_object_smart_callback_add(bt, "clicked", my_fl_1, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "2");
   evas_object_smart_callback_add(bt, "clicked", my_fl_2, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "3");
   evas_object_smart_callback_add(bt, "clicked", my_fl_3, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);
   
   bt = elm_button_add(win);
   elm_button_label_set(bt, "4");
   evas_object_smart_callback_add(bt, "clicked", my_fl_4, fl);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);
   
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif
