#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _App_Data
{
   Efl_Canvas_Animation        *cw_45_degrees_anim;
   Efl_Canvas_Animation        *ccw_45_degrees_anim;
   Elm_Button                  *button;

   Eina_Bool             is_btn_rotated;
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

   ad->is_btn_rotated = !(ad->is_btn_rotated);

   if (ad->is_btn_rotated)
     {
        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->cw_45_degrees_anim, 1.0, 0.0);
        efl_text_set(obj, "Start Rotate Animation from 45 to 0 degrees");
     }
   else
     {
        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->ccw_45_degrees_anim, 1.0, 0.0);
        efl_text_set(obj, "Start Rotate Animation from 0 to 45 degrees");
     }
}

static void
_win_del_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;
   free(ad);
}

void
test_efl_anim_rotate(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Rotate", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Rotate");
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

   //Rotate from 0 to 45 degrees Animation
   Efl_Canvas_Animation *cw_45_degrees_anim = efl_add(EFL_CANVAS_ROTATE_ANIMATION_CLASS, win);
   efl_animation_rotate_set(cw_45_degrees_anim, 0.0, 45.0, NULL, EINA_VECTOR2(0.5, 0.5));
   efl_animation_duration_set(cw_45_degrees_anim, 1.0);
   efl_animation_final_state_keep_set(cw_45_degrees_anim, EINA_TRUE);

   //Rotate from 45 to 0 degrees Animation
   Efl_Canvas_Animation *ccw_45_degrees_anim = efl_add(EFL_CANVAS_ROTATE_ANIMATION_CLASS, win);
   efl_animation_rotate_set(ccw_45_degrees_anim, 45.0, 0.0, NULL, EINA_VECTOR2(0.5, 0.5));
   efl_animation_duration_set(ccw_45_degrees_anim, 1.0);
   efl_animation_final_state_keep_set(ccw_45_degrees_anim, EINA_TRUE);

   //Initialize App Data
   ad->cw_45_degrees_anim = cw_45_degrees_anim;
   ad->ccw_45_degrees_anim = ccw_45_degrees_anim;
   ad->button = btn;
   ad->is_btn_rotated = EINA_FALSE;

   //Button to start animation
   Evas_Object *btn2 = elm_button_add(win);
   elm_object_text_set(btn2, "Start Rotate Animation from 0 to 45 degrees");
   evas_object_smart_callback_add(btn2, "clicked", _btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn2, 300, 50);
   evas_object_move(btn2, 50, 300);
   evas_object_show(btn2);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

void
test_efl_anim_rotate_relative(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Relative Rotate", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Relative Rotate");
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

   //Pivot to be center of the rotation
   Evas_Object *pivot = elm_button_add(win);
   elm_object_text_set(pivot, "Pivot");
   evas_object_size_hint_weight_set(pivot, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(pivot, 50, 50);
   evas_object_move(pivot, 350, 150);
   evas_object_show(pivot);

   //Rotate from 0 to 45 degrees Animation
   Efl_Canvas_Animation *cw_45_degrees_anim = efl_add(EFL_CANVAS_ROTATE_ANIMATION_CLASS, win);
   efl_animation_rotate_set(cw_45_degrees_anim, 0.0, 45.0, pivot, EINA_VECTOR2(0.5, 0.5));
   efl_animation_duration_set(cw_45_degrees_anim, 1.0);
   efl_animation_final_state_keep_set(cw_45_degrees_anim, EINA_TRUE);

   //Rotate from 45 to 0 degrees Animation
   Efl_Canvas_Animation *ccw_45_degrees_anim = efl_add(EFL_CANVAS_ROTATE_ANIMATION_CLASS, win);
   efl_animation_rotate_set(ccw_45_degrees_anim, 45.0, 0.0, pivot, EINA_VECTOR2(0.5, 0.5));
   efl_animation_duration_set(ccw_45_degrees_anim, 1.0);
   efl_animation_final_state_keep_set(ccw_45_degrees_anim, EINA_TRUE);

   //Initialize App Data
   ad->cw_45_degrees_anim = cw_45_degrees_anim;
   ad->ccw_45_degrees_anim = ccw_45_degrees_anim;
   ad->is_btn_rotated = EINA_FALSE;
   ad->button = btn;

   //Button to start animation
   Evas_Object *btn2 = elm_button_add(win);
   elm_object_text_set(btn2, "Start Rotate Animation from 0 to 45 degrees");
   evas_object_smart_callback_add(btn2, "clicked", _btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn2, 300, 50);
   evas_object_move(btn2, 50, 300);
   evas_object_show(btn2);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}

void
test_efl_anim_rotate_absolute(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Absolute Rotate", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Absolute Rotate");
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

   //Absolute coordinate (0, 0) to be center of the rotation
   Evas_Object *abs_center = elm_button_add(win);
   elm_object_text_set(abs_center, "(0, 0)");
   evas_object_size_hint_weight_set(abs_center, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(abs_center, 50, 50);
   evas_object_move(abs_center, 0, 0);
   evas_object_show(abs_center);

   //Rotate from 0 to 45 degrees Animation
   Efl_Canvas_Animation *cw_45_degrees_anim = efl_add(EFL_CANVAS_ROTATE_ANIMATION_CLASS, win);
   efl_animation_rotate_absolute_set(cw_45_degrees_anim, 0.0, 45.0, EINA_POSITION2D(0, 0));
   efl_animation_duration_set(cw_45_degrees_anim, 1.0);
   efl_animation_final_state_keep_set(cw_45_degrees_anim, EINA_TRUE);

   //Rotate from 45 to 0 degrees Animation
   Efl_Canvas_Animation *ccw_45_degrees_anim = efl_add(EFL_CANVAS_ROTATE_ANIMATION_CLASS, win);
   efl_animation_rotate_absolute_set(ccw_45_degrees_anim, 45.0, 0.0, EINA_POSITION2D(0, 0));
   efl_animation_duration_set(ccw_45_degrees_anim, 1.0);
   efl_animation_final_state_keep_set(ccw_45_degrees_anim, EINA_TRUE);

   //Initialize App Data
   ad->cw_45_degrees_anim = cw_45_degrees_anim;
   ad->ccw_45_degrees_anim = ccw_45_degrees_anim;
   ad->is_btn_rotated = EINA_FALSE;
   ad->button = btn;

   //Button to start animation
   Evas_Object *btn2 = elm_button_add(win);
   elm_object_text_set(btn2, "Start Rotate Animation from 0 to 45 degrees");
   evas_object_smart_callback_add(btn2, "clicked", _btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn2, 300, 50);
   evas_object_move(btn2, 50, 300);
   evas_object_show(btn2);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
