#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_segment_control_legacy_type_check)
{
   Evas_Object *win, *segment_control;
   const char *type;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "segment_control", ELM_WIN_BASIC);

   segment_control = elm_segment_control_add(win);

   type = elm_object_widget_type_get(segment_control);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Segment_Control"));

   type = evas_object_type_get(segment_control);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_segment_control"));

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *segmentcontrol;
   Efl_Access_Role role;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "segmentcontrol", ELM_WIN_BASIC);

   segmentcontrol = elm_segment_control_add(win);
   role = efl_access_role_get(segmentcontrol);

   ck_assert(role == EFL_ACCESS_ROLE_LIST);

   elm_shutdown();
}
END_TEST

void elm_test_segmentcontrol(TCase *tc)
{
   tcase_add_test(tc, elm_segment_control_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
