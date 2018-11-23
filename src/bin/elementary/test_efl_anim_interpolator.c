#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define INTERP_NUM 7
#define BTN_NUM (INTERP_NUM + 1)
#define BTN_W 50
#define BTN_H 50
#define WIN_H (BTN_NUM * BTN_H)
#define WIN_W WIN_H

typedef struct _App_Data
{
   Efl_Canvas_Animation        *anim[INTERP_NUM];
   Efl_Canvas_Animation_Player *anim_obj[INTERP_NUM];

   Evas_Object          *btn[INTERP_NUM];
   Evas_Object          *start_all_btn;

   Eina_Bool             running_anim_cnt;
} App_Data;

static Efl_Interpolator *
_interpolator_create(int index, Evas_Object *win)
{
   Efl_Interpolator *interp = NULL;

   if (index == 0)
     {
        interp = efl_add(EFL_LINEAR_INTERPOLATOR_CLASS, win);
     }
   else if (index == 1)
     {
        interp = efl_add(EFL_SINUSOIDAL_INTERPOLATOR_CLASS, win);
        efl_sinusoidal_interpolator_factor_set(interp, 1.0);
     }
   else if (index == 2)
     {
        interp = efl_add(EFL_DECELERATE_INTERPOLATOR_CLASS, win);
        efl_decelerate_interpolator_factor_set(interp, 1.0);
     }
   else if (index == 3)
     {
        interp = efl_add(EFL_ACCELERATE_INTERPOLATOR_CLASS, win);
        efl_accelerate_interpolator_factor_set(interp, 1.0);
     }
   else if (index == 4)
     {
        interp = efl_add(EFL_DIVISOR_INTERPOLATOR_CLASS, win);
        efl_divisor_interpolator_factors_set(interp, 1.0, 1.0);
     }
   else if (index == 5)
     {
        interp = efl_add(EFL_BOUNCE_INTERPOLATOR_CLASS, win);
        efl_bounce_interpolator_factors_set(interp, 1.0, 1.0);
     }
   else if (index == 6)
     {
        interp = efl_add(EFL_SPRING_INTERPOLATOR_CLASS, win);
        efl_spring_interpolator_factors_set(interp, 1.0, 1.0);
     }

   return interp;
}

static void
_anim_started_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   App_Data *ad = data;

   printf("Animation has been started!\n");

   ad->running_anim_cnt++;
}

static void
_anim_ended_cb(void *data, const Efl_Event *event)
{
   App_Data *ad = data;

   printf("Animation has been ended!\n");

   ad->running_anim_cnt--;

   int i;
   for (i = 0; i < INTERP_NUM; i++)
     {
        if (ad->anim_obj[i] == event->object)
          {
             elm_object_disabled_set(ad->btn[i], EINA_FALSE);
             break;
          }
     }

   if (ad->running_anim_cnt == 0)
     elm_object_disabled_set(ad->start_all_btn, EINA_FALSE);
}

static void
_anim_running_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Canvas_Animation_Player_Event_Running *event_running = event->info;
   double progress = event_running->progress;
   printf("Animation is running! Current progress(%lf)\n", progress);
}

static void
_anim_start(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;

   int index = (uintptr_t)evas_object_data_get(obj, "index");

   //Let Animation Object start animation
   efl_player_start(ad->anim_obj[index]);

   elm_object_disabled_set(obj, EINA_TRUE);
   elm_object_disabled_set(ad->start_all_btn, EINA_TRUE);
}

static void
_anim_start_all(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;

   int i;
   for (i = 0; i < INTERP_NUM; i++)
     {
        //Let Animation Object start animation
        efl_player_start(ad->anim_obj[i]);
        elm_object_disabled_set(ad->btn[i], EINA_TRUE);
     }

   elm_object_disabled_set(obj, EINA_TRUE);
}

static void
_win_del_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;
   free(ad);
}

void
test_efl_anim_interpolator(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   const char *modes[] = {"LINEAR", "SINUSOIDAL", "DECELERATE", "ACCELERATE",
                          "DIVISOR_INTERP", "BOUNCE", "SPRING"};

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Interpolator", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Interpolator");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, ad);

   //Button to start all animations
   Evas_Object *start_all_btn = elm_button_add(win);
   elm_object_text_set(start_all_btn, "Start All");
   evas_object_resize(start_all_btn, WIN_W, BTN_H);
   evas_object_move(start_all_btn, 0, (WIN_H - BTN_H));
   evas_object_show(start_all_btn);
   evas_object_smart_callback_add(start_all_btn, "clicked", _anim_start_all, ad);
   ad->start_all_btn = start_all_btn;

   int i;
   for (i = 0; i < INTERP_NUM; i++)
     {
        Evas_Object *label = elm_label_add(win);
        elm_object_text_set(label, modes[i]);
        evas_object_resize(label, WIN_W, BTN_H);
        evas_object_move(label, 0, (i * BTN_H));
        evas_object_show(label);

        //Button to be animated
        Evas_Object *btn = elm_button_add(win);
        evas_object_data_set(btn, "index", (void *)(uintptr_t)i);
        elm_object_text_set(btn, "Start");
        evas_object_resize(btn, BTN_W, BTN_H);
        evas_object_move(btn, 0, (i * BTN_H));
        evas_object_show(btn);
        evas_object_smart_callback_add(btn, "clicked", _anim_start, ad);
        ad->btn[i] = btn;

        Efl_Canvas_Animation *anim = efl_add(EFL_CANVAS_ANIMATION_TRANSLATE_CLASS, win);
        efl_animation_translate_set(anim, 0, 0, (WIN_W - BTN_W), 0);
        efl_animation_duration_set(anim, 2.0);
        efl_animation_final_state_keep_set(anim, EINA_FALSE);

        Efl_Interpolator *interp = _interpolator_create(i, win);
        efl_animation_interpolator_set(anim, interp);
        ad->anim[i] = anim;

        //Create Animation Object from Animation
        Efl_Canvas_Animation_Player *anim_obj = efl_add(EFL_CANVAS_ANIMATION_PLAYER_CLASS, win,
                                                 efl_animation_player_animation_set(efl_added, anim),
                                                 efl_animation_player_target_set(efl_added, btn));
        ad->anim_obj[i] = anim_obj;

        //Register callback called when animation starts
        efl_event_callback_add(anim_obj, EFL_ANIMATION_PLAYER_EVENT_STARTED, _anim_started_cb, ad);

        //Register callback called when animation ends
        efl_event_callback_add(anim_obj, EFL_ANIMATION_PLAYER_EVENT_ENDED, _anim_ended_cb, ad);

        //Register callback called while animation is executed
        efl_event_callback_add(anim_obj, EFL_ANIMATION_PLAYER_EVENT_RUNNING, _anim_running_cb, NULL);
     }

   ad->running_anim_cnt = 0;

   evas_object_resize(win, WIN_W, WIN_H);
   evas_object_show(win);
}
