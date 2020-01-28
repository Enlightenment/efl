#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"

#ifdef BUILD_VG_LOADER_JSON

EFL_START_TEST(vg_anim_playing_control)
{
   Evas_Object *win, *vg_anim;
   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_text_set(efl_added, "Efl_Ui_Vg_Animation demo"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   vg_anim = efl_add(EFL_UI_VG_ANIMATION_CLASS, win,
                     efl_content_set(win, efl_added));

   ck_assert_int_eq(efl_ui_vg_animation_state_get(vg_anim), EFL_UI_VG_ANIMATION_STATE_NOT_READY);

   // File load
   efl_file_simple_load(vg_anim, TESTS_SRC_DIR"/emoji_wink.json", NULL);
   ck_assert(efl_file_loaded_get(vg_anim));
   ck_assert_int_eq(efl_ui_vg_animation_state_get(vg_anim), EFL_UI_VG_ANIMATION_STATE_STOPPED);

   // Play start
   efl_player_playing_set(vg_anim, EINA_TRUE);
   ck_assert_int_eq(efl_ui_vg_animation_state_get(vg_anim), EFL_UI_VG_ANIMATION_STATE_PLAYING);

   // Play pause
   efl_player_paused_set(vg_anim, EINA_TRUE);
   ck_assert_int_eq(efl_ui_vg_animation_state_get(vg_anim), EFL_UI_VG_ANIMATION_STATE_PAUSED);

   // Play resume
   efl_player_paused_set(vg_anim, EINA_FALSE);
   ck_assert_int_eq(efl_ui_vg_animation_state_get(vg_anim), EFL_UI_VG_ANIMATION_STATE_PLAYING);

   // Playback speed
   efl_player_playback_speed_set(vg_anim, 2.0);
   ck_assert(EINA_DBL_EQ(efl_player_playback_speed_get(vg_anim), 2.0));

   efl_player_playback_speed_set(vg_anim, -2.0);
   ck_assert(EINA_DBL_EQ(efl_player_playback_speed_get(vg_anim), -2.0));

   // playing backwards
   ck_assert_int_eq(efl_ui_vg_animation_state_get(vg_anim), EFL_UI_VG_ANIMATION_STATE_PLAYING_BACKWARDS);

   // Play stop
   efl_player_playing_set(vg_anim, EINA_FALSE);
   ck_assert_int_eq(efl_ui_vg_animation_state_get(vg_anim), EFL_UI_VG_ANIMATION_STATE_STOPPED);

   // Auto repeat
   efl_player_playback_loop_set(vg_anim, EINA_TRUE);
   ck_assert_int_eq(efl_player_playback_loop_get(vg_anim), EINA_TRUE);

   // Auto play
   efl_player_autoplay_set(vg_anim, EINA_TRUE);
   ck_assert_int_eq(efl_player_autoplay_get(vg_anim), EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST(vg_anim_frame_control)
{
   Evas_Object *win, *vg_anim;
   int frame_count = 0;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_text_set(efl_added, "Efl_Ui_Vg_Animation demo"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   vg_anim = efl_add(EFL_UI_VG_ANIMATION_CLASS, win,
                     efl_content_set(win, efl_added));

   ck_assert_int_eq(efl_ui_vg_animation_state_get(vg_anim), EFL_UI_VG_ANIMATION_STATE_NOT_READY);

   // File load
   // emoji_wink.json is 60 frames sample.
   efl_file_simple_load(vg_anim, TESTS_SRC_DIR"/emoji_wink.json", NULL);
   ck_assert(efl_file_loaded_get(vg_anim));
   ck_assert_int_eq(efl_ui_vg_animation_state_get(vg_anim), EFL_UI_VG_ANIMATION_STATE_STOPPED);

   // Total frame
   frame_count = efl_ui_vg_animation_frame_count_get(vg_anim);
   ck_assert_int_eq(frame_count, 60);

   // Frame set, get
   efl_ui_vg_animation_frame_set(vg_anim, 30);
   ck_assert_int_eq(efl_ui_vg_animation_frame_get(vg_anim), 30);

   // Frame set, get
   efl_player_playback_progress_set(vg_anim, 0.3);
   ck_assert(EINA_DBL_EQ(efl_player_playback_progress_get(vg_anim), 0.3));

   // Min/Max frame set,get
   efl_ui_vg_animation_min_frame_set(vg_anim, 5);
   ck_assert_int_eq(efl_ui_vg_animation_min_frame_get(vg_anim), 5);
   efl_ui_vg_animation_frame_set(vg_anim, 1);
   ck_assert_int_eq(efl_ui_vg_animation_frame_get(vg_anim), 5);

   efl_ui_vg_animation_max_frame_set(vg_anim, 55);
   ck_assert_int_eq(efl_ui_vg_animation_max_frame_get(vg_anim), 55);
   efl_ui_vg_animation_frame_set(vg_anim, 60);
   ck_assert_int_eq(efl_ui_vg_animation_frame_get(vg_anim), 55);

   // Min/Max progress set,get
   efl_ui_vg_animation_min_progress_set(vg_anim, 0.2);
   ck_assert(EINA_DBL_EQ(efl_ui_vg_animation_min_progress_get(vg_anim), 0.2));

   efl_ui_vg_animation_max_progress_set(vg_anim, 0.8);
   ck_assert(EINA_DBL_EQ(efl_ui_vg_animation_max_progress_get(vg_anim), 0.8));
}
EFL_END_TEST

#endif

void efl_ui_test_vg_animation(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
#ifdef BUILD_VG_LOADER_JSON
   tcase_add_test(tc, vg_anim_playing_control);
   tcase_add_test(tc, vg_anim_frame_control);
#endif
}
