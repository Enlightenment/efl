#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <Emotion.h>
#include <emotion_modules.h>

#include <check.h>
#include "../efl_check.h"

void emotion_test_init(TCase *tc);

static const Efl_Test_Case etc[] = {
  { "Emotion", emotion_test_init },
  { NULL, NULL }
};

SUITE_INIT(emotion)
{
   ck_assert_int_eq(emotion_init(), 1);
}

SUITE_SHUTDOWN(emotion)
{
   ck_assert_int_eq(emotion_shutdown(), 1);
}

EFL_START_TEST(emotion_object_basic)
{
   Ecore_Evas  *ecore_evas = NULL;
   Evas        *evas       = NULL;
   Evas_Object *obj;
   const char *file;

   ecore_evas_init();
   ecore_evas = ecore_evas_new(NULL, 0, 0, 320, 240, NULL);
   evas = ecore_evas_get(ecore_evas);

   obj = emotion_object_add(evas);
   emotion_object_init(obj, "gstreamer1");

   /* We test an audio file here, no need to try decoding the video part */
   emotion_object_video_mute_set(obj, EINA_TRUE);

   emotion_object_file_set(obj, TESTS_SRC_DIR"/sample.ogg");
   file = emotion_object_file_get(obj);


   emotion_object_play_set(obj, 1);
   emotion_object_play_get(obj);
   emotion_object_play_set(obj, 0);
   emotion_object_play_get(obj);

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();
}
EFL_END_TEST

EFL_START_TEST(emotion_object_basic2)
{
   Ecore_Evas  *ecore_evas = NULL;
   Evas        *evas       = NULL;
   Evas_Object *obj;
   double pos, len;
   double ratio;
   int iw, ih;
   int l, r, t, b;
   const char *file;
   Emotion_Webcam *webcam;

   ecore_evas_init();
   ecore_evas = ecore_evas_new(NULL, 0, 0, 800, 600, NULL);
   evas = ecore_evas_get(ecore_evas);

   obj = emotion_object_add(evas);
   emotion_object_init(obj, "gstreamer1");


   emotion_object_module_option_set(obj, "foo", "bar");
   emotion_object_vis_set(obj, EMOTION_VIS_NONE);
   emotion_object_last_position_load(obj);
   emotion_object_play_set(obj, 0);
   emotion_object_play_set(obj, 1);
   emotion_object_audio_volume_set(obj, 0.5);
   pos = emotion_object_position_get(obj);
   len = emotion_object_play_length_get(obj);
   emotion_object_size_get(obj, &iw, &ih);
   ratio = emotion_object_ratio_get(obj);
   emotion_object_position_set(obj, 0.0);
   emotion_object_audio_channel_count(obj);
   emotion_object_video_channel_count(obj);
   emotion_object_spu_channel_count(obj);
   emotion_object_title_get(obj);
   emotion_object_progress_info_get(obj);
   emotion_object_progress_status_get(obj);
   emotion_object_ref_file_get(obj);
   emotion_object_ref_num_get(obj);
   emotion_object_spu_button_count_get(obj);
   emotion_object_spu_button_get(obj);
   emotion_object_event_simple_send(obj, EMOTION_EVENT_UP);
   emotion_object_event_simple_send(obj, EMOTION_EVENT_10);
   emotion_object_video_mute_get(obj);
   emotion_object_video_mute_set(obj, 0);
   emotion_object_video_mute_set(obj, 1);
   emotion_object_audio_mute_get(obj);
   emotion_object_audio_mute_set(obj, 0);
   emotion_object_audio_mute_set(obj, 1);
   emotion_object_seekable_get(obj);
   emotion_object_vis_supported(obj, EMOTION_VIS_NONE);
   file = emotion_object_file_get(obj);
   emotion_object_file_set(obj, file);
   emotion_object_last_position_save(obj);

   emotion_webcams_get();
   emotion_webcam_name_get(webcam);
   emotion_webcam_device_get(webcam);
   emotion_webcam_custom_get(NULL);

   emotion_object_extension_may_play_fast_get("file");
   emotion_object_extension_may_play_get("file");

   _emotion_pending_ecore_begin();
   _emotion_pending_ecore_end();

   _emotion_pending_object_ref();
   _emotion_pending_object_unref();

   emotion_object_image_get(obj);

   emotion_object_border_set(obj, 1, 1, 1, 1);
   emotion_object_border_get(obj, &l, &r, &t, &b);

   emotion_object_bg_color_set(obj, 1, 1, 1, 1);
   emotion_object_bg_color_get(obj, &l, &r, &t, &b);

   //emotion_object_keep_aspect_set(obj, aspect);
   emotion_object_keep_aspect_get(obj);

   emotion_object_play_get(obj);
   emotion_object_buffer_size_get(obj);
   emotion_object_video_handled_get(obj);
   emotion_object_audio_handled_get(obj);

   emotion_object_smooth_scale_set(obj, EINA_TRUE);
   emotion_object_smooth_scale_get(obj);

   emotion_object_audio_volume_get(obj);
   emotion_object_audio_channel_name_get(obj, 1);
   emotion_object_audio_channel_set(obj, 1);
   emotion_object_audio_channel_get(obj);

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();
}
EFL_END_TEST

void emotion_test_init(TCase *tc)
{
   tcase_add_test(tc, emotion_object_basic);
   //tcase_add_test(tc, emotion_object_basic);
   //tcase_add_test(tc, emotion_object_video);
   //tcase_add_test(tc, emotion_object_audio);
   //tcase_add_test(tc, emotion_object_misc);
   //tcase_add_test(tc, emotion_object_webcam);
}

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Emotion", etc, SUITE_INIT_FN(emotion), SUITE_SHUTDOWN_FN(emotion));

   return (failed_count == 0) ? 0 : 255;
}
