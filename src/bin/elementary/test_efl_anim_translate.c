#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _App_Data
{
   Efl_Animation        *translate_rb_anim;
   Efl_Animation        *translate_lt_anim;
   Efl_Animation_Player *anim_obj;

   Eina_Bool             is_btn_translated;
} App_Data;

static void
_anim_started_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   printf("Animation has been started!\n");
}

static void
_anim_ended_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   printf("Animation has been ended!\n");
}

static void
_anim_running_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Animation_Player_Running_Event_Info *event_info = event->info;
   double progress = event_info->progress;
   printf("Animation is running! Current progress(%lf)\n", progress);
}

static void
_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;

   ad->is_btn_translated = !(ad->is_btn_translated);

   if (ad->is_btn_translated)
     {
        //Create Animation Object from Animation
        efl_animation_player_animation_set(ad->anim_obj, ad->translate_rb_anim);
        efl_text_set(obj, "Start Translate Animation to left top");
     }
   else
     {
        //Create Animation Object from Animation
        efl_animation_player_animation_set(ad->anim_obj, ad->translate_lt_anim);
        efl_text_set(obj, "Start Translate Animation to right bottom");
     }
     
   //Let Animation Object start animation
   efl_player_start(ad->anim_obj);
}

static void
_win_del_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;
   free(ad);
}

void
test_efl_anim_translate(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Translate", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Translate");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, ad);

   //Button to be animated
   Evas_Object *btn = elm_button_add(win);
   elm_object_text_set(btn, "Target");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn, 150, 150);
   evas_object_move(btn, 125, 100);
   evas_object_show(btn);

   //Translate Animation to right bottom relatively
   Efl_Animation *translate_rb_anim = efl_add(EFL_ANIMATION_TRANSLATE_CLASS, win);
   efl_animation_translate_set(translate_rb_anim, 0, 0, 100, 100);
   efl_animation_duration_set(translate_rb_anim, 1.0);
   efl_animation_final_state_keep_set(translate_rb_anim, EINA_TRUE);

   //Translate Animation to left top relatively
   Efl_Animation *translate_lt_anim = efl_add(EFL_ANIMATION_TRANSLATE_CLASS, win);
   efl_animation_translate_set(translate_lt_anim, 100, 100, 0, 0);
   efl_animation_duration_set(translate_lt_anim, 1.0);
   efl_animation_final_state_keep_set(translate_lt_anim, EINA_TRUE);

   //Initialize App Data
   ad->translate_rb_anim = translate_rb_anim;
   ad->translate_lt_anim = translate_lt_anim;
   ad->anim_obj = efl_add(EFL_ANIMATION_PLAYER_CLASS, win,
                          efl_animation_player_target_set(efl_added, btn));
   //Register callback called when animation starts
   efl_event_callback_add(ad->anim_obj, EFL_ANIMATION_PLAYER_EVENT_STARTED, _anim_started_cb, NULL);
   //Register callback called when animation ends
   efl_event_callback_add(ad->anim_obj, EFL_ANIMATION_PLAYER_EVENT_ENDED, _anim_ended_cb, NULL);
   //Register callback called while animation is executed
   efl_event_callback_add(ad->anim_obj, EFL_ANIMATION_PLAYER_EVENT_RUNNING, _anim_running_cb, NULL);

   ad->is_btn_translated = EINA_FALSE;

   //Button to start animation
   Evas_Object *btn2 = elm_button_add(win);
   elm_object_text_set(btn2, "Start Translate Animation to right bottom");
   evas_object_smart_callback_add(btn2, "clicked", _btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn2, 300, 50);
   evas_object_move(btn2, 50, 300);
   evas_object_show(btn2);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

void
test_efl_anim_translate_absolute(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Absolute Translate", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Absolute Translate");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, ad);

   //Button to be animated
   Evas_Object *btn = elm_button_add(win);
   elm_object_text_set(btn, "Target");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn, 150, 150);
   evas_object_move(btn, 125, 100);
   evas_object_show(btn);

   //Absolute coordinate (0, 0) for absolute translation
   Evas_Object *abs_center = elm_button_add(win);
   elm_object_text_set(abs_center, "(0, 0)");
   evas_object_size_hint_weight_set(abs_center, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(abs_center, 50, 50);
   evas_object_move(abs_center, 0, 0);
   evas_object_show(abs_center);

   //Translate Animation to right bottom absolutely
   Efl_Animation *translate_rb_anim = efl_add(EFL_ANIMATION_TRANSLATE_CLASS, win);
   efl_animation_translate_absolute_set(translate_rb_anim, 0, 0, 100, 100);
   efl_animation_duration_set(translate_rb_anim, 1.0);
   efl_animation_final_state_keep_set(translate_rb_anim, EINA_TRUE);

   //Translate Animation to left top absolutely
   Efl_Animation *translate_lt_anim = efl_add(EFL_ANIMATION_TRANSLATE_CLASS, win);
   efl_animation_translate_absolute_set(translate_lt_anim, 100, 100, 0, 0);
   efl_animation_duration_set(translate_lt_anim, 1.0);
   efl_animation_final_state_keep_set(translate_lt_anim, EINA_TRUE);

   //Initialize App Data
   ad->translate_rb_anim = translate_rb_anim;
   ad->translate_lt_anim = translate_lt_anim;
   ad->is_btn_translated = EINA_FALSE;
   ad->anim_obj = efl_add(EFL_ANIMATION_PLAYER_CLASS, win,
                          efl_animation_player_target_set(efl_added, btn));
   //Register callback called when animation starts
   efl_event_callback_add(ad->anim_obj, EFL_ANIMATION_PLAYER_EVENT_STARTED, _anim_started_cb, NULL);
   //Register callback called when animation ends
   efl_event_callback_add(ad->anim_obj, EFL_ANIMATION_PLAYER_EVENT_ENDED, _anim_ended_cb, NULL);
   //Register callback called while animation is executed
   efl_event_callback_add(ad->anim_obj, EFL_ANIMATION_PLAYER_EVENT_RUNNING, _anim_running_cb, NULL);

   //Button to start animation
   Evas_Object *btn2 = elm_button_add(win);
   elm_object_text_set(btn2, "Start Translate Animation to right bottom");
   evas_object_smart_callback_add(btn2, "clicked", _btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn2, 300, 50);
   evas_object_move(btn2, 50, 300);
   evas_object_show(btn2);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
