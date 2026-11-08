#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
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
   tcase_add_test(tc, elm_video_engine_after_shutdown);
//   tcase_add_test(tc, elm_atspi_role_get);
}
