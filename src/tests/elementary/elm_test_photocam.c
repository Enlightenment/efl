#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_photocam_legacy_type_check)
{
   Evas_Object *win, *photocam;
   const char *type;

   win = win_add(NULL, "photocam", ELM_WIN_BASIC);

   photocam = elm_photocam_add(win);

   type = elm_object_widget_type_get(photocam);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Photocam"));

   type = evas_object_type_get(photocam);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_photocam"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *photocam;
   Efl_Access_Role role;

   win = win_add(NULL, "photocam", ELM_WIN_BASIC);

   photocam = elm_photocam_add(win);
   role = efl_access_object_role_get(photocam);

   ck_assert(role == EFL_ACCESS_ROLE_IMAGE);

}
EFL_END_TEST

EFL_START_TEST(elm_photocam_file)
{
   Evas_Object *win, *photocam;
   const char *buf = ELM_IMAGE_DATA_DIR "/images/logo_small.png";

   win = win_add(NULL, "photocam", ELM_WIN_BASIC);
   photocam = elm_photocam_add(win);

   ck_assert_int_eq(elm_photocam_file_set(photocam, buf), EVAS_LOAD_ERROR_NONE);
   ck_assert_str_eq(elm_photocam_file_get(photocam), buf);

   ck_assert_int_eq(elm_photocam_file_set(photocam, "non_existing.png"), EVAS_LOAD_ERROR_DOES_NOT_EXIST);
   ck_assert_str_eq(elm_photocam_file_get(photocam), "non_existing.png");
}
EFL_END_TEST

void elm_test_photocam(TCase *tc)
{
   tcase_add_test(tc, elm_photocam_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_photocam_file);
}
