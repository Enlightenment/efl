#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _App_Data
{
   Efl_Canvas_Animation        *show_anim;
   Efl_Canvas_Animation        *hide_anim;
   Elm_Button                  *button;

   Evas_Object          *pause_btn;

   Eina_Bool             is_btn_visible;
   Eina_Bool             is_anim_paused;
} App_Data;

static void
_anim_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *anim = event->info;
   App_Data *ad = data;

   if (anim)
     {
        printf("Animation has been started!\n");
        elm_object_disabled_set(ad->pause_btn, EINA_FALSE);
     }
   else
     {
        printf("Animation has been ended!\n");
        elm_object_disabled_set(ad->pause_btn, EINA_TRUE);
     }
}

static void
_anim_running_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   double *progress = event->info;
   printf("Animation is running! Current progress(%lf)\n", *progress);
}

EFL_CALLBACKS_ARRAY_DEFINE(animation_stats_cb,
  {EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_CHANGED, _anim_changed_cb },
  {EFL_CANVAS_OBJECT_ANIMATION_EVENT_ANIMATION_PROGRESS_UPDATED, _anim_running_cb },
)

static void
_start_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;

   ad->is_btn_visible = !(ad->is_btn_visible);

   if (ad->is_btn_visible)
     {
        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->show_anim, 1.0, 0.0);
        efl_text_set(obj, "Start Alpha Animation from 1.0 to 0.0");
     }
   else
     {
        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->hide_anim, 1.0, 0.0);
        efl_text_set(obj, "Start Alpha Animation from 0.0 to 1.0");
     }
}

static void
_pause_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;

   ad->is_anim_paused = !(ad->is_anim_paused);

   if (ad->is_anim_paused)
     {
        //Pause animation
        efl_canvas_object_animation_pause_set(ad->button, EINA_TRUE);
        elm_object_text_set(obj, "Resume Animation");
     }
   else
     {
        //Resume animation
        efl_canvas_object_animation_pause_set(ad->button, EINA_FALSE);
        elm_object_text_set(obj, "Pause Animation");
     }
}

static void
_win_del_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;
   free(ad);
}

void
test_efl_anim_pause(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Pause", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Pause");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, ad);

   //Button to be animated
   Evas_Object *btn = elm_button_add(win);
   elm_object_text_set(btn, "Button");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn, 200, 200);
   evas_object_move(btn, 100, 50);
   evas_object_show(btn);
   efl_event_callback_array_add(btn, animation_stats_cb(), ad);

   //Show Animation
   Efl_Canvas_Animation *show_anim = efl_add(EFL_CANVAS_ALPHA_ANIMATION_CLASS, win);
   efl_animation_alpha_set(show_anim, 0.0, 1.0);
   efl_animation_duration_set(show_anim, 2.0);
   efl_animation_final_state_keep_set(show_anim, EINA_TRUE);

   //Hide Animation
   Efl_Canvas_Animation *hide_anim = efl_add(EFL_CANVAS_ALPHA_ANIMATION_CLASS, win);
   efl_animation_alpha_set(hide_anim, 1.0, 0.0);
   efl_animation_duration_set(hide_anim, 2.0);
   efl_animation_final_state_keep_set(hide_anim, EINA_TRUE);


   //Button to start animation
   Evas_Object *start_btn = elm_button_add(win);
   elm_object_text_set(start_btn, "Start Alpha Animation from 1.0 to 0.0");
   evas_object_smart_callback_add(start_btn, "clicked", _start_btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(start_btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(start_btn, 200, 50);
   evas_object_move(start_btn, 100, 300);
   evas_object_show(start_btn);

   //Button to pause animation
   Evas_Object *pause_btn = elm_button_add(win);
   elm_object_text_set(pause_btn, "Pause Animation");
   evas_object_smart_callback_add(pause_btn, "clicked", _pause_btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(pause_btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(pause_btn, 200, 50);
   evas_object_move(pause_btn, 100, 350);
   evas_object_show(pause_btn);
   //Pause button becomes enabled only if the animation is started
   elm_object_disabled_set(pause_btn, EINA_TRUE);

   //Initialize App Data
   ad->show_anim = show_anim;
   ad->hide_anim = hide_anim;
   ad->button = btn;
   ad->pause_btn = pause_btn;
   ad->is_btn_visible = EINA_TRUE;
   ad->is_anim_paused = EINA_FALSE;

   evas_object_resize(win, 400, 450);
   evas_object_show(win);
}
