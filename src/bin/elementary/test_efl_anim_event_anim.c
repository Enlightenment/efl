#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _App_Data
{
   Efl_Canvas_Animation_Player *target;

   Eina_Bool             is_btn_visible;
} App_Data;

static void
_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;

   ad->is_btn_visible = !(ad->is_btn_visible);

   if (ad->is_btn_visible)
     {
        //Start animtion by showing button
        evas_object_show(ad->target);
        elm_object_text_set(obj, "Hide Button");
     }
   else
     {
        //Start animtion by hiding button
        evas_object_hide(ad->target);
        elm_object_text_set(obj, "Show Button");
     }
}

static void
_win_del_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;
   free(ad);
}

void
test_efl_anim_event_anim(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Event Animation", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Event Animation");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, ad);

   //Button to be animated
   Evas_Object *btn = elm_button_add(win);
   elm_object_text_set(btn, "Button");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn, 200, 200);
   evas_object_move(btn, 100, 50);

   //Show Animation
   Efl_Canvas_Animation *show_anim = efl_add(EFL_CANVAS_ANIMATION_ALPHA_CLASS, win);
   efl_animation_alpha_set(show_anim, 0.0, 1.0);
   efl_animation_duration_set(show_anim, 1.0);
   efl_animation_final_state_keep_set(show_anim, EINA_TRUE);
   efl_canvas_object_event_animation_set(btn, EFL_GFX_ENTITY_EVENT_SHOW, show_anim);

   //Show button after setting event animation to show animation for show event
   evas_object_show(btn);

   //Hide Animation
   Efl_Canvas_Animation *hide_anim = efl_add(EFL_CANVAS_ANIMATION_ALPHA_CLASS, win);
   efl_animation_alpha_set(hide_anim, 1.0, 0.0);
   efl_animation_duration_set(hide_anim, 1.0);
   efl_animation_final_state_keep_set(hide_anim, EINA_TRUE);
   efl_canvas_object_event_animation_set(btn, EFL_GFX_ENTITY_EVENT_HIDE, hide_anim);

   //Initialize App Data
   ad->target = btn;
   ad->is_btn_visible = EINA_TRUE;

   //Button to start animation
   Evas_Object *btn2 = elm_button_add(win);
   elm_object_text_set(btn2, "Hide Button");
   evas_object_smart_callback_add(btn2, "clicked", _btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn2, 200, 50);
   evas_object_move(btn2, 100, 300);
   evas_object_show(btn2);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
