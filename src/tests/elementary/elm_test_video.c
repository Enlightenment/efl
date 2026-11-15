#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include <Emotion.h>
#include "elm_suite.h"

EFL_START_TEST(elm_video_legacy_type_check)
{
   Evas_Object *win, *video;
   const char *type;

   win = win_add(NULL, "video", ELM_WIN_BASIC);

   video = elm_video_add(win);

   type = elm_object_widget_type_get(video);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Video"));

   type = evas_object_type_get(video);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_video"));

}
EFL_END_TEST

/*
EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *video;
   Efl_Access_Role role;

   win = win_add(NULL, "video", ELM_WIN_BASIC);

   video = elm_video_add(win);
   role = efl_access_object_role_get(video);

   ck_assert(role == EFL_ACCESS_ROLE_ANIMATION);

}
EFL_END_TEST
*/

/* Efl.Canvas.Video is the decoder: an Efl.Canvas.Group that draws frames and
 * answers for playback, audio and the image it is showing.
 *
 * None of this needs a file to be decoded, which is deliberate: a test that
 * needs a working gstreamer pipeline is a test that does not run everywhere.
 */

EFL_START_TEST(efl_canvas_video_test_playback_properties)
{
   Eo *win, *video;

   win = win_add(NULL, "video", ELM_WIN_BASIC);
   video = efl_add(EFL_CANVAS_VIDEO_CLASS, win);
   ck_assert(video != NULL);

   /* The decoder neither starts nor restarts by itself: both are refused and
      read back as off. */
   EXPECT_ERROR_START;
   efl_player_autoplay_set(video, EINA_TRUE);
   EXPECT_ERROR_END;
   ck_assert(!efl_player_autoplay_get(video));

   EXPECT_ERROR_START;
   efl_player_playback_loop_set(video, EINA_TRUE);
   EXPECT_ERROR_END;
   ck_assert(!efl_player_playback_loop_get(video));

   /* Nothing has been loaded, so there is nothing to play and nowhere to seek
      to. */
   ck_assert(!efl_playable_get(video));
   ck_assert(!efl_playable_seekable_get(video));
   ck_assert(EINA_DBL_EQ(efl_playable_length_get(video), 0.0));
}
EFL_END_TEST

EFL_START_TEST(efl_canvas_video_test_audio_control)
{
   Eo *win, *video;

   win = win_add(NULL, "video", ELM_WIN_BASIC);
   video = efl_add(EFL_CANVAS_VIDEO_CLASS, win);

   efl_audio_control_mute_set(video, EINA_TRUE);
   ck_assert(efl_audio_control_mute_get(video));
   efl_audio_control_mute_set(video, EINA_FALSE);
   ck_assert(!efl_audio_control_mute_get(video));
}
EFL_END_TEST

/* Efl.Gfx.Image and Efl.Gfx.Image_Load_Controller are answered by handing the
 * question to the canvas image the object draws into, so what is set comes
 * back.
 */
EFL_START_TEST(efl_canvas_video_test_image_properties)
{
   Eo *win, *video;
   int l, r, t, b;

   win = win_add(NULL, "video", ELM_WIN_BASIC);
   video = efl_add(EFL_CANVAS_VIDEO_CLASS, win);

   efl_gfx_image_can_upscale_set(video, EINA_FALSE);
   ck_assert(!efl_gfx_image_can_upscale_get(video));
   efl_gfx_image_can_downscale_set(video, EINA_FALSE);
   ck_assert(!efl_gfx_image_can_downscale_get(video));

   efl_gfx_image_scale_method_set(video, EFL_GFX_IMAGE_SCALE_METHOD_FIT);
   ck_assert_int_eq(efl_gfx_image_scale_method_get(video),
                    EFL_GFX_IMAGE_SCALE_METHOD_FIT);

   efl_gfx_image_border_insets_set(video, 1, 2, 3, 4);
   efl_gfx_image_border_insets_get(video, &l, &r, &t, &b);
   ck_assert_int_eq(l, 1);
   ck_assert_int_eq(r, 2);
   ck_assert_int_eq(t, 3);
   ck_assert_int_eq(b, 4);

   efl_gfx_image_border_insets_scale_set(video, 2.0);
   ck_assert(EINA_DBL_EQ(efl_gfx_image_border_insets_scale_get(video), 2.0));

   /* Efl.Gfx.Image_Load_Controller steers a raster loader, and a video is
      decoded by a media engine that takes none of it. Every one of these says
      so and answers neutrally rather than accepting a value and dropping it. */
   EXPECT_ERROR_START;
   efl_gfx_image_load_controller_load_scale_down_set(video, 2);
   ck_assert_int_eq(efl_gfx_image_load_controller_load_scale_down_get(video), 0);
   efl_gfx_image_load_controller_load_dpi_set(video, 300.0);
   ck_assert(EINA_DBL_EQ(efl_gfx_image_load_controller_load_dpi_get(video), 0.0));
   ck_assert(!efl_gfx_image_load_controller_load_region_support_get(video));
   efl_gfx_image_load_controller_load_size_set(video, EINA_SIZE2D(64, 64));
   EXPECT_ERROR_END;
}
EFL_END_TEST

/* Emotion goes down with Elementary and comes back with it: the engine has to
   be there the second time, after two videos in the first life. */
EFL_START_TEST(elm_video_engine_after_shutdown)
{
   Evas_Object *win;

   win = win_add(NULL, "video", ELM_WIN_BASIC);
   ck_assert(elm_video_add(win) != NULL);
   ck_assert(elm_video_add(win) != NULL);

   elm_shutdown();
   elm_init(0, NULL);

   // win_add() would hand back the window of the first life
   fail_on_errors_setup();
   win = elm_win_add(NULL, "video", ELM_WIN_BASIC);
   ck_assert(elm_video_add(win) != NULL);
   fail_on_errors_teardown();
}
EFL_END_TEST

void elm_test_video(TCase *tc)
{
   tcase_add_test(tc, elm_video_legacy_type_check);
   tcase_add_test(tc, efl_canvas_video_test_playback_properties);
   tcase_add_test(tc, efl_canvas_video_test_audio_control);
   tcase_add_test(tc, efl_canvas_video_test_image_properties);
   tcase_add_test(tc, elm_video_engine_after_shutdown);
//   tcase_add_test(tc, elm_atspi_role_get);
}
