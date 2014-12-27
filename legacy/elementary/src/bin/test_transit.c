#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define TWEEN_NUM 7
#define BTN_NUM (TWEEN_NUM + 1)
#define BTN_W 50
#define BTN_H 50
#define WIN_H (BTN_NUM * BTN_H)
#define WIN_W WIN_H
#define TRANSIT_DURATION 1.0

typedef struct _Custom_Effect Custom_Effect;

struct _Custom_Effect
{
   struct _size
     {
        Evas_Coord w, h;
     } from, to;
};

static void
_custom_op(Elm_Transit_Effect *effect, Elm_Transit *transit, double progress)
{
   if (!effect) return;
   Evas_Coord w, h;
   Evas_Object *obj;
   const Eina_List *elist;

   Custom_Effect *custom_effect = effect;
   const Eina_List *objs = elm_transit_objects_get(transit);

   if (progress < 0.5)
     {
        h = custom_effect->from.h + (custom_effect->to.h * progress * 2);
        w = custom_effect->from.w;
     }
   else
     {
        h = custom_effect->from.h + custom_effect->to.h;
        w = custom_effect->from.w + \
            (custom_effect->to.w * (progress - 0.5) * 2);
     }

   EINA_LIST_FOREACH(objs, elist, obj)
      evas_object_resize(obj, w, h);
}

static Elm_Transit_Effect *
_custom_context_new(Evas_Coord from_w, Evas_Coord from_h, Evas_Coord to_w, Evas_Coord to_h)
{
   Custom_Effect *custom_effect;

   custom_effect = calloc(1, sizeof(Custom_Effect));
   if (!custom_effect) return NULL;

   custom_effect->from.w = from_w;
   custom_effect->from.h = from_h;
   custom_effect->to.w = to_w - from_w;
   custom_effect->to.h = to_h - from_h;

   return custom_effect;
}

static void
_custom_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit EINA_UNUSED)
{
   Custom_Effect *custom_effect = effect;
   free(custom_effect);
}

static void
_transit_rotation_color(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Transit *trans;

   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);
   elm_transit_auto_reverse_set(trans, EINA_TRUE);
   elm_transit_repeat_times_set(trans, 2);

   /* Color Effect */
   elm_transit_effect_color_add(trans, 100, 255, 100, 255, 50, 30, 50, 50);

   /* Rotation Effect */
   elm_transit_effect_rotation_add(trans, 0.0, 135.0);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_wipe(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Transit *trans;

   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);
   elm_transit_auto_reverse_set(trans, EINA_TRUE);

   elm_transit_effect_wipe_add(trans,
                               ELM_TRANSIT_EFFECT_WIPE_TYPE_HIDE,
                               ELM_TRANSIT_EFFECT_WIPE_DIR_RIGHT);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_del_cb(void *data, Elm_Transit *transit EINA_UNUSED)
{
   evas_object_freeze_events_set(data, EINA_FALSE);
}

static void
_transit_image_animation(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_List *images = NULL;
   char buf[PATH_MAX];
   Elm_Transit *trans;
   Evas_Object *ic = data;

   snprintf(buf, sizeof(buf), "%s/images/icon_19.png", elm_app_data_dir_get());
   images = eina_list_append(images, eina_stringshare_add(buf));

   snprintf(buf, sizeof(buf), "%s/images/icon_00.png", elm_app_data_dir_get());
   images = eina_list_append(images, eina_stringshare_add(buf));

   snprintf(buf, sizeof(buf), "%s/images/icon_11.png", elm_app_data_dir_get());
   images = eina_list_append(images, eina_stringshare_add(buf));

   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   images = eina_list_append(images, eina_stringshare_add(buf));

   trans = elm_transit_add();
   elm_transit_del_cb_set(trans, _transit_del_cb, obj);
   elm_transit_object_add(trans, ic);
   elm_transit_effect_image_animation_add(trans, images);
   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);

   evas_object_freeze_events_set(obj, EINA_TRUE);
}

static void
_transit_resizing(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Transit *trans;

   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);

   elm_transit_effect_resizing_add(trans, 100, 50, 300, 150);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_flip(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Transit *trans;
   Evas_Object *obj2 = data;

   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);
   elm_transit_object_add(trans, obj2);

   elm_transit_effect_flip_add(trans, ELM_TRANSIT_EFFECT_FLIP_AXIS_X, EINA_TRUE);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_zoom(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Transit *trans;

   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);

   elm_transit_effect_zoom_add(trans, 1.0, 3.0);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_blend(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Transit *trans;
   Evas_Object *obj2 = data;

   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);
   elm_transit_object_add(trans, obj2);

   elm_transit_effect_blend_add(trans);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_fade(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Transit *trans;
   Evas_Object *obj2 = data;

   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);
   elm_transit_object_add(trans, obj2);

   elm_transit_effect_fade_add(trans);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_resizable_flip(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Transit *trans;
   Evas_Object *obj2 = data;

   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);
   elm_transit_object_add(trans, obj2);

   elm_transit_effect_resizable_flip_add(trans, ELM_TRANSIT_EFFECT_FLIP_AXIS_Y, EINA_TRUE);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_tween_del_cb(void *data, Elm_Transit *trans EINA_UNUSED)
{
   Evas_Object *btn = data;
   int disabled = (int)(uintptr_t)evas_object_data_get(btn, "disabled");
   if (disabled > 0)
     evas_object_data_set(btn, "disabled", (void *)(uintptr_t)(--disabled));
   if (disabled == 0) elm_object_disabled_set(btn, EINA_FALSE);
}

static void
_transit_tween(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Transit_Tween_Mode ettm;
   Elm_Transit *trans;
   Evas_Coord x;
   Evas_Object *bt_all = data;
   int disabled;

   evas_object_geometry_get(obj, &x, NULL, NULL, NULL);
   ettm = (Elm_Transit_Tween_Mode) evas_object_data_get(obj, "tween");

   trans = elm_transit_add();
   elm_transit_tween_mode_set(trans, ettm);
   elm_transit_effect_translation_add(trans, 0, 0,
                                      ((x ? -1 : 1) * (WIN_W - BTN_W)), 0);
   elm_transit_object_add(trans, obj);
   elm_transit_duration_set(trans, TRANSIT_DURATION);
   elm_transit_objects_final_state_keep_set(trans, EINA_TRUE);
   elm_transit_del_cb_set(trans, _transit_tween_del_cb, bt_all);
   elm_transit_go(trans);

   disabled = (int)(uintptr_t)evas_object_data_get(bt_all, "disabled");
   evas_object_data_set(bt_all, "disabled", (void *)(uintptr_t)(++disabled));
   elm_object_disabled_set(bt_all, EINA_TRUE);
}

static void
_transit_tween_all(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object **bt = data;
   Elm_Transit *trans;
   Evas_Coord x;
   int i;

   for (i = 0; i < TWEEN_NUM; i++)
     {
        evas_object_geometry_get(bt[i], &x, NULL, NULL, NULL);

        trans = elm_transit_add();
        elm_transit_tween_mode_set(trans, i);
        elm_transit_effect_translation_add(trans, 0, 0,
                                           ((x ? -1 : 1) * (WIN_W - BTN_W)), 0);
        elm_transit_object_add(trans, bt[i]);
        elm_transit_duration_set(trans, TRANSIT_DURATION);
        elm_transit_objects_final_state_keep_set(trans, EINA_TRUE);
        elm_transit_del_cb_set(trans, _transit_tween_del_cb, obj);
        elm_transit_go(trans);
     }

   elm_object_disabled_set(obj, EINA_TRUE);
}

static void
_win_delete_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object **bt = data;
   free(bt);
}

/* Translation, Rotation, Color, Wipe, ImagemAnimation Effect */
void
test_transit(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt, *ic;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("transit", "Transit");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   ic = elm_image_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_11.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Image Animation");
   elm_object_part_content_set(bt, "icon", ic);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);
   evas_object_smart_callback_add(bt, "clicked", _transit_image_animation, ic);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Rotation + Color");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked",
                                  _transit_rotation_color, NULL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Wipe Effect");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _transit_wipe, NULL);

   evas_object_resize(win, 300, 300);
   evas_object_show(win);
}

/* Resizing Effect */
void
test_transit_resizing(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt;

   win = elm_win_util_standard_add("transit2", "Transit Resizing");
   elm_win_autodel_set(win, EINA_TRUE);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Resizing Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 100);
   evas_object_resize(bt, 100, 50);
   evas_object_smart_callback_add(bt, "clicked", _transit_resizing, NULL);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

/* Flip Effect */
void
test_transit_flip(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt, *bt2;

   win = elm_win_util_standard_add("transit3", "Transit Flip");
   elm_win_autodel_set(win, EINA_TRUE);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Front Button - Flip Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 50);
   evas_object_resize(bt, 200, 200);

   bt2 = elm_button_add(win);
   elm_object_text_set(bt2, "Back Button - Flip Effect");
   evas_object_move(bt2, 50, 50);
   evas_object_resize(bt2, 200, 200);

   evas_object_resize(win, 300, 300);
   evas_object_show(win);

   evas_object_smart_callback_add(bt, "clicked", _transit_flip, bt2);
   evas_object_smart_callback_add(bt2, "clicked", _transit_flip, bt);
}

/* Zoom Effect */
void
test_transit_zoom(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt;

   win = elm_win_util_standard_add("transit4", "Transit Zoom");
   elm_win_autodel_set(win, EINA_TRUE);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Zoom Effect");
   evas_object_resize(bt, 100, 50);
   evas_object_move(bt, 100, 125);
   evas_object_show(bt);

   evas_object_smart_callback_add(bt, "clicked", _transit_zoom, NULL);

   evas_object_resize(win, 300, 300);
   evas_object_show(win);
}

/* Blend Effect */
void
test_transit_blend(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt, *bt2, *ic;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("transit5", "Transit Blend");
   elm_win_autodel_set(win, EINA_TRUE);

   ic = elm_image_add(win);
   snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_max_set(ic, 50, 50);

   bt = elm_button_add(win);
   elm_object_part_content_set(bt, "icon", ic);
   elm_object_text_set(bt, "Before Button - Blend Effect");
   evas_object_move(bt, 25, 125);
   evas_object_resize(bt, 250, 50);
   evas_object_show(bt);

   ic = elm_image_add(win);
   snprintf(buf, sizeof(buf), "%s/images/rock_02.jpg", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_max_set(ic, 50, 50);

   bt2 = elm_button_add(win);
   elm_object_part_content_set(bt2, "icon", ic);
   elm_object_text_set(bt2, "After Button - Blend Effect");
   evas_object_move(bt2, 25, 125);
   evas_object_resize(bt2, 250, 50);

   evas_object_resize(win, 300, 300);
   evas_object_show(win);

   evas_object_smart_callback_add(bt, "clicked", _transit_blend, bt2);
   evas_object_smart_callback_add(bt2, "clicked", _transit_blend, bt);
}

/* Fade Effect */
void
test_transit_fade(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt, *bt2, *ic;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("transit6","Transit Fade");
   elm_win_autodel_set(win, EINA_TRUE);

   ic = elm_image_add(win);
   snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_max_set(ic, 50, 50);

   bt = elm_button_add(win);
   elm_object_part_content_set(bt, "icon", ic);
   elm_object_text_set(bt, "Before Button - Fade Effect");
   evas_object_move(bt, 25, 125);
   evas_object_resize(bt, 250, 50);
   evas_object_show(bt);

   ic = elm_image_add(win);
   snprintf(buf, sizeof(buf), "%s/images/rock_02.jpg", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_max_set(ic, 50, 50);

   bt2 = elm_button_add(win);
   elm_object_part_content_set(bt2, "icon", ic);
   elm_object_text_set(bt2, "After Button - Fade Effect");
   evas_object_move(bt2, 25, 125);
   evas_object_resize(bt2, 250, 50);

   evas_object_resize(win, 300, 300);
   evas_object_show(win);

   evas_object_smart_callback_add(bt, "clicked", _transit_fade, bt2);
   evas_object_smart_callback_add(bt2, "clicked", _transit_fade, bt);
}

/* Resizable Flip Effect */
void
test_transit_resizable_flip(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt, *bt2;

   win = elm_win_util_standard_add("transit7", "Transit Resizable Flip");
   elm_win_autodel_set(win, EINA_TRUE);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Front Button - Resizable Flip Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 100);
   evas_object_resize(bt, 250, 30);

   bt2 = elm_button_add(win);
   elm_object_text_set(bt2, "Back Button - Resizable Flip Effect");
   evas_object_move(bt2, 50, 100);
   evas_object_resize(bt2, 300, 200);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   evas_object_smart_callback_add(bt, "clicked", _transit_resizable_flip, bt2);
   evas_object_smart_callback_add(bt2, "clicked", _transit_resizable_flip, bt);
}

/* Custom Effect */
void
test_transit_custom(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt;
   Elm_Transit *trans;
   Elm_Transit_Effect *effect_context;

   win = elm_win_util_standard_add("transit8", "Transit Custom");
   elm_win_autodel_set(win, EINA_TRUE);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button - Custom Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 50);
   evas_object_resize(bt, 150, 150);

   /* Adding Transit */
   trans = elm_transit_add();
   elm_transit_auto_reverse_set(trans, EINA_TRUE);
   elm_transit_tween_mode_set(trans, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
   effect_context = _custom_context_new(150, 150, 50, 50);
   elm_transit_object_add(trans, bt);
   elm_transit_effect_add(trans,
                          _custom_op, effect_context,
                          _custom_context_free);
   elm_transit_duration_set(trans, 5.0);
   elm_transit_repeat_times_set(trans, -1);
   elm_transit_go(trans);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

/* Chain Transit Effect */
void
test_transit_chain(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bt, *bt2, *bt3, *bt4;
   Elm_Transit *trans, *trans2, *trans3, *trans4;

   win = elm_win_util_standard_add("transit9", "Transit Chain");
   elm_win_autodel_set(win, EINA_TRUE);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Chain 1");
   evas_object_resize(bt, 100, 100);
   evas_object_move(bt, 0, 0);
   evas_object_show(bt);

   bt2 = elm_button_add(win);
   elm_object_text_set(bt2, "Chain 2");
   evas_object_resize(bt2, 100, 100);
   evas_object_move(bt2, 300, 0);
   evas_object_show(bt2);

   bt3 = elm_button_add(win);
   elm_object_text_set(bt3, "Chain 3");
   evas_object_resize(bt3, 100, 100);
   evas_object_move(bt3, 300, 300);
   evas_object_show(bt3);

   bt4 = elm_button_add(win);
   elm_object_text_set(bt4, "Chain 4");
   evas_object_resize(bt4, 100, 100);
   evas_object_move(bt4, 0, 300);
   evas_object_show(bt4);

   trans = elm_transit_add();
   elm_transit_tween_mode_set(trans, ELM_TRANSIT_TWEEN_MODE_ACCELERATE);
   elm_transit_effect_translation_add(trans, 0, 0, 300, 0);
   elm_transit_object_add(trans, bt);
   elm_transit_duration_set(trans, 1);
   elm_transit_objects_final_state_keep_set(trans, EINA_TRUE);
   elm_transit_go(trans);

   trans2 = elm_transit_add();
   elm_transit_tween_mode_set(trans2, ELM_TRANSIT_TWEEN_MODE_ACCELERATE);
   elm_transit_effect_translation_add(trans2, 0, 0, 0, 300);
   elm_transit_object_add(trans2, bt2);
   elm_transit_duration_set(trans2, 1);
   elm_transit_objects_final_state_keep_set(trans2, EINA_TRUE);
   elm_transit_chain_transit_add(trans, trans2);

   trans3 = elm_transit_add();
   elm_transit_tween_mode_set(trans3, ELM_TRANSIT_TWEEN_MODE_ACCELERATE);
   elm_transit_effect_translation_add(trans3, 0, 0, -300, 0);
   elm_transit_object_add(trans3, bt3);
   elm_transit_duration_set(trans3, 1);
   elm_transit_objects_final_state_keep_set(trans3, EINA_TRUE);
   elm_transit_chain_transit_add(trans2, trans3);

   trans4 = elm_transit_add();
   elm_transit_tween_mode_set(trans4, ELM_TRANSIT_TWEEN_MODE_ACCELERATE);
   elm_transit_effect_translation_add(trans4, 0, 0, 0, -300);
   elm_transit_object_add(trans4, bt4);
   elm_transit_duration_set(trans4, 1);
   elm_transit_objects_final_state_keep_set(trans4, EINA_TRUE);
   elm_transit_chain_transit_add(trans3, trans4);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

/* Transit Tween Mode */
void
test_transit_tween(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *label;
   Evas_Object **bt = malloc(sizeof(Evas_Object *) * BTN_NUM);
   int i;

   char *modes[] = {"LINEAR", "SINUSOIDAL", "DECELERATE", "ACCELERATE",
                    "DIVISOR_INTERP", "BOUNCE", "SPRING"};

   win = elm_win_add(NULL, "transit10",  ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit Tween");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _win_delete_cb, bt);

   bg = elm_bg_add(win);
   evas_object_size_hint_min_set(bg, WIN_W, WIN_H);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bt[TWEEN_NUM] = elm_button_add(win);
   elm_object_text_set(bt[TWEEN_NUM], "Go All");
   evas_object_resize(bt[TWEEN_NUM], WIN_W, BTN_H);
   evas_object_move(bt[TWEEN_NUM], 0, (WIN_H - BTN_H));
   evas_object_show(bt[TWEEN_NUM]);
   evas_object_smart_callback_add(bt[TWEEN_NUM], "clicked", _transit_tween_all,
                                  bt);

   for (i = 0; i < TWEEN_NUM; i++)
     {
        label = elm_label_add(win);
        elm_object_text_set(label, modes[i]);
        evas_object_resize(label, WIN_W, BTN_H);
        evas_object_move(label, 0, (i * BTN_H));
        evas_object_show(label);

        bt[i] = elm_button_add(win);
        evas_object_data_set(bt[i], "tween", (void *)(uintptr_t)i);
        elm_object_text_set(bt[i], "Go");
        evas_object_resize(bt[i], BTN_W, BTN_H);
        evas_object_move(bt[i], 0, (i * BTN_H));
        evas_object_show(bt[i]);
        evas_object_smart_callback_add(bt[i], "clicked", _transit_tween,
                                       bt[TWEEN_NUM]);
     }

   evas_object_resize(win, WIN_W, WIN_H);
   evas_object_show(win);
}
