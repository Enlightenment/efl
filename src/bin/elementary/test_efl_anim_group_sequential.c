#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _App_Data
{
   Efl_Canvas_Animation        *sequential_hide_anim;
   Elm_Button                  *button;

   Eina_Bool             is_btn_visible;
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

   ad->is_btn_visible = !(ad->is_btn_visible);

   if (ad->is_btn_visible)
     {
        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->sequential_hide_anim, -1.0, 0.0);
        efl_text_set(obj, "Start Sequential Group Animation to hide button");
     }
   else
     {
        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->sequential_hide_anim, 1.0, 0.0);
        efl_text_set(obj, "Start Sequential Group Animation to show button");
     }
}

static void
_win_del_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = data;
   free(ad);
}

void
test_efl_anim_group_sequential(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad = calloc(1, sizeof(App_Data));
   if (!ad) return;

   Evas_Object *win = elm_win_add(NULL, "Efl Animation Group Sequential", ELM_WIN_BASIC);
   elm_win_title_set(win, "Efl Animation Group Sequential");
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


   /* Animations to hide button */
   //Rotate from 0 to 45 degrees Animation
   Efl_Canvas_Animation *cw_45_degrees_anim = efl_add(EFL_CANVAS_ROTATE_ANIMATION_CLASS, win);
   efl_animation_rotate_set(cw_45_degrees_anim, 0.0, 45.0, NULL, EINA_VECTOR2(0.5, 0.5));

   //Scale Animation to zoom in
   Efl_Canvas_Animation *scale_double_anim = efl_add(EFL_CANVAS_SCALE_ANIMATION_CLASS, win);
   efl_animation_scale_set(scale_double_anim, EINA_VECTOR2(1.0, 1.0), EINA_VECTOR2(2.0, 2.0), NULL, EINA_VECTOR2(0.5, 0.5));

   //Hide Animation
   Efl_Canvas_Animation *hide_anim = efl_add(EFL_CANVAS_ALPHA_ANIMATION_CLASS, win);
   efl_animation_alpha_set(hide_anim, 1.0, 0.0);

   //Hide Sequential Group Animation
   Efl_Canvas_Animation *sequential_hide_anim = efl_add(EFL_CANVAS_SEQUENTIAL_GROUP_ANIMATION_CLASS, win);
   efl_animation_duration_set(sequential_hide_anim, 1.0);
   efl_animation_final_state_keep_set(sequential_hide_anim, EINA_TRUE);

   //Add animations to group animation
   efl_animation_group_animation_add(sequential_hide_anim, cw_45_degrees_anim);
   efl_animation_group_animation_add(sequential_hide_anim, scale_double_anim);
   efl_animation_group_animation_add(sequential_hide_anim, hide_anim);

   //Initialize App Data
   ad->sequential_hide_anim = sequential_hide_anim;
   ad->button = btn;

   ad->is_btn_visible = EINA_TRUE;

   //Button to start animation
   Evas_Object *btn2 = elm_button_add(win);
   elm_object_text_set(btn2, "Start Sequential Group Animation to hide button");
   evas_object_smart_callback_add(btn2, "clicked", _btn_clicked_cb, ad);
   evas_object_size_hint_weight_set(btn2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(btn2, 300, 50);
   evas_object_move(btn2, 50, 300);
   evas_object_show(btn2);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
