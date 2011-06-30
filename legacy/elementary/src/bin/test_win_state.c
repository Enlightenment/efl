#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
typedef struct _Testitem
{
   Elm_Genlist_Item *item;
   int mode, onoff;
} Testitem;

static int rotate_with_resize = 0;

static void
my_bt_38_alpha_on(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   evas_object_hide(bg);
   elm_win_alpha_set(win, 1);
}

static void
my_bt_38_alpha_off(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   evas_object_show(bg);
   elm_win_alpha_set(win, 0);
}

static void
my_ck_38_resize(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
//   Evas_Object *win = data;
   rotate_with_resize = elm_check_state_get(obj);
}

static void
my_bt_38_rot_0(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 0);
   else
     elm_win_rotation_set(win, 0);
}

static void
my_bt_38_rot_90(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 90);
   else
     elm_win_rotation_set(win, 90);
}

static void
my_bt_38_rot_180(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 180);
   else
     elm_win_rotation_set(win, 180);
}

static void
my_bt_38_rot_270(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 270);
   else
     elm_win_rotation_set(win, 270);
}

static void
my_win_move(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Coord x, y;
   elm_win_screen_position_get(obj, &x, &y);
   printf("MOVE - win geom: %4i %4i\n", x, y);
}

static void
_win_resize(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Coord w, h;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   printf("RESIZE - win geom: %4ix%4i\n", w, h);
}

static void
_win_foc_in(void *data __UNUSED__, Evas *e __UNUSED__, void *event_info __UNUSED__)
{
   printf("FOC IN\n");
}

static void
_win_foc_out(void *data __UNUSED__, Evas *e __UNUSED__, void *event_info __UNUSED__)
{
   printf("FOC OUT\n");
}

static void
_close_win(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_del(data);
}

void
test_win_state(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *sl, *bx, *bx2, *bt, *ck;

   win = elm_win_add(NULL, "window-states", ELM_WIN_BASIC);
   elm_win_title_set(win, "Window States");
   evas_object_smart_callback_add(win, "moved", my_win_move, NULL);
   evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, _win_resize, NULL);
   evas_event_callback_add(evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_FOCUS_IN, _win_foc_in, NULL);
   evas_event_callback_add(evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_FOCUS_OUT, _win_foc_out, NULL);
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);
   evas_object_data_set(win, "bg", bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Alpha On");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_alpha_on, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Alpha Off");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_alpha_off, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Test");
   elm_slider_span_size_set(sl, 100);
   evas_object_size_hint_align_set(sl, 0.5, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 50);
   elm_slider_inverted_set(sl, 1);
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   ck = elm_check_add(win);
   elm_check_label_set(ck, "resize");
   elm_check_state_set(ck, rotate_with_resize);
   evas_object_smart_callback_add(ck, "changed", my_ck_38_resize, win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ck, 0.02, 0.99);
   evas_object_show(ck);
   elm_box_pack_end(bx, ck);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 0");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_0, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 90");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_90, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 180");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_180, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 270");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_270, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 280, 400);
   evas_object_show(win);
}

void
test_win_state2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *sl, *bx, *bx2, *bt, *ck;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "window-states2", ELM_WIN_BASIC);
   elm_win_override_set(win, 1);
   evas_object_smart_callback_add(win, "moved", my_win_move, NULL);
   evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, _win_resize, NULL);
   elm_win_title_set(win, "Window States 2");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", PACKAGE_DATA_DIR);
   elm_bg_file_set(bg, buf, NULL);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);
   evas_object_data_set(win, "bg", bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Alpha On");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_alpha_on, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Alpha Off");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_alpha_off, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Close");
   evas_object_smart_callback_add(bt, "clicked", _close_win, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Override Redirect");
   elm_slider_span_size_set(sl, 100);
   evas_object_size_hint_align_set(sl, 0.5, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 50);
   elm_slider_inverted_set(sl, 1);
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   ck = elm_check_add(win);
   elm_check_label_set(ck, "resize");
   elm_check_state_set(ck, rotate_with_resize);
   evas_object_smart_callback_add(ck, "changed", my_ck_38_resize, win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ck, 0.02, 0.99);
   evas_object_show(ck);
   elm_box_pack_end(bx, ck);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_homogeneous_set(bx2, EINA_TRUE);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_fill_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 0");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_0, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 90");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_90, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 180");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_180, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rot 270");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_rot_270, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
#endif
