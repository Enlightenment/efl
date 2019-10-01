#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include <Efl_Ui.h>
#include "elm_priv.h" //FIXME remove this once efl.ui.text doesn't need elm_general.h
#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#ifndef EFL_EO_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif

#ifdef BUILD_VG_LOADER_JSON

static void
btn_clicked_cb(void *data , const Efl_Event *ev )
{
   Evas_Object *anim_view = data;
   const char *text = efl_text_get(ev->object);

   if (!text) return;

   if (!strcmp("Play", text))
     efl_ui_animation_view_play(anim_view);
   else if (!strcmp("Pause", text))
     efl_ui_animation_view_pause(anim_view);
   else if (!strcmp("Resume", text))
     efl_ui_animation_view_resume(anim_view);
   else if (!strcmp("Play Back", text))
     efl_ui_animation_view_play_back(anim_view);
   else if (!strcmp("Stop", text))
     efl_ui_animation_view_stop(anim_view);
}

static void
check_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *anim_view = data;
   efl_ui_animation_view_auto_repeat_set(anim_view, efl_ui_selectable_selected_get(event->object));
}

static void
speed_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *anim_view = data;
   double speed = 1;
   if (efl_ui_selectable_selected_get(event->object)) speed = 0.25;
   efl_ui_animation_view_speed_set(anim_view, speed);
}

static void
limit_frame_cb(void *data, const Efl_Event *event)
{
   Evas_Object *anim_view = data;
   int frame_count = efl_ui_animation_view_frame_count_get(anim_view);
   printf("Total Frame Count : %d\n", frame_count);
   if (efl_ui_selectable_selected_get(event->object))
     {
        efl_ui_animation_view_min_frame_set(anim_view, 5);
        efl_ui_animation_view_max_frame_set(anim_view, 10);
        printf("Frames to show 5-10 only\n");
     }
   else
     {
        efl_ui_animation_view_min_frame_set(anim_view, 0);
        efl_ui_animation_view_max_frame_set(anim_view, frame_count);
        printf("Showing all frames now\n");
     }
}

static void
update_anim_view_state(Evas_Object *anim_view, Evas_Object *label)
{
   Efl_Ui_Animation_View_State state = efl_ui_animation_view_state_get(anim_view);

   switch (state)
     {
      case EFL_UI_ANIMATION_VIEW_STATE_NOT_READY:
         efl_text_set(label, "State = Not Ready");
         break;
      case EFL_UI_ANIMATION_VIEW_STATE_PLAY:
         efl_text_set(label, "State = Playing");
         break;
      case EFL_UI_ANIMATION_VIEW_STATE_PLAY_BACK:
         efl_text_set(label, "State = Playing Back");
         break;
      case EFL_UI_ANIMATION_VIEW_STATE_PAUSE:
         efl_text_set(label, "State = Paused");
         break;
      case EFL_UI_ANIMATION_VIEW_STATE_STOP:
         efl_text_set(label, "State = Stopped");
         break;
     }
}

static void
_play_updated(void *data, Evas_Object *obj, void *ev EINA_UNUSED)
{
   Evas_Object *slider = data;
   efl_ui_range_value_set(slider, efl_ui_animation_view_progress_get(obj));
}

static void
_state_update(void *data, Evas_Object *obj, void *ev EINA_UNUSED)
{
   Evas_Object *label = data;
   update_anim_view_state(obj, label);
}

static void
_play_done(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   printf("done!\n");
}

static void
_play_repeated(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   printf("repeated!\n");
}

static void
_slider_changed_cb(void *data, const Efl_Event *ev)
{
   Evas_Object *anim_view = data;
   efl_ui_animation_view_progress_set(anim_view, efl_ui_range_value_get(ev->object));
}

static void
_slider_reset(void *data, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   Evas_Object *slider = data;
   efl_ui_range_value_set(slider, 0);
}

void
test_efl_ui_animation_view(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box, *box2, *box3, *box4, *label, *anim_view, *check, *slider;
   char buf[255];

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                                  efl_text_set(efl_added, "Efl_Ui_Animation_View demo"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   // Create a box in Canvas
   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND),
                 efl_content_set(win, efl_added));

   //Create a label to show state of animation
   label = efl_add(EFL_UI_TEXT_CLASS, win,
                   efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                   efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                   efl_pack(box, efl_added));

   //Create Animation View to play animation directly from JSON file
   snprintf(buf, sizeof(buf), "%s/images/emoji_wink.json", elm_app_data_dir_get());
   anim_view = efl_add(EFL_UI_ANIMATION_VIEW_CLASS, win,
                       efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND),
                       efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                       efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(600, 600)),
                       efl_file_set(efl_added, buf),
                       efl_pack(box, efl_added));

   // Controller Set : 0
   box2 = efl_add(EFL_UI_BOX_CLASS, box,
                  efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                  efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                  efl_pack(box, efl_added));

   //Loop
   check = efl_add(EFL_UI_CHECK_CLASS, box2,
                   efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                   efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                   efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                   efl_pack(box2, efl_added));
   efl_text_set(check, "Loop");
   efl_event_callback_add(check, EFL_UI_EVENT_SELECTED_CHANGED,  check_changed_cb, anim_view);

   //Speed: 0.5x
   check = efl_add(EFL_UI_CHECK_CLASS, box2,
                   efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                   efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                   efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                   efl_pack(box2, efl_added));
   efl_text_set(check, "Speed: 0.25x");
   efl_event_callback_add(check, EFL_UI_EVENT_SELECTED_CHANGED,  speed_changed_cb, anim_view);

   //Limit Frames
   check = efl_add(EFL_UI_CHECK_CLASS, box2,
                   efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                   efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                   efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                   efl_pack(box2, efl_added));
   efl_text_set(check, "Limit Frames");
   efl_event_callback_add(check, EFL_UI_EVENT_SELECTED_CHANGED,  limit_frame_cb, anim_view);


   //Duration Text
   snprintf(buf, sizeof(buf), "Duration: %1.2fs", efl_ui_animation_view_duration_time_get(anim_view));
   efl_add(EFL_UI_TEXT_CLASS, box2,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, buf),
           efl_pack(box2, efl_added));

   //Slider
   slider = efl_add(EFL_UI_SLIDER_CLASS, box,
                    efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                    efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                    efl_ui_range_limits_set(efl_added, 0, 1),
                    efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED, _slider_changed_cb, anim_view),
                    efl_pack(box, efl_added));

   //Controller Set: 1
   box3 = efl_add(EFL_UI_BOX_CLASS, box,
                  efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                  efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, 1),
                  efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                  efl_pack(box, efl_added));

   //Play Button
   efl_add(EFL_UI_BUTTON_CLASS, box3,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "Play"),
           efl_pack(box3, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, anim_view));


   //Play Back Button
   efl_add(EFL_UI_BUTTON_CLASS, box3,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "Play Back"),
           efl_pack(box3, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, anim_view));

   //Stop Button
   efl_add(EFL_UI_BUTTON_CLASS, box3,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "Stop"),
           efl_pack(box3, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, anim_view));

   //Controller Set: 2
   box4 = efl_add(EFL_UI_BOX_CLASS, box,
                  efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
                  efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, 1),
                  efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                  efl_pack(box, efl_added));

   //Pause Button
   efl_add(EFL_UI_BUTTON_CLASS, box3,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "Pause"),
           efl_pack(box4, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, anim_view));

   //Resume Button
   efl_add(EFL_UI_BUTTON_CLASS, box3,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_text_set(efl_added, "Resume"),
           efl_pack(box4, efl_added),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, btn_clicked_cb, anim_view));

   evas_object_smart_callback_add(anim_view, "play,start", _state_update, label);
   evas_object_smart_callback_add(anim_view, "play,stop", _state_update, label);
   evas_object_smart_callback_add(anim_view, "play,pause", _state_update, label);
   evas_object_smart_callback_add(anim_view, "play,resume", _state_update, label);

   evas_object_smart_callback_add(anim_view, "play,repeat", _play_repeated, label);
   evas_object_smart_callback_add(anim_view, "play,done", _play_done, label);

   evas_object_smart_callback_add(anim_view, "play,update", _play_updated, slider);
   evas_object_smart_callback_add(anim_view, "play,stop", _slider_reset, slider);

   update_anim_view_state(anim_view, label);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(600, 730));
}

#else

void
test_efl_ui_animation_view(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box;
   char buf[255];

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                                  efl_text_set(efl_added, "Efl_Ui_Animation_View demo"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   // Create a box
   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND),
                 efl_content_set(win, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, win,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
           efl_text_set(efl_added, "Evas Vg Json (Lottie) Loader is not supported, Only Static Vector Image is available!"),
           efl_pack(box, efl_added));

   //Create Vector object.
   snprintf(buf, sizeof(buf), "%s/images/tiger.svg", elm_app_data_dir_get());
   efl_add(EFL_CANVAS_VG_OBJECT_CLASS, win,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND),
           efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
           efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(600, 600)),
           efl_file_simple_load(efl_added, buf, NULL),
           efl_gfx_entity_visible_set(efl_added, EINA_TRUE),
           efl_pack(box, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(600, 730));
}

#endif
