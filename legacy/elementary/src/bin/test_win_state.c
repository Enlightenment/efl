#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _Testitem
{
   Elm_Object_Item *item;
   int mode, onoff;
} Testitem;

static int rotate_with_resize = 0;
static Eina_Bool fullscreen = EINA_FALSE;

static void
my_bt_38_alpha_on(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   evas_object_hide(bg);
   elm_win_alpha_set(win, EINA_TRUE);
}

static void
my_bt_38_alpha_off(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Evas_Object *bg = evas_object_data_get(win, "bg");
   evas_object_show(bg);
   elm_win_alpha_set(win, EINA_FALSE);
}

static Eina_Bool
_unic(void *data)
{
   printf("activate\n");
   elm_win_activate(data);
   return EINA_FALSE;
}

static Eina_Bool
_unwith(void *data)
{
   printf("show\n");
   evas_object_show(data);
   elm_win_activate(data);
   return EINA_FALSE;
}

static void
my_bt_38_iconify(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   printf("iconify, current %i\n", elm_win_iconified_get(win));
   elm_win_iconified_set(win, EINA_TRUE);
   ecore_timer_add(10.0, _unic, win);
}

static void
my_bt_38_withdraw(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   printf("withdraw, current %i\n", elm_win_withdrawn_get(win));
   elm_win_withdrawn_set(win, EINA_TRUE);
   ecore_timer_add(10.0, _unwith, win);
}

static void
my_bt_38_massive(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   evas_object_resize(win, 4000, 2400);
}

static void
my_ck_38_resize(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
//   Evas_Object *win = data;
   rotate_with_resize = elm_check_state_get(obj);
}

static void
my_bt_38_rot_0(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 0);
   else
     elm_win_rotation_set(win, 0);
}

static void
my_bt_38_rot_90(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 90);
   else
     elm_win_rotation_set(win, 90);
}

static void
my_bt_38_rot_180(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 180);
   else
     elm_win_rotation_set(win, 180);
}

static void
my_bt_38_rot_270(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   if (rotate_with_resize)
     elm_win_rotation_with_resize_set(win, 270);
   else
     elm_win_rotation_set(win, 270);
}

static void
my_ck_38_fullscreen(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   fullscreen = elm_check_state_get(obj);
   elm_win_fullscreen_set(win, fullscreen);
}

static void
my_ck_38_borderless(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   Eina_Bool borderless = elm_check_state_get(obj);
   elm_win_borderless_set(win, borderless);
}

static void
my_win_move(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Coord x, y;
   elm_win_screen_position_get(obj, &x, &y);
   printf("MOVE - win geom: %4i %4i\n", x, y);
}

static void
_win_resize(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Coord w, h;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   printf("RESIZE - win geom: %4ix%4i\n", w, h);
}

static void
_win_foc_in(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("FOC IN\n");
}

static void
_win_foc_out(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("FOC OUT\n");
}

static void
_close_win(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_del(data);
}

static void
_move_20_20(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_move(data, 20, 20);
}

static void
_move_0_0(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   evas_object_move(data, 0, 0);
}
static void
my_win_print(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("WIN: %s\n", (char *)data);
}

void
test_win_state(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *sl, *bx, *bx2, *bt, *ck;

   win = elm_win_add(NULL, "window-states", ELM_WIN_BASIC);
   elm_win_title_set(win, "Window States");
   evas_object_smart_callback_add(win, "moved", my_win_move, NULL);
   evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, _win_resize, NULL);
   evas_event_callback_add(evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_FOCUS_IN, _win_foc_in, NULL);
   evas_event_callback_add(evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_FOCUS_OUT, _win_foc_out, NULL);
   evas_object_smart_callback_add(win, "withdrawn", my_win_print, "withdrawn");
   evas_object_smart_callback_add(win, "iconified", my_win_print, "iconified");
   evas_object_smart_callback_add(win, "normal", my_win_print, "normal");
   evas_object_smart_callback_add(win, "stick", my_win_print, "stick");
   evas_object_smart_callback_add(win, "unstick", my_win_print, "unstick");
   evas_object_smart_callback_add(win, "fullscreen", my_win_print, "fullscreen");
   evas_object_smart_callback_add(win, "unfullscreen", my_win_print, "unfullscreen");
   evas_object_smart_callback_add(win, "maximized", my_win_print, "maximized");
   evas_object_smart_callback_add(win, "unmaximized", my_win_print, "unmaximized");
   evas_object_smart_callback_add(win, "ioerr", my_win_print, "ioerr");
   evas_object_smart_callback_add(win, "indicator,prop,changed", my_win_print, "indicator,prop,changed");
   evas_object_smart_callback_add(win, "rotation,changed", my_win_print, "rotation,changed");
   evas_object_smart_callback_add(win, "profile,changed", my_win_print, "profile,changed");
   evas_object_smart_callback_add(win, "focused", my_win_print, "focused");
   evas_object_smart_callback_add(win, "unfocused", my_win_print, "unfocused");
   evas_object_smart_callback_add(win, "focus,out", my_win_print, "focus,out");
   evas_object_smart_callback_add(win, "focus,in", my_win_print, "focus,in");
   evas_object_smart_callback_add(win, "delete,request", my_win_print, "delete,request");
   evas_object_smart_callback_add(win, "wm,rotation,changed", my_win_print, "wm,rotation,changed");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
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

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Iconify");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_iconify, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Withdraw");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_withdraw, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Massive");
   evas_object_smart_callback_add(bt, "clicked", my_bt_38_massive, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Move 20 20");
   evas_object_smart_callback_add(bt, "clicked", _move_20_20, win);
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
   elm_slider_inverted_set(sl, EINA_TRUE);
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "resize");
   elm_check_state_set(ck, rotate_with_resize);
   evas_object_smart_callback_add(ck, "changed", my_ck_38_resize, win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ck, 0.02, 0.99);
   evas_object_show(ck);
   elm_box_pack_end(bx, ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "fullscreen");
   elm_check_state_set(ck, fullscreen);
   evas_object_smart_callback_add(ck, "changed", my_ck_38_fullscreen, win);
   evas_object_size_hint_weight_set(ck, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ck, 0.02, 0.99);
   evas_object_show(ck);
   elm_box_pack_end(bx, ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "borderless");
   elm_check_state_set(ck, fullscreen);
   evas_object_smart_callback_add(ck, "changed", my_ck_38_borderless, win);
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

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Move 0 0");
   evas_object_smart_callback_add(bt, "clicked", _move_0_0, win);
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
test_win_state2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *sl, *bx, *bx2, *bt, *ck;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "window-states2", ELM_WIN_BASIC);
   elm_win_override_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "moved", my_win_move, NULL);
   evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, _win_resize, NULL);
   elm_win_title_set(win, "Window States 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/sky_02.jpg", elm_app_data_dir_get());
   elm_bg_file_set(bg, buf, NULL);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
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

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Move 20 20");
   evas_object_smart_callback_add(bt, "clicked", _move_20_20, win);
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
   elm_slider_inverted_set(sl, EINA_TRUE);
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "resize");
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

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Move 0 0");
   evas_object_smart_callback_add(bt, "clicked", _move_0_0, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
