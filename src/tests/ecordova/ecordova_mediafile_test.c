#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_file_tests.h"
#include "ecordova_suite.h"

#include <Eio.h>

#include <stdbool.h>
#include <stdlib.h>

static void
_setup(void)
{
   int ret = ecordova_init();
   ck_assert_int_eq(ret, 1);
}

static void
_teardown(void)
{
   int ret = ecordova_shutdown();
   ck_assert_int_eq(ret, 0);
}

static Ecordova_MediaFile *
_mediafile_new(const char *name,
               const char *url,
               const char *type,
               time_t last_modified_date,
               long size)
{
   return eo_add(ECORDOVA_MEDIAFILE_CLASS,
                 NULL,
                 ecordova_entry_name_set(name),
                 ecordova_entry_url_set(url),
                 ecordova_mediafile_type_set(type));
}

START_TEST(smoke)
{
   Ecordova_Media *mediafile = _mediafile_new("test.txt", "/", "text/plain", 0, 0);
   eo_unref(mediafile);
}
END_TEST

static Eina_Bool
_main_loop_quit(void *data EINA_UNUSED,
                Eo *obj EINA_UNUSED,
                const Eo_Event_Description *desc EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   fail_if(NULL == data);
   Ecordova_MediaFileData *mediafile_data = data;
   fail_if(NULL == event_info);
   *mediafile_data = *(Ecordova_MediaFileData*)event_info;

   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_bool_set(void *data,
          Eo *obj EINA_UNUSED,
          const Eo_Event_Description *desc EINA_UNUSED,
          void *event_info)
{
   Ecordova_ProgressEvent *progress = event_info;
   fail_if(NULL == progress);

   bool *value = data;
   *value = true;
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static bool
_format_data_get(Ecordova_MediaFile *file, Ecordova_MediaFileData *data)
{
   *data = (Ecordova_MediaFileData){0};
   bool error = false;
   bool timeout = false;

   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));

   eo_do(file, eo_event_callback_add(ECORDOVA_MEDIAFILE_EVENT_SUCCESS, _main_loop_quit, data),
               eo_event_callback_add(ECORDOVA_MEDIAFILE_EVENT_ERROR, _bool_set, &error),
               ecordova_mediafile_format_data_get());

   ecore_main_loop_begin();

   eo_do(file, eo_event_callback_del(ECORDOVA_MEDIAFILE_EVENT_SUCCESS, _main_loop_quit, data),
               eo_event_callback_del(ECORDOVA_MEDIAFILE_EVENT_ERROR, _bool_set, &error));

   eo_unref(timeout_timer);
   fail_if(timeout);

   return error;
}


START_TEST(formatdata_get)
{
   const char *media_sample_filename = "44khz32kbps.mp3";
   size_t len = strlen(TESTS_SRC_DIR) + 1 + strlen(media_sample_filename) + 1;
   char media_sample_url[len];
   snprintf(media_sample_url, len, "%s/%s", TESTS_SRC_DIR, media_sample_filename);

   Eina_File *media_sample_file = eina_file_open(media_sample_url, EINA_FALSE);
   size_t media_sample_file_size = eina_file_size_get(media_sample_file);
   time_t media_sample_modified_date = eina_file_mtime_get(media_sample_file);
   eina_file_close(media_sample_file);

   Ecordova_Media *mediafile = _mediafile_new(media_sample_filename,
                                              media_sample_url,
                                              "audio/mpeg3",
                                              media_sample_modified_date,
                                              media_sample_file_size);

   bool error = false;
   Ecordova_MediaFileData mediafile_data;
   fail_if(error = _format_data_get(mediafile, &mediafile_data));
   fail_unless(NULL == mediafile_data.codecs);
   ck_assert_int_eq(32000, mediafile_data.bitrate);
   ck_assert_int_eq(0, mediafile_data.height);
   ck_assert_int_eq(0, mediafile_data.width);
   ck_assert_int_eq(55, mediafile_data.duration);
   eo_unref(mediafile);
}
END_TEST

void
ecordova_mediafile_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, formatdata_get);
}
