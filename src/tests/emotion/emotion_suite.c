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

EFL_START_TEST(emotion_simple)
{
   Ecore_Evas  *ecore_evas = NULL;
   Evas        *evas       = NULL;
   Evas_Object *obj;
   double pos, len;
   double ratio;
   int iw, ih;
   const char *file;
   const Eina_List *wl, *l;
   Emotion_Webcam *webcam;

   ecore_evas_init();
   ecore_evas = ecore_evas_new(NULL, 0, 0, 800, 600, NULL);
   evas = ecore_evas_get(ecore_evas);

   obj = emotion_object_add(evas);
   emotion_object_init(obj, "gst1");
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

   wl = emotion_webcams_get();
   EINA_LIST_FOREACH(wl, l, webcam)
     {
        emotion_webcam_name_get(webcam);
        emotion_webcam_device_get(webcam);
     }

   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();
}
EFL_END_TEST

void emotion_test_init(TCase *tc)
{
   tcase_add_test(tc, emotion_simple);
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
