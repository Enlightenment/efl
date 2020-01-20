#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _App_Data
{
   Efl_Canvas_Animation        *show_anim;
   Efl_Canvas_Animation        *hide_anim;
   Elm_Button                  *button;

   Evas_Object          *start_btn;
   Evas_Object          *play_count_spin;
   Evas_Object          *repeat_mode_spin;

   Eina_Bool             is_btn_visible;
} App_Data;

Efl_Canvas_Animation_Repeat_Mode
_anim_repeat_mode_get(Evas_Object *spinner)
{
   int value = elm_spinner_value_get(spinner);

   if (value == 0)
     return EFL_CANVAS_ANIMATION_REPEAT_MODE_RESTART;
   else
     return EFL_CANVAS_ANIMATION_REPEAT_MODE_REVERSE;
}


static void
_anim_changed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *anim = event->info;
   App_Data *ad = data;

   if (anim)
     {
        printf("Animation has been started!\n");
     }
   else
     {
        printf("Animation has been ended!\n");
        Efl_Canvas_Animation_Repeat_Mode repeat_mode = _anim_repeat_mode_get(ad->repeat_mode_spin);
        if (repeat_mode == EFL_CANVAS_ANIMATION_REPEAT_MODE_REVERSE)
          {
             int play_count = elm_spinner_value_get(ad->play_count_spin);
             if (play_count % 2 == 0)
               {
                  ad->is_btn_visible = !(ad->is_btn_visible);
                  if (ad->is_btn_visible)
                    elm_object_text_set(ad->start_btn, "Start Alpha Animation from 1.0 to 0.0");
                  else
                    elm_object_text_set(ad->start_btn, "Start Alpha Animation from 0.0 to 1.0");
               }
          }
        elm_object_disabled_set(ad->play_count_spin, EINA_FALSE);
        elm_object_disabled_set(ad->repeat_mode_spin, EINA_FALSE);
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

   int play_count = elm_spinner_value_get(ad->play_count_spin);
   elm_object_disabled_set(ad->play_count_spin, EINA_TRUE);

   Efl_Canvas_Animation_Repeat_Mode repeat_mode = _anim_repeat_mode_get(ad->repeat_mode_spin);
   elm_object_disabled_set(ad->repeat_mode_spin, EINA_TRUE);

   if (ad->is_btn_visible)
     {
        //Set animation play count
        efl_animation_play_count_set(ad->show_anim, play_count);

        //Set animation repeat mode
        efl_animation_repeat_mode_set(ad->show_anim, repeat_mode);

        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->show_anim, 1.0, 0.0);
        efl_text_set(obj, "Start Alpha Animation from 1.0 to 0.0");
     }
   else
     {
        //Set animation repeat count
        efl_animation_play_count_set(ad->hide_anim, play_count);

        //Set animation repeat mode
        efl_animation_repeat_mode_set(ad->hide_anim, repeat_mode);

        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->hide_anim, 1.0, 0.0);
        efl_text_set(obj, "Start Alpha Animation from 0.0 to 1.0");
     }
}

static void
_win_del_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;
   free(ad);
}

void
test_efl_anim_repeat(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_util_standard_add(NULL, "Efl Animation Repeat");
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
   efl_animation_duration_set(show_anim, 1.0);
   efl_animation_final_state_keep_set(show_anim, EINA_TRUE);

   //Hide Animation
   Efl_Canvas_Animation *hide_anim = efl_add(EFL_CANVAS_ALPHA_ANIMATION_CLASS, win);
   efl_animation_alpha_set(hide_anim, 1.0, 0.0);
   efl_animation_duration_set(hide_anim, 1.0);
   efl_animation_final_state_keep_set(hide_anim, EINA_TRUE);


   //Button to start animation
   Evas_Object *start_btn = elm_button_add(win);
   elm_object_text_set(start_btn, "Start Alpha Animation from 1.0 to 0.0");
   evas_object_smart_callback_add(start_btn, "clicked", _start_btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(start_btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(start_btn, 200, 50);
   evas_object_move(start_btn, 100, 300);
   evas_object_show(start_btn);

   //Spinner to set animation play count
   Evas_Object *play_count_spin = elm_spinner_add(win);
   elm_spinner_label_format_set(play_count_spin, "Play Count (0 is infinite): %d");
   elm_spinner_editable_set(play_count_spin, EINA_FALSE);
   elm_spinner_min_max_set(play_count_spin, 0, 3);
   elm_spinner_value_set(play_count_spin, 1);
   evas_object_resize(play_count_spin, 200, 50);
   evas_object_move(play_count_spin, 100, 350);
   evas_object_show(play_count_spin);

   //Spinner to set animation repeat mode
   Evas_Object *repeat_mode_spin = elm_spinner_add(win);
   elm_spinner_label_format_set(repeat_mode_spin, "Repeat Mode: %s");
   elm_spinner_editable_set(repeat_mode_spin, EINA_FALSE);
   elm_spinner_min_max_set(repeat_mode_spin, 0, 1);
   elm_spinner_value_set(repeat_mode_spin, 0);
   elm_spinner_special_value_add(repeat_mode_spin, 0, "Restart");
   elm_spinner_special_value_add(repeat_mode_spin, 1, "Reverse");
   evas_object_resize(repeat_mode_spin, 200, 50);
   evas_object_move(repeat_mode_spin, 100, 400);
   evas_object_show(repeat_mode_spin);

   //Initialize App Data
   ad->show_anim = show_anim;
   ad->hide_anim = hide_anim;
   ad->start_btn = start_btn;
   ad->play_count_spin = play_count_spin;
   ad->repeat_mode_spin = repeat_mode_spin;
   ad->is_btn_visible = EINA_TRUE;
   ad->button = btn;

   evas_object_resize(win, 400, 500);
   evas_object_show(win);
}
