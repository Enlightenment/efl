#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
static void my_win_del(void *data, Evas_Object *obj, void *event_info);
static void my_bt_1(void *data, Evas_Object *obj, void *event_info);
static void my_win_main(void);

static void
my_win_del(void *data, Evas_Object *obj, void *event_info)
{
   /* called when my_win_main is requested to be deleted */
   elm_exit(); /* exit the program's main loop that runs in elm_run() */
}

static void
my_bt_1(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg;

   win = elm_win_add(NULL, "bg-plain", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bg Plain");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   /* allow bg to expand in x & y */
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   /* set size hints. a minimum size for the bg. this should propagate back
    * to the window thus limiting its size based off the bg as the bg is one
    * of the window's resize objects. */
   evas_object_size_hint_min_set(bg, 160, 160);
   /* and set a maximum size. not needed very often. normally used together
    * with evas_object_size_hint_min_set() at the same size to make a
    * window not resizable */
   evas_object_size_hint_max_set(bg, 640, 640);
   /* and now just resize the window to a size you want. normally widgets
    * will determine the initial size though */
   evas_object_resize(win, 320, 320);
   /* and show the window */
   evas_object_show(win);
}

static void
my_bt_2(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "bg-image", ELM_WIN_BASIC);
   elm_win_title_set(win, "Bg Image");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
   elm_bg_file_set(bg, buf, NULL);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

static void
my_bt_3(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "icon-transparent", ELM_WIN_BASIC);
   elm_win_title_set(win, "Icon Transparent");
   elm_win_autodel_set(win, 1);
   elm_win_alpha_set(win, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   elm_win_resize_object_add(win, ic);
   evas_object_show(ic);

   evas_object_show(win);
}

static void
my_bt_4(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "box-vert", ELM_WIN_BASIC);
   elm_win_title_set(win, "Box Vert");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.0, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 1.0, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   evas_object_show(win);
}

static void
my_bt_5(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "box-horiz", ELM_WIN_BASIC);
   elm_win_title_set(win, "Box Horiz");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, 1);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.0, 1.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   evas_object_show(win);
}

static void
my_bt_6(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *ic, *bt;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "buttons", ELM_WIN_BASIC);
   elm_win_title_set(win, "Buttons");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Icon sized to button");
   elm_button_icon_set(bt, ic);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Icon no scale");
   elm_button_icon_set(bt, ic);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Label Only");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   bt = elm_button_add(win);
   elm_button_icon_set(bt, ic);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);

   evas_object_show(win);
}

static void
my_bt_7(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *ic, *tg;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "toggles", ELM_WIN_BASIC);
   elm_win_title_set(win, "Toggles");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   tg = elm_toggle_add(win);
   evas_object_size_hint_weight_set(tg, 1.0, 1.0);
   evas_object_size_hint_align_set(tg, -1.0, 0.5);
   elm_toggle_label_set(tg, "Icon sized to toggle");
   elm_toggle_icon_set(tg, ic);
   elm_toggle_state_set(tg, 1);
   elm_toggle_states_labels_set(tg, "Yes", "No");
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   tg = elm_toggle_add(win);
   elm_toggle_label_set(tg, "Icon no scale");
   elm_toggle_icon_set(tg, ic);
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);
   evas_object_show(ic);

   tg = elm_toggle_add(win);
   elm_toggle_label_set(tg, "Label Only");
   elm_toggle_states_labels_set(tg, "Big long fun times label", "Small long happy fun label");
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   tg = elm_toggle_add(win);
   elm_toggle_icon_set(tg, ic);
   elm_box_pack_end(bx, tg);
   evas_object_show(tg);
   evas_object_show(ic);

   evas_object_show(win);
}

static void
my_bt_8(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *tb, *bt;

   win = elm_win_add(NULL, "table", ELM_WIN_BASIC);
   elm_win_title_set(win, "Table");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   tb = elm_table_add(win);
   elm_win_resize_object_add(win, tb);
   evas_object_size_hint_weight_set(tb, 1.0, 1.0);
   evas_object_show(tb);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button 1");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   elm_table_pack(tb, bt, 0, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   elm_table_pack(tb, bt, 1, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button 3");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   elm_table_pack(tb, bt, 2, 0, 1, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button 4");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   elm_table_pack(tb, bt, 0, 1, 2, 1);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button 5");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   elm_table_pack(tb, bt, 2, 1, 1, 3);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button 6");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   elm_table_pack(tb, bt, 0, 2, 2, 2);
   evas_object_show(bt);

   evas_object_show(win);
}

static void
my_bt_9(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *ck;

   win = elm_win_add(NULL, "clock", ELM_WIN_BASIC);
   elm_win_title_set(win, "Clock");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ck = elm_clock_add(win);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_clock_add(win);
   elm_clock_show_am_pm_set(ck, 1);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_clock_add(win);
   elm_clock_show_seconds_set(ck, 1);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_clock_add(win);
   elm_clock_show_seconds_set(ck, 1);
   elm_clock_show_am_pm_set(ck, 1);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ck = elm_clock_add(win);
   elm_clock_edit_set(ck, 1);
   elm_clock_show_seconds_set(ck, 1);
   elm_clock_show_am_pm_set(ck, 1);
   elm_clock_time_set(ck, 10, 11, 12);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   evas_object_show(win);
}

static void
my_bt_10(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *ly, *bt;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "layout", ELM_WIN_BASIC);
   elm_win_title_set(win, "Layout");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", PACKAGE_DATA_DIR);
   elm_layout_file_set(ly, buf, "layout");
   evas_object_size_hint_weight_set(ly, 1.0, 1.0);
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

static void
my_hover_bt_1(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *hv = data;

   evas_object_show(hv);
}

static void
my_bt_11(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bt, *hv, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "hover", ELM_WIN_BASIC);
   elm_win_title_set(win, "Hover");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   hv = elm_hover_add(win);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button");
   evas_object_smart_callback_add(bt, "clicked", my_hover_bt_1, hv);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_hover_parent_set(hv, win);
   elm_hover_target_set(hv, bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Popup");
   elm_hover_content_set(hv, "middle", bt);
   evas_object_show(bt);

   bx = elm_box_add(win);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Top 1");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Top 2");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Top 3");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(bx);
   elm_hover_content_set(hv, "top", bx);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Bottom");
   elm_hover_content_set(hv, "bottom", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Left");
   elm_hover_content_set(hv, "left", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Right");
   elm_hover_content_set(hv, "right", bt);
   evas_object_show(bt);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

static void
my_bt_12(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bt, *hv, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "hover2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Hover 2");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   hv = elm_hover_add(win);
   elm_object_style_set(hv, "popout");

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Button");
   evas_object_smart_callback_add(bt, "clicked", my_hover_bt_1, hv);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_hover_parent_set(hv, win);
   elm_hover_target_set(hv, bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Popup");
   elm_hover_content_set(hv, "middle", bt);
   evas_object_show(bt);

   bx = elm_box_add(win);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Top 1");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Top 2");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Top 3");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(bx);
   elm_hover_content_set(hv, "top", bx);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Bot");
   elm_hover_content_set(hv, "bottom", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Left");
   elm_hover_content_set(hv, "left", bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Right");
   elm_hover_content_set(hv, "right", bt);
   evas_object_show(bt);

   evas_object_size_hint_min_set(bg, 160, 160);
   evas_object_size_hint_max_set(bg, 640, 640);
   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

static void
my_entry_bt_1(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *en = data;
   elm_entry_entry_set(en, "");
}

static void
my_entry_bt_2(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *en = data;
   const char *s = elm_entry_entry_get(en);
   printf("ENTRY:\n");
   if (s) printf("%s\n", s);
}

static void
my_entry_bt_3(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *en = data;
   const char *s = elm_entry_selection_get(en);
   printf("SELECTION:\n");
   if (s) printf("%s\n", s);
}

static void
my_entry_bt_4(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *en = data;
   elm_entry_entry_insert(en, "Insert some <b>BOLD</> text");
}

static void
my_bt_13(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *en;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "entry", ELM_WIN_BASIC);
   elm_win_title_set(win, "Entry");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = elm_entry_add(win);
   elm_entry_line_wrap_set(en, 0);
   elm_entry_entry_set(en,
		       "This is an entry widget in this window that<br>"
		       "uses markup <b>like this</> for styling and<br>"
		       "formatting <em>like this</>, as well as<br>"
		       "<a href=X><link>links in the text</></a>, so enter text<br>"
		       "in here to edit it. By the way, links are<br>"
		       "called <a href=anc-02>Anchors</a> so you will need<br>"
		       "to refer to them this way.");
   evas_object_size_hint_weight_set(en, 1.0, 1.0);
   evas_object_size_hint_align_set(en, -1.0, -1.0);
   elm_box_pack_end(bx, en);
   evas_object_show(en);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_1, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Print");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_2, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Selection");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_3, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Insert");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_4, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   elm_object_focus(en);
   evas_object_show(win);
}

static void
my_bt_14(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *en, *sc;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "entry-scrolled", ELM_WIN_BASIC);
   elm_win_title_set(win, "Entry Scrolled");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   sc = elm_scroller_add(win);
   elm_scroller_content_min_limit(sc, 0, 1);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   evas_object_size_hint_weight_set(sc, 1.0, 0.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(bx, sc);

   en = elm_entry_add(win);
   elm_entry_single_line_set(en, 1);
   elm_entry_entry_set(en, "This is a single line");
   evas_object_size_hint_weight_set(en, 1.0, 0.0);
   evas_object_size_hint_align_set(en, -1.0, 0.0);
   elm_entry_select_all(en);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);

   evas_object_show(sc);

   sc = elm_scroller_add(win);
   elm_scroller_content_min_limit(sc, 0, 1);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   evas_object_size_hint_weight_set(sc, 1.0, 0.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(bx, sc);

   en = elm_entry_add(win);
   elm_entry_password_set(en, 1);
   elm_entry_entry_set(en, "Password here");
   evas_object_size_hint_weight_set(en, 1.0, 0.0);
   evas_object_size_hint_align_set(en, -1.0, 0.0);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);

   evas_object_show(sc);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
   elm_box_pack_end(bx, sc);

   en = elm_entry_add(win);
   elm_entry_entry_set(en,
		       "This is an entry widget in this window that<br>"
		       "uses markup <b>like this</> for styling and<br>"
		       "formatting <em>like this</>, as well as<br>"
		       "<a href=X><link>links in the text</></a>, so enter text<br>"
		       "in here to edit it. By the way, links are<br>"
		       "called <a href=anc-02>Anchors</a> so you will need<br>"
		       "to refer to them this way. At the end here is a really long line to test line wrapping to see if it works. But just in case this line is not long enough I will add more here to really test it out, as Elementary really needs some good testing to see if entry widgets work as advertised.");
   evas_object_size_hint_weight_set(en, 1.0, 1.0);
   evas_object_size_hint_align_set(en, -1.0, -1.0);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);

   evas_object_show(sc);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_1, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Print");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_2, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Selection");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_3, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Insert");
   evas_object_smart_callback_add(bt, "clicked", my_entry_bt_4, en);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 300);

   elm_object_focus(win);
   evas_object_show(win);
}

static void
my_notepad_bt_1(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *np = data;
}

static void
my_notepad_bt_2(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *np = data;
}

static void
my_notepad_bt_3(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *np = data;
}

static void
my_bt_15(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *ic, *np;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "notepad", ELM_WIN_BASIC);
   elm_win_title_set(win, "Notepad");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   np = elm_notepad_add(win);
   elm_notepad_file_set(np, "note.txt", ELM_TEXT_FORMAT_PLAIN_UTF8);
   evas_object_size_hint_weight_set(np, 1.0, 1.0);
   evas_object_size_hint_align_set(np, -1.0, -1.0);
   elm_box_pack_end(bx, np);
   evas_object_show(np);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "arrow_left");
   elm_icon_scale_set(ic, 1, 0);
   elm_button_icon_set(bt, ic);
   evas_object_show(ic);
   evas_object_smart_callback_add(bt, "clicked", my_notepad_bt_1, np);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "close");
   elm_icon_scale_set(ic, 1, 0);
   elm_button_icon_set(bt, ic);
   evas_object_show(ic);
   evas_object_smart_callback_add(bt, "clicked", my_notepad_bt_2, np);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "arrow_right");
   elm_icon_scale_set(ic, 1, 0);
   elm_button_icon_set(bt, ic);
   evas_object_show(ic);
   evas_object_smart_callback_add(bt, "clicked", my_notepad_bt_3, np);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 300);

   elm_object_focus(win);
   evas_object_show(win);
}

static void
my_anchorview_bt(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *av = data;
   elm_anchorview_hover_end(av);
}

static void
my_anchorview_anchor(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *av = data;
   Elm_Entry_Anchorview_Info *ei = event_info;
   Evas_Object *bt, *bx;

   bt = elm_button_add(obj);
   elm_button_label_set(bt, ei->name);
   elm_hover_content_set(ei->hover, "middle", bt);
   evas_object_show(bt);

   // hints as to where we probably should put hover contents (buttons etc.).
   if (ei->hover_top)
     {
	bx = elm_box_add(obj);
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Top 1");
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Top 2");
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Top 3");
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
	elm_hover_content_set(ei->hover, "top", bx);
	evas_object_show(bx);
     }
   if (ei->hover_bottom)
     {
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Bot");
	elm_hover_content_set(ei->hover, "bottom", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
     }
   if (ei->hover_left)
     {
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Left");
	elm_hover_content_set(ei->hover, "left", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
     }
   if (ei->hover_right)
     {
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Right");
	elm_hover_content_set(ei->hover, "right", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorview_bt, av);
	evas_object_show(bt);
     }
}

static void
my_bt_16(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *av;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "anchorview", ELM_WIN_BASIC);
   elm_win_title_set(win, "Anchorview");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   av = elm_anchorview_add(win);
   elm_anchorview_hover_style_set(av, "popout");
   elm_anchorview_hover_parent_set(av, win);
   elm_anchorview_text_set(av,
		       "This is an entry widget in this window that<br>"
		       "uses markup <b>like this</> for styling and<br>"
		       "formatting <em>like this</>, as well as<br>"
		       "<a href=X><link>links in the text</></a>, so enter text<br>"
		       "in here to edit it. By the way, links are<br>"
		       "called <a href=anc-02>Anchors</a> so you will need<br>"
		       "to refer to them this way.");
   evas_object_size_hint_weight_set(av, 1.0, 1.0);
   evas_object_smart_callback_add(av, "anchor,clicked", my_anchorview_anchor, av);
   elm_win_resize_object_add(win, av);
   evas_object_show(av);

   evas_object_resize(win, 320, 300);

   elm_object_focus(win);
   evas_object_show(win);
}

static void
my_anchorblock_bt(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *av = data;
   elm_anchorblock_hover_end(av);
}

static void
my_anchorblock_anchor(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *av = data;
   Elm_Entry_Anchorblock_Info *ei = event_info;
   Evas_Object *bt, *bx;

   bt = elm_button_add(obj);
   elm_button_label_set(bt, ei->name);
   elm_hover_content_set(ei->hover, "middle", bt);
   evas_object_show(bt);

   // hints as to where we probably should put hover contents (buttons etc.).
   if (ei->hover_top)
     {
	bx = elm_box_add(obj);
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Top 1");
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Top 2");
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Top 3");
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
	elm_hover_content_set(ei->hover, "top", bx);
	evas_object_show(bx);
     }
   if (ei->hover_bottom)
     {
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Bot");
	elm_hover_content_set(ei->hover, "bottom", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
     }
   if (ei->hover_left)
     {
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Left");
	elm_hover_content_set(ei->hover, "left", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
     }
   if (ei->hover_right)
     {
	bt = elm_button_add(obj);
	elm_button_label_set(bt, "Right");
	elm_hover_content_set(ei->hover, "right", bt);
	evas_object_smart_callback_add(bt, "clicked", my_anchorblock_bt, av);
	evas_object_show(bt);
     }
}

static void
my_anchorblock_edge_left(void *data, Evas_Object *obj, void *event_info)
{
   printf("left\n");
}

static void
my_anchorblock_edge_right(void *data, Evas_Object *obj, void *event_info)
{
   printf("right\n");
}

static void
my_anchorblock_edge_top(void *data, Evas_Object *obj, void *event_info)
{
   printf("top\n");
}

static void
my_anchorblock_edge_bottom(void *data, Evas_Object *obj, void *event_info)
{
   printf("bottom\n");
}

static void
my_anchorblock_scroll(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Coord x, y, w, h, vw, vh;

   elm_scroller_region_get(obj, &x, &y, &w, &h);
   elm_scroller_child_size_get(obj, &vw, &vh);
   printf("scroll %ix%i +%i+%i in %ix%i\n", w, h, x, y, vw, vh);
}

static void
my_bt_17(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *av, *sc, *bx, *bb, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "anchorblock", ELM_WIN_BASIC);
   elm_win_title_set(win, "Anchorblock");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   elm_win_resize_object_add(win, sc);

   evas_object_smart_callback_add(sc, "edge_left", my_anchorblock_edge_left, NULL);
   evas_object_smart_callback_add(sc, "edge_right", my_anchorblock_edge_right, NULL);
   evas_object_smart_callback_add(sc, "edge_top", my_anchorblock_edge_top, NULL);
   evas_object_smart_callback_add(sc, "edge_bottom", my_anchorblock_edge_bottom, NULL);
   evas_object_smart_callback_add(sc, "scroll", my_anchorblock_scroll, NULL);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 0.0);
   evas_object_size_hint_align_set(bx, -1.0, -1.0);
   elm_scroller_content_set(sc, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_bubble_label_set(bb, "Message 3");
   elm_bubble_info_set(bb, "10:32 4/11/2008");
   elm_bubble_icon_set(bb, ic);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, 1.0, 0.0);
   evas_object_size_hint_align_set(bb, -1.0,-1.0);
   av = elm_anchorblock_add(win);
   elm_anchorblock_hover_style_set(av, "popout");
   elm_anchorblock_hover_parent_set(av, win);
   elm_anchorblock_text_set(av,
			   "Hi there. This is the most recent message in the "
			   "list of messages. It has one <a href=tel:+614321234>+61 432 1234</a> "
			   "(phone number) to click on.");
   evas_object_smart_callback_add(av, "anchor,clicked", my_anchorblock_anchor, av);
   elm_bubble_content_set(bb, av);
   evas_object_show(av);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_bubble_label_set(bb, "Message 2");
   elm_bubble_info_set(bb, "7:16 27/10/2008");
   elm_bubble_icon_set(bb, ic);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, 1.0, 0.0);
   evas_object_size_hint_align_set(bb, -1.0,-1.0);
   av = elm_anchorblock_add(win);
   elm_anchorblock_hover_style_set(av, "popout");
   elm_anchorblock_hover_parent_set(av, win);
   elm_anchorblock_text_set(av,
			   "Hey what are you doing? This is the second last message "
			   "Hi there. This is the most recent message in the "
			   "list. It's a longer one so it can wrap more and "
			   "contains a <a href=contact:john>John</a> contact "
			   "link in it to test popups on links. The idea is that "
			   "all SMS's are scanned for things that look like phone "
			   "numbers or names that are in your contacts list, and "
			   "if they are, they become clickable links that pop up "
			   "a menus of obvious actions to perform on this piece "
			   "of information. This of course can be later explicitly "
			   "done by links maybe running local apps or even being "
			   "web URL's too that launch the web browser and point it "
			   "to that URL.");
   evas_object_smart_callback_add(av, "anchor,clicked", my_anchorblock_anchor, av);
   elm_bubble_content_set(bb, av);
   evas_object_show(av);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_bubble_label_set(bb, "Message 1");
   elm_bubble_info_set(bb, "20:47 18/6/2008");
   elm_bubble_icon_set(bb, ic);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, 1.0, 0.0);
   evas_object_size_hint_align_set(bb, -1.0,-1.0);
   av = elm_anchorblock_add(win);
   elm_anchorblock_hover_style_set(av, "popout");
   elm_anchorblock_hover_parent_set(av, win);
   elm_anchorblock_text_set(av,
			   "This is a short message");
   evas_object_smart_callback_add(av, "anchor,clicked", my_anchorblock_anchor, av);
   elm_bubble_content_set(bb, av);
   evas_object_show(av);
   elm_box_pack_end(bx, bb);
   evas_object_show(bb);

   evas_object_show(sc);

   evas_object_resize(win, 320, 300);

   elm_object_focus(win);
   evas_object_show(win);
}

static void
tb_1(void *data, Evas_Object *obj, void *event_info)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/panel_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_2(void *data, Evas_Object *obj, void *event_info)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_3(void *data, Evas_Object *obj, void *event_info)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/wood_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_4(void *data, Evas_Object *obj, void *event_info)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(data, buf);
}

static void
tb_5(void *data, Evas_Object *obj, void *event_info)
{
   elm_photo_file_set(data, NULL);
}

static void
my_bt_18(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *tb, *ic, *ph;
   Evas_Object *ph1, *ph2, *ph3, *ph4;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "toolbar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Toolbar");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_show(bx);

   tb = elm_toolbar_add(win);
   evas_object_size_hint_weight_set(tb, 0.0, 0.0);
   evas_object_size_hint_align_set(tb, -1.0, 0.0);

   ph1 = elm_photo_add(win);
   ph2 = elm_photo_add(win);
   ph3 = elm_photo_add(win);
   ph4 = elm_photo_add(win);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "Hello", tb_1, ph1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "World", tb_2, ph1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "Here", tb_3, ph4);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "Comes", tb_4, ph4);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_toolbar_item_add(tb, ic, "Elementary", tb_5, ph4);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   tb = elm_table_add(win);
//   elm_table_homogenous_set(tb, 1);
   evas_object_size_hint_weight_set(tb, 0.0, 1.0);
   evas_object_size_hint_align_set(tb, -1.0, -1.0);

   ph = ph1;
   elm_photo_size_set(ph, 40);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, 1.0, 1.0);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 0, 1, 1);
   evas_object_show(ph);

   ph = ph2;
   elm_photo_size_set(ph, 80);
   evas_object_size_hint_weight_set(ph, 1.0, 1.0);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 0, 1, 1);
   evas_object_show(ph);

   ph = ph3;
   elm_photo_size_set(ph, 20);
   snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, 1.0, 1.0);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 0, 1, 1, 1);
   evas_object_show(ph);

   ph = ph4;
   elm_photo_size_set(ph, 60);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", PACKAGE_DATA_DIR);
   elm_photo_file_set(ph, buf);
   evas_object_size_hint_weight_set(ph, 1.0, 1.0);
   evas_object_size_hint_align_set(ph, 0.5, 0.5);
   elm_table_pack(tb, ph, 1, 1, 1, 1);
   evas_object_show(ph);

   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   evas_object_resize(win, 320, 300);

   evas_object_show(win);
}

static void
my_bt_19(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bt, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "hoversel", ELM_WIN_BASIC);
   elm_win_title_set(win, "Hoversel");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_show(bx);

   bt = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(bt, win);
   elm_hoversel_label_set(bt, "Labels");
   elm_hoversel_item_add(bt, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 3", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 4 - Long Label Here", NULL, ELM_ICON_NONE, NULL, NULL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(bt, win);
   elm_hoversel_label_set(bt, "Some Icons");
   elm_hoversel_item_add(bt, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 3", "home", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 4", "close", ELM_ICON_STANDARD, NULL, NULL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(bt, win);
   elm_hoversel_label_set(bt, "All Icons");
   elm_hoversel_item_add(bt, "Item 1", "apps", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 2", "arrow_down", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 3", "home", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 4", "close", ELM_ICON_STANDARD, NULL, NULL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(bt, win);
   elm_hoversel_label_set(bt, "All Icons");
   elm_hoversel_item_add(bt, "Item 1", "apps", ELM_ICON_STANDARD, NULL, NULL);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", PACKAGE_DATA_DIR);
   elm_hoversel_item_add(bt, "Item 2", buf, ELM_ICON_FILE, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 3", "home", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 4", "close", ELM_ICON_STANDARD, NULL, NULL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_hoversel_add(win);
   elm_hoversel_hover_parent_set(bt, win);
   elm_hoversel_label_set(bt, "Icon + Label");

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_hoversel_icon_set(bt, ic);
   evas_object_show(ic);

   elm_hoversel_item_add(bt, "Item 1", "apps", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 2", "arrow_down", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 3", "home", ELM_ICON_STANDARD, NULL, NULL);
   elm_hoversel_item_add(bt, "Item 4", "close", ELM_ICON_STANDARD, NULL, NULL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, 0.5, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320, 300);

   evas_object_show(win);
}

static void
my_bt_20(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *li, *ic, *ic2, *bx;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "list", ELM_WIN_BASIC);
   elm_win_title_set(win, "List");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   li = elm_list_add(win);
   elm_win_resize_object_add(win, li);
   evas_object_size_hint_weight_set(li, 1.0, 1.0);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 1, 1);
   elm_list_item_append(li, "Hello", ic, NULL,  NULL, NULL);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_scale_set(ic, 0, 0);
   elm_icon_file_set(ic, buf, NULL);
   elm_list_item_append(li, "world", ic, NULL,  NULL, NULL);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_icon_scale_set(ic, 0, 0);
   elm_list_item_append(li, ".", ic, NULL,  NULL, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_icon_scale_set(ic, 0, 0);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_icon_scale_set(ic2, 0, 0);
   elm_list_item_append(li, "How", ic, ic2,  NULL, NULL);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.0, 1.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);
   elm_list_item_append(li, "are", bx, NULL,  NULL, NULL);

   elm_list_item_append(li, "you", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "doing", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "out", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "there", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "today", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "?", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Here", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "are", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "some", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "more", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "items", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Is this label long enough?", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Maybe this one is even longer so we can test long long items.", NULL, NULL,  NULL, NULL);

   elm_list_go(li);

   evas_object_show(li);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

static void
my_li2_clear(void *data, Evas_Object *obj, void *event_info)
{
   elm_list_clear(data);
}

static void
my_li2_sel(void *data, Evas_Object *obj, void *event_info)
{
   Elm_List_Item *it = elm_list_selected_item_get(obj);
   elm_list_item_selected_set(it, 0);
//   elm_list_item_selected_set(event_info, 0);
}

static void
my_bt_21(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *li, *ic, *ic2, *bx, *bx2, *bt;
   char buf[PATH_MAX];
   Elm_List_Item *it;

   win = elm_win_add(NULL, "list-2", ELM_WIN_BASIC);
   elm_win_title_set(win, "List 2");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
   elm_bg_file_set(bg, buf, NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   li = elm_list_add(win);
   evas_object_size_hint_align_set(li, -1.0, -1.0);
   evas_object_size_hint_weight_set(li, 1.0, 1.0);
   elm_list_horizontal_mode_set(li, ELM_LIST_LIMIT);
//   elm_list_multi_select_set(li, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   it = elm_list_item_append(li, "Hello", ic, NULL,  my_li2_sel, NULL);
   elm_list_item_selected_set(it, 1);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_scale_set(ic, 0, 0);
   elm_icon_file_set(ic, buf, NULL);
   elm_list_item_append(li, "world", ic, NULL,  NULL, NULL);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_icon_scale_set(ic, 0, 0);
   elm_list_item_append(li, ".", ic, NULL,  NULL, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_icon_scale_set(ic, 0, 0);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_icon_scale_set(ic2, 0, 0);
   elm_list_item_append(li, "How", ic, ic2,  NULL, NULL);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx2, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx2, ic);
   evas_object_show(ic);
   elm_list_item_append(li, "are", bx2, NULL, NULL, NULL);

   elm_list_item_append(li, "you", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "doing", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "out", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "there", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "today", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "?", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Here", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "are", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "some", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "more", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "items", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Longer label.", NULL, NULL,  NULL, NULL);

   elm_list_go(li);

   elm_box_pack_end(bx, li);
   evas_object_show(li);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Clear");
   evas_object_smart_callback_add(bt, "clicked", my_li2_clear, li);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

static void
my_bt_22(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *li, *ic, *ic2, *bx;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "list-3", ELM_WIN_BASIC);
   elm_win_title_set(win, "List 3");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   li = elm_list_add(win);
   elm_win_resize_object_add(win, li);
   evas_object_size_hint_weight_set(li, 1.0, 1.0);
   elm_list_horizontal_mode_set(li, ELM_LIST_COMPRESS);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_list_item_append(li, "Hello", ic, NULL,  NULL, NULL);
   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_scale_set(ic, 0, 0);
   elm_icon_file_set(ic, buf, NULL);
   elm_list_item_append(li, "world", ic, NULL,  NULL, NULL);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "edit");
   elm_icon_scale_set(ic, 0, 0);
   elm_list_item_append(li, ".", ic, NULL,  NULL, NULL);

   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "delete");
   elm_icon_scale_set(ic, 0, 0);
   ic2 = elm_icon_add(win);
   elm_icon_standard_set(ic2, "clock");
   elm_icon_scale_set(ic2, 0, 0);
   elm_list_item_append(li, "How", ic, ic2,  NULL, NULL);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, 1);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.0, 1.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);
   elm_list_item_append(li, "are", bx, NULL,  NULL, NULL);

   elm_list_item_append(li, "you", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "doing", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "out", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "there", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "today", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "?", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Here", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "are", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "some", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "more", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "items", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Is this label long enough?", NULL, NULL,  NULL, NULL);
   elm_list_item_append(li, "Maybe this one is even longer so we can test long long items.", NULL, NULL,  NULL, NULL);

   elm_list_go(li);

   evas_object_show(li);

   evas_object_resize(win, 320, 300);
   evas_object_show(win);
}

static void
my_bt_23(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "carousel", ELM_WIN_BASIC);
   elm_win_title_set(win, "Carousel");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   evas_object_resize(win, 320, 240);
   evas_object_show(win);
}

static void
my_bt_24(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *inwin, *lb;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "inwin", ELM_WIN_BASIC);
   elm_win_title_set(win, "Inwin");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   inwin = elm_win_inwin_add(win);
   evas_object_show(inwin);

   lb = elm_label_add(win);
   elm_label_label_set(lb,
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

static void
my_bt_25(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *inwin, *lb;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "inwin", ELM_WIN_BASIC);
   elm_win_title_set(win, "Inwin");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   inwin = elm_win_inwin_add(win);
   elm_object_style_set(inwin, "minimal_vertical");
   evas_object_show(inwin);

   lb = elm_label_add(win);
   elm_label_label_set(lb,
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

static void
my_bt_26(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bt;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "scaling", ELM_WIN_BASIC);
   elm_win_title_set(win, "Scaling");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Scale: 0.5");
   elm_object_scale_set(bt, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Scale: 0.75");
   elm_object_scale_set(bt, 0.75);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Scale: 1.0");
   elm_object_scale_set(bt, 1.0);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Scale: 1.5");
   elm_object_scale_set(bt, 1.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Scale: 2.0");
   elm_object_scale_set(bt, 2.0);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Scale: 3.0");
   elm_object_scale_set(bt, 3.0);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

static void
my_bt_27(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *fr, *lb;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "scaling-2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Scaling 2");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fr = elm_frame_add(win);
   elm_object_scale_set(fr, 0.5);
   elm_frame_label_set(fr, "Scale: 0.5");
   lb = elm_label_add(win);
   elm_label_label_set(lb,
		       "Parent frame scale<br>"
		       "is 0.5. Child should<br>"
		       "inherit it.");
   elm_frame_content_set(fr, lb);
   evas_object_show(lb);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   fr = elm_frame_add(win);
   elm_frame_label_set(fr, "Scale: 1.0");
   lb = elm_label_add(win);
   elm_label_label_set(lb,
		       "Parent frame scale<br>"
		       "is 1.0. Child should<br>"
		       "inherit it.");
   elm_frame_content_set(fr, lb);
   evas_object_show(lb);
   elm_object_scale_set(fr, 1.0);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   fr = elm_frame_add(win);
   elm_frame_label_set(fr, "Scale: 2.0");
   lb = elm_label_add(win);
   elm_label_label_set(lb,
		       "Parent frame scale<br>"
		       "is 2.0. Child should<br>"
		       "inherit it.");
   elm_frame_content_set(fr, lb);
   evas_object_show(lb);
   elm_object_scale_set(fr, 2.0);
   elm_box_pack_end(bx, fr);
   evas_object_show(fr);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

static void
my_bt_28(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *sl, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "slider", ELM_WIN_BASIC);
   elm_win_title_set(win, "Slider");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Label");
   elm_slider_icon_set(sl, ic);
   elm_slider_unit_format_set(sl, "%1.1f units");
   elm_slider_span_size_set(sl, 120);
   evas_object_size_hint_align_set(sl, -1.0, 0.5);
   evas_object_size_hint_weight_set(sl, 1.0, 1.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(ic);
   evas_object_show(sl);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Label 2");
   elm_slider_span_size_set(sl, 80);
   evas_object_size_hint_align_set(sl, -1.0, 0.5);
   evas_object_size_hint_weight_set(sl, 1.0, 1.0);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 80);
   elm_slider_inverted_set(sl, 1);
   evas_object_size_hint_align_set(sl, 0.5, 0.5);
   evas_object_size_hint_weight_set(sl, 0.0, 0.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(ic);
   evas_object_show(sl);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Label 3");
   elm_slider_unit_format_set(sl, "units");
   elm_slider_span_size_set(sl, 40);
   evas_object_size_hint_align_set(sl, -1.0, 0.5);
   evas_object_size_hint_weight_set(sl, 1.0, 1.0);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 80);
   elm_slider_inverted_set(sl, 1);
   elm_object_scale_set(sl, 2.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(ic);
   evas_object_show(sl);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);
   sl = elm_slider_add(win);
   elm_slider_icon_set(sl, ic);
   elm_slider_label_set(sl, "Label 4");
   elm_slider_unit_format_set(sl, "units");
   elm_slider_span_size_set(sl, 60);
   evas_object_size_hint_align_set(sl, 0.5, -1.0);
   evas_object_size_hint_weight_set(sl, 0.0, 1.0);
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_value_set(sl, 0.2);
   elm_object_scale_set(sl, 1.0);
   elm_slider_horizontal_set(sl, 0);
   elm_box_pack_end(bx, sl);
   evas_object_show(ic);
   evas_object_show(sl);

   evas_object_show(win);
}

static Elm_Genlist_Item_Class itc1;
char *gl_label_get(const void *data, Evas_Object *obj, const char *part)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item # %i", (int)data);
   return strdup(buf);
}
Evas_Object *gl_icon_get(const void *data, Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX];
   Evas_Object *ic = elm_icon_add(obj);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}
Eina_Bool gl_state_get(const void *data, Evas_Object *obj, const char *part)
{
   return EINA_FALSE;
}
void gl_del(const void *data, Evas_Object *obj)
{
}

static void
gl_sel(void *data, Evas_Object *obj, void *event_info)
{
   printf("sel item data [%p] on genlist obj [%p], item pointer [%p]\n", data, obj, event_info);
}

static void
my_bt_29(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *gl;
   Elm_Genlist_Item *gli;
   int i;

   win = elm_win_add(NULL, "genlist", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   gl = elm_genlist_add(win);
   elm_genlist_horizontal_mode_set(gl, ELM_LIST_LIMIT);
   elm_win_resize_object_add(win, gl);
   evas_object_size_hint_weight_set(gl, 1.0, 1.0);
   evas_object_show(gl);

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;

   for (i = 0; i < 2000; i++)
     {
	gli = elm_genlist_item_append(gl, &itc1,
				      (void *)i/* item data */,
				      NULL/* parent */,
				      ELM_GENLIST_ITEM_NONE,
				      gl_sel/* func */,
				      (void *)(i * 10)/* func data */);
     }
   evas_object_resize(win, 480, 800);
   evas_object_show(win);
}

static void
my_gl_clear(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *gl = data;
   elm_genlist_clear(gl);
}

static void
my_gl_add(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *gl = data;
   Elm_Genlist_Item *gli;
   static int i = 0;

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;

   gli = elm_genlist_item_append(gl, &itc1,
				 (void *)i/* item data */,
				 NULL/* parent */,
				 ELM_GENLIST_ITEM_NONE,
				 gl_sel/* func */,
				 (void *)(i * 10)/* func data */);
   i++;
}

static void
my_gl_del(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *gl = data;
   Elm_Genlist_Item *gli = elm_genlist_selected_item_get(gl);
   if (!gli)
     {
	printf("no item selected\n");
	return;
     }
   elm_genlist_item_del(gli);
}

static void
my_gl_disable(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *gl = data;
   Elm_Genlist_Item *gli = elm_genlist_selected_item_get(gl);
   if (!gli)
     {
	printf("no item selected\n");
	return;
     }
   elm_genlist_item_disabled_set(gli, 1);
   elm_genlist_item_selected_set(gli, 0);
   elm_genlist_item_update(gli);
}

static void
my_gl_update_all(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *gl = data;
   int i = 0;
   Elm_Genlist_Item *it = elm_genlist_first_item_get(gl);
   while (it)
     {
	elm_genlist_item_update(it);
	printf("%i\n", i);
	i++;
	it = elm_genlist_item_next_get(it);
     }
}

static void
my_gl_first(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *gl = data;
   Elm_Genlist_Item *gli = elm_genlist_first_item_get(gl);
   if (!gli) return;
   elm_genlist_item_show(gli);
   elm_genlist_item_selected_set(gli, 1);
}

static void
my_gl_last(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *gl = data;
   Elm_Genlist_Item *gli = elm_genlist_last_item_get(gl);
   if (!gli) return;
   elm_genlist_item_show(gli);
   elm_genlist_item_selected_set(gli, 1);
}

static void
my_bt_30(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *gl, *bx, *bx2, *bt;
   Elm_Genlist_Item *gli[10];
   char buf[PATH_MAX];
   int i;

   win = elm_win_add(NULL, "genlist-2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist 2");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/plant_01.jpg", PACKAGE_DATA_DIR);
   elm_bg_file_set(bg, buf, NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, -1.0, -1.0);
   evas_object_size_hint_weight_set(gl, 1.0, 1.0);
   evas_object_show(gl);

   itc1.item_style     = "default";
   itc1.func.label_get = gl_label_get;
   itc1.func.icon_get  = gl_icon_get;
   itc1.func.state_get = gl_state_get;
   itc1.func.del       = gl_del;

   gli[0] = elm_genlist_item_append(gl, &itc1,
				    (void *)1001/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
				    (void *)1001/* func data */);
   gli[1] = elm_genlist_item_append(gl, &itc1,
				    (void *)1002/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
				    (void *)1002/* func data */);
   gli[2] = elm_genlist_item_append(gl, &itc1,
				    (void *)1003/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
				    (void *)1003/* func data */);
   gli[3] = elm_genlist_item_prepend(gl, &itc1,
				     (void *)1004/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
				     (void *)1004/* func data */);
   gli[4] = elm_genlist_item_prepend(gl, &itc1,
				     (void *)1005/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
				     (void *)1005/* func data */);
   gli[5] = elm_genlist_item_insert_before(gl, &itc1,
					   (void *)1006/* item data */, gli[2]/* rel */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
					   (void *)1006/* func data */);
   gli[6] = elm_genlist_item_insert_after(gl, &itc1,
					  (void *)1007/* item data */, gli[2]/* rel */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
					  (void *)1007/* func data */);

   elm_box_pack_end(bx, gl);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "/\\");
   evas_object_smart_callback_add(bt, "clicked", my_gl_first, gl);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "\\/");
   evas_object_smart_callback_add(bt, "clicked", my_gl_last, gl);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "#");
   evas_object_smart_callback_add(bt, "clicked", my_gl_disable, gl);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "U");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update_all, gl);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "X");
   evas_object_smart_callback_add(bt, "clicked", my_gl_clear, gl);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "+");
   evas_object_smart_callback_add(bt, "clicked", my_gl_add, gl);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "-");
   evas_object_smart_callback_add(bt, "clicked", my_gl_del, gl);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

typedef struct _Testitem
{
   Elm_Genlist_Item *item;
   int mode;
   int onoff;
} Testitem;
static Elm_Genlist_Item_Class itc2;
char *gl2_label_get(const void *data, Evas_Object *obj, const char *part)
{
   Testitem *tit = data;
   char buf[256];
   snprintf(buf, sizeof(buf), "Item mode %i", tit->mode);
   return strdup(buf);
}
Evas_Object *gl2_icon_get(const void *data, Evas_Object *obj, const char *part)
{
   Testitem *tit = data;
   char buf[PATH_MAX];
   Evas_Object *ic = elm_icon_add(obj);
   if (!strcmp(part, "elm.swallow.icon"))
     {
	if ((tit->mode & 0x3) == 0)
	  snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
	else if ((tit->mode & 0x3) == 1)
	  snprintf(buf, sizeof(buf), "%s/images/logo.png", PACKAGE_DATA_DIR);
	else if ((tit->mode & 0x3) == 2)
	  snprintf(buf, sizeof(buf), "%s/images/panel_01.jpg", PACKAGE_DATA_DIR);
	else if ((tit->mode & 0x3) == 3)
	  snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", PACKAGE_DATA_DIR);
	elm_icon_file_set(ic, buf, NULL);
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
	if ((tit->mode & 0x3) == 0)
	  snprintf(buf, sizeof(buf), "%s/images/sky_01.jpg", PACKAGE_DATA_DIR);
	else if ((tit->mode & 0x3) == 1)
	  snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", PACKAGE_DATA_DIR);
	else if ((tit->mode & 0x3) == 2)
	  snprintf(buf, sizeof(buf), "%s/images/sky_03.jpg", PACKAGE_DATA_DIR);
	else if ((tit->mode & 0x3) == 3)
	  snprintf(buf, sizeof(buf), "%s/images/sky_04.jpg", PACKAGE_DATA_DIR);
	elm_icon_file_set(ic, buf, NULL);
     }
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   return ic;
}
Eina_Bool gl2_state_get(const void *data, Evas_Object *obj, const char *part)
{
   return EINA_FALSE;
}
void gl2_del(const void *data, Evas_Object *obj)
{
}

static void
my_gl_update(void *data, Evas_Object *obj, void *event_info)
{
   Testitem *tit = data;
   tit->mode++;
   elm_genlist_item_update(tit->item);
}

static void
my_bt_31(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *gl, *bx, *bx2, *bt;
   static Testitem tit[3];
   int i;

   win = elm_win_add(NULL, "genlist-3", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist 3");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, -1.0, -1.0);
   evas_object_size_hint_weight_set(gl, 1.0, 1.0);
   evas_object_show(gl);

   itc2.item_style     = "default";
   itc2.func.label_get = gl2_label_get;
   itc2.func.icon_get  = gl2_icon_get;
   itc2.func.state_get = gl2_state_get;
   itc2.func.del       = gl2_del;

   tit[0].mode = 0;
   tit[0].item = elm_genlist_item_append(gl, &itc2,
					 &(tit[0])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
					 NULL/* func data */);
   tit[1].mode = 1;
   tit[1].item = elm_genlist_item_append(gl, &itc2,
					 &(tit[1])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
					 NULL/* func data */);
   tit[2].mode = 2;
   tit[2].item = elm_genlist_item_append(gl, &itc2,
					 &(tit[2])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
					 NULL/* func data */);

   elm_box_pack_end(bx, gl);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[1]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[0]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[2]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[1]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[3]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[2]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

static void
my_gl_item_check_changed(void *data, Evas_Object *obj, void *event_info)
{
   Testitem *tit = data;
   tit->onoff = elm_check_state_get(obj);
   printf("item %p onoff = %i\n", tit, tit->onoff);
}

static Elm_Genlist_Item_Class itc3;
char *gl3_label_get(const void *data, Evas_Object *obj, const char *part)
{
   Testitem *tit = data;
   char buf[256];
   snprintf(buf, sizeof(buf), "Item mode %i", tit->mode);
   return strdup(buf);
}
Evas_Object *gl3_icon_get(const void *data, Evas_Object *obj, const char *part)
{
   Testitem *tit = data;
   char buf[PATH_MAX];
   if (!strcmp(part, "elm.swallow.icon"))
     {
	Evas_Object *bx = elm_box_add(obj);
	Evas_Object *ic;
	elm_box_horizontal_set(bx, 1);
	ic = elm_icon_add(obj);
	snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
	elm_icon_file_set(ic, buf, NULL);
	elm_icon_scale_set(ic, 0, 0);
	evas_object_show(ic);
	elm_box_pack_end(bx, ic);
	ic = elm_icon_add(obj);
	elm_icon_file_set(ic, buf, NULL);
	elm_icon_scale_set(ic, 0, 0);
	evas_object_show(ic);
	elm_box_pack_end(bx, ic);
	evas_object_show(bx);
	return bx;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
	Evas_Object *ck;
	ck = elm_check_add(obj);
	evas_object_propagate_events_set(ck, 0);
	elm_check_state_set(ck, tit->onoff);
	evas_object_smart_callback_add(ck, "changed", my_gl_item_check_changed, data);
	evas_object_show(ck);
	return ck;
     }
   return NULL;
}
Eina_Bool gl3_state_get(const void *data, Evas_Object *obj, const char *part)
{
   return EINA_FALSE;
}
void gl3_del(const void *data, Evas_Object *obj)
{
}

static void
my_bt_32(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *gl, *bx, *bx2, *bt;
   static Testitem tit[3];
   int i;

   win = elm_win_add(NULL, "genlist-4", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist 4");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   elm_genlist_multi_select_set(gl, 1);
   evas_object_size_hint_align_set(gl, -1.0, -1.0);
   evas_object_size_hint_weight_set(gl, 1.0, 1.0);
   evas_object_show(gl);

   itc3.item_style     = "default";
   itc3.func.label_get = gl3_label_get;
   itc3.func.icon_get  = gl3_icon_get;
   itc3.func.state_get = gl3_state_get;
   itc3.func.del       = gl3_del;

   tit[0].mode = 0;
   tit[0].item = elm_genlist_item_append(gl, &itc3,
					 &(tit[0])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
					 NULL/* func data */);
   tit[1].mode = 1;
   tit[1].item = elm_genlist_item_append(gl, &itc3,
					 &(tit[1])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
					 NULL/* func data */);
   tit[2].mode = 2;
   tit[2].item = elm_genlist_item_append(gl, &itc3,
					 &(tit[2])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
					 NULL/* func data */);

   elm_box_pack_end(bx, gl);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[1]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[0]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[2]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[1]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[3]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[2]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

static void
my_bt_33(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *ic, *ck;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "check", ELM_WIN_BASIC);
   elm_win_title_set(win, "Checks");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   ck = elm_check_add(win);
   evas_object_size_hint_weight_set(ck, 1.0, 1.0);
   evas_object_size_hint_align_set(ck, -1.0, 0.5);
   elm_check_label_set(ck, "Icon sized to check");
   elm_check_icon_set(ck, ic);
   elm_check_state_set(ck, 1);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   ck = elm_check_add(win);
   elm_check_label_set(ck, "Icon no scale");
   elm_check_icon_set(ck, ic);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);
   evas_object_show(ic);

   ck = elm_check_add(win);
   elm_check_label_set(ck, "Label Only");
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   ck = elm_check_add(win);
   elm_check_icon_set(ck, ic);
   elm_box_pack_end(bx, ck);
   evas_object_show(ck);
   evas_object_show(ic);

   evas_object_show(win);
}

static void
my_bt_34(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *ic, *rd, *rdg;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "radio", ELM_WIN_BASIC);
   elm_win_title_set(win, "Radios");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 0);
   evas_object_size_hint_weight_set(rd, 1.0, 1.0);
   evas_object_size_hint_align_set(rd, -1.0, 0.5);
   elm_radio_label_set(rd, "Icon sized to radio");
   elm_radio_icon_set(rd, ic);
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);
   evas_object_show(ic);
   rdg = rd;

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 1);
   elm_radio_group_add(rd, rdg);
   elm_radio_label_set(rd, "Icon no scale");
   elm_radio_icon_set(rd, ic);
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);
   evas_object_show(ic);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 2);
   elm_radio_group_add(rd, rdg);
   elm_radio_label_set(rd, "Label Only");
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, 3);
   elm_radio_group_add(rd, rdg);
   elm_radio_icon_set(rd, ic);
   elm_box_pack_end(bx, rd);
   evas_object_show(rd);
   evas_object_show(ic);

   elm_radio_value_set(rdg, 2);

   evas_object_show(win);
}

typedef struct _Pginfo Pginfo;

struct _Pginfo
{
   Evas_Object *pager, *pg1, *pg2, *pg3;
};

static void
my_pager_1(void *data, Evas_Object *obj, void *event_info)
{
   Pginfo *info = data;
   elm_pager_content_promote(info->pager, info->pg2);
}

static void
my_pager_2(void *data, Evas_Object *obj, void *event_info)
{
   Pginfo *info = data;
   elm_pager_content_promote(info->pager, info->pg3);
}

static void
my_pager_3(void *data, Evas_Object *obj, void *event_info)
{
   Pginfo *info = data;
   elm_pager_content_promote(info->pager, info->pg1);
}

static void
my_pager_pop(void *data, Evas_Object *obj, void *event_info)
{
   Pginfo *info = data;
   elm_pager_content_pop(info->pager);
}

static void
my_bt_35(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *pg, *bx, *lb, *bt;
   char buf[PATH_MAX];
   static Pginfo info;

   win = elm_win_add(NULL, "pager", ELM_WIN_BASIC);
   elm_win_title_set(win, "Pager");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   pg = elm_pager_add(win);
   elm_win_resize_object_add(win, pg);
   evas_object_show(pg);

   info.pager = pg;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_show(bx);

   lb = elm_label_add(win);
   elm_label_label_set(lb,
		       "This is page 1 in a pager stack.<br>"
		       "<br>"
		       "So what is a pager stack? It is a stack<br>"
		       "of pages that hold widgets in it. The<br>"
		       "pages can be pushed and popped on and<br>"
		       "off the stack, activated and otherwise<br>"
		       "activated if already in the stack<br>"
		       "(activated means promoted to the top of<br>"
		       "the stack).<br>"
		       "<br>"
		       "The theme may define the animation how<br>"
		       "show and hide of pages."
		       );
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Flip to 2");
   evas_object_smart_callback_add(bt, "clicked", my_pager_1, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Popme");
   evas_object_smart_callback_add(bt, "clicked", my_pager_pop, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_pager_content_push(pg, bx);
   info.pg1 = bx;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_show(bx);
   lb = elm_label_add(win);
   elm_label_label_set(lb,
		       "This is page 2 in a pager stack.<br>"
		       "<br>"
		       "This is just like the previous page in<br>"
		       "the pager stack."
		       );
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Flip to 3");
   evas_object_smart_callback_add(bt, "clicked", my_pager_2, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Popme");
   evas_object_smart_callback_add(bt, "clicked", my_pager_pop, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_pager_content_push(pg, bx);
   info.pg2 = bx;

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_show(bx);
   lb = elm_label_add(win);
   elm_label_label_set(lb,
		       "This is page 3 in a pager stack.<br>"
		       "<br>"
		       "This is just like the previous page in<br>"
		       "the pager stack."
		       );
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Flip to 1");
   evas_object_smart_callback_add(bt, "clicked", my_pager_3, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   bt = elm_button_add(win);
   elm_button_label_set(bt, "Popme");
   evas_object_smart_callback_add(bt, "clicked", my_pager_pop, &info);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   elm_pager_content_push(pg, bx);
   info.pg3 = bx;

   evas_object_show(win);
}

static Elm_Genlist_Item_Class itc4;

static void
gl4_sel(void *data, Evas_Object *obj, void *event_info)
{
}
static void
gl4_exp(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   Evas_Object *gl = elm_genlist_item_genlist_get(it);
   int val = (int)elm_genlist_item_data_get(it);
   val *= 10;
   elm_genlist_item_append(gl, &itc4,
			   (void *)(val + 1)/* item data */, it/* parent */, ELM_GENLIST_ITEM_NONE, gl4_sel/* func */,
			   NULL/* func data */);
   elm_genlist_item_append(gl, &itc4,
			   (void *)(val + 2)/* item data */, it/* parent */, ELM_GENLIST_ITEM_NONE, gl4_sel/* func */,
			   NULL/* func data */);
   elm_genlist_item_append(gl, &itc4,
			   (void *)(val + 3)/* item data */, it/* parent */, ELM_GENLIST_ITEM_SUBITEMS, gl4_sel/* func */,
			   NULL/* func data */);
}
static void
gl4_con(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_subitems_clear(it);
}

static void
gl4_exp_req(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 1);
}
static void
gl4_con_req(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Genlist_Item *it = event_info;
   elm_genlist_item_expanded_set(it, 0);
}

char *gl4_label_get(const void *data, Evas_Object *obj, const char *part)
{
   char buf[256];
   snprintf(buf, sizeof(buf), "Item mode %i", (int)data);
   return strdup(buf);
}
Evas_Object *gl4_icon_get(const void *data, Evas_Object *obj, const char *part)
{
   char buf[PATH_MAX];
   if (!strcmp(part, "elm.swallow.icon"))
     {
	Evas_Object *ic = elm_icon_add(obj);
	snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
	elm_icon_file_set(ic, buf, NULL);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	evas_object_show(ic);
	return ic;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
	Evas_Object *ck;
	ck = elm_check_add(obj);
	evas_object_show(ck);
	return ck;
     }
   return NULL;
}
Eina_Bool gl4_state_get(const void *data, Evas_Object *obj, const char *part)
{
   return EINA_FALSE;
}
void gl4_del(const void *data, Evas_Object *obj)
{
}

static void
my_bt_36(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *gl, *bx, *bx2, *bt;

   win = elm_win_add(NULL, "genlist-tree", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist Tree");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   evas_object_size_hint_align_set(gl, -1.0, -1.0);
   evas_object_size_hint_weight_set(gl, 1.0, 1.0);
   evas_object_show(gl);

   itc4.item_style     = "default";
   itc4.func.label_get = gl4_label_get;
   itc4.func.icon_get  = gl4_icon_get;
   itc4.func.state_get = gl4_state_get;
   itc4.func.del       = gl4_del;

   elm_genlist_item_append(gl, &itc4,
			   (void *)1/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_SUBITEMS, gl4_sel/* func */,
			   NULL/* func data */);
   elm_genlist_item_append(gl, &itc4,
			   (void *)2/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_SUBITEMS, gl4_sel/* func */,
			   NULL/* func data */);
   elm_genlist_item_append(gl, &itc4,
			   (void *)3/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl4_sel/* func */,
			   NULL/* func data */);

   evas_object_smart_callback_add(gl, "expand,request", gl4_exp_req, gl);
   evas_object_smart_callback_add(gl, "contract,request", gl4_con_req, gl);
   evas_object_smart_callback_add(gl, "expanded", gl4_exp, gl);
   evas_object_smart_callback_add(gl, "contracted", gl4_con, gl);

   elm_box_pack_end(bx, gl);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[1]");
//   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[0]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[2]");
//   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[1]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[3]");
//   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[2]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

static void
my_gl_item_check_changed2(void *data, Evas_Object *obj, void *event_info)
{
   Testitem *tit = data;
   tit->onoff = elm_check_state_get(obj);
   printf("item %p onoff = %i\n", tit, tit->onoff);
}

static Elm_Genlist_Item_Class itc5;
char *gl5_label_get(const void *data, Evas_Object *obj, const char *part)
{
   Testitem *tit = data;
   char buf[256];
   if (!strcmp(part, "elm.text"))
     {
	snprintf(buf, sizeof(buf), "Item mode %i", tit->mode);
     }
   else if (!strcmp(part, "elm.text.sub"))
     {
	snprintf(buf, sizeof(buf), "%i bottles on the wall", tit->mode);
     }
   return strdup(buf);
}
Evas_Object *gl5_icon_get(const void *data, Evas_Object *obj, const char *part)
{
   Testitem *tit = data;
   char buf[PATH_MAX];
   if (!strcmp(part, "elm.swallow.icon"))
     {
	Evas_Object *bx = elm_box_add(obj);
	Evas_Object *ic;
	elm_box_horizontal_set(bx, 1);
	ic = elm_icon_add(obj);
	snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
	elm_icon_file_set(ic, buf, NULL);
	elm_icon_scale_set(ic, 0, 0);
	evas_object_show(ic);
	elm_box_pack_end(bx, ic);
	ic = elm_icon_add(obj);
	elm_icon_file_set(ic, buf, NULL);
	elm_icon_scale_set(ic, 0, 0);
	evas_object_show(ic);
	elm_box_pack_end(bx, ic);
	evas_object_show(bx);
	return bx;
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
	Evas_Object *ck;
	ck = elm_check_add(obj);
	evas_object_propagate_events_set(ck, 0);
	elm_check_state_set(ck, tit->onoff);
	evas_object_smart_callback_add(ck, "changed", my_gl_item_check_changed2, data);
	evas_object_show(ck);
	return ck;
     }
   return NULL;
}
Eina_Bool gl5_state_get(const void *data, Evas_Object *obj, const char *part)
{
   return EINA_FALSE;
}
void gl5_del(const void *data, Evas_Object *obj)
{
}

static void
my_bt_37(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *gl, *bx, *bx2, *bt;
   static Testitem tit[3];
   int i;

   win = elm_win_add(NULL, "genlist-5", ELM_WIN_BASIC);
   elm_win_title_set(win, "Genlist 5");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   gl = elm_genlist_add(win);
   elm_genlist_always_select_mode_set(gl, 1);
   evas_object_size_hint_align_set(gl, -1.0, -1.0);
   evas_object_size_hint_weight_set(gl, 1.0, 1.0);
   evas_object_show(gl);
   itc5.item_style     = "double_label";
   itc5.func.label_get = gl5_label_get;
   itc5.func.icon_get  = gl5_icon_get;
   itc5.func.state_get = gl5_state_get;
   itc5.func.del       = gl5_del;

   tit[0].mode = 0;
   tit[0].item = elm_genlist_item_append(gl, &itc5,
					 &(tit[0])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
					 NULL/* func data */);
   tit[1].mode = 1;
   tit[1].item = elm_genlist_item_append(gl, &itc5,
					 &(tit[1])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
					 NULL/* func data */);
   tit[2].mode = 2;
   tit[2].item = elm_genlist_item_append(gl, &itc5,
					 &(tit[2])/* item data */, NULL/* parent */, ELM_GENLIST_ITEM_NONE, gl_sel/* func */,
					 NULL/* func data */);

   elm_box_pack_end(bx, gl);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[1]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[0]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[2]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[1]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "[3]");
   evas_object_smart_callback_add(bt, "clicked", my_gl_update, &(tit[2]));
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

static void
my_bt_38_alpha_on(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   evas_object_hide(bg);
   elm_win_alpha_set(win, 1);
}

static void
my_bt_38_alpha_off(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   evas_object_show(bg);
   elm_win_alpha_set(win, 0);
}

static void
my_bt_38_rot_0(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   elm_win_rotation_set(win, 0);
}

static void
my_bt_38_rot_90(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   elm_win_rotation_set(win, 90);
}

static void
my_bt_38_rot_180(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   elm_win_rotation_set(win, 180);
}

static void
my_bt_38_rot_270(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   elm_win_rotation_set(win, 270);
}

static void
my_bt_38(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *sl, *bx, *bx2, *bt;
   static Testitem tit[3];
   int i;

   win = elm_win_add(NULL, "window-state", ELM_WIN_BASIC);
   elm_win_title_set(win, "Window States");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);
   evas_object_data_set(win, "bg", bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Alpha On");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_alpha_on, win);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Alpha Off");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_alpha_off, win);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 1.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   sl = elm_slider_add(win);
   elm_slider_label_set(sl, "Test");
   elm_slider_span_size_set(sl, 100);
   evas_object_size_hint_align_set(sl, 0.5, 0.5);
   evas_object_size_hint_weight_set(sl, 1.0, 1.0);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 50);
   elm_slider_inverted_set(sl, 1);
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, 1.0, 0.0);
   evas_object_size_hint_align_set(bx2, -1.0, -1.0);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Rot 0");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_0, win);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Rot 90");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_90, win);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Rot 180");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_180, win);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Rot 270");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_270, win);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_size_hint_weight_set(bt, 1.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 280, 400);
   evas_object_show(win);
}

typedef struct Progressbar
{
   Evas_Object *pb1;
   Evas_Object *pb2;
   Evas_Object *pb3;
   Evas_Object *pb4;
   Evas_Object *pb5;
   Evas_Object *pb6;
   Eina_Bool run;
   Ecore_Timer *timer;
} Progressbar;

static Progressbar _test_progressbar;

static int
_my_bt_39_progressbar_value_set (void *data)
{
   double progress;

   progress = elm_progressbar_value_get (_test_progressbar.pb1);
   if (progress < 1.0) progress += 0.0123;
   else progress = 0.0;
   elm_progressbar_value_set(_test_progressbar.pb1, progress);
   elm_progressbar_value_set(_test_progressbar.pb4, progress);
   elm_progressbar_value_set(_test_progressbar.pb3, progress);
   elm_progressbar_value_set(_test_progressbar.pb6, progress);
   if (progress < 1.0) return ECORE_CALLBACK_RENEW;
   _test_progressbar.run = 0;
   return ECORE_CALLBACK_CANCEL;
}

static void
my_bt_39_test_start(void *data, Evas_Object *obj, void *event_info)
{
   elm_progressbar_pulse(_test_progressbar.pb2, EINA_TRUE);
   elm_progressbar_pulse(_test_progressbar.pb5, EINA_TRUE);
   if (!_test_progressbar.run)
     {
        _test_progressbar.timer = ecore_timer_add(0.1, _my_bt_39_progressbar_value_set, NULL);
        _test_progressbar.run = EINA_TRUE;
     }
}

static void
my_bt_39_test_stop(void *data, Evas_Object *obj, void *event_info)
{
   elm_progressbar_pulse(_test_progressbar.pb2, EINA_FALSE);
   elm_progressbar_pulse(_test_progressbar.pb5, EINA_FALSE);
   if (_test_progressbar.run)
     {
        ecore_timer_del(_test_progressbar.timer);
        _test_progressbar.run = EINA_FALSE;
     }
}

static void
my_bt_39_destroy(void *data, Evas_Object *obj, void *event_info)
{
   my_bt_39_test_stop(NULL, NULL, NULL);
   evas_object_del(obj);
}

static void
my_bt_39(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *pb, *bx, *pbx, *hbx, *bt, *bt_bx, *ic1, *ic2;
   Progressbar *test;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "progressbar", ELM_WIN_BASIC);
   elm_win_title_set(win, "Progressbar");
   evas_object_smart_callback_add(win, "delete-request", my_bt_39_destroy, test);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   evas_object_show(bx);

   pb = elm_progressbar_add(win);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   evas_object_size_hint_align_set(pb, -1.0, 0.5);
   elm_box_pack_end(bx, pb);
//   elm_progressbar_horizontal_set(pb, EINA_TRUE);
//   elm_progressbar_label_set(pb, "Progression %");
//   elm_progressbar_unit_format_set(pb, NULL);
   evas_object_show(pb);
   _test_progressbar.pb1 = pb;

   pb = elm_progressbar_add(win);
   evas_object_size_hint_align_set(pb, -1.0, 0.5);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   elm_progressbar_label_set(pb, "Infinite bounce");
   elm_progressbar_pulse_set(pb, EINA_TRUE);
   elm_box_pack_end(bx, pb);
   evas_object_show(pb);
   _test_progressbar.pb2 = pb;

   ic1 = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(ic1, buf, NULL);
   evas_object_size_hint_aspect_set(ic1, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   pb = elm_progressbar_add(win);
   elm_progressbar_label_set(pb, "Label");
   elm_progressbar_icon_set(pb, ic1);
   elm_progressbar_inverted_set(pb, 1);
   elm_progressbar_unit_format_set(pb, "%1.1f units");
   elm_progressbar_span_size_set(pb, 200);
   evas_object_size_hint_align_set(pb, -1.0, 0.5);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   elm_box_pack_end(bx, pb);
   evas_object_show(ic1);
   evas_object_show(pb);
   _test_progressbar.pb3 = pb;

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, 1.0, 1.0);
   evas_object_size_hint_align_set(hbx, -1.0, -1.0);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   pb = elm_progressbar_add(win);
   elm_progressbar_horizontal_set(pb, EINA_FALSE);
   evas_object_size_hint_align_set(pb, -1.0, -1.0);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   elm_box_pack_end(hbx, pb);
   elm_progressbar_span_size_set(pb, 60);
   elm_progressbar_label_set(pb, "percent");
   evas_object_show(pb);
   _test_progressbar.pb4 = pb;

   pb = elm_progressbar_add(win);
   elm_progressbar_horizontal_set(pb, EINA_FALSE);
   evas_object_size_hint_align_set(pb, -1.0, 0.5);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   elm_progressbar_span_size_set(pb, 80);
   elm_progressbar_pulse_set(pb, EINA_TRUE);
   elm_progressbar_unit_format_set(pb, NULL);
   elm_progressbar_label_set(pb, "Infinite bounce");
   elm_box_pack_end(hbx, pb);
   evas_object_show(pb);
   _test_progressbar.pb5 = pb;

   ic2 = elm_icon_add(win);
   elm_icon_file_set(ic2, buf, NULL);
   evas_object_size_hint_aspect_set(ic2, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);
   pb = elm_progressbar_add(win);
   elm_progressbar_horizontal_set(pb, EINA_FALSE);
   elm_progressbar_label_set(pb, "Label");
   elm_progressbar_icon_set(pb, ic2);
   elm_progressbar_inverted_set(pb, 1);
   elm_progressbar_unit_format_set(pb, "%1.2f%%");
   elm_progressbar_span_size_set(pb, 200);
   evas_object_size_hint_align_set(pb, -1.0, 0.5);
   evas_object_size_hint_weight_set(pb, 1.0, 1.0);
   elm_box_pack_end(hbx, pb);
   evas_object_show(ic2);
   evas_object_show(pb);
   _test_progressbar.pb6 = pb;

   bt_bx = elm_box_add(win);
   elm_box_horizontal_set(bt_bx, 1);
   evas_object_size_hint_weight_set(bt_bx, 1.0, 1.0);
   elm_box_pack_end(bx, bt_bx);
   evas_object_show(bt_bx);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Start");
   evas_object_smart_callback_add(bt, "clicked", my_bt_39_test_start, NULL);
   elm_box_pack_end(bt_bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Stop");
   evas_object_smart_callback_add(bt, "clicked", my_bt_39_test_stop, NULL);
   elm_box_pack_end(bt_bx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}

static void
my_bt_40_done(void *data, Evas_Object *obj, void *event_info)
{
   /* event_info conatin the full path of the selected file
    * or NULL if none is selected or cancel is pressed */
   const char *selected = event_info;
   
   if (selected)
     printf("Selected file: %s\n", selected);
   else
     evas_object_del(data); /* delete the test window */
}

static void
my_bt_40_selected(void *data, Evas_Object *obj, void *event_info)
{
   /* event_info conatin the full path of the selected file */
   const char *selected = event_info;
   printf("Selected file: %s\n", selected);
   
   /* or you can query the selection */
   printf("or: %s\n", elm_fileselector_selected_get(obj));
}

static void
my_bt_40(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *fs, *bg;
   char buf[PATH_MAX];

   /* Set the locale according to the system pref.
    * If you dont do so the file selector will order the files list in
    * a case sensitive manner
    */
   setlocale(LC_ALL, "");

   win = elm_win_add(NULL, "fileselector", ELM_WIN_BASIC);
   elm_win_title_set(win, "File Selector");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   fs = elm_fileselector_add(win);
   elm_fileselector_expandable_set(fs, EINA_FALSE);
   /* start the fileselector in the home dir */
   elm_fileselector_path_set(fs, getenv("HOME"));
   /* allow fs to expand in x & y */
   evas_object_size_hint_weight_set(fs, 1.0, 1.0);
   elm_win_resize_object_add(win, fs);
   evas_object_show(fs);
   /* the 'done' cb is called when the user press ok/cancel */
   evas_object_smart_callback_add(fs, "done", my_bt_40_done, win);
   evas_object_smart_callback_add(fs, "selected", my_bt_40_selected, win);

   evas_object_resize(win, 240, 350);
   evas_object_show(win);
}

static void
my_win_main(void)
{
   Evas_Object *win, *bg, *bx0, *lb, *li, *fr;

   /* 1 create an elm window - it returns an evas object. this is a little
    * special as the object lives in the canvas that is inside the window
    * so what is returned is really inside the window, but as you manipulate
    * the evas object returned - the window will respond. elm_win makes sure
    * of that so you can blindly treat it like any other evas object
    * pretty much, just as long as you know it has special significance */
   /* the first parameter is a "parent" window - eg for a dialog you want to
    * have a main window it is related to, here it is NULL meaning there
    * is no parent. "main" is the name of the window - used by the window
    * manager for identifying the window uniquely amongst all the windows
    * within this application (and all instances of the application). the
    * type is a basic window (the final parameter) */
   win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
   /* set the title of the window - this is in the titlebar */
   elm_win_title_set(win, "Elementary Tests");

   /* set a callback on the window when "delete-request" is emitted as
    * a callback. when this happens my_win_del() is called and the
    * data pointer (first param) is passed the final param here (in this
    * case it is NULL). This is how you can pass specific things to a
    * callback like objects or data layered on top */
   evas_object_smart_callback_add(win, "delete-request", my_win_del, NULL);

   /* add a background to our window. this just uses the standard theme set
    * background. without a backgorund, you could make a window seem
    * transparent with elm_win_alpha_set(win, 1); for example. if you have
    * a compositor running this will make the window able to be
    * semi-transparent and any space not filled by object/widget pixels will
    * be transparent or translucent based on alpha. if you do not have a
    * comnpositor running this should fall back to using shaped windows
    * (which have a mask). both these features will be slow and rely on
    * a lot more resources, so only use it if you need it. */
   bg = elm_bg_add(win);
   /* set weight to 1.0 x 1.0 == expand in both x and y direction */
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   /* tell the window that this object is to be resized along with the window.
    * also as a result this object will be one of several objects that
    * controls the minimum/maximum size of the window */
   elm_win_resize_object_add(win, bg);
   /* and show the background */
   evas_object_show(bg);

   /* add a box layout widget to the window */
   bx0 = elm_box_add(win);
   /* allow base box (bx0) to expand in x and y */
   evas_object_size_hint_weight_set(bx0, 1.0, 1.0);
   /* tell the window that the box affects window size and also will be
    * resized when the window is */
   elm_win_resize_object_add(win, bx0);
   evas_object_show(bx0);

   fr = elm_frame_add(win);
   elm_frame_label_set(fr, "Information");
   elm_box_pack_end(bx0, fr);
   evas_object_show(fr);

   lb = elm_label_add(win);
   elm_label_label_set(lb,
		       "Please select a test from the list below<br>"
		       "by clicking the test button to show the<br>"
		       "test window.");
   elm_frame_content_set(fr, lb);
   evas_object_show(lb);

   li = elm_list_add(win);
   elm_list_always_select_mode_set(li, 1);
   evas_object_size_hint_weight_set(li, 1.0, 1.0);
   evas_object_size_hint_align_set(li, -1.0, -1.0);
   elm_box_pack_end(bx0, li);
   evas_object_show(li);

   elm_list_item_append(li, "Bg Plain", NULL, NULL, my_bt_1, NULL);
   elm_list_item_append(li, "Bg Image", NULL, NULL, my_bt_2, NULL);
   elm_list_item_append(li, "Icon Transparent", NULL, NULL, my_bt_3, NULL);
   elm_list_item_append(li, "Box Vert", NULL, NULL, my_bt_4, NULL);
   elm_list_item_append(li, "Box Horiz", NULL, NULL, my_bt_5, NULL);
   elm_list_item_append(li, "Buttons", NULL, NULL, my_bt_6, NULL);
   elm_list_item_append(li, "Toggles", NULL, NULL, my_bt_7, NULL);
   elm_list_item_append(li, "Table", NULL, NULL, my_bt_8, NULL);
   elm_list_item_append(li, "Clock", NULL, NULL, my_bt_9, NULL);
   elm_list_item_append(li, "Layout", NULL, NULL, my_bt_10, NULL);
   elm_list_item_append(li, "Hover", NULL, NULL, my_bt_11, NULL);
   elm_list_item_append(li, "Hover 2", NULL, NULL, my_bt_12, NULL);
   elm_list_item_append(li, "Entry", NULL, NULL, my_bt_13, NULL);
   elm_list_item_append(li, "Entry Scrolled", NULL, NULL, my_bt_14, NULL);
   elm_list_item_append(li, "Notepad", NULL, NULL, my_bt_15, NULL);
   elm_list_item_append(li, "Anchorview", NULL, NULL, my_bt_16, NULL);
   elm_list_item_append(li, "Anchorblock", NULL, NULL, my_bt_17, NULL);
   elm_list_item_append(li, "Toolbar", NULL, NULL, my_bt_18, NULL);
   elm_list_item_append(li, "Hoversel", NULL, NULL, my_bt_19, NULL);
   elm_list_item_append(li, "List", NULL, NULL, my_bt_20, NULL);
   elm_list_item_append(li, "List 2", NULL, NULL, my_bt_21, NULL);
   elm_list_item_append(li, "List 3", NULL, NULL, my_bt_22, NULL);
   elm_list_item_append(li, "Carousel", NULL, NULL, my_bt_23, NULL);
   elm_list_item_append(li, "Inwin", NULL, NULL, my_bt_24, NULL);
   elm_list_item_append(li, "Inwin 2", NULL, NULL, my_bt_25, NULL);
   elm_list_item_append(li, "Scaling", NULL, NULL, my_bt_26, NULL);
   elm_list_item_append(li, "Scaling 2", NULL, NULL, my_bt_27, NULL);
   elm_list_item_append(li, "Slider", NULL, NULL, my_bt_28, NULL);
   elm_list_item_append(li, "Genlist", NULL, NULL, my_bt_29, NULL);
   elm_list_item_append(li, "Genlist 2", NULL, NULL, my_bt_30, NULL);
   elm_list_item_append(li, "Genlist 3", NULL, NULL, my_bt_31, NULL);
   elm_list_item_append(li, "Genlist 4", NULL, NULL, my_bt_32, NULL);
   elm_list_item_append(li, "Checks", NULL, NULL, my_bt_33, NULL);
   elm_list_item_append(li, "Radios", NULL, NULL, my_bt_34, NULL);
   elm_list_item_append(li, "Pager", NULL, NULL, my_bt_35, NULL);
   elm_list_item_append(li, "Genlist Tree", NULL, NULL, my_bt_36, NULL);
   elm_list_item_append(li, "Genlist 5", NULL, NULL, my_bt_37, NULL);
   elm_list_item_append(li, "Window States", NULL, NULL, my_bt_38, NULL);
   elm_list_item_append(li, "Progressbar", NULL, NULL, my_bt_39, NULL);
   elm_list_item_append(li, "File Selector", NULL, NULL, my_bt_40, NULL);

   elm_list_go(li);

   /* set an initial window size */
   evas_object_resize(win, 240, 480);
   /* show the window */
   evas_object_show(win);
}

/* this is your elementary main function - it MUSt be called IMMEDIATELY
 * after elm_init() and MUSt be passed argc and argv, and MUST be called
 * elm_main and not be static - must be a visible symbol with EAPI infront */
EAPI int
elm_main(int argc, char **argv)
{
   /* put ere any init specific to this app like parsing args etc. */
   my_win_main(); /* create main window */
   elm_run(); /* and run the program now  and handle all events etc. */
   /* if the mainloop that elm_run() runs exist - we exit the app */
   elm_shutdown(); /* clean up and shut down */
   /* exit code */
   return 0;
}
#endif
/* all emeentary apps should use this. but it right after elm_main() */
ELM_MAIN()
