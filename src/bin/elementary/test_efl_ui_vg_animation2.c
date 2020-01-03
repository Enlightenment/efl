#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include <Efl_Ui.h>

#ifndef EFL_BETA_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#ifndef EFL_EO_API_SUPPORT
#define EFL_EO_API_SUPPORT
#endif

#ifdef BUILD_VG_LOADER_JSON

Evas_Object *color_slider_1, *color_slider_2, *color_slider_3, *stroke_slider;

static void
btn_clicked_cb(void *data , const Efl_Event *ev )
{
   Evas_Object *anim_view = data;
   const char *text = efl_text_get(ev->object);

   if (!text) return;

   if (!strcmp("Play", text))
     {
        double speed = efl_player_playback_speed_get(anim_view);
        efl_player_playback_speed_set(anim_view, speed < 0 ? speed * -1 : speed);
        efl_player_playing_set(anim_view, EINA_TRUE);
     }
   else if (!strcmp("Pause", text))
     efl_player_paused_set(anim_view, EINA_TRUE);
   else if (!strcmp("Resume", text))
     efl_player_paused_set(anim_view, EINA_FALSE);
   else if (!strcmp("Play Backwards", text))
     {
        double speed = efl_player_playback_speed_get(anim_view);
        efl_player_playback_speed_set(anim_view, speed > 0 ? speed * -1 : speed);
        efl_player_playing_set(anim_view, EINA_TRUE);
     }
   else if (!strcmp("Stop", text))
     efl_player_playing_set(anim_view, EINA_FALSE);
}

static void
check_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *anim_view = data;
   efl_ui_vg_animation_autorepeat_set(anim_view, efl_ui_selectable_selected_get(event->object));
}

static void
speed_changed_cb(void *data, const Efl_Event *event)
{
   Evas_Object *anim_view = data;
   double speed = 1;
   if (efl_ui_selectable_selected_get(event->object)) speed = 0.25;
   efl_player_playback_speed_set(anim_view, speed);
}

static void
limit_frame_cb(void *data, const Efl_Event *event)
{
   Evas_Object *anim_view = data;
   int frame_count = efl_ui_vg_animation_frame_count_get(anim_view);
   printf("Total Frame Count : %d\n", frame_count);
   if (efl_ui_selectable_selected_get(event->object))
     {
        efl_ui_vg_animation_min_frame_set(anim_view, 5);
        efl_ui_vg_animation_max_frame_set(anim_view, 10);
        printf("Frames to show 5-10 only\n");
     }
   else
     {
        efl_ui_vg_animation_min_frame_set(anim_view, 0);
        efl_ui_vg_animation_max_frame_set(anim_view, frame_count);
        printf("Showing all frames now\n");
     }
}

static void
update_anim_view_state(Evas_Object *anim_view, Evas_Object *label)
{
   Efl_Ui_Vg_Animation_State state = efl_ui_vg_animation_state_get(anim_view);

   switch (state)
     {
      case EFL_UI_VG_ANIMATION_STATE_NOT_READY:
         efl_text_set(label, "State = Not Ready");
         break;
      case EFL_UI_VG_ANIMATION_STATE_PLAYING:
         efl_text_set(label, "State = Playing");
         break;
      case EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS:
         efl_text_set(label, "State = Playing Backwards");
         break;
      case EFL_UI_VG_ANIMATION_STATE_PAUSED:
         efl_text_set(label, "State = Paused");
         break;
      case EFL_UI_VG_ANIMATION_STATE_STOPPED:
         efl_text_set(label, "State = Stopped");
         break;
     }
}

static void
_play_updated(void *data, Evas_Object *obj, void *ev EINA_UNUSED)
{
   Evas_Object *slider = data;
   efl_ui_range_value_set(slider, efl_player_playback_progress_get(obj));
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
   efl_player_playback_progress_set(anim_view, efl_ui_range_value_get(ev->object));
}

static void
_color_slider_changed_cb(void *data, const Efl_Event *ev)
{
   Evas_Object *anim_view = data;

   int r = efl_ui_range_value_get(color_slider_1);
   int g = efl_ui_range_value_get(color_slider_2);
   int b = efl_ui_range_value_get(color_slider_3);
   int stroke = efl_ui_range_value_get(stroke_slider);

   Eo *vp = efl_add(EFL_GFX_VG_VALUE_PROVIDER_CLASS, anim_view);
   efl_gfx_vg_value_provider_keypath_set(vp, "Shape Layer 5.Shape 1.*");
   efl_gfx_vg_value_provider_fill_color_set(vp, r, g, b, 255);
   efl_ui_vg_animation_value_provider_override(anim_view, vp);

   Eo *vp2 = efl_add(EFL_GFX_VG_VALUE_PROVIDER_CLASS, anim_view);
   efl_gfx_vg_value_provider_keypath_set(vp2, "**");
   efl_gfx_vg_value_provider_stroke_width_set(vp2, stroke);
   efl_ui_vg_animation_value_provider_override(anim_view, vp2);
}

static void
_slider_reset(void *data, Evas_Object *obj EINA_UNUSED, void *ev EINA_UNUSED)
{
   Evas_Object *slider = data;
   efl_ui_range_value_set(slider, 0);
}

void
left_box_content(Evas_Object* box)
{
   Eo *box2, *box3, *box4, *label, *anim_view, *check, *slider;
   char buf[255];

   //Create a label to show state of animation
   label = efl_add(EFL_UI_TEXTBOX_CLASS, box,
                   efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                   efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
                   efl_pack(box, efl_added));

   //Create Animation View to play animation directly from JSON file
   snprintf(buf, sizeof(buf), "%s/images/snoopd.json", elm_app_data_dir_get());
   anim_view = efl_add(EFL_UI_VG_ANIMATION_CLASS, box,
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
   snprintf(buf, sizeof(buf), "Duration(Length): %1.2fs", efl_playable_length_get(anim_view));
   efl_add(EFL_UI_TEXTBOX_CLASS, box2,
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
           efl_text_set(efl_added, "Play Backwards"),
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
}

void
right_box_content(Evas_Object* box)
{
   Eo *label, *anim_view, *slider;
   char buf[255];

   //Create a label to show state of animation

   //Create Animation View to play animation directly from JSON file
   snprintf(buf, sizeof(buf), "%s/images/snoopd.json", elm_app_data_dir_get());
   anim_view = efl_add(EFL_UI_VG_ANIMATION_CLASS, box,
                       efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND,EFL_GFX_HINT_EXPAND),
                       efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                       efl_gfx_entity_size_set(efl_added, EINA_SIZE2D(300, 300)),
                       efl_file_set(efl_added, buf),
                       efl_pack(box, efl_added));
   efl_ui_vg_animation_autorepeat_set(anim_view, EINA_TRUE);
   efl_player_playing_set(anim_view, EINA_TRUE);

   //Slider
   efl_add(EFL_UI_TEXTBOX_CLASS, box,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
           efl_gfx_hint_align_set(efl_added, 0.0, EVAS_HINT_FILL),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
           efl_text_set(efl_added, "Red Color"),
           efl_pack(box, efl_added));

   color_slider_1 = efl_add(EFL_UI_SLIDER_CLASS, box,
                    efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                    efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                    efl_ui_range_limits_set(efl_added, 0, 255),
                    efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED, _color_slider_changed_cb, anim_view),
                    efl_pack(box, efl_added));

   efl_add(EFL_UI_TEXTBOX_CLASS, box,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
           efl_gfx_hint_align_set(efl_added, 0.0, EVAS_HINT_FILL),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
           efl_text_set(efl_added, "Green Color"),
           efl_pack(box, efl_added));

   //Slider
   color_slider_2 = efl_add(EFL_UI_SLIDER_CLASS, box,
                    efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                    efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                    efl_ui_range_limits_set(efl_added, 0, 255),
                    efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED, _color_slider_changed_cb, anim_view),
                    efl_pack(box, efl_added));
   efl_add(EFL_UI_TEXTBOX_CLASS, box,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
           efl_gfx_hint_align_set(efl_added, 0.0, EVAS_HINT_FILL),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
           efl_text_set(efl_added, "Blue Color"),
           efl_pack(box, efl_added));

   //Slider
   color_slider_3 = efl_add(EFL_UI_SLIDER_CLASS, box,
                    efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                    efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                    efl_ui_range_limits_set(efl_added, 0, 255),
                    efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED, _color_slider_changed_cb, anim_view),
                    efl_pack(box, efl_added));

   efl_add(EFL_UI_TEXTBOX_CLASS, box,
           efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
           efl_gfx_hint_align_set(efl_added, 0.0, EVAS_HINT_FILL),
           efl_gfx_hint_fill_set(efl_added, EINA_FALSE, EINA_FALSE),
           efl_text_set(efl_added, "Stroke width"),
           efl_pack(box, efl_added));

   //Slider
   stroke_slider = efl_add(EFL_UI_SLIDER_CLASS, box,
                    efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, 0.1),
                    efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL),
                    efl_ui_range_limits_set(efl_added, 1, 20),
                    efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED, _color_slider_changed_cb, anim_view),
                    efl_pack(box, efl_added));
}

void
test_efl_ui_vg_animation(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   setenv("ELM_ACCEL", "gl", 1);

   Eo *win, *main_box, *left_box, *right_box;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_text_set(efl_added, "Efl_Ui_Vg_Animation demo"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   // Create a box in Canvas
   main_box = efl_add(EFL_UI_BOX_CLASS, win,
                      efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND),
                      efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                      efl_content_set(win, efl_added));

   // Create a box in Canvas
   left_box = efl_add(EFL_UI_BOX_CLASS, main_box,
                      efl_gfx_hint_align_set(efl_added, 0.0, EVAS_HINT_FILL),
                      efl_gfx_hint_weight_set(efl_added, 0.5, EFL_GFX_HINT_EXPAND),
                      efl_pack(main_box, efl_added));

   left_box_content(left_box);

   // Create a box in Canvas
   right_box = efl_add(EFL_UI_BOX_CLASS, main_box,
                       efl_gfx_hint_align_set(efl_added, 1.0, EVAS_HINT_FILL),
                       efl_gfx_hint_weight_set(efl_added, 0.5, EFL_GFX_HINT_EXPAND),
                       efl_pack(main_box, efl_added));

   right_box_content(right_box);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(1200, 730));
}

#else

void
test_efl_ui_vg_animation(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *box;
   char buf[255];

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl_Ui_Vg_Animation demo"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   // Create a box
   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_gfx_hint_weight_set(efl_added, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND),
                 efl_content_set(win, efl_added));

   efl_add(EFL_UI_TEXTBOX_CLASS, win,
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
