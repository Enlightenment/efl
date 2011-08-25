#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

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
_custom_context_free(Elm_Transit_Effect *effect, Elm_Transit *transit __UNUSED__)
{
   Custom_Effect *custom_effect = effect;
   free(custom_effect);
}

static void
_transit_rotation_translation_color(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Transit *trans;

   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);
   elm_transit_auto_reverse_set(trans, EINA_TRUE);
   elm_transit_repeat_times_set(trans, 2);

   /* Translation Effect */
   elm_transit_effect_translation_add(trans, -70.0, -150.0, 70.0, 150.0);

   /* Color Effect */
   elm_transit_effect_color_add(trans, 100, 255, 100, 255, 200, 50, 200, 50);

   /* Rotation Effect */
   elm_transit_effect_rotation_add(trans, 0.0, 135.0);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_wipe(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
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
_transit_image_animation(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
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
   elm_transit_object_add(trans, ic);

   elm_transit_effect_image_animation_add(trans, images);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_resizing(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Transit *trans;

   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);

   elm_transit_effect_resizing_add(trans, 100, 50, 300, 150);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_flip(void *data, Evas_Object *obj, void *event_info __UNUSED__)
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
_transit_zoom(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Transit *trans;

   trans = elm_transit_add();
   elm_transit_object_add(trans, obj);

   elm_transit_effect_zoom_add(trans, 1.0, 3.0);

   elm_transit_duration_set(trans, 5.0);
   elm_transit_go(trans);
}

static void
_transit_blend(void *data, Evas_Object *obj, void *event_info __UNUSED__)
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
_transit_fade(void *data, Evas_Object *obj, void *event_info __UNUSED__)
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
_transit_resizable_flip(void *data, Evas_Object *obj, void *event_info __UNUSED__)
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

/* Translation, Rotation, Color, Wipe, ImagemAnimation Effect */
void
test_transit(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bt, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "transit", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_min_set(bx, 318, 318);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_11.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "ImageAnimation Effect");
   elm_button_icon_set(bt, ic);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_show(ic);
   evas_object_smart_callback_add(bt, "clicked", _transit_image_animation, ic);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Color, Rotation and Translation");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked",
                                  _transit_rotation_translation_color, NULL);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Wipe Effect");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _transit_wipe, NULL);

   evas_object_show(win);
}

/* Resizing Effect */
void
test_transit2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt;

   win = elm_win_add(NULL, "transit2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 2");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 400, 400);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Resizing Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 100);
   evas_object_resize(bt, 100, 50);
   evas_object_smart_callback_add(bt, "clicked", _transit_resizing, NULL);

   evas_object_show(win);
}

/* Flip Effect */
void
test_transit3(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt, *bt2;

   win = elm_win_add(NULL, "transit3", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 3");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 300, 300);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Front Button - Flip Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 50);
   evas_object_resize(bt, 200, 200);

   bt2 = elm_button_add(win);
   elm_object_text_set(bt2, "Back Button - Flip Effect");
   evas_object_move(bt2, 50, 50);
   evas_object_resize(bt2, 200, 200);

   evas_object_show(win);

   evas_object_smart_callback_add(bt, "clicked", _transit_flip, bt2);
   evas_object_smart_callback_add(bt2, "clicked", _transit_flip, bt);
}

/* Zoom Effect */
void
test_transit4(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt;

   win = elm_win_add(NULL, "transit4", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 4");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 300, 300);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Zoom Effect");
   evas_object_resize(bt, 100, 50);
   evas_object_move(bt, 100, 125);
   evas_object_show(bt);

   evas_object_smart_callback_add(bt, "clicked", _transit_zoom, NULL);

   evas_object_show(win);
}

/* Blend Effect */
void
test_transit5(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt, *bt2, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "transit5", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 5");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 300, 300);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_max_set(ic, 50, 50);

   bt = elm_button_add(win);
   elm_button_icon_set(bt, ic);
   elm_object_text_set(bt, "Before Button - Blend Effect");
   evas_object_move(bt, 25, 125);
   evas_object_resize(bt, 250, 50);
   evas_object_show(bt);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/rock_02.jpg", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_max_set(ic, 50, 50);

   bt2 = elm_button_add(win);
   elm_button_icon_set(bt2, ic);
   elm_object_text_set(bt2, "After Button - Blend Effect");
   evas_object_move(bt2, 25, 125);
   evas_object_resize(bt2, 250, 50);

   evas_object_show(win);

   evas_object_smart_callback_add(bt, "clicked", _transit_blend, bt2);
   evas_object_smart_callback_add(bt2, "clicked", _transit_blend, bt);
}

/* Fade Effect */
void
test_transit6(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt, *bt2, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "transit6", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 6");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 300, 300);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/rock_01.jpg", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_max_set(ic, 50, 50);

   bt = elm_button_add(win);
   elm_button_icon_set(bt, ic);
   elm_object_text_set(bt, "Before Button - Fade Effect");
   evas_object_move(bt, 25, 125);
   evas_object_resize(bt, 250, 50);
   evas_object_show(bt);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/rock_02.jpg", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_max_set(ic, 50, 50);

   bt2 = elm_button_add(win);
   elm_button_icon_set(bt2, ic);
   elm_object_text_set(bt2, "After Button - Fade Effect");
   evas_object_move(bt2, 25, 125);
   evas_object_resize(bt2, 250, 50);

   evas_object_show(win);

   evas_object_smart_callback_add(bt, "clicked", _transit_fade, bt2);
   evas_object_smart_callback_add(bt2, "clicked", _transit_fade, bt);
}

/* Resizable Flip Effect */
void
test_transit7(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt, *bt2;

   win = elm_win_add(NULL, "transit7", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 7");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 400, 400);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Front Button - Resizable Flip Effect");
   evas_object_show(bt);
   evas_object_move(bt, 50, 100);
   evas_object_resize(bt, 250, 30);

   bt2 = elm_button_add(win);
   elm_object_text_set(bt2, "Back Button - Resizable Flip Effect");
   evas_object_move(bt2, 50, 100);
   evas_object_resize(bt2, 300, 200);

   evas_object_show(win);

   evas_object_smart_callback_add(bt, "clicked", _transit_resizable_flip, bt2);
   evas_object_smart_callback_add(bt2, "clicked", _transit_resizable_flip, bt);
}

/* Custom Effect */
void
test_transit8(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt;
   Elm_Transit *trans;
   Elm_Transit_Effect *effect_context;

   win = elm_win_add(NULL, "transit8", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 8");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

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
}

/* Chain Transit Effect */
void
test_transit9(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bt, *bt2, *bt3, *bt4;
   Elm_Transit *trans, *trans2, *trans3, *trans4;

   win = elm_win_add(NULL, "transit9", ELM_WIN_BASIC);
   elm_win_title_set(win, "Transit 9");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 400, 400);
   evas_object_show(win);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

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
}


#endif
