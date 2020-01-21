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
   Efl_Ui_Button *btns[INTERP_NUM];

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
        efl_sinusoidal_interpolator_slope_set(interp, 1.0);
     }
   else if (index == 2)
     {
        interp = efl_add(EFL_DECELERATE_INTERPOLATOR_CLASS, win);
        efl_decelerate_interpolator_slope_set(interp, 1.0);
     }
   else if (index == 3)
     {
        interp = efl_add(EFL_ACCELERATE_INTERPOLATOR_CLASS, win);
        efl_accelerate_interpolator_slope_set(interp, 1.0);
     }
   else if (index == 4)
     {
        interp = efl_add(EFL_DIVISOR_INTERPOLATOR_CLASS, win);
        efl_divisor_interpolator_divisor_set(interp, 1.0);
        efl_divisor_interpolator_power_set(interp, 1);
     }
   else if (index == 5)
     {
        interp = efl_add(EFL_BOUNCE_INTERPOLATOR_CLASS, win);
        efl_bounce_interpolator_rigidness_set(interp, 1.0);
        efl_bounce_interpolator_bounces_set(interp, 1);
     }
   else if (index == 6)
     {
        interp = efl_add(EFL_SPRING_INTERPOLATOR_CLASS, win);
        efl_spring_interpolator_decay_set(interp, 1.0);
        efl_spring_interpolator_oscillations_set(interp, 1);
     }

   return interp;
}

static void
_anim_changed_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   Eo *anim = event->info;
   App_Data *ad = data;

   if (anim)
     {
        printf("Animation has been started!\n");
        ad->running_anim_cnt++;
     }
   else
     {
        int i;

        printf("Animation has been ended!\n");
        ad->running_anim_cnt--;

        for (i = 0; i < INTERP_NUM; i++)
          {
             if (ad->btns[i] == event->object)
               {
                  elm_object_disabled_set(ad->btn[i], EINA_FALSE);
                  break;
               }
          }

        if (ad->running_anim_cnt == 0)
          elm_object_disabled_set(ad->start_all_btn, EINA_FALSE);
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
_anim_start(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;

   int index = (uintptr_t)evas_object_data_get(obj, "index");

   //Let Animation Object start animation
   efl_canvas_object_animation_start(ad->btns[index], ad->anim[index], 1.0, 0.0);

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
        efl_canvas_object_animation_start(ad->btns[i], ad->anim[i], 1.0, 0.0);
        elm_object_disabled_set(ad->btn[i], EINA_TRUE);
     }

   elm_object_disabled_set(obj, EINA_TRUE);
}

static void
_win_del_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;
   int i;

   for (i = 0; i < INTERP_NUM; i++)
     {
        //Unregister callback called when window deletes
        efl_event_callback_array_del(ad->btns[i], animation_stats_cb(), ad);
     }

   free(ad);
}

void
test_efl_anim_interpolator(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int i;

   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   const char *modes[] = {"LINEAR", "SINUSOIDAL", "DECELERATE", "ACCELERATE",
                          "DIVISOR_INTERP", "BOUNCE", "SPRING"};

   Evas_Object *win =
     elm_win_add(NULL, "Efl Animation Interpolator", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Interpolator");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, ad);

   //Button to start all animations
   Evas_Object *start_all_btn = elm_button_add(win);
   elm_object_text_set(start_all_btn, "Start All");
   evas_object_resize(start_all_btn, WIN_W, BTN_H);
   evas_object_move(start_all_btn, 0, (WIN_H - BTN_H));
   evas_object_show(start_all_btn);
   evas_object_smart_callback_add(start_all_btn, "clicked",
                                  _anim_start_all, ad);
   ad->start_all_btn = start_all_btn;

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

        Efl_Canvas_Animation *anim =
          efl_add(EFL_CANVAS_TRANSLATE_ANIMATION_CLASS, win);
        efl_animation_translate_set(anim, EINA_POSITION2D(0, 0), EINA_POSITION2D((WIN_W - BTN_W), 0));
        efl_animation_duration_set(anim, 2.0);
        efl_animation_final_state_keep_set(anim, EINA_FALSE);

        Efl_Interpolator *interp = _interpolator_create(i, win);
        efl_animation_interpolator_set(anim, interp);
        ad->anim[i] = anim;

        //Create Animation Object from Animation
        ad->btns[i] = btn;
        efl_event_callback_array_add(btn, animation_stats_cb(), ad);
     }

   ad->running_anim_cnt = 0;

   evas_object_resize(win, WIN_W, WIN_H);
   evas_object_show(win);
}
