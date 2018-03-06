#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_video_legacy_type_check)
{
   Evas_Object *win, *video;
   const char *type;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "video", ELM_WIN_BASIC);

   video = elm_video_add(win);

   type = elm_object_widget_type_get(video);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Video"));

   type = evas_object_type_get(video);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_video"));

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *video;
   Efl_Access_Role role;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "video", ELM_WIN_BASIC);

   video = elm_video_add(win);
   role = efl_access_role_get(video);

   ck_assert(role == EFL_ACCESS_ROLE_ANIMATION);

   elm_shutdown();
}
END_TEST

void elm_test_video(TCase *tc)
{
   tcase_add_test(tc, elm_video_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
