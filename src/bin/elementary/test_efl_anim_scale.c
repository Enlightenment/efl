#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _App_Data
{
   Efl_Canvas_Animation        *scale_double_anim;
   Efl_Canvas_Animation        *scale_half_anim;
   Elm_Button                  *button;

   Eina_Bool             is_btn_scaled;
} App_Data;

static void
_anim_changed_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   Eo *anim = event->info;

   if (anim)
     {
        printf("Animation has been started!\n");
     }
   else
     {
        printf("Animation has been ended!\n");
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
_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;

   ad->is_btn_scaled = !(ad->is_btn_scaled);

   if (ad->is_btn_scaled)
     {
        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->scale_double_anim, 1.0, 0.0);
        efl_text_set(obj, "Start Scale Animation to zoom out");
     }
   else
     {
        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->scale_half_anim, 1.0, 0.0);
        efl_text_set(obj, "Start Scale Animation to zoom in");
     }
}

static void
_win_del_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;
   free(ad);
}

void
test_efl_anim_scale(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Scale", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Scale");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, ad);

   //Button to be animated
   Evas_Object *btn = elm_button_add(win);
   elm_object_text_set(btn, "Target");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn, 150, 150);
   evas_object_move(btn, 125, 100);
   evas_object_show(btn);
   efl_event_callback_array_add(btn, animation_stats_cb(), ad);

   //Scale Animation to zoom in
   Efl_Canvas_Animation *scale_double_anim = efl_add(EFL_CANVAS_SCALE_ANIMATION_CLASS, win);
   efl_animation_scale_set(scale_double_anim, EINA_VECTOR2(1.0, 1.0), EINA_VECTOR2(2.0, 2.0), NULL, EINA_VECTOR2(0.5, 0.5));
   efl_animation_duration_set(scale_double_anim, 1.0);
   efl_animation_final_state_keep_set(scale_double_anim, EINA_TRUE);

   //Scale Animation to zoom out
   Efl_Canvas_Animation *scale_half_anim = efl_add(EFL_CANVAS_SCALE_ANIMATION_CLASS, win);
   efl_animation_scale_set(scale_half_anim, EINA_VECTOR2(2.0, 2.0), EINA_VECTOR2(1.0, 1.0), NULL, EINA_VECTOR2(0.5, 0.5));
   efl_animation_duration_set(scale_half_anim, 1.0);
   efl_animation_final_state_keep_set(scale_half_anim, EINA_TRUE);

   //Initialize App Data
   ad->scale_double_anim = scale_double_anim;
   ad->scale_half_anim = scale_half_anim;
   ad->is_btn_scaled = EINA_FALSE;
   ad->button = btn;

   //Button to start animation
   Evas_Object *btn2 = elm_button_add(win);
   elm_object_text_set(btn2, "Start Scale Animation to zoom in");
   evas_object_smart_callback_add(btn2, "clicked", _btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn2, 300, 50);
   evas_object_move(btn2, 50, 300);
   evas_object_show(btn2);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

void
test_efl_anim_scale_relative(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Relative Scale", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Relative Scale");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, ad);

   //Button to be animated
   Evas_Object *btn = elm_button_add(win);
   elm_object_text_set(btn, "Target");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn, 150, 150);
   evas_object_move(btn, 125, 100);
   evas_object_show(btn);
   efl_event_callback_array_add(btn, animation_stats_cb(), ad);

   //Pivot to be center of the scaling
   Evas_Object *pivot = elm_button_add(win);
   elm_object_text_set(pivot, "Pivot");
   evas_object_size_hint_weight_set(pivot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(pivot, 50, 50);
   evas_object_move(pivot, 350, 150);
   evas_object_show(pivot);

   //Scale Animation to zoom in
   Efl_Canvas_Animation *scale_double_anim = efl_add(EFL_CANVAS_SCALE_ANIMATION_CLASS, win);
   efl_animation_scale_set(scale_double_anim, EINA_VECTOR2(1.0, 1.0), EINA_VECTOR2(2.0, 2.0), pivot, EINA_VECTOR2(0.5, 0.5));
   efl_animation_duration_set(scale_double_anim, 1.0);
   efl_animation_final_state_keep_set(scale_double_anim, EINA_TRUE);

   //Scale Animation to zoom out
   Efl_Canvas_Animation *scale_half_anim = efl_add(EFL_CANVAS_SCALE_ANIMATION_CLASS, win);
   efl_animation_scale_set(scale_half_anim, EINA_VECTOR2(2.0, 2.0), EINA_VECTOR2(1.0, 1.0), pivot, EINA_VECTOR2(0.5, 0.5));
   efl_animation_duration_set(scale_half_anim, 1.0);
   efl_animation_final_state_keep_set(scale_half_anim, EINA_TRUE);

   //Initialize App Data
   ad->scale_double_anim = scale_double_anim;
   ad->scale_half_anim = scale_half_anim;
   ad->button = btn;
   ad->is_btn_scaled = EINA_FALSE;

   //Button to start animation
   Evas_Object *btn2 = elm_button_add(win);
   elm_object_text_set(btn2, "Start Scale Animation to zoom in");
   evas_object_smart_callback_add(btn2, "clicked", _btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn2, 300, 50);
   evas_object_move(btn2, 50, 300);
   evas_object_show(btn2);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

void
test_efl_anim_scale_absolute(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Absolute Scale", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Absolute Scale");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, ad);

   //Button to be animated
   Evas_Object *btn = elm_button_add(win);
   elm_object_text_set(btn, "Target");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn, 150, 150);
   evas_object_move(btn, 125, 100);
   evas_object_show(btn);
   efl_event_callback_array_add(btn, animation_stats_cb(), ad);

   //Absolute coordinate (0, 0) to be center of the scaling
   Evas_Object *abs_center = elm_button_add(win);
   elm_object_text_set(abs_center, "(0, 0)");
   evas_object_size_hint_weight_set(abs_center, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(abs_center, 50, 50);
   evas_object_move(abs_center, 0, 0);
   evas_object_show(abs_center);

   //Scale Animation to zoom in
   Efl_Canvas_Animation *scale_double_anim = efl_add(EFL_CANVAS_SCALE_ANIMATION_CLASS, win);
   efl_animation_scale_absolute_set(scale_double_anim, EINA_VECTOR2(1.0, 1.0), EINA_VECTOR2(2.0, 2.0), EINA_POSITION2D(0, 0));
   efl_animation_duration_set(scale_double_anim, 1.0);
   efl_animation_final_state_keep_set(scale_double_anim, EINA_TRUE);

   //Scale Animation to zoom out
   Efl_Canvas_Animation *scale_half_anim = efl_add(EFL_CANVAS_SCALE_ANIMATION_CLASS, win);
   efl_animation_scale_absolute_set(scale_half_anim, EINA_VECTOR2(2.0, 2.0), EINA_VECTOR2(1.0, 1.0), EINA_POSITION2D(0, 0));
   efl_animation_duration_set(scale_half_anim, 1.0);
   efl_animation_final_state_keep_set(scale_half_anim, EINA_TRUE);

   //Initialize App Data
   ad->scale_double_anim = scale_double_anim;
   ad->scale_half_anim = scale_half_anim;
   ad->button = btn;
   ad->is_btn_scaled = EINA_FALSE;

   //Button to start animation
   Evas_Object *btn2 = elm_button_add(win);
   elm_object_text_set(btn2, "Start Scale Animation to zoom in");
   evas_object_smart_callback_add(btn2, "clicked", _btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn2, 300, 50);
   evas_object_move(btn2, 50, 300);
   evas_object_show(btn2);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
