#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _App_Data
{
   Efl_Canvas_Animation        *translate_rb_anim;
   Efl_Canvas_Animation        *translate_lt_anim;
   Elm_Button                  *button;

   Eina_Bool             is_btn_translated;
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

   ad->is_btn_translated = !(ad->is_btn_translated);

   if (ad->is_btn_translated)
     {
        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->translate_rb_anim, 1.0, 0.0);
        efl_text_set(obj, "Start Translate Animation to left top");
     }
   else
     {
        //Create Animation Object from Animation
        efl_canvas_object_animation_start(ad->button, ad->translate_lt_anim, 1.0, 0.0);
        efl_text_set(obj, "Start Translate Animation to right bottom");
     }
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
   efl_event_callback_array_add(btn, animation_stats_cb(), ad);

   //Translate Animation to right bottom relatively
   Efl_Canvas_Animation *translate_rb_anim = efl_add(EFL_CANVAS_TRANSLATE_ANIMATION_CLASS, win);
   efl_animation_translate_set(translate_rb_anim, EINA_POSITION2D(0, 0), EINA_POSITION2D(100, 100));
   efl_animation_duration_set(translate_rb_anim, 1.0);
   efl_animation_final_state_keep_set(translate_rb_anim, EINA_TRUE);

   //Translate Animation to left top relatively
   Efl_Canvas_Animation *translate_lt_anim = efl_add(EFL_CANVAS_TRANSLATE_ANIMATION_CLASS, win);
   efl_animation_translate_set(translate_lt_anim, EINA_POSITION2D(100, 100), EINA_POSITION2D(0, 0));
   efl_animation_duration_set(translate_lt_anim, 1.0);
   efl_animation_final_state_keep_set(translate_lt_anim, EINA_TRUE);

   //Initialize App Data
   ad->translate_rb_anim = translate_rb_anim;
   ad->translate_lt_anim = translate_lt_anim;
   ad->button = btn;
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
   efl_event_callback_array_add(btn, animation_stats_cb(), ad);

   //Absolute coordinate (0, 0) for absolute translation
   Evas_Object *abs_center = elm_button_add(win);
   elm_object_text_set(abs_center, "(0, 0)");
   evas_object_size_hint_weight_set(abs_center, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_resize(abs_center, 50, 50);
   evas_object_move(abs_center, 0, 0);
   evas_object_show(abs_center);

   //Translate Animation to right bottom absolutely
   Efl_Canvas_Animation *translate_rb_anim = efl_add(EFL_CANVAS_TRANSLATE_ANIMATION_CLASS, win);
   efl_animation_translate_absolute_set(translate_rb_anim, EINA_POSITION2D(0, 0), EINA_POSITION2D(100, 100));
   efl_animation_duration_set(translate_rb_anim, 1.0);
   efl_animation_final_state_keep_set(translate_rb_anim, EINA_TRUE);

   //Translate Animation to left top absolutely
   Efl_Canvas_Animation *translate_lt_anim = efl_add(EFL_CANVAS_TRANSLATE_ANIMATION_CLASS, win);
   efl_animation_translate_absolute_set(translate_lt_anim, EINA_POSITION2D(100, 100), EINA_POSITION2D(0, 0));
   efl_animation_duration_set(translate_lt_anim, 1.0);
   efl_animation_final_state_keep_set(translate_lt_anim, EINA_TRUE);

   //Initialize App Data
   ad->translate_rb_anim = translate_rb_anim;
   ad->translate_lt_anim = translate_lt_anim;
   ad->is_btn_translated = EINA_FALSE;
   ad->button = btn;

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
