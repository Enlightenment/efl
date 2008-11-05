#include <Elementary.h>

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
   elm_toggle_states_labels_set(tg, "Big", "Small");
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
   elm_hover_style_set(hv, "popout");
   
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

   // FIXME: not exported
   elm_widget_focus_set(win, 1);
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
   evas_object_size_hint_weight_set(sc, 1.0, 0.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(bx, sc);
   
   en = elm_entry_add(win);
   elm_entry_single_line_set(en, 1);
   elm_entry_entry_set(en, "This is a single line");
   evas_object_size_hint_weight_set(en, 1.0, 1.0);
   evas_object_size_hint_align_set(en, -1.0, -1.0);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);

   evas_object_show(sc);
   
   sc = elm_scroller_add(win);
   elm_scroller_content_min_limit(sc, 0, 1);
   evas_object_size_hint_weight_set(sc, 1.0, 0.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
   elm_box_pack_end(bx, sc);
   
   en = elm_entry_add(win);
   elm_entry_password_set(en, 1);
   elm_entry_entry_set(en, "Password here");
   evas_object_size_hint_weight_set(en, 1.0, 1.0);
   evas_object_size_hint_align_set(en, -1.0, -1.0);
   elm_scroller_content_set(sc, en);
   evas_object_show(en);

   evas_object_show(sc);
   
   sc = elm_scroller_add(win);
   evas_object_size_hint_weight_set(sc, 1.0, 1.0);
   evas_object_size_hint_align_set(sc, -1.0, -1.0);
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
   
   // FIXME: not exported
   elm_widget_focus_set(win, 1);
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
   
   // FIXME: not exported
   elm_widget_focus_set(win, 1);
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
   
   // FIXME: not exported
   elm_widget_focus_set(win, 1);
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
   
   // FIXME: not exported
   elm_widget_focus_set(win, 1);
   evas_object_show(win);
}

static void
my_win_main(void)
{
  Evas_Object *win, *bg, *bx0, *lb, *bx, *bt, *sc, *fr;
  
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
                      "Please slect a test from the list below<br>"
                      "by clicking the test button to show the<br>"
                      "test window.");
  elm_frame_content_set(fr, lb);
  evas_object_show(lb);
  
  /* add a scroller object - anything inside is scrollable */
  sc = elm_scroller_add(win);
  /* add scroller to main box */
  evas_object_size_hint_weight_set(sc, 1.0, 1.0);
  evas_object_size_hint_align_set(sc, -1.0, -1.0);
  elm_box_pack_end(bx0, sc);
  evas_object_show(sc);
  
  /* add a box layout widget to the window */
  bx = elm_box_add(win);
  /* set weight to 1.0 x 1.0 == expand in x and y) */
  evas_object_size_hint_weight_set(bx, 1.0, 0.0);
  
  /* set the box ad the content in the scrolled view*/
  elm_scroller_content_set(sc, bx);
  /* show the box, scroller and main box */
  evas_object_show(bx);
  
  /* create a button */
  bt = elm_button_add(win);
  /* set the button label */
  elm_button_label_set(bt, "Bg Plain");
  /* add a callback to be run when the button is clicked */
  evas_object_smart_callback_add(bt, "clicked", my_bt_1, NULL);
  /* the button should fill any space it is given horizontally */
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  /* put the button at the end of the box */
  elm_box_pack_end(bx, bt);
  /* show the button */
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Bg Image");
  evas_object_smart_callback_add(bt, "clicked", my_bt_2, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Icon Transparent");
  evas_object_smart_callback_add(bt, "clicked", my_bt_3, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Box Vert");
  evas_object_smart_callback_add(bt, "clicked", my_bt_4, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Box Horiz");
  evas_object_smart_callback_add(bt, "clicked", my_bt_5, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Buttons");
  evas_object_smart_callback_add(bt, "clicked", my_bt_6, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Toggles");
  evas_object_smart_callback_add(bt, "clicked", my_bt_7, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Table");
  evas_object_smart_callback_add(bt, "clicked", my_bt_8, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Clock");
  evas_object_smart_callback_add(bt, "clicked", my_bt_9, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Layout");
  evas_object_smart_callback_add(bt, "clicked", my_bt_10, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Hover");
  evas_object_smart_callback_add(bt, "clicked", my_bt_11, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Hover 2");
  evas_object_smart_callback_add(bt, "clicked", my_bt_12, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Entry");
  evas_object_smart_callback_add(bt, "clicked", my_bt_13, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Entry Scrolled");
  evas_object_smart_callback_add(bt, "clicked", my_bt_14, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Notepad");
  evas_object_smart_callback_add(bt, "clicked", my_bt_15, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Anchorview");
  evas_object_smart_callback_add(bt, "clicked", my_bt_16, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  bt = elm_button_add(win);
  elm_button_label_set(bt, "Anchorblock");
  evas_object_smart_callback_add(bt, "clicked", my_bt_17, NULL);
  evas_object_size_hint_align_set(bt, -1.0, 0.0);
  elm_box_pack_end(bx, bt);
  evas_object_show(bt);
  
  /* set an initial window size */
  evas_object_resize(win, 320, 520);
  /* show the window */
  evas_object_show(win);
}

int
main(int argc, char **argv)
{
   /* init Elementary (all Elementary calls begin with elm_ and all data
    * types, enums and macros will be Elm_ and ELM_ etc.) */
   elm_init(argc, argv);

   my_win_main();
   
   elm_run(); /* and run the program now  and handle all events etc. */
   
   elm_shutdown(); /* clean up and shut down */
   return 0; 
}
